#!/usr/bin/env bash
# mayhem/build.sh — headless fceux core (no Qt/SDL GUI; upstream's CMake build requires the Qt
# frontend) + libFuzzer harnesses + the known-answer self-test. Runs in /mayhem as `mayhem`.
# Re-runnable and offline: only the in-image toolchain and zlib (apt) are used; make is incremental.
set -euo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${STANDALONE_FUZZ_MAIN:=/opt/mayhem/StandaloneFuzzTargetMain.c}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
export CC CXX

SRC="${SRC:-/mayhem}"
M="$SRC/mayhem"
cd "$M"

# 1) The emulation core, instrumented: libfceuxcore.a, libgwavi.a (AVI RIFF reader), driver_stub.o.
# SanitizerCoverage on the project and harness objects so libFuzzer/Mayhem get edge feedback.
FUZZ_FLAGS="-O1 -fsanitize=fuzzer-no-link $SANITIZER_FLAGS $DEBUG_FLAGS"
# UBSan's array-bounds check is relaxed for the core only: fceux maps memory through deliberately
# biased base pointers (cart.cpp ResetCartMapping: Page[x] = nothing - x*2048, later indexed with
# the full CPU address), which -fsanitize=array-bounds reports on every ROM load. Real out-of-bounds
# accesses through those pointers are still caught by ASan; the rest of UBSan stays on and halting.
make -j"$MAYHEM_JOBS" BUILD=build/fuzz FLAGS="$FUZZ_FLAGS -fno-sanitize=array-bounds"

INC="-I$SRC/src -I$SRC/src/drivers -DFCEUDEF_DEBUGGER -DHAVE_ASPRINTF -DPSS_STYLE=1"
B=build/fuzz
$CXX $SANITIZER_FLAGS $DEBUG_FLAGS -c lsan_off.cc -o $B/lsan_off.o
$CC $SANITIZER_FLAGS $DEBUG_FLAGS -c "$STANDALONE_FUZZ_MAIN" -o $B/standalone_main.o
CORE_LIBS="$B/driver_stub.o $B/libfceuxcore.a -lz -lpthread"

link() {  # link <name> <source> <extra defs> <libs...>
  local name=$1 src=$2 defs=$3; shift 3
  $CXX $FUZZ_FLAGS -std=gnu++17 -w $INC $defs -c "harnesses/$src" -o "$B/$name.o"
  $CXX $FUZZ_FLAGS $LIB_FUZZING_ENGINE "$B/$name.o" $B/lsan_off.o "$@" -o "$SRC/$name"
  $CXX $FUZZ_FLAGS "$B/$name.o" $B/standalone_main.o $B/lsan_off.o "$@" -o "$SRC/$name-standalone"
}
link fuzz_ines      fuzz_rom.cpp       '-DROM_EXT=".nes"' $CORE_LIBS
link fuzz_unif      fuzz_rom.cpp       '-DROM_EXT=".unf"' $CORE_LIBS
link fuzz_nsf       fuzz_rom.cpp       '-DROM_EXT=".nsf"' $CORE_LIBS
link fuzz_fds       fuzz_rom.cpp       '-DROM_EXT=".fds"' $CORE_LIBS
link fuzz_patch     fuzz_patch.cpp     '' $CORE_LIBS
link fuzz_savestate fuzz_savestate.cpp '' $CORE_LIBS
link fuzz_fm2       fuzz_fm2.cpp       '' $CORE_LIBS
link fuzz_fcm       fuzz_fcm.cpp       '' $CORE_LIBS
link fuzz_cheat     fuzz_cheat.cpp     '' $CORE_LIBS
link fuzz_ld65dbg   fuzz_ld65dbg.cpp   '' $CORE_LIBS
link fuzz_nl        fuzz_nl.cpp        '' $CORE_LIBS
link fuzz_netplay   fuzz_netplay.cpp   '' $CORE_LIBS
link fuzz_avi       fuzz_avi.cpp       '' $B/libgwavi.a

# 2) The known-answer self-test with the project's normal flags (no sanitizers) for test.sh.
T=build/test
make -j"$MAYHEM_JOBS" BUILD=$T FLAGS="-O2 $COVERAGE_FLAGS"
$CXX -O2 $COVERAGE_FLAGS -std=gnu++17 -w $INC harnesses/fceux_selftest.cpp $T/driver_stub.o $T/libfceuxcore.a \
  -lz -lpthread -o "$M/build/fceux_selftest"
