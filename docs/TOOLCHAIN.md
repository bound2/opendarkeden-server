# Toolchain notes

Replacing the XML dependency made a reproducible Zig/Clang compiler toolchain
viable. That toolchain is now used by both container builders.

## 1. xerces-c replaced by vendored tinyxml2

### What xerces was doing

Four live call sites, all `LoadFromFile`, all at gameserver startup:

| Call site | File | Size |
|---|---|---|
| `GQuestInfo.cpp:100` | `data/SimpleGQuest.xml` | 17 KB |
| `GQuestInfo.cpp:116` | `data/EventGQuest.xml` | 24 KB |
| `GQuestCheckPoint.cpp:15` | `data/EventCheckPoint.xml` | 4.4 KB |
| `GQuestCheckPoint.cpp:46` | `data/TravelWay.xml` | 3.8 KB |

48 KB, read once. `data/EventGQuestB.xml` is never loaded. Every `SaveToFile`
call in the tree is commented out, and `LoadFromMem` had no callers, so the
write path and the in-memory parse path were both dead.

`SXml.cpp` enabled `fgXercesSchema`, `fgSAX2CoreValidation` and
`fgXercesDynamic`, but no data file references a schema, DTD or namespace —
`fgXercesDynamic` validates only when a grammar is found, so validation never
ran. The project linked a W3C compliance engine that had nothing to check.

### Two bugs this surfaced

**Element text was silently discarded.** `XMLTreeGenerator::characters()`
declared its length parameter `unsigned int`; Xerces-C 3.x declares that
virtual as `XMLSize_t` (i.e. `size_t`). The signatures never matched, so the
override never bound and `DefaultHandler`'s no-op ran instead. Every `<Title>`,
`<Script>`, `<CompleteMessage>` and `<FailMessage>` body in every file was
dropped. The mismatch dates to Xerces-C 3.0 (2008) changing that signature;
`SXml.cpp` was written against Xerces 2.x in 2003.

Nothing reads `XMLTree::GetText()` today, which is why this went unnoticed.
The replacement captures text, so `GetText()` now returns content where it
previously returned `""`. No caller is affected. `-Woverloaded-virtual`, or
C++11 `override`, would have caught this at compile time.

**Non-ASCII text was locale-dependent.** Values went through
`XMLString::transcode()` to the process's *local code page*. Under the
container's POSIX locale that double-encoded every non-ASCII byte
(`0xBA` → `U+00BA` → `0xC2 0xBA`), so the parsed text differed depending on
the `LANG` of whoever started the server.

tinyxml2 does no transcoding: bytes reach `XMLTree` exactly as they sit in the
file. That is deterministic. Re-encoding is a separate decision and belongs
where the text is used, not in the parser.

### Encoding state of the data files

The files declare `encoding="iso-8859-1"` but actually hold a **mix** of
EUC-KR and UTF-8 — evidence of a half-finished encoding migration. Of 19
distinct non-ASCII attribute values, 13 are EUC-KR and 6 are UTF-8. Byte
passthrough preserves that distinction so a proper cleanup can be done as its
own reviewable change; the old path double-encoded both uniformly and
destroyed it.

Converting `data/*.xml` to UTF-8 is **not** done here. It is a data change,
and it would need the client's expectations checked alongside it.

### Evidence

`tests/xml_parse_test.cpp` pins the parsed shape of all four files against
`tests/golden/xml-parse.txt`, with every non-ASCII byte spelled `\xNN`.
Re-record deliberately with `UPDATE_GOLDENS=1 ./bin/wire_tests`.

Comparing the old and new backends over those files:

```
line counts                : 2664 vs 2664   (identical structure)
identical lines            : 2565
differ, all explained by    :   99   xerces = UTF-8 re-encoding of the raw
                                     byte read as latin-1 (double-encoding)
differ, unexplained         :    0
```

Every node, attribute name, attribute count and child count matches exactly.
The only differences are the two intended ones above.

`tests/tools/xml_dump.cpp` is the standalone dumper used to produce that
comparison; it is not built by default.

## 2. Building with `zig cc`

`Dockerfile` and `Dockerfile.dev` pin Zig 0.16.0, whose C++ driver reports
Clang 21.1.0. CMake always reaches it through `cmake/zig-toolchain.cmake`, so
the distro's compiler version no longer controls the language features used by
container builds.

For the fast development-volume build:

```bash
docker build -f Dockerfile.dev -t darkeden-dev .
make dev-test
make dev-build
```

The production image uses the same compiler and requires C++20:

```bash
docker build -t darkeden:local .
```

Both image builds accept `--build-arg ZIG_VERSION=...` for an intentional
toolchain update. The current tree requires C++20; the former C++17 rollback
lane was retired when cooperative `std::jthread` ownership entered the zone
tick.

For a manually installed Zig toolchain, the equivalent direct CMake commands
are:

```bash
cmake -B build-zig -DCMAKE_TOOLCHAIN_FILE=cmake/zig-toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=20
cmake --build build-zig -j
```

Zig is located as `$ZIG`, else `zig` on `$PATH`, else `python3 -m ziglang`
(the PyPI `ziglang` package ships the full toolchain).

`tools/devbuild.sh` gives every Zig version, target and build type
its own CMake tree, output root, ccache directory and Zig cache directory. Run
`tools/devbuild.sh output-dir` with the same environment to print that lane's
artifact root. For manual builds, use one build tree and output root per
toolchain configuration, for example
`-DDARKEDEN_OUTPUT_ROOT="$PWD/output-zig-cxx20"`.

### Why it works here

The source is unusually portable for its age: no `__attribute__`, no
`#pragma GCC`, no inline asm, no `__builtin_*`, no `typeof`. The only compiler
extension in the tree is `__PRETTY_FUNCTION__` (108 uses), which clang
supports.

Dropping xerces was the prerequisite. `zig c++` links LLVM's libc++, not GNU
libstdc++, and `SXml.cpp` caught `const XMLException&` across the xerces
boundary — cross-runtime `type_info` matching is exactly where libc++abi and
libsupc++ mixing goes wrong, and it fails by silently falling through to
`catch (...)` rather than by crashing. With xerces gone, every remaining
external dependency (libmysqlclient, lua, zlib) exposes a C API and carries no
C++ ABI. **Re-introducing a C++-API dependency reopens this.**

### What it buys

- One compiler binary, byte-identical on every machine and CI runner, rather
  than whatever `g++` the distribution shipped.
- Zig can select a target and libc version at configure time for code whose
  complete dependency set supports that target:
  ```bash
  cmake -B build-zig-glibc217 \
        -DCMAKE_TOOLCHAIN_FILE=cmake/zig-toolchain.cmake \
        -DDARKEDEN_ZIG_TARGET=x86_64-linux-gnu.2.17 \
        -DDARKEDEN_OUTPUT_ROOT="$PWD/output-zig-glibc217"
  ```
  The target is a CMake cache value, so it is visible in the generated Ninja
  commands and changing it requires a separate build tree.
- Cross-target compilation support without constructing a libc sysroot by
  hand.

The current container installs MySQL, Lua and zlib development libraries from
its Ubuntu 20.04 package repository. Those host libraries are not a Zig cross
sysroot, so a complete server binary built by these Dockerfiles is supported
only for the container's native target. A non-native target or older glibc
floor requires target-compatible builds of all three dependencies; do not
infer CentOS 7 compatibility from `DARKEDEN_ZIG_TARGET` alone.

### What it found immediately

Zig's Debug build enables UBSan by default. The first run of `wire_tests`
under it aborted on pre-existing undefined behaviour in the wire layer,
unrelated to any change here:

```
panic: store of misaligned address 0x2dcbaf76 for type 'unsigned int',
       which requires 4 byte alignment
  src/Core/SocketOutputStream.h:174   *((T*)(m_Buffer + m_Tail)) = buf;
  src/Core/CGWhisper.cpp:62           oStream.write(m_Color);
```

`SocketOutputStream::write<T>` and `SocketInputStream::read<T>` cast a `char*`
ring buffer at an arbitrary byte offset to `T*` and dereference it. That is UB
at any misaligned offset; x86 tolerates it in hardware, which is why GCC builds
have run for two decades without complaint. It is a real portability and
optimisation hazard, not a false positive.

Fixed at all five sites (three stores, two loads) by using `memcpy` with the
same `sizeof(T)` length — the idiom the wrap-around branch of that very
function already used. For a compile-time-constant size this lowers to the
same single instruction, so there is no cost in the hot path. The wire golden
tests pass unchanged under both toolchains, which is what establishes the
output is byte-identical.

The C++17/C++20 migration pass found a second checked-conversion case:
`tileDistance` intentionally narrows a distance to the protocol's byte width.
The code now spells that modulo operation explicitly, preserving the existing
300 → 44 behavior without asking Zig's checked Debug mode to perform an
out-of-range cast.

This is the argument for sanitizers in one example: the bug was reachable from
an ordinary `CGWhisper` round-trip, sat in the packet write path used by every
outbound packet, and no amount of reading found it in twenty years.

### What it is not

`zig cc` is a *compiler*, not a build system or a package manager. Zig's own
package management (`build.zig.zon`) is reachable only through `zig build`,
which would mean replacing CMake wholesale. With three external C
dependencies that trade does not pay for itself, so this setup keeps CMake and
uses Zig only as the compiler driver.

## 3. Where C++20 pays off in DarkEden

Changing the compiler flag does not by itself make the server safer or faster.
The value is that new/refactored code can use a stronger vocabulary at the
places where this codebase currently relies on conventions, raw buffer pairs,
macros and polling loops.

The project now requires C++20. The first production use is cooperative zone
worker shutdown; new facilities should still be adopted at focused boundaries
instead of through tree-wide style conversions.

| Priority | C++20 facility | Project seam | Main benefit |
|---|---|---|---|
| P0 | `std::jthread`, `std::stop_token`, atomic wait/notify | `Thread`, `ZoneGroupThread`, `SMSServiceThread`, `NetmarbleGuildRegisterThread` | Cooperative shutdown and owned joins instead of unsupported `stop()`, `while (true)` and sleep polling |
| P0 | `std::span`, concepts, `std::endian`, `std::bit_cast` | `SocketInputStream`, `SocketOutputStream`, packet codecs | Bound buffer lengths to their data and reject unsafe wire types at compile time |
| P1 | `constexpr`/`consteval` metadata and concepts | `AllPacketFactories.inc`, `PacketIDSet`, packet factory classes | Detect duplicate IDs, invalid sizes and incomplete registrations during compilation |
| P1 | `std::source_location` | `Assert.h`, `Exception.h`, DB/error macros | Preserve call-site diagnostics without compiler-specific macros or repeated file/line plumbing |
| P1 | `std::latch`, `std::barrier`, `std::counting_semaphore` | thread startup phases and bounded work queues | Replace timing assumptions with explicit readiness and back-pressure |
| P2 | ranges, views, `contains`, `erase_if` | manager/registry traversal | Reduce hand-written iterator and double-lookup mistakes once ownership and lock boundaries are explicit |

### Structured thread lifetime and cancellation

The legacy `Thread` backend remains for services outside the gameserver
migration. Gameserver zone, login-link, shared-link, GDR, and enabled
billing/mofus workers now opt into `ManagedThread`, which owns a
`CooperativeThread` backed by `std::jthread`.

`std::jthread` owns the join operation and propagates a `std::stop_token` to
the worker. A stop-aware condition-variable wait can wake immediately during
shutdown instead of waiting for the next polling interval. Where only a state
change is needed, `std::atomic::wait`/`notify_*` can avoid a mutex and repeated
wakeups. Cancellation is cooperative: it cannot interrupt arbitrary C++ code,
a held mutex, or a synchronous library call.

`ZoneGroupThread` retains its one-millisecond wait and existing group lock.
Start/stop operations are serialized, stop-before-start permanently cancels
the worker, and an in-progress join does not prevent another caller requesting
stop. Worker exceptions are retained for reporting after join and request a
failed process shutdown. Pool startup rolls back already-started workers on
any exception; shutdown requests all stops before joining. Derived destructors
must join before their members disappear; a base destructor alone is too late.

SIGTERM/SIGINT only store a lock-free atomic request, using operations permitted
in [C++ signal handlers](https://eel.is/c++draft/support.signal). The main client
loop returns, all zone and auxiliary workers are requested to stop and joined,
then main flushes its status message and calls `_Exit`. The OS reclaims the
legacy singleton graph: its destructor dependency order is not fully audited,
so normal process shutdown deliberately does not invoke that graph. Explicit
`GameServer` destruction also joins workers before releasing dependencies.
Joining workers does **not** provide a new world-save/transaction guarantee.

MySQL connections default to a five-second connect timeout and 300-second
read/write timeouts, allowing longer normal-service queries. Operators can set
`DARKEDEN_DB_CONNECT_TIMEOUT_SECONDS` and `DARKEDEN_DB_IO_TIMEOUT_SECONDS` to
positive integer seconds (Compose forwards them). These are per-operation
limits with [client retry semantics](https://dev.mysql.com/doc/c-api/8.0/en/mysql-options.html),
not an overall query or shutdown time guarantee. Stop is checked between zone
connection setup operations. The login-link UDP socket is nonblocking so idle
traffic cannot prevent shutdown. A separate watchdog starts before server
initialization and allows 30 seconds after a signal/error/shutdown request;
if I/O or gameplay code stays blocked, it exits the whole process with failure
without freeing memory underneath live workers. The container supervisor keeps
login/shared alive while gameserver drains, enforces a 35-second backstop, and
Compose has a 45-second stop grace period.

Regression tests exercise concurrent lifecycle calls, worker errors, pool
rollback and stop/join ordering, dependency lifetime, signals, a stuck worker,
a silent MySQL peer, and the real `docker/start.sh` with stand-in processes.
The pinned-Zig CI job runs these tests and builds all production targets and
the production image only on master pushes/merges. PRs and feature-branch
commits use local verification to conserve Actions minutes. CMake probes the
required library facilities, so an
unsupported toolchain fails at configuration instead of deep in compilation.

### Type-safe packet buffers

Done for the stream layer; the packet codecs above it are unchanged.

`SocketInputStream::read<T>` and `SocketOutputStream::write<T>` copy
`sizeof(T)` bytes of an object's representation out of / into the socket ring
buffer, so whatever compiles *defines* the protocol. Before this change a
pointer, a `std::string`, a padded class or a platform-sized integer (`long`
and `size_t` are 8 bytes on the Linux server and 4 on the Win32 client) all
compiled silently. `src/Core/WireTypes.h` now states the rule once as a
`de::WireScalar` concept, and both templates are constrained by it.

The accept list came from evidence rather than taste: every `T` that
instantiates the two templates across de-kernel and all three servers was
enumerated first, with a throwaway deprecated-probe build. It is `bool`,
`char`, `signed char`, `unsigned char`, `short`, `unsigned short`, `int`,
`unsigned int` and `std::uint64_t`. That covers every `Types.h` field alias,
since `BYTE`/`WORD`/`DWORD` are `unsigned char`/`unsigned short`/`unsigned
int`. Pointers, arrays, class types, floating point, enumerations and
signed 64-bit in every spelling are rejected — trivially copyable
aggregates included, because "can be memcpy'd" is not the question the
wire asks.

One caveat is unavoidable, and the header spells it out. The Exchange packets
(`CGExchangeBuy`, `GCExchangeBuy`, `GCExchangeList`) write their listing id as
a `uint64_t`, so that has to stay admitted — and on the LP64 server
`std::uint64_t` **is** `unsigned long`, so `unsigned long` and `size_t` are
admitted with it and a stray `write(size_t)` still compiles here while it
would be four bytes on the client. Signed 64-bit is not admitted, and that is
what keeps plain `long` out; its only instantiation sites were the dead
`readEncrypt(long&)` / `writeEncrypt(long)` overloads, now removed. The test
asserts this split relative to `std::uint64_t` rather than to a spelling, so
it states the intent on a platform where the two differ.

Constraining alone would have been a hazard rather than a fix: with
`write<T>` constrained away, `write(someCharPointer)` would have found
`write(const string&)` through a user-defined conversion and put a
*different* byte sequence on the wire. Each template therefore has an
unconstrained sibling whose only statement is a `static_assert`, so a
rejected type is a named compile error instead.

The buffer paths gained `read` / `peek(std::span<std::byte>)` and
`write(std::span<const std::byte>)`. The `read` and `write` spans carry the
implementation, and are defined **inline in the headers** so the scalar
templates keep handing their `memcpy` a compile-time-constant `sizeof(T)`:
at `-O2` a four-byte field is still the single load/store the section above
describes, not a call with a runtime length. (`peek` stays out of line --
nothing calls it with a constant size.) The
`char*` / `const char*` plus `uint` signatures are kept for the existing call
sites and forward to them, so the hundreds of callers and the exceptions they
rely on (`InvalidProtocolException` for a zero-length request,
`InsufficientDataException` for a short buffer) are unchanged. The scalar
templates forward as well, which removes the hand-copied second version of
the ring-buffer walk — the copy the misaligned-access fix above had to patch
separately.

`std::endian` states the deployed byte order once, as a `static_assert` in
`WireTypes.h`: the wire format is the little-endian in-memory representation,
no packet swaps bytes, and none is introduced here. `std::bit_cast` is
deliberately **not** used. The aliasing casts it would replace are already
gone (that is what the five `memcpy` sites above are), and the remaining
pointer conversion is to `std::byte*`, which may legally alias any object,
whereas `bit_cast` would add a copy through a temporary array in a hot path.

`tests/wire_types_test.cpp` pins both lists with `static_assert`s, round-trips
every admitted scalar and a span buffer through real loopback sockets, and
drives a 16-byte ring buffer so the wrap-around branch and its split copy
actually run. The wire-layout inventory and every golden are unchanged, which
is what establishes that no packet moved.

What remains: the codecs above the stream still read and write field by
field with no declarative layout; packet sizes are still hand-maintained
(`writePacket` only *warns* when `getPacketSize()` disagrees with the bytes
written); and string fields still carry hand-written length prefixes, which
is why `CGExchangeBuy` needs a comment telling the next author not to pass a
`std::string` to `write` — something the concept now enforces.

### Compile-time packet metadata

Hundreds of packet factories are registered through generated lists and then
validated by tests and ratchets. Keep runtime object creation, but move the
static facts -- packet ID, direction, fixed/minimum size and factory mapping --
into a `constexpr` table. A `consteval` builder can reject duplicate IDs,
out-of-range sizes and missing metadata during compilation. Concepts can also
state the required packet/factory interface and produce a local diagnostic
instead of a template error far inside registration code.

This complements rather than replaces the golden tests: compile-time checks
prove internal consistency, while goldens prove compatibility with the client
and the encrypted wire format.

### Diagnostics without location macros

`Assert.h`, `Exception.h` and the database helpers pass `__FILE__`, `__LINE__`
and `__PRETTY_FUNCTION__` through macros. A defaulted
`std::source_location::current()` parameter captures the caller in an ordinary
function, preserves richer function names, and makes the diagnostic path
unit-testable. This is a low-risk first use of a C++20-only library feature;
the public logging format should remain stable during the migration.

### Explicit coordination and bounded work

Use `std::latch` for one-shot worker readiness, `std::barrier` only where a
repeated phase boundary genuinely exists, and `std::counting_semaphore` for a
bounded producer/consumer queue. These primitives are preferable to adding
another sleep-and-check loop, but they should follow an ownership audit: a new
primitive cannot make shared gameplay state safe if its owner and lock scope
are unclear.

### Safer collection traversal

Ranges and named operations such as `contains` and `erase_if` can remove a lot
of iterator boilerplate in managers and registries. Apply them during focused
ownership refactors, not as a tree-wide style conversion. Views borrow their
source; returning or storing a view into a temporary or into a container after
its lock is released would replace visible iterator code with a subtler
lifetime bug.

### Features that are not first moves

- Coroutines do not turn the current blocking socket and thread-per-zone
  architecture into asynchronous I/O. They become useful only with an evented
  transport, explicit cancellation and back-pressure; that is an architecture
  project, not a language cleanup.
- Modules should wait until the macro-heavy include graph and generated packet
  headers have cleaner boundaries. Measure their effect against the current
  ccache/Ninja build before accepting the extra build-system complexity.
- Parallel algorithms are unsafe as a blanket optimisation over mutable
  gameplay containers. Deterministic ordering, ownership and lock scope must
  be established first.
- `std::format` is useful for typed diagnostics, but changing the logging
  surface is lower value than thread and packet safety and must be benchmarked
  on hot paths.
