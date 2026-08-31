#!/usr/bin/env bash
#
# devbuild.sh — fast containerised build and test for local development.
#
# WHY THIS EXISTS
#
# The obvious way to build in a container on Windows is to bind-mount the
# checkout and run cmake against it. That is what we did, and a full build
# took ~20 minutes at ~20% CPU on an 8-core machine. The compiler was not the
# bottleneck; the mount was. Measured inside the container, against the
# checkout on the Windows filesystem versus the container's own:
#
#     stat 2000 headers      0.813s   vs  0.005s   (160x)
#     read 500 headers       1.173s   vs  0.008s   (145x)
#
# Every translation unit opens dozens of headers, and every incremental build
# re-stats the whole dependency graph, so the build spends its life waiting on
# 9p round-trips instead of compiling.
#
# The build inputs are small — src/ and tests/ are ~37 MB across ~4,500 files,
# while build/, lib/ and bin/ are ~5.6 GB of artifacts. So this script copies
# the *inputs* into a container-local volume once (incrementally thereafter)
# and keeps every artifact off the mount entirely.
#
# It also switches the dev build to the same toolchain the production
# Dockerfile already uses and explains there: Ninja rather than Unix
# Makefiles, because with Makefiles a target's objects are not compiled until
# every library it links against has finished *linking*, which idles cores at
# each Core -> Packets -> server boundary; plus ccache, so a rebuild after a
# branch switch reuses objects instead of recompiling them.
#
# USAGE (from the repository root)
#
#     tools/devbuild.sh test              # build wire_tests + run ctest
#     tools/devbuild.sh test --record     # same, but re-record goldens first
#     tools/devbuild.sh build             # build every production target
#     tools/devbuild.sh build wire_tests  # build one target
#     tools/devbuild.sh shell             # interactive shell in the workspace
#     tools/devbuild.sh clean             # drop the workspace and ccache
#
# Generated test data (goldens, the wire-layout inventory, the generated
# factory list) is copied back into the checkout after a run, so a --record
# leaves a reviewable diff exactly as an in-tree build would.
#
set -euo pipefail

IMAGE=darkeden-dev
# Overridable so parallel checkouts (git worktrees) can each build in their
# own workspace volume without racing this script's rsync --delete. The
# ccache volume is safe to share — ccache is designed for concurrent use.
WORK_VOLUME=${DEVBUILD_WORK_VOLUME:-darkeden-work}
CCACHE_VOLUME=${DEVBUILD_CCACHE_VOLUME:-darkeden-ccache}
BUILD_TYPE=${BUILD_TYPE:-Debug}

# Git Bash mangles absolute paths in docker arguments unless this is set.
export MSYS_NO_PATHCONV=1

repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
# docker needs the Windows form of the path on this host; pwd -W provides it
# under Git Bash and fails harmlessly elsewhere.
repo_mount=$(cd "$repo_root" && { pwd -W 2>/dev/null || pwd; })

command=${1:-test}
shift || true

if [ "$command" = "clean" ]; then
    docker volume rm -f "$WORK_VOLUME" "$CCACHE_VOLUME" >/dev/null 2>&1 || true
    echo "removed the $WORK_VOLUME and $CCACHE_VOLUME volumes"
    exit 0
fi

record=0
args=()
for arg in "$@"; do
    if [ "$arg" = "--record" ]; then
        record=1
    else
        args+=("$arg")
    fi
done

# Only the build inputs are synced. Everything else in the checkout (the
# 2.7 GB of build trees, lib/, bin/, .git) never crosses the mount.
sync_in='rsync -a --delete --exclude=.git \
    /repo/src /repo/tests /repo/third_party /repo/data \
    /repo/CMakeLists.txt /repo/Makefile /work/'

# Copy generated test data back so a re-record shows up as a normal diff.
# --checksum because the container clock and the mount can disagree on mtime.
sync_out='rsync -a --checksum \
    /work/tests/golden /work/tests/generated /work/tests/wire-layout.txt \
    /repo/tests/'

configure='cmake -G Ninja -B /work/build -S /work \
    -DCMAKE_BUILD_TYPE='"$BUILD_TYPE"' \
    -DDARKEDEN_BUILD_TESTS=ON \
    -DCMAKE_C_COMPILER_LAUNCHER=ccache \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache'

case "$command" in
    test)
        target=${args[0]:-wire_tests}
        recorder=""
        [ "$record" = "1" ] && recorder='echo "--- recording goldens"; (cd /work/build && UPDATE_GOLDENS=1 /work/bin/wire_tests >/dev/null);'
        script="$sync_in && $configure >/dev/null && cmake --build /work/build --target $target -j\$(nproc) && $recorder (cd /work/build && ctest --output-on-failure); rc=\$?; $sync_out; exit \$rc"
        ;;
    build)
        target=${args[0]:-}
        target_arg=""
        [ -n "$target" ] && target_arg="--target $target"
        script="$sync_in && $configure >/dev/null && cmake --build /work/build $target_arg -j\$(nproc)"
        ;;
    shell)
        script="$sync_in; echo 'workspace is /work (sources synced from the mounted checkout)'; exec bash"
        ;;
    *)
        echo "usage: tools/devbuild.sh {test|build|shell|clean} [target] [--record]" >&2
        exit 2
        ;;
esac

# A TTY is only available (and only wanted) when a human is driving; asking
# for one from a script or CI makes docker refuse to start the container.
tty_args=()
if [ -t 0 ] && [ -t 1 ]; then
    tty_args=(-it)
elif [ "$command" = "shell" ]; then
    echo "devbuild.sh shell needs a terminal" >&2
    exit 2
fi

# The checkout stays read-only except tests/, the one place sync_out writes:
# a nested rw mount over the ro one. Without it a --record run's rsync back
# fails on the read-only filesystem — and silently, since exit $rc reports
# ctest's status, not the copy's.
exec docker run --rm "${tty_args[@]}" \
    -v "$repo_mount:/repo:ro" \
    -v "$repo_mount/tests:/repo/tests" \
    -v "$WORK_VOLUME:/work" \
    -v "$CCACHE_VOLUME:/ccache" \
    -e CCACHE_DIR=/ccache \
    -w /work \
    "$IMAGE" bash -c "$script"
