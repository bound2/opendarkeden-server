//! Controlled endpoints for the client repository's transport probes (the
//! native `transport_tests` and the browser `browser-tests transport` probe),
//! which need neither a database nor an account:
//!
//! * a gateway on 127.0.0.1:18740 that admits `http://127.0.0.1:18739` and
//!   native clients, routing `192.0.2.2:9999` and `192.0.2.3:9998` to
//! * an echo backend on a random loopback port, which drops any connection
//!   whose first line is not a loopback PROXY v1 header and echoes the rest;
//! * a plain WebSocket server on 127.0.0.1:18741 that sends one text frame to
//!   requests whose `port` parameter is 9997 and echoes binary otherwise, so
//!   probes can check that clients reject text.
//!
//! `--bind <ip>` moves the two WebSocket listeners to another address, e.g.
//! `--bind 0.0.0.0` inside a container. The echo backend stays on loopback,
//! where the gateway dials it; with a non-loopback bind it accepts any IPv4
//! client address in the PROXY header, since clients then arrive from a
//! bridge address rather than 127.0.0.1.

use std::io::{self, Write};
use std::net::{IpAddr, Ipv4Addr, SocketAddr};
use std::process::ExitCode;

use darkeden_gateway::{parse_query, shutdown_signal, Config, Gateway};
use futures_util::{SinkExt, StreamExt};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::{TcpListener, TcpStream};
use tokio_tungstenite::tungstenite::handshake::server::{ErrorResponse, Request, Response};
use tokio_tungstenite::tungstenite::http::header::SEC_WEBSOCKET_PROTOCOL;
use tokio_tungstenite::tungstenite::http::HeaderValue;
use tokio_tungstenite::tungstenite::{Bytes, Message};
use tracing_subscriber::EnvFilter;

const GATEWAY_PORT: u16 = 18740;
const TEXT_FIXTURE_PORT: u16 = 18741;
const BROWSER_ORIGIN: &str = "http://127.0.0.1:18739";
const TEXT_ROUTE_PORT: &str = "9997";
const TEXT_PAYLOAD: &str = "text must never become game packet bytes";
/// The longest PROXY v1 line, CRLF included.
const MAX_PROXY_LINE: usize = 108;
const USAGE: &str = "usage: client-fixture [--bind <ip>]";

fn main() -> ExitCode {
    tracing_subscriber::fmt()
        .with_env_filter(
            EnvFilter::try_from_default_env().unwrap_or_else(|_| EnvFilter::new("warn")),
        )
        .with_writer(io::stderr)
        .init();
    let bind = match parse_args() {
        Ok(Some(bind)) => bind,
        Ok(None) => {
            println!("{USAGE}");
            return ExitCode::SUCCESS;
        }
        Err(message) => {
            eprintln!("client-fixture: {message}\n{USAGE}");
            return ExitCode::from(2);
        }
    };
    let runtime = match tokio::runtime::Builder::new_multi_thread()
        .enable_all()
        .build()
    {
        Ok(runtime) => runtime,
        Err(error) => {
            eprintln!("client-fixture: cannot start the async runtime: {error}");
            return ExitCode::FAILURE;
        }
    };
    match runtime.block_on(run(bind)) {
        Ok(()) => ExitCode::SUCCESS,
        Err(error) => {
            eprintln!("client-fixture: {error}");
            ExitCode::FAILURE
        }
    }
}

/// The `--bind` address, or `None` when help was requested.
fn parse_args() -> Result<Option<IpAddr>, String> {
    let mut bind = IpAddr::V4(Ipv4Addr::LOCALHOST);
    let mut args = std::env::args().skip(1);
    while let Some(arg) = args.next() {
        let value = match arg.as_str() {
            "-h" | "--help" => return Ok(None),
            "--bind" => args.next().ok_or("--bind needs an IP address")?,
            other => match other.strip_prefix("--bind=") {
                Some(value) => value.to_owned(),
                None => return Err(format!("unknown argument {other:?}")),
            },
        };
        bind = value
            .parse()
            .map_err(|_| format!("--bind {value:?} is not an IP address"))?;
    }
    Ok(Some(bind))
}

async fn run(bind: IpAddr) -> io::Result<()> {
    let backend = TcpListener::bind((Ipv4Addr::LOCALHOST, 0)).await?;
    let backend_port = backend.local_addr()?.port();
    tokio::spawn(serve_backend(backend, bind.is_loopback()));

    let config = Config::from_value(&serde_json::json!({
        "bind": bind.to_string(),
        "port": GATEWAY_PORT,
        "origins": [BROWSER_ORIGIN],
        "allowNative": true,
        "routes": { "192.0.2.2:9999": backend_port, "192.0.2.3:9998": backend_port },
    }))
    .map_err(|error| io::Error::new(io::ErrorKind::InvalidInput, error))?;
    let gateway = Gateway::bind(config).await?;

    let text_fixture = TcpListener::bind((bind, TEXT_FIXTURE_PORT))
        .await
        .map_err(|error| {
            io::Error::new(
                error.kind(),
                format!("cannot listen on {bind}:{TEXT_FIXTURE_PORT}: {error}"),
            )
        })?;
    tokio::spawn(serve_text_fixture(text_fixture));

    let mut stdout = io::stdout().lock();
    writeln!(
        stdout,
        "Gateway fixture: ws://{}/game",
        SocketAddr::new(bind, GATEWAY_PORT)
    )?;
    writeln!(
        stdout,
        "Text rejection fixture: ws://{}/game",
        SocketAddr::new(bind, TEXT_FIXTURE_PORT)
    )?;
    stdout.flush()?;
    drop(stdout);

    gateway.serve(shutdown_signal()).await;
    Ok(())
}

async fn serve_backend(listener: TcpListener, loopback_clients_only: bool) {
    loop {
        match listener.accept().await {
            Ok((socket, _)) => {
                tokio::spawn(async move {
                    if let Err(error) = echo_after_header(socket, loopback_clients_only).await {
                        tracing::debug!("echo backend connection failed: {error}");
                    }
                });
            }
            Err(error) => {
                tracing::warn!("echo backend accept failed: {error}");
                tokio::time::sleep(std::time::Duration::from_millis(100)).await;
            }
        }
    }
}

/// Checks the PROXY line, then echoes every byte after it.
async fn echo_after_header(mut socket: TcpStream, loopback_clients_only: bool) -> io::Result<()> {
    let mut received = Vec::with_capacity(MAX_PROXY_LINE);
    let mut chunk = [0_u8; 4096];
    let rest = loop {
        let count = socket.read(&mut chunk).await?;
        if count == 0 {
            return Ok(());
        }
        received.extend_from_slice(&chunk[..count]);
        if let Some(end) = received.windows(2).position(|pair| pair == b"\r\n") {
            let rest = received.split_off(end + 2);
            received.truncate(end);
            break rest;
        }
        if received.len() >= MAX_PROXY_LINE {
            return Ok(());
        }
    };
    if !valid_proxy_line(&received, loopback_clients_only) {
        tracing::warn!(
            "echo backend dropped a connection with header {:?}",
            String::from_utf8_lossy(&received)
        );
        return Ok(());
    }
    socket.write_all(&rest).await?;
    let (mut reader, mut writer) = socket.split();
    tokio::io::copy(&mut reader, &mut writer).await?;
    Ok(())
}

/// `^PROXY TCP4 127\.0\.0\.1 127\.0\.0\.1 [0-9]+ [0-9]+$`, or any IPv4 client
/// address when clients are not on loopback.
fn valid_proxy_line(line: &[u8], loopback_clients_only: bool) -> bool {
    let Ok(line) = std::str::from_utf8(line) else {
        return false;
    };
    let number = |field: &str| !field.is_empty() && field.bytes().all(|byte| byte.is_ascii_digit());
    let fields: Vec<&str> = line.split(' ').collect();
    match fields.as_slice() {
        ["PROXY", "TCP4", client, "127.0.0.1", source, destination] => {
            let client_ok = if loopback_clients_only {
                *client == "127.0.0.1"
            } else {
                client.parse::<Ipv4Addr>().is_ok()
            };
            client_ok && number(source) && number(destination)
        }
        _ => false,
    }
}

async fn serve_text_fixture(listener: TcpListener) {
    loop {
        match listener.accept().await {
            Ok((socket, _)) => {
                tokio::spawn(async move {
                    if let Err(error) = text_fixture_connection(socket).await {
                        tracing::debug!("text fixture connection failed: {error}");
                    }
                });
            }
            Err(error) => {
                tracing::warn!("text fixture accept failed: {error}");
                tokio::time::sleep(std::time::Duration::from_millis(100)).await;
            }
        }
    }
}

/// Accepts any path and origin, selects the first offered subprotocol (as the
/// `ws` library did), and either sends one text frame or echoes as binary.
// tungstenite's handshake callback signature fixes the large error type.
#[allow(clippy::result_large_err)]
async fn text_fixture_connection(
    socket: TcpStream,
) -> Result<(), tokio_tungstenite::tungstenite::Error> {
    let mut port = None;
    let callback = |request: &Request, mut response: Response| -> Result<Response, ErrorResponse> {
        port = request.uri().query().and_then(|query| {
            parse_query(query)
                .into_iter()
                .find_map(|(name, value)| (name == "port").then_some(value))
        });
        let first = request
            .headers()
            .get_all(SEC_WEBSOCKET_PROTOCOL)
            .iter()
            .filter_map(|value| value.to_str().ok())
            .flat_map(|value| value.split(','))
            .map(str::trim)
            .find(|protocol| !protocol.is_empty())
            .and_then(|protocol| HeaderValue::from_str(protocol).ok());
        if let Some(protocol) = first {
            response
                .headers_mut()
                .insert(SEC_WEBSOCKET_PROTOCOL, protocol);
        }
        Ok(response)
    };
    let mut socket = tokio_tungstenite::accept_hdr_async(socket, callback).await?;
    let echo = port.as_deref() != Some(TEXT_ROUTE_PORT);
    if !echo {
        socket.send(Message::text(TEXT_PAYLOAD)).await?;
        println!("Adversarial text frame sent");
        let _ = io::stdout().flush();
    }
    while let Some(message) = socket.next().await {
        match message? {
            Message::Binary(bytes) if echo => socket.send(Message::Binary(bytes)).await?,
            // `ws` echoed every message as binary, text included.
            Message::Text(text) if echo => socket.send(Message::Binary(Bytes::from(text))).await?,
            _ => {}
        }
    }
    Ok(())
}
