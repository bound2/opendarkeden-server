# Toolchain notes

Two related changes, proof-of-concept quality: the XML dependency was replaced,
which in turn made an alternative compiler toolchain viable.

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

```bash
cmake -B build-zig -DCMAKE_TOOLCHAIN_FILE=cmake/zig-toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Debug
cmake --build build-zig -j
```

Zig is located as `$ZIG`, else `zig` on `$PATH`, else `python3 -m ziglang`
(the PyPI `ziglang` package ships the full toolchain).

> **One build tree at a time.** `CMAKE_*_OUTPUT_DIRECTORY` point at the shared
> source-tree `bin/` and `lib/`, so `build/` and `build-zig/` overwrite each
> other's binaries while each still considers its own targets up to date.

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
- A bundled libc, so the glibc floor is chosen at build time instead of
  discovered at deploy time:
  ```bash
  ZIG_TARGET=x86_64-linux-gnu.2.17 cmake --build build-zig
  ```
  produces a binary that runs on CentOS 7-era glibc from a modern host.
- Cross-compilation without assembling a sysroot by hand.

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

This is the argument for sanitizers in one example: the bug was reachable from
an ordinary `CGWhisper` round-trip, sat in the packet write path used by every
outbound packet, and no amount of reading found it in twenty years.

### What it is not

`zig cc` is a *compiler*, not a build system or a package manager. Zig's own
package management (`build.zig.zon`) is reachable only through `zig build`,
which would mean replacing CMake wholesale. With three external C
dependencies that trade does not pay for itself, so this PoC keeps CMake and
uses Zig only as the compiler driver.
