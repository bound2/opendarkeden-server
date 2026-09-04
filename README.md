# opendarkeden-server

## Install using Docker

Everything below builds the server **from the sources in this repository** - no
pre-built image is downloaded.

## Quick start (docker compose)

```sh
cd docker
docker compose up -d --build
```

That command:

1. builds `../Dockerfile`, which compiles `loginserver`, `sharedserver` and
   `gameserver` as C++17 with pinned Zig 0.16.0/Clang 21.1.0, then packages
   the binaries in an Ubuntu 20.04 runtime together with `data/` and
   `docker/conf/`;
2. starts MySQL 5.7 and imports `initdb/*.sql` on first run;
3. applies `docker/initdb-docker.sql`, which points `DARKEDEN.WorldDBInfo` and
   `DARKEDEN.GameServerInfo` at this stack (the dumps ship with the original
   developers' LAN addresses);
4. starts the three servers in order once the database is ready
   (see `docker/start.sh`).

Follow the logs:

```sh
docker compose logs -f odk-server
```

Stop everything (the database keeps its data in the `odk-mysql-data` volume):

```sh
docker compose down
```

Add `-v` to `docker compose down` to wipe the database as well.

**NOTE:** the compose setup assumes server and client run on the same machine.
To run the client on another machine, set the server IP in the
`DARKEDEN.GameServerInfo` table (or in `docker/initdb-docker.sql` before the
first start) and restart the server container.

### Rebuild after changing the code

```sh
cd docker
docker compose up -d --build
```

The image defaults to `CMAKE_BUILD_TYPE=Release` and C++17. The compose build
arguments can select Debug or the supported C++20 lane:

```sh
BUILD_TYPE=Debug docker compose up -d --build
CXX_STANDARD=20 docker compose up -d --build
```

### Start the servers by hand

Set `command: ["sleep","infinity"]` on the `odk-server` service, then:

```sh
docker exec -w /home/darkeden/vs/bin -it odk-server /bin/bash
./start.sh
```

## Development builds and tests

`Dockerfile.dev` provides the same pinned Zig/Clang compiler as the production
builder. Build it once from the repository root:

```bash
docker build -f Dockerfile.dev -t darkeden-dev .
```

The development helper copies only build inputs into a Docker volume, avoiding
the cost of compiling directly from a Windows bind mount. Artifacts remain in
that volume rather than updating the checkout's `bin/` and `lib/` directories.

```bash
make dev-test
make dev-build

# Exercise the C++20-ready build in its own build tree.
CXX_STANDARD=20 make dev-test
CXX_STANDARD=20 make dev-build

make dev-shell
```

## Howto

### Login to the MySQL

```sh
docker exec -it odk-mysql mysql -u elcastle -pelca110
```

```SQL
use DARKEDEN;
update GameServerInfo set IP = '192.168.0.16';
```

### Pack pre-built binaries into an image

`Dockerfile.pub` packages an already-compiled `bin/` directory instead of
compiling from source, which is useful when publishing a release image:

```sh
docker build . -t darkeden:latest -f Dockerfile.pub
```
