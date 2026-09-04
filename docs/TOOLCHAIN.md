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

# Exercise the transitional C++17 compatibility lane with:
CXX_STANDARD=17 make dev-test
CXX_STANDARD=17 make dev-build
```

The production image uses the same compiler and defaults to C++20:

```bash
docker build -t darkeden:local .
docker build --build-arg CXX_STANDARD=17 -t darkeden:cxx17 .
```

Both image builds accept `--build-arg ZIG_VERSION=...` for an intentional
toolchain update. The current tree is verified under C++17 and C++20; C++17 is
a transition/rollback lane, not the language level new design should target.

For a manually installed Zig toolchain, the equivalent direct CMake commands
are:

```bash
cmake -B build-zig -DCMAKE_TOOLCHAIN_FILE=cmake/zig-toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=20
cmake --build build-zig -j
```

Zig is located as `$ZIG`, else `zig` on `$PATH`, else `python3 -m ziglang`
(the PyPI `ziglang` package ships the full toolchain).

`tools/devbuild.sh` gives every Zig version, target, C++ standard and build type
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

The current tree still builds as C++17 to provide a transition and rollback
lane. The items below are C++20 adoption work: each slice must either retire
that lane deliberately or hide the new facility behind a small compatibility
boundary. Do not spread project-wide `#if __cplusplus` branches merely to keep
both modes alive.

| Priority | C++20 facility | Project seam | Main benefit |
|---|---|---|---|
| P0 | `std::jthread`, `std::stop_token`, atomic wait/notify | `Thread`, `ZoneGroupThread`, `SMSServiceThread`, `NetmarbleGuildRegisterThread` | Cooperative shutdown and owned joins instead of unsupported `stop()`, `while (true)` and sleep polling |
| P0 | `std::span`, concepts, `std::endian`, `std::bit_cast` | `SocketInputStream`, `SocketOutputStream`, packet codecs | Bound buffer lengths to their data and reject unsafe wire types at compile time |
| P1 | `constexpr`/`consteval` metadata and concepts | `AllPacketFactories.inc`, `PacketIDSet`, packet factory classes | Detect duplicate IDs, invalid sizes and incomplete registrations during compilation |
| P1 | `std::source_location` | `Assert.h`, `Exception.h`, DB/error macros | Preserve call-site diagnostics without compiler-specific macros or repeated file/line plumbing |
| P1 | `std::latch`, `std::barrier`, `std::counting_semaphore` | thread startup phases and bounded work queues | Replace timing assumptions with explicit readiness and back-pressure |
| P2 | ranges, views, `contains`, `erase_if` | manager/registry traversal | Reduce hand-written iterator and double-lookup mistakes once ownership and lock boundaries are explicit |

### Structured thread lifetime and cancellation

The base `Thread` class manually wraps `pthread_create`/`pthread_join`, exposes
a status field, and has a default `stop()` that throws `UnsupportedError`.
Several derived services run `while (true)` and periodically call `usleep` or
`sleep`; `ZoneGroupThread` does this in the main gameplay tick.

`std::jthread` owns the join operation and propagates a `std::stop_token` to
the worker. A stop-aware condition-variable wait can wake immediately during
shutdown instead of waiting for the next polling interval. Where only a state
change is needed, `std::atomic::wait`/`notify_*` can avoid a mutex and repeated
wakeups. This would make orderly process shutdown testable and remove a class
of use-after-free and stuck-join failures.

This should be migrated from the leaves inward: start with an isolated polling
service such as `SMSServiceThread`, establish request-stop/join ownership, and
only then change the `ZoneGroupThread` tick loop. The gameplay tick cadence and
the existing lock around `ZoneGroup` must remain unchanged in the first slice.

### Type-safe packet buffers

The socket streams currently expose raw pointer-plus-length overloads and
unconstrained `read<T>`/`write<T>` templates that copy `sizeof(T)` bytes. Any
accidentally passed pointer, padded class, platform-sized integer or other
non-wire type therefore compiles. The earlier misaligned-access fix proved
that this hot path benefits from making representation rules explicit.

Use `std::span<std::byte>`/`std::span<const std::byte>` for buffer regions and
a `WireScalar` concept for the scalar overloads. That concept should permit
only the fixed-width, trivially-copyable types approved by the protocol.
`std::endian` can document the existing byte order, while `std::bit_cast`
provides representation conversion without aliasing violations. These tools
do not perform byte swapping or bounds validation automatically; the codec
must still check sizes and preserve the deployed client's byte order.

Migrate one packet family at a time behind the existing stream API. Every
slice must keep the wire-layout inventory, per-code encryption goldens and
round-trip tests byte-identical. Packet safety is the highest-value C++20 use
because a compile-time rejection is much cheaper than diagnosing a corrupted
live session.

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
