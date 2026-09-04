# ============================================================================
# CMake toolchain file: build with `zig cc` / `zig c++` instead of the host
# GCC or Clang.
#
#   cmake -B build-zig -DCMAKE_TOOLCHAIN_FILE=cmake/zig-toolchain.cmake \
#         -DCMAKE_BUILD_TYPE=Debug
#   cmake --build build-zig -j
#
# Why this is possible at all: the source is unusually portable for its age --
# no __attribute__, no #pragma GCC, no inline asm, no __builtin_*, no typeof.
# The only compiler extension in the tree is __PRETTY_FUNCTION__, which clang
# (and therefore zig cc) supports.
#
# What it buys:
#   * one compiler binary, byte-identical on every machine and CI runner,
#     instead of "whatever g++ the distro shipped"
#   * a bundled libc and cross-target compiler support. Complete server
#     portability still requires target-compatible MySQL, Lua and zlib
#     libraries; the host libraries found below are not a cross sysroot.
#
# Prerequisite: this project must have no C++-API external dependency, since
# `zig c++` links libc++ rather than libstdc++. That became true when
# xerces-c was replaced by the vendored tinyxml2; libmysqlclient, lua and
# zlib are all C APIs and carry no C++ ABI.
# ============================================================================

set(CMAKE_SYSTEM_NAME Linux)

get_filename_component(_zig_dir "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)

set(CMAKE_C_COMPILER "${_zig_dir}/zig-cc" CACHE FILEPATH "zig cc wrapper")
set(CMAKE_CXX_COMPILER "${_zig_dir}/zig-c++" CACHE FILEPATH "zig c++ wrapper")

# Capture the target while CMake configures the compiler. CMake then records
# it in every compile command and reruns ABI detection in a target-specific
# build tree. Reading ZIG_TARGET only inside the wrappers made target changes
# invisible to both CMake and Ninja.
set(DARKEDEN_ZIG_TARGET "$ENV{ZIG_TARGET}" CACHE STRING "Optional Zig target triple")
if(DARKEDEN_ZIG_TARGET)
    set(CMAKE_C_COMPILER_TARGET "${DARKEDEN_ZIG_TARGET}")
    set(CMAKE_CXX_COMPILER_TARGET "${DARKEDEN_ZIG_TARGET}")
endif()

# Zig ships its own linker (LLD) and archiver; using the host binutils
# alongside a Zig-produced object set is the usual source of confusing link
# errors, so route those through Zig too.
set(CMAKE_AR "${_zig_dir}/zig-ar" CACHE FILEPATH "zig ar wrapper")
set(CMAKE_RANLIB "${_zig_dir}/zig-ranlib" CACHE FILEPATH "zig ranlib wrapper")

# Deliberately NOT setting CMAKE_<LANG>_COMPILER_FORCED: it skips CMake's
# feature detection, and any dependency calling target_compile_features()
# (googletest does) then fails with "no known features for CXX compiler".
# Zig's driver handles the probe fine, so let CMake run it.
