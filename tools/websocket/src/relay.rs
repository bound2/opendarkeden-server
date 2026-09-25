//! Moving bytes between an upgraded WebSocket and its backend TCP connection.
//!
//! Each direction awaits its write before it reads again, so a slow reader on
//! either side stops the other side's reads instead of growing a queue: at
//! most one WebSocket message (at most 1 MiB) or one TCP chunk is in flight
//! per direction.

use std::sync::atomic::{AtomicBool, Ordering};
use std::time::Duration;

use futures_util::stream::{SplitSink, SplitStream};
use futures_util::{SinkExt, StreamExt};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::tcp::{OwnedReadHalf, OwnedWriteHalf};
use tokio::net::TcpStream;
use tokio::sync::watch;
use tokio::time::{interval_at, timeout, Instant, MissedTickBehavior};
use tokio_tungstenite::tungstenite::error::ProtocolError;
use tokio_tungstenite::tungstenite::protocol::frame::coding::CloseCode;
use tokio_tungstenite::tungstenite::protocol::frame::Utf8Bytes;
use tokio_tungstenite::tungstenite::protocol::CloseFrame;
use tokio_tungstenite::tungstenite::{Bytes, Error as WsError, Message};
use tokio_tungstenite::WebSocketStream;

use crate::{stopped, Timeouts};

type Socket = WebSocketStream<TcpStream>;

/// Largest TCP read forwarded as one WebSocket message.
const CHUNK_BYTES: usize = 64 * 1024;

/// Why a relay stopped.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum Ending {
    /// The client sent a close frame or closed its connection.
    ClientClosed,
    /// The backend closed its connection.
    BackendClosed,
    /// The client broke the gateway's rules; it is told why.
    Refused(CloseCode, &'static str),
    /// A read or write failed on either side.
    Failed,
    /// No pong arrived between two pings.
    Unresponsive,
    /// The gateway is shutting down.
    Shutdown,
}

/// Relays until either side ends, then closes the WebSocket accordingly.
/// The backend connection is dropped before the WebSocket close handshake.
pub(crate) async fn run(
    socket: Socket,
    backend: TcpStream,
    timeouts: &Timeouts,
    stop: &mut watch::Receiver<bool>,
) {
    let (mut sink, mut stream) = socket.split();
    let (mut backend_rx, mut backend_tx) = backend.into_split();
    let alive = AtomicBool::new(true);
    let ending = tokio::select! {
        ending = client_to_backend(&mut stream, &mut backend_tx, &alive) => ending,
        ending = backend_to_client(&mut backend_rx, &mut sink, &alive, timeouts.ping_interval) => ending,
        () = stopped(stop) => Ending::Shutdown,
    };
    tracing::debug!(?ending, "relay finished");
    drop(backend_rx);
    drop(backend_tx);
    finish(ending, sink, stream, timeouts.close_grace).await;
}

async fn client_to_backend(
    stream: &mut SplitStream<Socket>,
    backend: &mut OwnedWriteHalf,
    alive: &AtomicBool,
) -> Ending {
    while let Some(message) = stream.next().await {
        match message {
            Ok(Message::Binary(bytes)) => {
                if backend.write_all(&bytes).await.is_err() {
                    return Ending::Failed;
                }
            }
            Ok(Message::Pong(_)) => alive.store(true, Ordering::Relaxed),
            // tungstenite answers pings itself.
            Ok(Message::Ping(_) | Message::Frame(_)) => {}
            Ok(Message::Close(_)) => return Ending::ClientClosed,
            Ok(Message::Text(_)) | Err(WsError::Utf8(_)) => {
                return Ending::Refused(CloseCode::Unsupported, "Text messages are not accepted")
            }
            Err(WsError::Capacity(_)) => {
                return Ending::Refused(CloseCode::Size, "Message too big")
            }
            // The client dropped the connection; there is no one to tell.
            Err(WsError::Protocol(ProtocolError::ResetWithoutClosingHandshake)) => {
                return Ending::Failed
            }
            Err(WsError::Protocol(_)) => {
                return Ending::Refused(CloseCode::Protocol, "WebSocket protocol error")
            }
            Err(_) => return Ending::Failed,
        }
    }
    Ending::ClientClosed
}

async fn backend_to_client(
    backend: &mut OwnedReadHalf,
    sink: &mut SplitSink<Socket, Message>,
    alive: &AtomicBool,
    ping_interval: Duration,
) -> Ending {
    let mut heartbeat = interval_at(Instant::now() + ping_interval, ping_interval);
    heartbeat.set_missed_tick_behavior(MissedTickBehavior::Delay);
    loop {
        let mut chunk = Vec::with_capacity(CHUNK_BYTES);
        tokio::select! {
            read = backend.read_buf(&mut chunk) => match read {
                Ok(0) => return Ending::BackendClosed,
                Ok(_) => {
                    if sink.send(Message::Binary(Bytes::from(chunk))).await.is_err() {
                        return Ending::Failed;
                    }
                }
                Err(_) => return Ending::Failed,
            },
            _ = heartbeat.tick() => {
                // The client must have answered the previous ping by now.
                if !alive.swap(false, Ordering::Relaxed) {
                    return Ending::Unresponsive;
                }
                if sink.send(Message::Ping(Bytes::new())).await.is_err() {
                    return Ending::Failed;
                }
            }
        }
    }
}

async fn finish(
    ending: Ending,
    mut sink: SplitSink<Socket, Message>,
    mut stream: SplitStream<Socket>,
    grace: Duration,
) {
    let (code, reason) = match ending {
        Ending::ClientClosed => {
            // Flushes tungstenite's reply to the client's close frame.
            let _ = timeout(grace, sink.close()).await;
            return;
        }
        Ending::Failed | Ending::Unresponsive => return,
        Ending::BackendClosed => (CloseCode::Normal, "Server disconnected"),
        Ending::Refused(code, reason) => (code, reason),
        Ending::Shutdown => (CloseCode::Away, "Gateway shutting down"),
    };
    let frame = CloseFrame {
        code,
        reason: Utf8Bytes::from_static(reason),
    };
    let _ = timeout(grace, async {
        if sink.send(Message::Close(Some(frame))).await.is_ok() {
            // Wait for the client's reply so the close is orderly on both ends.
            while let Some(Ok(_)) = stream.next().await {}
        }
    })
    .await;
}
