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
   `gameserver` inside an Ubuntu 20.04 container and packages the binaries
   together with `data/` and `docker/conf/`;
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

The image is built with `CMAKE_BUILD_TYPE=Release`. For a debug build:

```sh
docker build -t darkeden:local --build-arg BUILD_TYPE=Debug .
```

### Start the servers by hand

Set `command: ["sleep","infinity"]` on the `odk-server` service, then:

```sh
docker exec -w /home/darkeden/vs/bin -it odk-server /bin/bash
./start.sh
```

## Building the binaries into the working tree (development)

Use `Dockerfile.dev` when you want the compiled binaries in your local `bin/`
directory instead of inside an image.

First, build the development image:

```bash
docker build -t darkeden:dev . -f Dockerfile.dev
```

Second, run the container with the repository mounted:

```bash
docker run -v `pwd`:/home/darkeden/vs/ -it darkeden:dev /bin/bash
```

On Windows `pwd` should be changed to %cd%

```
docker run -v %cd%/:/home/darkeden/vs/ -it darkeden:dev /bin/bash
```

Third, build the darkeden server binary files

```
make
```

`make` produces a debug build; use `make release` for an optimized one. The
build uses every core it can find, so no `-j` flag is needed.

When the build process finishes, exit docker; loginserver/sharedserver/gameserver
are in the `bin/` directory.

To run those binaries with compose, uncomment the volume mounts in
`docker/docker-compose.yml` and mount `../bin/` as well.

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
