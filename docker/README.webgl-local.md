# Local browser client

Open <http://127.0.0.1:18739>, choose **Load game**, then **Play**.

Verified locally: browser login, character selection, game entry, movement and
disconnect. The server repairs for Debug startup and character login (#272),
the client's audio path fix (#276), the Rust WebSocket gateway (#273) and the
Rust browser test harness (#277) are all merged on master.

The normal `docker compose up -d --build` command in this directory starts the
game servers, the WebSocket gateway and the static client server alongside the
existing MySQL service. The committed `docker-compose.yml` provides the gateway
(`odk-websocket`, built from `../src/server/websocketproxyserver`, configured by
`websocket.json` here, published on `127.0.0.1:8080`); the local
`docker-compose.override.yml` adds the Debug build and the static client server.

The static server reads `../../client/build/web`. Its `client-config.json` is
mounted from `web-client-config.json` here, so rebuilding the client does not
overwrite the local connection settings. The gateway shares the game server's
network namespace to reach its loopback proxy listeners on ports 19099 and
19098 (`GatewayProxyPort` in `conf/loginserver.conf` and `conf/gameserver.conf`).

The existing database uses the `docker_odk-mysql-data` volume. It was backed up
before applying the repository's migrations 002 and 003; the backup is
`../build/webgl-local/database-before-webgl-20260925-064227.sql`.

After changing server sources, rebuild with `docker compose build odk-server`
and recreate both the server and its shared-network gateway together, since
the gateway's namespace belongs to the server container:

```powershell
docker compose up -d --no-deps --force-recreate odk-server odk-websocket
```
