#!/usr/bin/env bash
# Shrink-only ratchets for docs/RESTRUCTURING.md, plus generated-file pins.
#
# Each ratchet compares a measured count against the baseline recorded here.
#   measured > baseline  -> FAIL: new debt of a kind being paid down.
#   measured < baseline  -> FAIL: progress! tighten the baseline below (and
#                           the table in docs/RESTRUCTURING.md) in the same
#                           commit, so the number stays honest.
# Never raise a baseline.
#
# Run from the repository root (ctest does this via WORKING_DIRECTORY).
set -uo pipefail

fail=0

check_ratchet() {
    local id="$1" desc="$2" baseline="$3" measured="$4"
    # A non-numeric measurement (e.g. `wc -l < missing-file` yields "")
    # would make both [ -gt ] and [ -lt ] fail and fall through to [OK] —
    # a ratchet that silently passes when its subject disappears.
    if ! [[ "$measured" =~ ^[0-9]+$ ]]; then
        echo "[FAIL] $id $desc: measurement produced '$measured' (not a number — file missing or renamed?)"
        fail=1
    elif [ "$measured" -gt "$baseline" ]; then
        echo "[FAIL] $id $desc: measured $measured > baseline $baseline (new debt — remove it instead)"
        fail=1
    elif [ "$measured" -lt "$baseline" ]; then
        echo "[FAIL] $id $desc: measured $measured < baseline $baseline (progress — tighten the baseline in tests/ratchet/ratchets.sh and docs/RESTRUCTURING.md in this commit)"
        fail=1
    else
        echo "[OK]   $id $desc: $measured"
    fi
}

# --- R1: g_p* global-singleton extern declarations -------------------------
R1=$(grep -rE '^extern .*\* g_p' src --include='*.h' --include='*.cpp' | wc -l)
check_ratchet R1 "global singleton externs" 351 "$R1"

# --- R2: files with inline SQL in the gameserver root ----------------------
R2=$(grep -lE 'executeQuery' src/server/gameserver/*.cpp src/server/gameserver/*.h 2>/dev/null | wc -l)
check_ratchet R2 "gameserver-root files with inline SQL" 10 "$R2"

# --- R3: files with inline SQL outside database/ and repository/ -----------
# gameserver/repository/ joined the exclusion 2026-09-01 (baseline 317→314:
# two files cleansed, one pilot impl no longer counted). The original note
# said the impl files stay counted, but that only worked while an extraction
# cleansed at least as many files as it created: the PlayerCreature round
# (4 tables, 2 files) would have RAISED a shrink-only ratchet. Repository
# impls are the sanctioned quarantine for SQL — R3 measures SQL loose in
# game logic, and still counts loginserver/sharedserver and the dead
# theoneserver tree. (Trailing slash: only the directory is excluded.)
R3=$(grep -rlE 'executeQuery' src --include='*.cpp' | grep -v 'server/database' |
    grep -v 'server/gameserver/repository/' | wc -l)
check_ratchet R3 "files with inline SQL outside database/, repository/" 211 "$R3"

# --- R4: packet headers still carrying execute() on the packet -------------
R4=$(grep -rlE 'void execute\(Player' src/Core --include='*.h' | wc -l)
check_ratchet R4 "packet headers with execute()" 0 "$R4"

# --- R5: __BEGIN_TRY control-flow macro sites in gameserver ----------------
# handler/ and packetfill/ are excluded: those sources moved there from
# src/Core in task 2.4, where this metric never counted them — including
# them would jump the baseline without any new debt. Fold them in (with a
# re-baseline note) when they become de-core extraction targets in 3.x.
R5=$(grep -rE '__BEGIN_TRY' src/server/gameserver --include='*.cpp' | grep -vE 'gameserver/(handler|packetfill)/' | wc -l)
check_ratchet R5 "__BEGIN_TRY sites in gameserver" 5899 "$R5"

# --- R6: god-file line counts (task 3.3 files only, so far) -----------------
# Formula extraction to de-core (src/domain) shrinks these; each delegation
# that moves math out must tighten the number here. The doc's other god
# files join when their own extractions start. R6a-c baselines measured
# 2026-08-31 post-3.3-extraction; R6d added 2026-09-01 with the
# SkillFormula.cpp computeOutput extraction (the doc's 08-29 numbers
# predate the clang-format-18 pass and are superseded).
R6a=$(wc -l < src/server/gameserver/skill/SkillUtil.cpp 2>/dev/null || echo missing)
check_ratchet R6a "SkillUtil.cpp lines" 6745 "$R6a"
R6b=$(wc -l < src/server/gameserver/InitAllStat.cpp 2>/dev/null || echo missing)
check_ratchet R6b "InitAllStat.cpp lines" 4803 "$R6b"
R6c=$(wc -l < src/server/gameserver/skill/HitRoll.cpp 2>/dev/null || echo missing)
check_ratchet R6c "HitRoll.cpp lines" 774 "$R6c"
R6d=$(wc -l < src/server/gameserver/skill/SkillFormula.cpp 2>/dev/null || echo missing)
check_ratchet R6d "SkillFormula.cpp lines" 820 "$R6d"

# --- Generated factory list is fresh ---------------------------------------
# The generator only writes to $OUT, so point it at a scratch copy of the
# tree's file rather than overwriting the tracked one: an interrupt (Ctrl-C,
# ctest timeout) between generate and restore used to leave the committed
# file replaced or truncated.
INC=tests/generated/AllPacketFactories.inc
scratch_dir=$(mktemp -d)
trap 'rm -rf "$scratch_dir"' EXIT
mkdir -p "$scratch_dir/tests/generated" "$scratch_dir/tests/tools" \
         "$scratch_dir/tests/arch" "$scratch_dir/src"
cp -r src/Core "$scratch_dir/src/" 2>/dev/null
# The generator reads the kernel membership list (the compiled packet set
# since the 2.4 flip), not the CMake source lists it used to parse.
cp tests/arch/kernel_files.txt "$scratch_dir/tests/arch/"
cp tests/tools/gen_factory_list.sh "$scratch_dir/tests/tools/"
if (cd "$scratch_dir" && bash tests/tools/gen_factory_list.sh > /dev/null 2>&1) &&
   diff -q "$INC" "$scratch_dir/$INC" > /dev/null 2>&1; then
    echo "[OK]   AllPacketFactories.inc matches a fresh generation"
else
    echo "[FAIL] AllPacketFactories.inc is stale — run tests/tools/gen_factory_list.sh and commit the result"
    fail=1
fi

# --- Every server-side factory the manager registers is in the inventory ---
# Registrations deliberately outside the inventory are listed (with reasons)
# in tests/ratchet/factory_exceptions.txt — currently empty since the
# __GAME_CLIENT__ relic registrations were deleted in 2.4. The file must
# still exist: with zero entries nothing exercises this plumbing, so a
# deleted or mistyped path would otherwise pass silently. The `|| true`
# keeps the no-match grep exit (1) from mattering if this script ever
# adopts `set -e` like its siblings.
if [ ! -f tests/ratchet/factory_exceptions.txt ]; then
    echo "[FAIL] tests/ratchet/factory_exceptions.txt is missing"
    fail=1
fi
registered=$(mktemp)
inventory=$(mktemp)
sed -n '/void PacketFactoryManager::init/,/^}/p' src/Core/PacketFactoryManager.cpp |
    grep -oE 'addFactory\(new [A-Za-z0-9_]+' | sed 's/addFactory(new //' | sort -u > "$registered"
{
    grep -oE 'new [A-Za-z0-9_]+Factory' tests/generated/AllPacketFactories.inc | sed 's/new //'
    grep -vE '^\s*(#|$)' tests/ratchet/factory_exceptions.txt || true
} | sort -u > "$inventory"
missing=$(comm -23 "$registered" "$inventory")
rm -f "$registered" "$inventory"
if [ -n "$missing" ]; then
    echo "[FAIL] factories registered in PacketFactoryManager but missing from the wire inventory:"
    echo "$missing" | sed 's/^/         /'
    echo "         (add the packet sources to tests/arch/kernel_files.txt and regenerate, or"
    echo "          justify an entry in tests/ratchet/factory_exceptions.txt)"
    fail=1
else
    echo "[OK]   every registered factory is covered by the wire inventory"
fi

# --- Every encrypter-using packet has per-code goldens -------------------
# A packet whose read/write call readEncrypt/writeEncrypt puts bytes on the
# wire that depend on the session encrypt code, and for the shuffled ones
# on which code % N case runs. packet_encrypter_test.cpp pins them all;
# this catches the next packet that starts using the encrypter without a
# golden (exceptions, with reasons: tests/ratchet/encrypter_exceptions.txt).
# Checks for code 5 specifically: that is the code that reaches the
# SHUFFLE_STATEMENT_5 case 0 order through the encrypted branch.
unpinned=$(grep -lE '(read|write)Encrypt\(' src/Core/*.cpp |
    xargs -n1 basename | sed 's/\.cpp$//' | sort -u |
    grep -vxFf <(grep -vE '^\s*(#|$)' tests/ratchet/encrypter_exceptions.txt) |
    while read -r name; do
        [ -f "tests/golden/$name.code5.hex" ] || echo "$name"
    done)
if [ -n "$unpinned" ]; then
    echo "[FAIL] packets use the encrypter but have no per-code goldens (tests/golden/<Name>.code5.hex):"
    echo "$unpinned" | sed 's/^/         /'
    echo "         (add fill()/expectEqual() + ENCRYPTER_PACKET_TESTS in tests/packet_encrypter_test.cpp and record)"
    fail=1
else
    echo "[OK]   every encrypter-using packet has per-code goldens"
fi

exit $fail
