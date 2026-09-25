//! End-to-end tests: a real gateway on a loopback port in front of an echo
//! backend that records the PROXY line, driven by a tungstenite client.

use std::io::{BufRead, BufReader};
use std::net::SocketAddr;
use std::process::{Child, Command, Stdio};
use std::sync::{mpsc, Arc, Mutex};
use std::time::{Duration, Instant};

use darkeden_gateway::{Config, Gateway, Timeouts};
use futures_util::{SinkExt, StreamExt};
use serde_json::{json, Value};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::{TcpListener, TcpStream};
use tokio::sync::oneshot;
use tokio::task::JoinHandle;
use tokio::time::timeout;
use tokio_tungstenite::tungstenite::client::IntoClientRequest;
use tokio_tungstenite::tungstenite::handshake::client::Response;
use tokio_tungstenite::tungstenite::http::{HeaderName, HeaderValue};
use tokio_tungstenite::tungstenite::protocol::frame::coding::{Data, OpCode};
use tokio_tungstenite::tungstenite::protocol::frame::Frame;
use tokio_tungstenite::tungstenite::{Bytes, Error as WsError, Message};
use tokio_tungstenite::{client_async, WebSocketStream};

type Client = WebSocketStream<TcpStream>;

const ORIGIN: &str = "https://game.example";
const BINARY: (&str, &str) = ("Sec-WebSocket-Protocol", "binary");
const BROWSER: (&str, &str) = ("Origin", ORIGIN);
const LIMIT: Duration = Duration::from_secs(10);
const MIB: usize = 1024 * 1024;

/// Production limits, except a short close grace: tests leave clients that
/// never answer the gateway's close frame.
fn quick() -> Timeouts {
    Timeouts {
        close_grace: Duration::from_millis(500),
        ..Timeouts::default()
    }
}

/// A gateway on 127.0.0.1:0 routing `192.0.2.2:9999` and `192.0.2.3:9998`
/// to an echo backend.
struct Fixture {
    gateway: SocketAddr,
    backend_port: u16,
    proxy_lines: Arc<Mutex<Vec<String>>>,
    stop: Option<oneshot::Sender<()>>,
    server: Option<JoinHandle<()>>,
}

impl Fixture {
    async fn start(extra: Value) -> Self {
        Self::start_with(extra, quick()).await
    }

    /// `extra` keys replace the defaults; a `null` restores a key's default.
    async fn start_with(extra: Value, timeouts: Timeouts) -> Self {
        let backend = TcpListener::bind("127.0.0.1:0").await.unwrap();
        let backend_port = backend.local_addr().unwrap().port();
        let proxy_lines = Arc::new(Mutex::new(Vec::new()));
        tokio::spawn(echo_backend(backend, Arc::clone(&proxy_lines)));
        let mut config = json!({
            "port": 0,
            "origins": [ORIGIN],
            "allowNative": true,
            "routes": { "192.0.2.2:9999": backend_port, "192.0.2.3:9998": backend_port },
        });
        if let (Some(config), Value::Object(extra)) = (config.as_object_mut(), extra) {
            config.extend(extra);
        }
        let gateway = Gateway::bind(Config::from_value(&config).unwrap())
            .await
            .unwrap()
            .with_timeouts(timeouts);
        let address = gateway.local_addr().unwrap();
        let (stop, stopped) = oneshot::channel::<()>();
        let server = tokio::spawn(gateway.serve(async {
            let _ = stopped.await;
        }));
        Self {
            gateway: address,
            backend_port,
            proxy_lines,
            stop: Some(stop),
            server: Some(server),
        }
    }

    fn url(&self, host: &str, port: u16) -> String {
        format!("ws://{}/game?host={host}&port={port}", self.gateway)
    }

    fn login(&self) -> String {
        self.url("192.0.2.2", 9999)
    }

    fn proxy_lines(&self) -> Vec<String> {
        self.proxy_lines.lock().unwrap().clone()
    }

    /// Requests shutdown and waits for `serve` to return.
    async fn shutdown(&mut self) {
        if let Some(stop) = self.stop.take() {
            let _ = stop.send(());
        }
        if let Some(server) = self.server.take() {
            timeout(LIMIT, server)
                .await
                .expect("serve returns after shutdown")
                .unwrap();
        }
    }
}

/// Records each connection's first line, then echoes everything after it.
async fn echo_backend(listener: TcpListener, proxy_lines: Arc<Mutex<Vec<String>>>) {
    while let Ok((mut socket, _)) = listener.accept().await {
        let proxy_lines = Arc::clone(&proxy_lines);
        tokio::spawn(async move {
            let Some((line, rest)) = read_line(&mut socket).await else {
                return;
            };
            proxy_lines.lock().unwrap().push(line);
            if socket.write_all(&rest).await.is_err() {
                return;
            }
            let (mut reader, mut writer) = socket.split();
            let _ = tokio::io::copy(&mut reader, &mut writer).await;
        });
    }
}

/// Reads through the first CRLF; returns the line and the bytes after it.
async fn read_line(socket: &mut TcpStream) -> Option<(String, Vec<u8>)> {
    let mut received = Vec::new();
    let mut chunk = [0_u8; 4096];
    loop {
        let count = socket.read(&mut chunk).await.ok()?;
        if count == 0 {
            return None;
        }
        received.extend_from_slice(&chunk[..count]);
        if let Some(end) = received.windows(2).position(|pair| pair == b"\r\n") {
            let rest = received.split_off(end + 2);
            received.truncate(end);
            return Some((String::from_utf8(received).ok()?, rest));
        }
    }
}

/// Opens a WebSocket with the given extra request headers.
async fn connect(url: &str, headers: &[(&str, &str)]) -> Result<(Client, Response), WsError> {
    let mut request = url.into_client_request()?;
    for (name, value) in headers {
        request.headers_mut().append(
            HeaderName::from_bytes(name.as_bytes()).unwrap(),
            HeaderValue::from_str(value).unwrap(),
        );
    }
    let address = request.uri().authority().unwrap().as_str().to_owned();
    let stream = TcpStream::connect(address).await?;
    timeout(LIMIT, client_async(request, stream))
        .await
        .expect("the handshake finishes")
}

/// The handshake's HTTP status: 101 on success.
fn status(result: Result<(Client, Response), WsError>) -> u16 {
    match result {
        Ok(_) => 101,
        Err(WsError::Http(response)) => response.status().as_u16(),
        Err(error) => panic!("handshake failed without a response: {error}"),
    }
}

async fn receive_exactly(client: &mut Client, length: usize) -> Vec<u8> {
    let mut received = Vec::with_capacity(length);
    while received.len() < length {
        match timeout(LIMIT, client.next())
            .await
            .expect("the echo arrives")
        {
            Some(Ok(Message::Binary(bytes))) => received.extend_from_slice(&bytes),
            Some(Ok(Message::Ping(_) | Message::Pong(_))) => {}
            other => panic!("expected echoed bytes, got {other:?}"),
        }
    }
    received
}

/// Waits for the connection to end; returns the close code, if one was sent.
async fn closed(client: &mut Client) -> Option<u16> {
    loop {
        match timeout(LIMIT, client.next())
            .await
            .expect("the connection closes")
        {
            Some(Ok(Message::Close(frame))) => return frame.map(|frame| u16::from(frame.code)),
            Some(Ok(Message::Ping(_) | Message::Pong(_))) => {}
            Some(Ok(other)) => panic!("expected a close, got {other:?}"),
            Some(Err(_)) | None => return None,
        }
    }
}

async fn echo_once(client: &mut Client, payload: &[u8]) {
    client
        .send(Message::binary(payload.to_vec()))
        .await
        .unwrap();
    assert_eq!(receive_exactly(client, payload.len()).await, payload);
}

#[tokio::test(flavor = "multi_thread", worker_threads = 2)]
async fn binary_stream_survives_fragmentation_chunking_and_reconnects() {
    let mut fixture = Fixture::start(json!({})).await;
    let bytes: Vec<u8> = (0..200_000_u32).map(|index| (index % 256) as u8).collect();
    let mut expected = Vec::new();
    for (host, port) in [
        ("192.0.2.2", 9999),
        ("192.0.2.3", 9998),
        ("192.0.2.2", 9999),
    ] {
        let (mut client, response) = connect(&fixture.url(host, port), &[BINARY, BROWSER])
            .await
            .unwrap();
        assert_eq!(response.headers()["sec-websocket-protocol"], "binary");
        let source_port = client.get_ref().local_addr().unwrap().port();
        expected.push(format!(
            "PROXY TCP4 127.0.0.1 127.0.0.1 {source_port} {}",
            fixture.backend_port
        ));

        // One message in two fragments, then a second message.
        let first = Frame::message(
            Bytes::copy_from_slice(&bytes[..7]),
            OpCode::Data(Data::Binary),
            false,
        );
        let last = Frame::message(
            Bytes::copy_from_slice(&bytes[7..70_000]),
            OpCode::Data(Data::Continue),
            true,
        );
        client.send(Message::Frame(first)).await.unwrap();
        client.send(Message::Frame(last)).await.unwrap();
        client
            .send(Message::binary(bytes[70_000..].to_vec()))
            .await
            .unwrap();
        let echoed = receive_exactly(&mut client, bytes.len()).await;
        assert!(
            echoed == bytes,
            "the echoed stream differs from the sent one"
        );

        client.close(None).await.unwrap();
        closed(&mut client).await;
    }
    assert_eq!(fixture.proxy_lines(), expected);
    fixture.shutdown().await;
}

#[tokio::test]
async fn forwarded_address_is_honoured_only_from_a_trusted_proxy() {
    for (trusted, client_ip) in [(false, "127.0.0.1"), (true, "203.0.113.44")] {
        let proxies = if trusted {
            json!(["127.0.0.1"])
        } else {
            json!([])
        };
        let mut fixture = Fixture::start(json!({ "trustedProxies": proxies })).await;
        let (mut client, _) = connect(
            &fixture.login(),
            &[BINARY, ("X-Forwarded-For", "203.0.113.44")],
        )
        .await
        .unwrap();
        echo_once(&mut client, &[1]).await;
        let lines = fixture.proxy_lines();
        assert!(
            lines[0].starts_with(&format!("PROXY TCP4 {client_ip} 127.0.0.1 ")),
            "{lines:?}"
        );
        fixture.shutdown().await;
    }

    let mut fixture = Fixture::start(json!({ "trustedProxies": ["127.0.0.1"] })).await;
    for forwarded in [
        &[][..],
        &[("X-Forwarded-For", "203.0.113.44, 198.51.100.1")][..],
        &[
            ("X-Forwarded-For", "203.0.113.44"),
            ("X-Forwarded-For", "203.0.113.45"),
        ][..],
        &[("X-Forwarded-For", "2001:db8::1")][..],
    ] {
        let headers: Vec<_> = [BINARY].iter().chain(forwarded).copied().collect();
        assert_eq!(
            status(connect(&fixture.login(), &headers).await),
            400,
            "{forwarded:?}"
        );
    }
    assert!(fixture.proxy_lines().is_empty());
    fixture.shutdown().await;
}

#[tokio::test]
async fn refuses_unknown_routes_origins_ambiguous_queries_and_other_paths() {
    let mut fixture = Fixture::start(json!({})).await;
    let login = fixture.login();
    let cases = [
        (fixture.url("192.0.2.2", 22), vec![BINARY], 403),
        (
            login.clone(),
            vec![BINARY, ("Origin", "https://attacker.example")],
            403,
        ),
        (format!("{login}&port=9999"), vec![BINARY], 404),
        (login.replace("/game?", "/other?"), vec![BINARY], 404),
    ];
    for (url, headers, expected) in cases {
        assert_eq!(
            status(connect(&url, &headers).await),
            expected,
            "{url} {headers:?}"
        );
    }
    assert!(
        fixture.proxy_lines().is_empty(),
        "refused requests reach no backend"
    );
    fixture.shutdown().await;
}

#[tokio::test]
async fn text_messages_close_the_connection() {
    let mut fixture = Fixture::start(json!({})).await;
    let (mut client, _) = connect(&fixture.login(), &[BINARY]).await.unwrap();
    client
        .send(Message::text("text is not the game protocol"))
        .await
        .unwrap();
    assert_eq!(closed(&mut client).await, Some(1003));
    fixture.shutdown().await;
}

#[tokio::test]
async fn oversized_messages_close_the_connection() {
    let mut fixture = Fixture::start(json!({})).await;
    let (mut client, _) = connect(&fixture.login(), &[BINARY]).await.unwrap();
    echo_once(&mut client, &vec![7; MIB]).await;

    // The gateway may close before the whole frame is written.
    let _ = client.send(Message::binary(vec![0; MIB + 1])).await;
    assert!(matches!(closed(&mut client).await, Some(1009) | None));

    let (mut client, _) = connect(&fixture.login(), &[BINARY]).await.unwrap();
    let first = Frame::message(vec![1; MIB / 2], OpCode::Data(Data::Binary), false);
    let last = Frame::message(vec![2; MIB / 2 + 1], OpCode::Data(Data::Continue), true);
    let _ = client.send(Message::Frame(first)).await;
    let _ = client.send(Message::Frame(last)).await;
    assert!(matches!(closed(&mut client).await, Some(1009) | None));
    fixture.shutdown().await;
}

#[tokio::test]
async fn binary_subprotocol_is_required() {
    let mut fixture = Fixture::start(json!({})).await;
    let login = fixture.login();
    assert_eq!(status(connect(&login, &[]).await), 400);
    assert_eq!(
        status(connect(&login, &[("Sec-WebSocket-Protocol", "chat")]).await),
        400
    );
    let (mut client, response) = connect(&login, &[("Sec-WebSocket-Protocol", "chat, binary")])
        .await
        .unwrap();
    assert_eq!(response.headers()["sec-websocket-protocol"], "binary");
    echo_once(&mut client, b"offered among others").await;
    fixture.shutdown().await;
}

#[tokio::test]
async fn an_empty_origin_is_a_native_client() {
    let mut open = Fixture::start(json!({})).await;
    let (mut client, _) = connect(&open.login(), &[BINARY, ("Origin", "")])
        .await
        .unwrap();
    echo_once(&mut client, b"native").await;
    open.shutdown().await;

    let mut strict = Fixture::start(json!({ "allowNative": false })).await;
    let login = strict.login();
    assert_eq!(
        status(connect(&login, &[BINARY, ("Origin", "")]).await),
        403
    );
    assert_eq!(status(connect(&login, &[BINARY]).await), 403);
    let (mut client, _) = connect(&login, &[BINARY, BROWSER]).await.unwrap();
    echo_once(&mut client, b"browser").await;
    strict.shutdown().await;
}

#[tokio::test]
async fn plain_http_requests_get_the_banner() {
    let mut fixture = Fixture::start(json!({})).await;
    let mut stream = TcpStream::connect(fixture.gateway).await.unwrap();
    stream
        .write_all(b"GET / HTTP/1.1\r\nHost: gateway\r\n\r\n")
        .await
        .unwrap();
    let mut response = String::new();
    timeout(LIMIT, stream.read_to_string(&mut response))
        .await
        .unwrap()
        .unwrap();
    assert!(response.starts_with("HTTP/1.1 200 OK\r\n"), "{response}");
    assert!(
        response.contains("Content-Type: text/plain\r\n"),
        "{response}"
    );
    assert!(
        response.ends_with("\r\n\r\nDarkEden WebSocket gateway\n"),
        "{response}"
    );
    fixture.shutdown().await;
}

#[tokio::test]
async fn a_dead_backend_fails_the_handshake() {
    let unused = TcpListener::bind("127.0.0.1:0").await.unwrap();
    let dead_port = unused.local_addr().unwrap().port();
    drop(unused);
    let mut fixture = Fixture::start(json!({ "routes": { "192.0.2.9:7777": dead_port } })).await;
    let url = fixture.url("192.0.2.9", 7777);
    assert_eq!(status(connect(&url, &[BINARY]).await), 502);
    fixture.shutdown().await;
}

#[tokio::test]
async fn backend_eof_closes_the_websocket_normally() {
    let backend = TcpListener::bind("127.0.0.1:0").await.unwrap();
    let port = backend.local_addr().unwrap().port();
    tokio::spawn(async move {
        let (mut socket, _) = backend.accept().await.unwrap();
        let (line, _) = read_line(&mut socket).await.unwrap();
        assert!(line.starts_with("PROXY TCP4 127.0.0.1 127.0.0.1 "));
        socket.write_all(b"bye").await.unwrap();
    });
    let mut fixture = Fixture::start(json!({ "routes": { "192.0.2.9:7777": port } })).await;
    let (mut client, _) = connect(&fixture.url("192.0.2.9", 7777), &[BINARY])
        .await
        .unwrap();
    assert_eq!(receive_exactly(&mut client, 3).await, b"bye");
    assert_eq!(closed(&mut client).await, Some(1000));
    fixture.shutdown().await;
}

#[tokio::test]
async fn the_connection_cap_counts_unfinished_handshakes() {
    let mut fixture = Fixture::start(json!({ "maxConnections": 1 })).await;
    let idle = TcpStream::connect(fixture.gateway).await.unwrap();
    tokio::time::sleep(Duration::from_millis(200)).await;

    let mut excess = TcpStream::connect(fixture.gateway).await.unwrap();
    let mut byte = [0_u8; 1];
    let read = timeout(LIMIT, excess.read(&mut byte))
        .await
        .expect("excess connection is dropped");
    assert!(matches!(read, Ok(0) | Err(_)), "{read:?}");

    drop(idle);
    let deadline = Instant::now() + LIMIT;
    loop {
        let mut stream = TcpStream::connect(fixture.gateway).await.unwrap();
        let _ = stream.write_all(b"GET / HTTP/1.1\r\n\r\n").await;
        let mut response = String::new();
        let _ = timeout(LIMIT, stream.read_to_string(&mut response)).await;
        if response.starts_with("HTTP/1.1 200") {
            break;
        }
        assert!(Instant::now() < deadline, "the slot was never released");
        tokio::time::sleep(Duration::from_millis(50)).await;
    }
    fixture.shutdown().await;
}

#[tokio::test]
async fn handshakes_must_finish_in_time() {
    let timeouts = Timeouts {
        handshake: Duration::from_millis(300),
        ..quick()
    };
    let mut fixture = Fixture::start_with(json!({}), timeouts).await;
    for partial in [
        &b""[..],
        &b"GET /game?host=192.0.2.2&port=9999 HTTP/1.1\r\n"[..],
    ] {
        let mut stream = TcpStream::connect(fixture.gateway).await.unwrap();
        stream.write_all(partial).await.unwrap();
        let mut rest = Vec::new();
        let read = timeout(LIMIT, stream.read_to_end(&mut rest))
            .await
            .expect("the gateway hangs up");
        assert!(matches!(read, Ok(0) | Err(_)), "{read:?}");
    }
    fixture.shutdown().await;
}

#[tokio::test]
async fn clients_that_miss_a_pong_are_disconnected() {
    let timeouts = Timeouts {
        ping_interval: Duration::from_millis(200),
        ..quick()
    };
    let mut fixture = Fixture::start_with(json!({}), timeouts).await;

    // Reading answers pings, so a client that keeps talking stays connected
    // across several ping intervals...
    let (mut chatty, _) = connect(&fixture.login(), &[BINARY]).await.unwrap();
    let chatter = tokio::spawn(async move {
        let until = Instant::now() + Duration::from_millis(1200);
        while Instant::now() < until {
            echo_once(&mut chatty, b"still here").await;
            tokio::time::sleep(Duration::from_millis(20)).await;
        }
    });

    // ...while a client that never reads never answers, and is dropped.
    let (mut silent, _) = connect(&fixture.login(), &[BINARY]).await.unwrap();
    tokio::time::sleep(Duration::from_millis(1000)).await;
    assert_eq!(closed(&mut silent).await, None);

    chatter.await.unwrap();
    fixture.shutdown().await;
}

#[tokio::test]
async fn shutdown_closes_connections_as_going_away() {
    let mut fixture = Fixture::start(json!({})).await;
    let (mut client, _) = connect(&fixture.login(), &[BINARY]).await.unwrap();
    echo_once(&mut client, b"before").await;
    let closing = tokio::spawn(async move { closed(&mut client).await });
    fixture.shutdown().await;
    assert_eq!(closing.await.unwrap(), Some(1001));
    assert!(TcpStream::connect(fixture.gateway).await.is_err());
}

#[test]
fn configuration_errors_name_the_key() {
    let base = json!({ "origins": [ORIGIN], "routes": { "login.example:9999": 19099 } });
    let with = |key: &str, value: Value| {
        let mut config = base.clone();
        config[key] = value;
        config
    };
    let cases = [
        (json!([]), ""),
        (json!({ "routes": { "a:1": 1 } }), "origins"),
        (with("origins", json!([])), "origins"),
        (with("origins", json!("https://game.example")), "origins"),
        (with("origins", json!([ORIGIN, 5])), "origins[1]"),
        (with("origins", json!([""])), "origins[0]"),
        (json!({ "origins": [ORIGIN] }), "routes"),
        (with("routes", json!({})), "routes"),
        (
            with("routes", json!({ "no port": 1 })),
            "routes[\"no port\"]",
        ),
        (
            with("routes", json!({ "host:65536": 1 })),
            "routes[\"host:65536\"]",
        ),
        (with("routes", json!({ "host:0": 1 })), "routes[\"host:0\"]"),
        (with("routes", json!({ "host:1": 0 })), "routes[\"host:1\"]"),
        (
            with("routes", json!({ "host:1": 65536 })),
            "routes[\"host:1\"]",
        ),
        (
            with("routes", json!({ "host:1": "19099" })),
            "routes[\"host:1\"]",
        ),
        (
            with("routes", json!({ "host:1": 1.5 })),
            "routes[\"host:1\"]",
        ),
        (with("maxConnections", json!(0)), "maxConnections"),
        (with("maxConnections", json!(100_001)), "maxConnections"),
        (with("maxConnections", json!("2000")), "maxConnections"),
        (with("port", json!(65536)), "port"),
        (with("port", json!(-1)), "port"),
        (with("bind", json!("")), "bind"),
        (with("bind", json!(127)), "bind"),
        (with("allowNative", json!("yes")), "allowNative"),
        (with("trustedProxies", json!("127.0.0.1")), "trustedProxies"),
        (
            with("trustedProxies", json!(["proxy.local"])),
            "trustedProxies[0]",
        ),
    ];
    for (config, key) in cases {
        let error = Config::from_value(&config).expect_err(&config.to_string());
        assert_eq!(error.key(), key, "{config}");
        if !key.is_empty() {
            assert!(error.to_string().contains(&format!("`{key}`")), "{error}");
        }
    }
    let error = Config::from_json("{ not json").unwrap_err();
    assert!(error.to_string().contains("not valid JSON"), "{error}");
}

#[test]
fn configuration_defaults_and_the_example_file() {
    let config = Config::from_value(&json!({
        "origins": [ORIGIN],
        "routes": { "login.example:9999": 19099 },
        "port": null,
        "maxConnections": 50.0,
        "comment": "unknown keys are ignored",
    }))
    .unwrap();
    assert_eq!(config.bind(), "127.0.0.1");
    assert_eq!(config.port(), 8080);
    assert_eq!(config.max_connections(), 50);
    assert!(!config.allow_native());
    assert_eq!(config.route("login.example:9999"), Some(19099));
    assert!(config.origin_allowed(ORIGIN));
    assert!(!config.is_trusted_proxy("127.0.0.1".parse().unwrap()));

    let example = Config::from_json(include_str!("../config.example.json")).unwrap();
    assert_eq!(example.max_connections(), 2000);
    assert!(example.allow_native());
    assert_eq!(example.route("127.0.0.1:9999"), Some(19099));
    assert_eq!(example.route("127.0.0.1:9998"), Some(19098));
}

/// Kills the child process when dropped.
struct ChildGuard(Child);

impl Drop for ChildGuard {
    fn drop(&mut self) {
        let _ = self.0.kill();
        let _ = self.0.wait();
    }
}

/// Starts the `client-fixture` binary on its fixed ports and probes each
/// endpoint the client repository's transport tests use.
#[tokio::test]
async fn client_fixture_serves_the_transport_probes() {
    let mut child = Command::new(env!("CARGO_BIN_EXE_client-fixture"))
        .stdout(Stdio::piped())
        .spawn()
        .unwrap();
    let stdout = child.stdout.take().unwrap();
    let _child = ChildGuard(child);
    let (lines_tx, lines) = mpsc::channel();
    std::thread::spawn(move || {
        for line in BufReader::new(stdout).lines().map_while(Result::ok) {
            if lines_tx.send(line).is_err() {
                break;
            }
        }
    });
    let next_line = || {
        lines
            .recv_timeout(LIMIT)
            .expect("the fixture prints its endpoints")
    };
    assert_eq!(next_line(), "Gateway fixture: ws://127.0.0.1:18740/game");
    assert_eq!(
        next_line(),
        "Text rejection fixture: ws://127.0.0.1:18741/game"
    );

    let gateway = "ws://127.0.0.1:18740/game";
    let browser = ("Origin", "http://127.0.0.1:18739");
    let (mut client, _) = connect(
        &format!("{gateway}?host=192.0.2.2&port=9999"),
        &[BINARY, browser],
    )
    .await
    .unwrap();
    echo_once(&mut client, b"login").await;
    let (mut client, _) = connect(
        &format!("{gateway}?host=192.0.2.3&port=9998"),
        &[BINARY, ("Origin", "")],
    )
    .await
    .unwrap();
    echo_once(&mut client, b"world").await;
    let rejected = connect(&format!("{gateway}?host=192.0.2.2&port=9997"), &[BINARY]).await;
    assert_eq!(status(rejected), 403);

    let text = "ws://127.0.0.1:18741/game";
    let (mut client, response) = connect(&format!("{text}?host=192.0.2.2&port=9997"), &[BINARY])
        .await
        .unwrap();
    assert_eq!(response.headers()["sec-websocket-protocol"], "binary");
    match timeout(LIMIT, client.next()).await.unwrap() {
        Some(Ok(Message::Text(text))) => {
            assert_eq!(text.as_str(), "text must never become game packet bytes");
        }
        other => panic!("expected the text frame, got {other:?}"),
    }
    assert_eq!(next_line(), "Adversarial text frame sent");
    let (mut client, _) = connect(&format!("{text}?host=192.0.2.2&port=9999"), &[BINARY])
        .await
        .unwrap();
    echo_once(&mut client, b"echo").await;
}
