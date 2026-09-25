//! The HTTP side of a connection: reading the request head, deciding whether
//! to admit it, and the responses the gateway writes before any WebSocket
//! frame.
//!
//! The admission rules run in the order the Node.js gateway applied them:
//! path and query (404), route (403), Origin (403), client address (400),
//! then the WebSocket handshake headers themselves (405/400).

use std::collections::HashSet;
use std::io;
use std::net::{IpAddr, Ipv4Addr, SocketAddr};

use tokio::io::AsyncReadExt;
use tokio::net::TcpStream;
use tokio_tungstenite::tungstenite::handshake::derive_accept_key;

use crate::Config;

/// Largest request head accepted; Node's default `maxHeaderSize`.
pub(crate) const MAX_HEAD_BYTES: usize = 16 * 1024;
const MAX_HEADERS: usize = 64;
/// The only subprotocol the gateway speaks: raw game bytes in binary messages.
pub(crate) const SUBPROTOCOL: &str = "binary";
/// Body of the answer to a plain (non-upgrade) HTTP request.
pub(crate) const BANNER: &str = "DarkEden WebSocket gateway\n";

/// A parsed HTTP request head.
#[derive(Debug)]
pub(crate) struct RequestHead {
    method: String,
    target: String,
    /// Lower-case names and values trimmed of surrounding whitespace.
    headers: Vec<(String, Vec<u8>)>,
}

/// Why no request head could be read.
#[derive(Debug)]
pub(crate) enum HeadError {
    /// The peer closed the connection first.
    Closed,
    Io(io::Error),
    Malformed,
    TooLarge,
}

enum Header<'a> {
    Missing,
    One(&'a [u8]),
    Many,
}

impl RequestHead {
    fn values<'a>(&'a self, name: &'a str) -> impl Iterator<Item = &'a [u8]> + 'a {
        self.headers
            .iter()
            .filter(move |(key, _)| key == name)
            .map(|(_, value)| value.as_slice())
    }

    fn single<'a>(&'a self, name: &'a str) -> Header<'a> {
        let mut values = self.values(name);
        match (values.next(), values.next()) {
            (None, _) => Header::Missing,
            (Some(value), None) => Header::One(value),
            (Some(_), Some(_)) => Header::Many,
        }
    }

    /// Node's http parser reports an upgrade when the request carries an
    /// `Upgrade` header and `Connection` lists the `upgrade` token.
    fn is_upgrade(&self) -> bool {
        self.values("upgrade").next().is_some()
            && self
                .values("connection")
                .flat_map(|value| value.split(|byte| *byte == b','))
                .any(|token| trim(token).eq_ignore_ascii_case(b"upgrade"))
    }
}

/// Reads one request head. Returns it with any bytes the client sent after it.
pub(crate) async fn read_head(stream: &mut TcpStream) -> Result<(RequestHead, Vec<u8>), HeadError> {
    let mut buffer = Vec::with_capacity(1024);
    let mut chunk = [0_u8; 2048];
    loop {
        let count = stream.read(&mut chunk).await.map_err(HeadError::Io)?;
        if count == 0 {
            return Err(HeadError::Closed);
        }
        buffer.extend_from_slice(&chunk[..count]);
        if let Some(parsed) = parse_head(&buffer)? {
            return Ok(parsed);
        }
        if buffer.len() >= MAX_HEAD_BYTES {
            return Err(HeadError::TooLarge);
        }
    }
}

fn parse_head(buffer: &[u8]) -> Result<Option<(RequestHead, Vec<u8>)>, HeadError> {
    let mut headers = [httparse::EMPTY_HEADER; MAX_HEADERS];
    let mut request = httparse::Request::new(&mut headers);
    match request.parse(buffer) {
        Ok(httparse::Status::Complete(length)) if length <= MAX_HEAD_BYTES => {
            let head = RequestHead {
                method: request.method.unwrap_or_default().to_owned(),
                target: request.path.unwrap_or_default().to_owned(),
                headers: request
                    .headers
                    .iter()
                    .map(|header| {
                        (
                            header.name.to_ascii_lowercase(),
                            trim(header.value).to_vec(),
                        )
                    })
                    .collect(),
            };
            Ok(Some((head, buffer[length..].to_vec())))
        }
        Ok(httparse::Status::Complete(_)) | Err(httparse::Error::TooManyHeaders) => {
            Err(HeadError::TooLarge)
        }
        Ok(httparse::Status::Partial) => Ok(None),
        Err(_) => Err(HeadError::Malformed),
    }
}

/// What to do with a request head.
#[derive(Debug, PartialEq, Eq)]
pub(crate) enum Decision {
    /// Not a WebSocket upgrade: answer with the banner.
    Banner {
        head_only: bool,
    },
    Reject(Rejection),
    Upgrade(Upgrade),
}

/// An admitted WebSocket upgrade.
#[derive(Debug, PartialEq, Eq)]
pub(crate) struct Upgrade {
    /// The local GatewayProxyPort the route names.
    pub(crate) backend_port: u16,
    /// The client's IPv4 address for the PROXY header.
    pub(crate) client_ip: Ipv4Addr,
    /// The `Sec-WebSocket-Accept` value.
    pub(crate) accept_key: String,
}

/// A refused request: an HTTP status and a reason for the log.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub(crate) struct Rejection {
    pub(crate) status: u16,
    pub(crate) reason: &'static str,
}

impl Rejection {
    pub(crate) const fn new(status: u16, reason: &'static str) -> Self {
        Self { status, reason }
    }

    /// The complete response. The body is empty, as the Node gateway's was.
    pub(crate) fn response(self) -> String {
        let text = match self.status {
            400 => "Bad Request",
            403 => "Forbidden",
            404 => "Not Found",
            405 => "Method Not Allowed",
            431 => "Request Header Fields Too Large",
            502 => "Bad Gateway",
            _ => "Error",
        };
        let extra = if self == BAD_VERSION {
            "Sec-WebSocket-Version: 13, 8\r\n"
        } else {
            ""
        };
        format!(
            "HTTP/1.1 {} {text}\r\nConnection: close\r\n{extra}Content-Length: 0\r\n\r\n",
            self.status
        )
    }
}

const NOT_FOUND: Rejection = Rejection::new(404, "not /game?host=...&port=...");
const BAD_VERSION: Rejection = Rejection::new(400, "missing or invalid Sec-WebSocket-Version");

/// The answer to a plain HTTP request.
pub(crate) fn banner_response(head_only: bool) -> String {
    format!(
        "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: {}\r\nConnection: close\r\n\r\n{}",
        BANNER.len(),
        if head_only { "" } else { BANNER }
    )
}

/// The answer that completes a WebSocket handshake.
pub(crate) fn switching_protocols(accept_key: &str) -> String {
    format!(
        "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: {accept_key}\r\nSec-WebSocket-Protocol: {SUBPROTOCOL}\r\n\r\n"
    )
}

/// Applies the gateway's admission rules to a request from `peer`.
pub(crate) fn decide(head: &RequestHead, peer: SocketAddr, config: &Config) -> Decision {
    if head.method == "CONNECT" {
        return Decision::Reject(Rejection::new(405, "CONNECT is not supported"));
    }
    if !head.is_upgrade() {
        return Decision::Banner {
            head_only: head.method == "HEAD",
        };
    }
    match admit(head, peer, config) {
        Ok(upgrade) => Decision::Upgrade(upgrade),
        Err(rejection) => Decision::Reject(rejection),
    }
}

fn admit(head: &RequestHead, peer: SocketAddr, config: &Config) -> Result<Upgrade, Rejection> {
    let backend_port = route(head, config)?;
    check_origin(head, config)?;
    let client_ip = client_address(head, peer, config)?;
    let accept_key = check_websocket(head)?;
    Ok(Upgrade {
        backend_port,
        client_ip,
        accept_key,
    })
}

/// `/game` with exactly one `host` and one `port` parameter, naming a route.
fn route(head: &RequestHead, config: &Config) -> Result<u16, Rejection> {
    let target = head.target.split('#').next().unwrap_or_default();
    let (path, query) = target.split_once('?').unwrap_or((target, ""));
    if path != "/game" {
        return Err(NOT_FOUND);
    }
    let mut host = None;
    let mut port = None;
    for (name, value) in parse_query(query) {
        let slot = match name.as_str() {
            "host" => &mut host,
            "port" => &mut port,
            _ => return Err(NOT_FOUND),
        };
        if slot.replace(value).is_some() {
            return Err(NOT_FOUND);
        }
    }
    let (Some(host), Some(port)) = (host, port) else {
        return Err(NOT_FOUND);
    };
    config
        .route(&format!("{host}:{port}"))
        .ok_or(Rejection::new(403, "route is not configured"))
}

/// Browsers must send a listed Origin. A missing or empty Origin marks a
/// native client (the native client sends `Origin: ` on purpose), admitted
/// only with `allowNative`.
fn check_origin(head: &RequestHead, config: &Config) -> Result<(), Rejection> {
    match head.single("origin") {
        Header::Missing | Header::One(b"") if config.allow_native() => Ok(()),
        Header::Missing | Header::One(b"") => {
            Err(Rejection::new(403, "native clients are not allowed"))
        }
        Header::One(origin) => match std::str::from_utf8(origin) {
            Ok(origin) if config.origin_allowed(origin) => Ok(()),
            _ => Err(Rejection::new(403, "origin is not allowed")),
        },
        Header::Many => Err(Rejection::new(403, "more than one Origin header")),
    }
}

/// The client's IPv4 address: the TCP peer, or the one `X-Forwarded-For`
/// address a trusted reverse proxy supplies.
fn client_address(
    head: &RequestHead,
    peer: SocketAddr,
    config: &Config,
) -> Result<Ipv4Addr, Rejection> {
    let peer = peer.ip().to_canonical();
    if config.is_trusted_proxy(peer) {
        let forwarded = match head.single("x-forwarded-for") {
            Header::One(value) => std::str::from_utf8(value)
                .ok()
                .and_then(|value| value.parse::<Ipv4Addr>().ok()),
            Header::Missing | Header::Many => None,
        };
        return forwarded.ok_or(Rejection::new(
            400,
            "a trusted proxy must send exactly one IPv4 X-Forwarded-For address",
        ));
    }
    match peer {
        IpAddr::V4(address) => Ok(address),
        IpAddr::V6(_) => Err(Rejection::new(400, "client address is not IPv4")),
    }
}

/// The RFC 6455 handshake headers, checked as the `ws` library checked them,
/// plus the `binary` subprotocol. Returns the `Sec-WebSocket-Accept` value.
fn check_websocket(head: &RequestHead) -> Result<String, Rejection> {
    if head.method != "GET" {
        return Err(Rejection::new(405, "WebSocket handshake must be GET"));
    }
    match head.single("upgrade") {
        Header::One(value) if value.eq_ignore_ascii_case(b"websocket") => {}
        _ => return Err(Rejection::new(400, "invalid Upgrade header")),
    }
    let key = match head.single("sec-websocket-key") {
        Header::One(key) if valid_key(key) => key,
        _ => return Err(Rejection::new(400, "missing or invalid Sec-WebSocket-Key")),
    };
    match head.single("sec-websocket-version") {
        Header::One(b"13" | b"8") => {}
        _ => return Err(BAD_VERSION),
    }
    // Repeated headers are one comma-separated list, as Node joined them.
    let offered = head
        .values("sec-websocket-protocol")
        .collect::<Vec<_>>()
        .join(&b", "[..]);
    let offered = parse_subprotocols(&offered)
        .ok_or(Rejection::new(400, "invalid Sec-WebSocket-Protocol header"))?;
    if !offered.contains(SUBPROTOCOL.as_bytes()) {
        return Err(Rejection::new(
            400,
            "the binary subprotocol was not offered",
        ));
    }
    Ok(derive_accept_key(key))
}

/// Base64 of 16 bytes: `^[+/0-9A-Za-z]{22}==$`.
fn valid_key(key: &[u8]) -> bool {
    key.len() == 24
        && key.ends_with(b"==")
        && key[..22]
            .iter()
            .all(|byte| byte.is_ascii_alphanumeric() || *byte == b'+' || *byte == b'/')
}

/// A comma-separated list of distinct tokens, or `None` if malformed.
fn parse_subprotocols(header: &[u8]) -> Option<HashSet<&[u8]>> {
    let mut protocols = HashSet::new();
    for token in header.split(|byte| *byte == b',') {
        let token = trim(token);
        if token.is_empty() || !token.iter().all(|byte| is_tchar(*byte)) || !protocols.insert(token)
        {
            return None;
        }
    }
    Some(protocols)
}

/// RFC 7230 `tchar`.
fn is_tchar(byte: u8) -> bool {
    byte.is_ascii_alphanumeric() || b"!#$%&'*+-.^_`|~".contains(&byte)
}

fn trim(bytes: &[u8]) -> &[u8] {
    let is_space = |byte: &u8| *byte == b' ' || *byte == b'\t';
    let start = bytes
        .iter()
        .position(|byte| !is_space(byte))
        .unwrap_or(bytes.len());
    let end = bytes
        .iter()
        .rposition(|byte| !is_space(byte))
        .map_or(start, |end| end + 1);
    &bytes[start..end]
}

/// Decodes an `application/x-www-form-urlencoded` query string the way the
/// WHATWG `URLSearchParams` does: pairs split on `&`, empty pairs skipped,
/// `+` as space, percent escapes decoded, invalid UTF-8 replaced.
pub fn parse_query(query: &str) -> Vec<(String, String)> {
    query
        .split('&')
        .filter(|pair| !pair.is_empty())
        .map(|pair| {
            let (name, value) = pair.split_once('=').unwrap_or((pair, ""));
            (form_decode(name), form_decode(value))
        })
        .collect()
}

fn form_decode(text: &str) -> String {
    let bytes = text.as_bytes();
    let mut decoded = Vec::with_capacity(bytes.len());
    let mut index = 0;
    while index < bytes.len() {
        let escaped = match bytes.get(index..index + 3) {
            Some([b'%', high, low]) => hex(*high).zip(hex(*low)).map(|(high, low)| high << 4 | low),
            _ => None,
        };
        if let Some(byte) = escaped {
            decoded.push(byte);
            index += 3;
        } else {
            decoded.push(if bytes[index] == b'+' {
                b' '
            } else {
                bytes[index]
            });
            index += 1;
        }
    }
    String::from_utf8_lossy(&decoded).into_owned()
}

fn hex(digit: u8) -> Option<u8> {
    char::from(digit)
        .to_digit(16)
        .and_then(|value| u8::try_from(value).ok())
}

#[cfg(test)]
mod tests {
    use super::*;

    fn config(extra: serde_json::Value) -> Config {
        let mut value = serde_json::json!({
            "origins": ["https://game.example"],
            "allowNative": true,
            "routes": { "192.0.2.2:9999": 19099 }
        });
        if let (Some(base), Some(extra)) = (value.as_object_mut(), extra.as_object()) {
            base.extend(extra.clone());
        }
        Config::from_value(&value).expect("test config is valid")
    }

    fn head(request: &str) -> RequestHead {
        match parse_head(request.as_bytes()) {
            Ok(Some((head, _))) => head,
            other => panic!("unparsable request: {other:?}"),
        }
    }

    fn upgrade(target: &str, extra_headers: &str) -> RequestHead {
        head(&format!(
            "GET {target} HTTP/1.1\r\nHost: gateway\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n\
             Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n{extra_headers}\r\n"
        ))
    }

    fn peer() -> SocketAddr {
        SocketAddr::from(([127, 0, 0, 1], 40000))
    }

    fn status(decision: Decision) -> u16 {
        match decision {
            Decision::Reject(rejection) => rejection.status,
            Decision::Upgrade(_) => 101,
            Decision::Banner { .. } => 200,
        }
    }

    const ROUTE: &str = "/game?host=192.0.2.2&port=9999";
    const BINARY: &str = "Sec-WebSocket-Protocol: binary\r\n";

    #[test]
    fn admits_a_native_upgrade_and_derives_the_accept_key() {
        let decision = decide(
            &upgrade(ROUTE, BINARY),
            peer(),
            &config(serde_json::json!({})),
        );
        assert_eq!(
            decision,
            Decision::Upgrade(Upgrade {
                backend_port: 19099,
                client_ip: Ipv4Addr::LOCALHOST,
                // RFC 6455 section 1.3.
                accept_key: "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=".to_owned(),
            })
        );
    }

    #[test]
    fn plain_requests_get_the_banner() {
        let config = config(serde_json::json!({}));
        let get = head("GET /anything HTTP/1.1\r\nHost: x\r\n\r\n");
        assert_eq!(
            decide(&get, peer(), &config),
            Decision::Banner { head_only: false }
        );
        let head_request = head("HEAD / HTTP/1.1\r\n\r\n");
        assert_eq!(
            decide(&head_request, peer(), &config),
            Decision::Banner { head_only: true }
        );
        let no_connection_token = head("GET /game HTTP/1.1\r\nUpgrade: websocket\r\n\r\n");
        assert_eq!(status(decide(&no_connection_token, peer(), &config)), 200);
    }

    #[test]
    fn path_and_query_must_name_exactly_one_route() {
        let config = config(serde_json::json!({}));
        for target in [
            "/other?host=192.0.2.2&port=9999",
            "/game/?host=192.0.2.2&port=9999",
            "/game",
            "/game?host=192.0.2.2",
            "/game?host=192.0.2.2&port=9999&port=9999",
            "/game?host=192.0.2.2&port=9999&x=1",
            "/game?host=192.0.2.2&port=9999&x",
        ] {
            assert_eq!(
                status(decide(&upgrade(target, BINARY), peer(), &config)),
                404,
                "{target}"
            );
        }
        assert_eq!(
            status(decide(
                &upgrade("/game?host=192.0.2.2&port=22", BINARY),
                peer(),
                &config
            )),
            403
        );
        for target in [
            "/game?port=9999&host=192.0.2.2",
            "/game?host=192%2E0.2.2&&port=9999&",
            "/game?host=192.0.2.2&port=9999#fragment",
        ] {
            assert_eq!(
                status(decide(&upgrade(target, BINARY), peer(), &config)),
                101,
                "{target}"
            );
        }
    }

    #[test]
    fn websocket_headers_are_validated_after_admission() {
        let config = config(serde_json::json!({}));
        let post = head(&format!(
            "POST {ROUTE} HTTP/1.1\r\nUpgrade: websocket\r\nConnection: upgrade\r\n\r\n"
        ));
        assert_eq!(status(decide(&post, peer(), &config)), 405);
        for protocols in [
            "",
            "Sec-WebSocket-Protocol: chat\r\n",
            "Sec-WebSocket-Protocol: binary,\r\n",
            "Sec-WebSocket-Protocol: binary, binary\r\n",
            "Sec-WebSocket-Protocol: bin ary\r\n",
        ] {
            assert_eq!(
                status(decide(&upgrade(ROUTE, protocols), peer(), &config)),
                400,
                "{protocols:?}"
            );
        }
        for protocols in [
            "Sec-WebSocket-Protocol: chat, binary\r\n",
            "Sec-WebSocket-Protocol: chat\r\nSec-WebSocket-Protocol: binary\r\n",
        ] {
            assert_eq!(
                status(decide(&upgrade(ROUTE, protocols), peer(), &config)),
                101,
                "{protocols:?}"
            );
        }
        let old_version = head(&format!(
            "GET {ROUTE} HTTP/1.1\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n\
             Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 7\r\n{BINARY}\r\n"
        ));
        let rejection = decide(&old_version, peer(), &config);
        assert_eq!(rejection, Decision::Reject(BAD_VERSION));
        assert!(BAD_VERSION
            .response()
            .contains("Sec-WebSocket-Version: 13, 8\r\n"));
    }

    #[test]
    fn native_clients_send_no_or_an_empty_origin() {
        let strict = config(serde_json::json!({ "allowNative": false }));
        let open = config(serde_json::json!({}));
        let browser = format!("Origin: https://game.example\r\n{BINARY}");
        let empty = format!("Origin: \r\n{BINARY}");
        let attacker = format!("Origin: https://attacker.example\r\n{BINARY}");
        assert_eq!(
            status(decide(&upgrade(ROUTE, &browser), peer(), &strict)),
            101
        );
        assert_eq!(status(decide(&upgrade(ROUTE, &empty), peer(), &open)), 101);
        assert_eq!(status(decide(&upgrade(ROUTE, BINARY), peer(), &open)), 101);
        assert_eq!(
            status(decide(&upgrade(ROUTE, &empty), peer(), &strict)),
            403
        );
        assert_eq!(
            status(decide(&upgrade(ROUTE, BINARY), peer(), &strict)),
            403
        );
        assert_eq!(
            status(decide(&upgrade(ROUTE, &attacker), peer(), &open)),
            403
        );
    }

    #[test]
    fn forwarded_addresses_need_a_trusted_peer() {
        let trusted = config(serde_json::json!({ "trustedProxies": ["127.0.0.1"] }));
        let forwarded = |value: &str| format!("X-Forwarded-For: {value}\r\n{BINARY}");
        let client = |decision| match decision {
            Decision::Upgrade(upgrade) => Some(upgrade.client_ip),
            _ => None,
        };
        let mapped = SocketAddr::from(([0, 0, 0, 0, 0, 0xffff, 0x7f00, 1], 40000));
        assert_eq!(
            client(decide(
                &upgrade(ROUTE, &forwarded("203.0.113.44")),
                mapped,
                &trusted
            )),
            Some(Ipv4Addr::new(203, 0, 113, 44))
        );
        let untrusted = config(serde_json::json!({}));
        assert_eq!(
            client(decide(
                &upgrade(ROUTE, &forwarded("203.0.113.44")),
                peer(),
                &untrusted
            )),
            Some(Ipv4Addr::LOCALHOST)
        );
        for value in ["203.0.113.44, 10.0.0.1", "::1", "", "203.0.113.044"] {
            assert_eq!(
                status(decide(&upgrade(ROUTE, &forwarded(value)), peer(), &trusted)),
                400,
                "{value:?}"
            );
        }
        let twice = format!(
            "X-Forwarded-For: 203.0.113.44\r\n{}",
            forwarded("203.0.113.44")
        );
        assert_eq!(
            status(decide(&upgrade(ROUTE, &twice), peer(), &trusted)),
            400
        );
        assert_eq!(
            status(decide(&upgrade(ROUTE, BINARY), peer(), &trusted)),
            400
        );
        let v6 = SocketAddr::from(([0, 0, 0, 0, 0, 0, 0, 1], 40000));
        assert_eq!(status(decide(&upgrade(ROUTE, BINARY), v6, &untrusted)), 400);
    }

    #[test]
    fn query_strings_decode_like_url_search_params() {
        assert_eq!(
            parse_query("a=1&&b=%41+c&c&d=%zz&e=%E2%82%AC&=f"),
            [
                ("a", "1"),
                ("b", "A c"),
                ("c", ""),
                ("d", "%zz"),
                ("e", "\u{20ac}"),
                ("", "f"),
            ]
            .map(|(name, value)| (name.to_owned(), value.to_owned()))
        );
    }

    #[test]
    fn oversized_or_malformed_heads_are_refused() {
        assert!(matches!(
            parse_head(b"GET / HTTP/1.1\r\nHost: x\r\n"),
            Ok(None)
        ));
        assert!(matches!(
            parse_head(b"\x01\x02 nonsense\r\n\r\n"),
            Err(HeadError::Malformed)
        ));
        let many: String = (0..=MAX_HEADERS)
            .map(|index| format!("X-{index}: y\r\n"))
            .collect();
        assert!(matches!(
            parse_head(format!("GET / HTTP/1.1\r\n{many}\r\n").as_bytes()),
            Err(HeadError::TooLarge)
        ));
    }
}
