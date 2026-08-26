#!/bin/bash
#
# Build liboqs.a for the ASU firmware (asufw) on rv32imac.
#
#   bash build_liboqs.sh [-c <cross-prefix>] [-o <output>] [-w <workdir>]
#                        [-C <contract-dir>] [-h]
#
#   This file is mode 644 by design, so invoke it through bash; ./build_liboqs.sh
#   gives Permission denied.
#
#   -c  toolchain prefix, e.g. riscv64-unknown-elf (Vitis), riscv-none-elf
#       (xPack) or riscv32-unknown-elf. Default: $CROSS, else the first of
#       those three found on PATH, searched in that order.
#   -o  where to write the archive. Default: ./liboqs.a
#   -w  scratch directory. Default: a fresh mktemp dir, removed on success.
#   -C  directory holding oqs_mldsa_internal.h and oqs_mlkem_internal.h, which
#       the build is checked against. Default: this script's own directory.
#   -h  print this header and exit.
#
# Clones the pinned liboqs release (LIBOQS_TAG below), re-enables XKCP's Keccak
# unrolling knob that liboqs' vendored copy disabled, and builds the result for
# bare-metal rv32. See README.liboqs.md for why each piece is here.
#
set -euo pipefail

LIBOQS_TAG=0.16.0
LIBOQS_COMMIT=5a1a854b0dc9f2141bdc771c555ee60c37950183
PARAM_SETS="KEM_ml_kem_512;KEM_ml_kem_768;KEM_ml_kem_1024;SIG_ml_dsa_44;SIG_ml_dsa_65;SIG_ml_dsa_87"

OUT=$PWD/liboqs.a
WORK=
CROSS=${CROSS:-}
CONTRACT_DIR=$(cd "$(dirname "$0")" && pwd)

while getopts "c:o:w:C:h" opt; do
    case $opt in
        c) CROSS=$OPTARG ;;
        o) OUT=$(readlink -f "$OPTARG") ;;
        w) WORK=$(readlink -f "$OPTARG") ;;
        C) CONTRACT_DIR=$(readlink -f "$OPTARG") ;;
        # Print the header block by following it, not by line number: a fixed
        # range silently truncates the moment a line is added above.
        h) awk 'NR>1 { if (!/^#/) exit; sub(/^# ?/, ""); print }' "$0"; exit 0 ;;
        *) exit 2 ;;
    esac
done

die () { echo "error: $*" >&2; exit 1; }
say () { echo "==> $*"; }

# ---------------------------------------------------------------- toolchain
if [ -z "$CROSS" ]; then
    for p in riscv64-unknown-elf riscv-none-elf riscv32-unknown-elf; do
        command -v "$p-gcc" >/dev/null 2>&1 && { CROSS=$p; break; }
    done
fi
[ -n "$CROSS" ] || die "no RISC-V toolchain found; pass -c <prefix> or set \$CROSS"
for t in gcc ar ranlib nm; do
    command -v "$CROSS-$t" >/dev/null 2>&1 || die "$CROSS-$t not found on PATH"
done
command -v cmake >/dev/null 2>&1 || die "cmake not found"
say "toolchain: $($CROSS-gcc -dumpversion) ($CROSS)"

if [ -z "$WORK" ]; then
    WORK=$(mktemp -d); TEMPWORK=1
else
    mkdir -p "$WORK"; TEMPWORK=0
fi
say "workdir:   $WORK"

# ------------------------------------------------------------------- source
say "cloning liboqs $LIBOQS_TAG"
rm -rf "$WORK/src"
git clone -q --branch "$LIBOQS_TAG" --depth 1 \
    https://github.com/open-quantum-safe/liboqs.git "$WORK/src"
[ "$(git -C "$WORK/src" rev-parse HEAD)" = "$LIBOQS_COMMIT" ] \
    || die "unexpected commit; expected $LIBOQS_COMMIT"

# ---------------------------------------------- roll the Keccak permutation
# liboqs ships XKCP's "opt64" Keccak-p[1600] with '#define FullUnrolling'
# hardcoded ahead of the unrolling macros, expanding all 24 rounds inline.
# That does not link: it overruns the 232,704-byte resident region by 56,768
# bytes. No -D can override it either, because the macros test
# defined(FullUnrolling) before their 'Unrolling == N' chain, and the #define
# sits after anything the command line could inject.
#
# XKCP itself does not hardcode this. It selects with
#   #ifdef KeccakP1600_..._fullUnrolling  -> #define FullUnrolling
#   #else                                 -> #define Unrolling KeccakP1600_..._unrolling
# and declares the loop counter under the matching #ifndef at each site that
# needs one. liboqs' vendored copy flattened both away, which is why
# KeccakP-1600-opt64-config.h still defines a KeccakP1600_fullUnrolling that
# nothing in the tree reads.
#
# The two edits below restore that dispatch rather than customise it, and
# bring the permutation's resident cost to 3,702 bytes. The four sites needing
# a counter are Permute_24rounds, Permute_12rounds and the two
# FastLoop_Absorb variants; Permute_Nrounds already declares one. The real fix
# belongs upstream in liboqs: see README.liboqs.md section 3.
say "rolling Keccak permutation to one round per iteration"
KSRC=$WORK/src/src/common/sha3/xkcp_low/KeccakP-1600/plain-64bits/KeccakP-1600-opt64.c
grep -qx '#define FullUnrolling' "$KSRC" \
    || die "no '#define FullUnrolling' in $KSRC; upstream layout changed"
sed -i 's/^#define FullUnrolling$/#define Unrolling 1/' "$KSRC"

awk '
    /declareABCDE/               { print; pending = 1; next }
    pending && /unsigned int i;/ { pending = 0; print; next }
    pending                      { print "\tunsigned int i;"; added++
                                   pending = 0; print; next }
                                 { print }
    END                          { exit (added == 4 ? 0 : 3) }
' "$KSRC" > "$KSRC.asu" \
    || die "expected 4 loop counters to declare in $KSRC; upstream layout changed"
mv "$KSRC.asu" "$KSRC"

# -------------------------------------------------- file the ASU build adds
# A bare-metal fprintf. liboqs' OQS_EXIT_IF_NULLPTR guard in common.h calls
# fprintf(stderr, ...) and xkcp_sha3.c uses it, so the symbol is reachable.
# Defining it here means newlib's fprintf member is never extracted from libc.a,
# and neither is the vfprintf + _dtoa_r machinery behind it: 46,560 bytes of
# instruction RAM that asufw does not have. There is no stderr on this target,
# so discarding the message matches what the hardware can do.
#
# This stub only silences the message. The guard then calls exit(EXIT_FAILURE),
# and both exit and _exit are linked into asufw, so it is not recoverable. What
# it guards is OQS_MEM_aligned_alloc(32, 224) for one incremental SHA3/SHAKE
# context, taken from the firmware heap, on paths both ML-DSA and ML-KEM use.
# A heap too small to serve it therefore stops the firmware instead of failing
# the command. Unresolved; the heap it draws on is measured and sized in the
# comment above _HEAP_SIZE in lscript.ld.
cat > "$WORK/asu_fprintf.c" <<'EOF'
#include <stdio.h>

int fprintf(FILE *__restrict stream, const char *__restrict format, ...) {
	(void)stream;
	(void)format;
	return 0;
}
EOF

# ---------------------------------------------------------------- configure
say "configuring"
# MLK/MLD_CONFIG_SERIAL_FIPS202_ONLY drop the 4-way SHAKE backend from
# mlkem-native and mldsa-native. Doing so leaves mlkem-native's glue referring
# to a shake128ctx that only that backend defines. Both of the names it needs
# are plain substitutions, so they can be supplied as -D rather than a
# force-included shim header.
#
# MLD_CONFIG_REDUCE_RAM materialises ML-DSA's matrix A lazily: ~56 KB less
# stack. Do not drop it; the instruction-RAM region has no room for that.
#
# MLD_CONFIG_CONTEXT_PARAMETER must stay ABSENT. mldsa-native builds every
# prototype and call site through MLD_CONTEXT_PARAMETERS_N (common.h), which
# drops the trailing context argument when the macro is undefined. Defining it
# adds that argument without changing any symbol name, so the prototypes in
# oqs_mldsa_internal.h (see its header comment) would still link and then pass
# arguments the callee does not expect. Note it must be paired with
# MLD_CONFIG_CONTEXT_PARAMETER_TYPE; mldsa-native #errors on one without the
# other, so only defining both produces the silent mismatch.
ASU_FLAGS="-march=rv32imac_zicsr_zifencei -mabi=ilp32 -ffreestanding \
-DMLK_CONFIG_SERIAL_FIPS202_ONLY -DMLD_CONFIG_SERIAL_FIPS202_ONLY -DMLD_CONFIG_REDUCE_RAM \
-Dshake128ctx=OQS_SHA3_shake128_inc_ctx -Dshake128_release=shake128_inc_ctx_release"

rm -rf "$WORK/build"
# Two of the flags below are not self-evident:
#   CMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY - CMake's compiler probe links
#     a test executable by default, which cannot be done for a bare-metal
#     target with no startup files or linker script. Every probe would fail and
#     configure would abort before reaching liboqs. Building the probe as a
#     static library stops at the archive step, which does work here.
#   OQS_EMBEDDED_BUILD=ON - "Embedded environment without a full standard
#     library". It leaves OQS_USE_PTHREADS off even when CMake finds pthreads,
#     open-codes OQS_MEM_strdup with malloc/memcpy instead of strdup, and
#     replaces the OS entropy path with an OQS_randombytes_system that refuses
#     to run. Anything calling OQS_randombytes must therefore install a
#     generator via OQS_randombytes_custom_algorithm() first; asufw drives the
#     ML-DSA/ML-KEM internals directly and supplies its own randomness, so it
#     never reaches that path.
cmake -S "$WORK/src" -B "$WORK/build" -G "Unix Makefiles" \
    -DCMAKE_SYSTEM_NAME=Generic \
    -DCMAKE_SYSTEM_PROCESSOR=riscv \
    -DCMAKE_C_COMPILER="$CROSS-gcc" \
    -DCMAKE_AR="$(command -v "$CROSS-ar")" \
    -DCMAKE_RANLIB="$(command -v "$CROSS-ranlib")" \
    -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS="$ASU_FLAGS" \
    -DCMAKE_C_FLAGS_RELEASE="-O2 -flto -ffat-lto-objects -DNDEBUG" \
    -DBUILD_SHARED_LIBS=OFF \
    -DOQS_BUILD_ONLY_LIB=ON \
    -DOQS_DIST_BUILD=OFF \
    -DOQS_USE_OPENSSL=OFF \
    -DOQS_EMBEDDED_BUILD=ON \
    -DOQS_MINIMAL_BUILD="$PARAM_SETS" \
    > "$WORK/configure.log" 2>&1 || { tail -30 "$WORK/configure.log"; die "configure failed"; }

# --------------------------------------------- verify the firmware contract
# oqs_mldsa_internal.h and oqs_mlkem_internal.h declare the native entry points
# asufw calls directly, and those declarations hold only for this exact
# configuration. Both backends build every prototype through
# MLD/MLK_CONTEXT_PARAMETERS_N, which appends a trailing context argument when
# MLD/MLK_CONFIG_CONTEXT_PARAMETER is defined WITHOUT changing any symbol name,
# so a drop that defines it still links and then passes arguments the callee
# does not expect.
#
# The finished archive cannot be checked for this: it carries no DWARF, only
# fat-LTO GIMPLE, and a C symbol name encodes no argument list. The source can
# be. Each probe below puts the upstream declaration and the firmware contract
# in one translation unit, where C makes a disagreeing pair a hard error
# ('conflicting types for ...'). That tests the declaration the build actually
# produced rather than the flags that produced it, so it still fires if
# upstream renames the macro or flips its default. Reading the flags back out
# of CMake's own flags.make, instead of restating them, is part of the same
# idea.
#
# The probes also re-derive the constants both headers restate: a silently
# renumbered MLD_PREHASH_* would pick a different hash algorithm and produce
# signatures that verify nowhere, which is worse than an argument mismatch.
say "verifying the firmware contract"
for h in oqs_mldsa_internal.h oqs_mlkem_internal.h; do
    [ -f "$CONTRACT_DIR/$h" ] || die "$h not found in $CONTRACT_DIR; pass -C <dir>"
done

# Capture each shared constant into an enum, then #undef the name so the
# contract header can define its own without colliding, then compare.
gen_capture () {
    echo "enum {"
    for m in "$@"; do echo "	up_$m = $m,"; done
    echo "};"
    for m in "$@"; do echo "#undef $m"; done
}
gen_assert () {
    for m in "$@"; do
        echo "_Static_assert($m == up_$m, \"$m disagrees with liboqs as configured\");"
    done
}

run_probe () {
    local flags=$1 src=$2 args
    [ -f "$flags" ] || die "flags.make missing: $flags (was -G 'Unix Makefiles' used?)"
    # These lines are shell words as make would hand them to the compiler,
    # escaped quotes around MLD/MLK_CONFIG_FILE included, so let the shell
    # split them exactly the same way.
    eval "args=($(sed -n 's/^C_FLAGS = //p; s/^C_DEFINES = //p; s/^C_INCLUDES = //p' "$flags"))"
    "$CROSS-gcc" "${args[@]}" -I"$CONTRACT_DIR" -fsyntax-only "$src" \
        || die "$(basename "$src" .c): the contract headers do not match this build"
}

MLD_SHARED="MLDSA_SEEDBYTES MLDSA_RNDBYTES MLDSA_CRHBYTES
MLD_DOMAIN_SEPARATION_MAX_BYTES
MLD_PREHASH_NONE MLD_PREHASH_SHA2_224 MLD_PREHASH_SHA2_256 MLD_PREHASH_SHA2_384
MLD_PREHASH_SHA2_512 MLD_PREHASH_SHA2_512_224 MLD_PREHASH_SHA2_512_256
MLD_PREHASH_SHA3_224 MLD_PREHASH_SHA3_256 MLD_PREHASH_SHA3_384
MLD_PREHASH_SHA3_512 MLD_PREHASH_SHAKE_128 MLD_PREHASH_SHAKE_256"

# shellcheck disable=SC2086
for s in 44 65 87; do
    P=$WORK/contract_mldsa_$s.c
    {
        echo '#include "mldsa/src/sign.h"'
        gen_capture $MLD_SHARED
        echo '#include "oqs_mldsa_internal.h"'
        gen_assert $MLD_SHARED
    } > "$P"
    run_probe "$WORK/build/src/sig/ml_dsa/CMakeFiles/ml_dsa_${s}_ref.dir/flags.make" "$P"
done

# mlkem-native states the two seed lengths as multiples of MLKEM_SYMBYTES, so
# those two are compared against the expression rather than a like-named macro.
for s in 512 768 1024; do
    P=$WORK/contract_mlkem_$s.c
    {
        echo '#include "mlkem/src/kem.h"'
        gen_capture MLKEM_SSBYTES MLKEM_SYMBYTES
        echo '#include "oqs_mlkem_internal.h"'
        gen_assert MLKEM_SSBYTES
        echo '_Static_assert(MLKEM_KEYPAIR_SEEDBYTES == 2 * up_MLKEM_SYMBYTES,'
        echo '	"MLKEM_KEYPAIR_SEEDBYTES disagrees with liboqs as configured");'
        echo '_Static_assert(MLKEM_ENCAPS_SEEDBYTES == up_MLKEM_SYMBYTES,'
        echo '	"MLKEM_ENCAPS_SEEDBYTES disagrees with liboqs as configured");'
    } > "$P"
    run_probe "$WORK/build/src/kem/ml_kem/CMakeFiles/ml_kem_${s}_ref.dir/flags.make" "$P"
done

say "building"
cmake --build "$WORK/build" -j"$(nproc)" > "$WORK/build.log" 2>&1 \
    || { tail -40 "$WORK/build.log"; die "build failed"; }

A=$WORK/build/lib/liboqs.a
[ -f "$A" ] || die "no archive produced"

# ----------------------------------------------------- add the fprintf stub
# Reuse the exact flags CMake used, so the added member matches the rest of the
# archive (same -O2 -flto -ffat-lto-objects). Recovering them from flags.make is
# the reason this script asks CMake for -G "Unix Makefiles".
say "adding bare-metal fprintf"
KD=$WORK/build/src/common/sha3/xkcp_low/CMakeFiles/xkcp_low_keccakp_1600_plain64.dir
[ -f "$KD/flags.make" ] || die "flags.make missing (was -G 'Unix Makefiles' used?)"
KFLAGS=$(sed -n 's/^C_FLAGS = //p' "$KD/flags.make" | head -1)

# 'ar r' APPENDS when the member name is not already present, which is what is
# wanted here, so the count must grow by exactly one.
BEFORE=$($CROSS-ar t "$A" | wc -l)
# shellcheck disable=SC2086
$CROSS-gcc $KFLAGS -c "$WORK/asu_fprintf.c" -o "$WORK/asu_fprintf.c.obj"
(cd "$WORK" && $CROSS-ar r "$A" asu_fprintf.c.obj)
$CROSS-ranlib "$A"
MEMBERS=$($CROSS-ar t "$A" | wc -l)
[ "$MEMBERS" = "$((BEFORE + 1))" ] || die "unexpected member count $MEMBERS"

# ------------------------------------------------------------------- checks
# Dump the symbol table once. Do not pipe nm straight into 'grep -q': grep
# exits on the first match, nm takes SIGPIPE, and 'set -o pipefail' then
# reports the whole pipeline as failed.
$CROSS-nm "$A" > "$WORK/syms.txt" 2>/dev/null || true
[ -s "$WORK/syms.txt" ] || die "could not read symbols from $A"

# The archive must DEFINE fprintf. If it only references it, the linker will
# satisfy that from libc.a and drag vfprintf + _dtoa_r in with it. Checking for
# 'vfprintf' here would be useless: that symbol lives in libc, never in liboqs.
if [ "$(grep -c ' T fprintf$' "$WORK/syms.txt")" = 0 ]; then
    die "archive does not define fprintf; newlib's would be linked instead"
fi
if [ "$(grep -c ' T KeccakP1600_Permute_24rounds$' "$WORK/syms.txt")" = 0 ]; then
    die "Keccak entry point missing"
fi

mkdir -p "$(dirname "$OUT")"
cp "$A" "$OUT"
# 'NF>2' skips the 'member.c.obj:' header lines nm interleaves with the symbols.
$CROSS-nm --defined-only "$A" > "$WORK/defined.txt" 2>/dev/null || true
SYMS=$(awk 'NF>2{print $NF}' "$WORK/defined.txt" | sort -u | wc -l)

say "done"
echo
echo "    archive : $OUT"
echo "    size    : $(stat -c%s "$OUT") bytes"
echo "    members : $MEMBERS"
echo "    symbols : $SYMS defined"
echo
echo "    Install with:"
echo "      cp $OUT <embeddedsw>/lib/sw_services/xilasu/src/server/liboqs.a"

[ "$TEMPWORK" = 1 ] && rm -rf "$WORK"
exit 0
