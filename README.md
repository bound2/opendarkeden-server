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
   `gameserver` as C++20 with pinned Zig 0.16.0/Clang 21.1.0, then packages
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

The image uses C++20 and defaults to `CMAKE_BUILD_TYPE=Release`. The compose
build arguments can select Debug:

```sh
BUILD_TYPE=Debug docker compose up -d --build
```

`docker compose down` requests gameserver shutdown first and keeps the
login/shared processes alive until its workers finish. Gameserver has a
30-second shutdown deadline; Compose allows 45 seconds before killing the
container. A deadline expiry is a failed, forced exit, not a completed drain.
This joins workers but does not introduce a full world-save operation.

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

### Accounts and passwords

`initdb/DARKEDEN.sql` ships two development accounts, each with characters:

| Account  | Password |
|----------|----------|
| `111111` | `111111` |
| `222222` | `222222` |

Passwords are stored as argon2id hashes in `Player.Password` (the
loginserver's `PasswordHash` module, over the vendored `third_party/argon2`),
never in plain text. Registering from the client creates a hashed account.
To set or reset a password by hand, hash it with `bin/hashpw` (the password
is read from stdin so it stays out of shell history) and store the result:

```sh
docker exec -i odk-server ./hashpw <<< 'new-password'
```

```SQL
UPDATE DARKEDEN.Player SET Password = '$argon2id$v=19$...' WHERE PlayerID = 'someone';
```

An existing database needs the column widened once
(`initdb/migrations/001-argon2-password-column.sql`). Its rows can keep
their old plaintext value: the loginserver still accepts it and rewrites the
row as a hash on that account's next successful login, so nobody is locked
out. `bin/hashpw --verify '<stored value>'` checks a password against a
stored value.

### Pack pre-built binaries into an image

`Dockerfile.pub` packages an already-compiled `bin/` directory instead of
compiling from source, which is useful when publishing a release image. It
installs the same runtime libraries and applies the same `start.sh`/CRLF
handling as the source build's runtime stage. The checkout's `bin/` must hold
Linux binaries built for the Ubuntu 20.04 runtime (e.g. copied out of the
`darkeden-dev` volume; `make dev-build` does not update `bin/`). BuildKit is
required so that `Dockerfile.pub.dockerignore` (which keeps `bin/` in the
context) is used instead of `.dockerignore`:

```sh
DOCKER_BUILDKIT=1 docker build . -t darkeden:latest -f Dockerfile.pub
```
