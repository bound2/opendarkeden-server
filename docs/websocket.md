# Browser and native WebSocket connections

The gateway carries the existing binary TCP byte stream in binary WebSocket
messages. Packet layouts, encryption, authentication and login/world handoffs
are unchanged. Native TCP remains available. Clients select a gateway URL and
send the original advertised host and port as routing parameters; the gateway
only accepts routes configured by the operator.

## Local setup

Add a different private listener port to each server's configuration:

```ini
# loginserver.conf
GatewayProxyPort : 19099

# gameserver.conf (choose a distinct port for every game process)
GatewayProxyPort : 19098
```

These listeners bind **127.0.0.1 only**. Run the gateway in the same network
namespace as the C++ processes. It sends a PROXY v1 header with the original
IPv4 peer before any game bytes. The C++ listener validates and consumes that
header before admitting a player, preserving the IP identity used by login and
world-transfer records. Never expose a forwarded-identity listener publicly.
Other local processes share this trust boundary.

Install Node.js 22 or newer, then:

```sh
cd tools/websocket
npm ci
cp config.example.json config.json
node gateway.mjs config.json
```

Set `routes` to **every advertised host:port** in the client configuration,
login replies, game transfers and return-to-login replies. Each value is that
process's `GatewayProxyPort`, not its public TCP port. Unknown routes fail
closed; the gateway cannot connect to arbitrary destinations. The example maps
`127.0.0.1:9999` to the login process and `127.0.0.1:9998` to the game process;
adjust these to the deployment's actual settings.

Set the browser client's `client-config.json` `websocketUrl` to
`ws://127.0.0.1:8080/game` for local development. Its `loginHost` and `loginPort`
must name a configured route. On native clients set `DARKEDEN_WEBSOCKET_URL`
to the same URL; leaving it unset uses TCP. Optional `DARKEDEN_LOGIN_HOST` and
`DARKEDEN_LOGIN_PORT` override the initial login endpoint.

## TLS and reverse proxies

Production browser pages use HTTPS and a `wss://` endpoint. Terminate TLS at a
reverse proxy, retaining the gateway's loopback bind. Configure exact allowed
page origins; wildcards are not supported. `allowNative` permits clients that
omit the browser Origin header. This is origin validation, not authentication:
accounts are still authenticated by the game server.

When using a reverse proxy, list its actual peer address in `trustedProxies`.
It must **overwrite**, not append to, `X-Forwarded-For` with one verified IPv4
address. Untrusted peers' forwarding headers are ignored. An example nginx
location, with nginx and the gateway on the same host:

```nginx
location = /game {
    proxy_pass http://127.0.0.1:8080;
    proxy_http_version 1.1;
    proxy_set_header Upgrade $http_upgrade;
    proxy_set_header Connection "upgrade";
    proxy_set_header X-Forwarded-For $remote_addr;
    proxy_set_header Host $host;
    proxy_read_timeout 75s;
}
```

Use `trustedProxies: ["127.0.0.1"]` with this configuration. Public client
addresses must be IPv4 because the existing game protocol and handoff tables
use IPv4. IPv6-only clients are rejected rather than assigned a shared identity.
The existing game's limitations for multiple accounts behind one NAT remain.

For Docker, build `tools/websocket/Dockerfile` using `tools/websocket` as its
context and mount the configuration at `/config/config.json`. Use Compose
`network_mode: "service:server"` to share the C++ server's namespace. A reverse
proxy in that namespace can keep the gateway bound to loopback; otherwise bind
the gateway to the container's private interface and restrict access to the
reverse proxy. Do not publish the `GatewayProxyPort` ports.

The gateway limits messages to 1 MiB, queued writes to 2 MiB, and connections
to `maxConnections` (default 2000). It applies backpressure in both directions,
rejects text, disables compression, and checks liveness with WebSocket pings.
The C++ listener caps unfinished headers at 128 and expires them after five
seconds without blocking the player loop.

## Verification

`make dev-test` includes real-socket tests for split headers, byte preservation,
distinct client identities, malformed headers, premature close and timeout.
`cd tools/websocket && npm ci && npm test` exercises binary fragmentation,
handoff routing, origin and destination rejection, forwarding trust, oversized
frames and text rejection. `node client-fixture.mjs` starts controlled endpoints
for the client repository's native `transport_tests` and browser
`tools/web/test-transport.mjs` probes.
