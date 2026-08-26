# `liboqs.a` — provenance and rebuild

`liboqs.a` in this directory is a prebuilt static archive of
[liboqs](https://github.com/open-quantum-safe/liboqs) 0.16.0, cross-compiled for
rv32imac, providing ML-KEM (FIPS 203) and ML-DSA (FIPS 204) to the ASU firmware.
`CMakeLists.txt` copies it to `liboqs_mbv.a` and hard-fails at configure time if
it is missing.

**The checked-in archive is the output of `build_liboqs.sh` in this directory,
nothing else.** It was produced by running the command in
[§1](#1-rebuilding-it), with nothing edited, stripped or repacked afterwards, so
following this README reproduces what ships.

Rebuilds are reproducible in member list, symbol set and linked firmware, but not
byte for byte. Two consecutive runs of the same script, from the same commit,
with the same `-w`, produce archives of different size and checksum; changing
`-w` moves it further still, because the scratch path is embedded in the LTO
sections. Nothing that links changes.

No size or checksum is therefore recorded anywhere in this README. Any value
written down would be wrong after the next rebuild, and a differing one is not
evidence that the source, the configuration or the toolchain changed — it is the
expected result of building twice. Compare archives only by what is stable: the
member count, the symbol set, the contract check
([§2](#how-that-assumption-is-checked)) and the linked firmware
([§5.7](#57-check-and-install)). Never by `cmp`, `sha256sum` or file size.

---

## 1. Rebuilding it

From a clean shell, three steps and about 20 seconds:

```bash
# 1. put the RISC-V toolchain on PATH (alternatives under Toolchain, below)
source <vitis-install>/settings64.sh

# 2. go to this directory
cd <embeddedsw>/lib/sw_services/xilasu/src/server

# 3. build, writing the archive in place
bash build_liboqs.sh -o "$PWD/liboqs.a"
```

`<embeddedsw>` is the root of the checkout you are building for — the directory
holding `lib/`, `scripts/` and `XilinxProcessorIPLib/`. From anywhere inside one,
`git rev-parse --show-toplevel` prints it. `<vitis-install>` is the directory
holding `settings64.sh`.

Two things bite people here. Everything in this directory is mode 644, this
script included, so it has to be run through `bash`; `./build_liboqs.sh` gives
`Permission denied`. And skipping step 1 stops the build at once with `error: no
RISC-V toolchain found` — `-c` and `$CROSS` will not save you, they choose a
prefix rather than a location.

Step 3 writes straight over the checked-in `liboqs.a`, which is the point when
rebuilding it. Give `-o` a different path to leave the shipped one alone, then
copy it in when you are satisfied:

```bash
cp liboqs.a <embeddedsw>/lib/sw_services/xilasu/src/server/liboqs.a
```

`build_liboqs.sh` clones upstream at the pinned tag, re-enables XKCP's Keccak
unrolling knob in the clone — which liboqs' vendored copy disabled, see
[§3](#3-why-the-keccak-permutation-has-to-be-rolled) — builds it, and writes a
single `.a`. It also checks the build against the two contract headers next to
it before compiling anything ([§2](#how-that-assumption-is-checked)). Run
`bash build_liboqs.sh -h` for the options; the useful ones are `-c` to pick a
toolchain prefix, `-w` to keep the scratch tree for inspection, and `-C` to
locate the contract headers when running a copy of the script from elsewhere.

### Toolchain

Any RISC-V bare-metal GCC works. The script takes `-c <prefix>`, or looks for
`riscv64-unknown-elf`, `riscv-none-elf` and `riscv32-unknown-elf` on `PATH`, in
that order. The first two are verified to produce a 74-member archive with the
same 599 defined symbols:

| | |
|---|---|
| Vitis 2026.2 `riscv64-unknown-elf-gcc` 13.4.0 | used for the archive currently checked in |
| [xPack `riscv-none-elf-gcc`](https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases) 15.2.0 | public download, no AMD tooling needed |

Put the compiler on `PATH` before running the script. This is the step that is
easy to miss, and without it the command in [§1](#1-rebuilding-it) stops at
`error: no RISC-V toolchain found`. Either source the Vitis environment or add
the compiler's `bin` directory yourself:

```bash
source <vitis-install>/settings64.sh              # or
export PATH=<vitis-install>/gnu/riscv/lin/bin:$PATH
```

`-c <prefix>` and `$CROSS` only choose *which* prefix to use. The script still
resolves every tool with `command -v`, so neither is a substitute for having the
directory on `PATH`.

Also needs `cmake` >= 3.16, `make` and `git`, plus network access to github.com
for the shallow clone. OpenSSL is **not** required.

One caveat if you rebuild only part of the flow: fat-LTO IR is
compiler-version specific, so an archive built by GCC 15.2 cannot be linked
into a firmware built by GCC 13.4. Use one toolchain for both.

---

## 2. What the script changes, and why

Three things: one edit to the clone's Keccak permutation, one added C file, and
a handful of `-D` flags. Nothing else differs from a stock liboqs build. All
three are load-bearing:

| Change | Consequence of omitting it |
|---|---|
| roll the Keccak permutation ([§3](#3-why-the-keccak-permutation-has-to-be-rolled)) | does not link, 56,768 bytes over |
| add `asu_fprintf.c` | does not link, 46,560 bytes over |
| the two `shake128` defines | does not compile, `shake128ctx` undefined |

**`asu_fprintf.c`** defines `fprintf`. liboqs' `OQS_EXIT_IF_NULLPTR` guard in
`common.h` calls `fprintf(stderr, ...)`, and `xkcp_sha3.c` uses that guard, so
the symbol is genuinely reachable — this is not a debug path. Defining it in
the archive means newlib's `fprintf` is never extracted from `libc.a`, and
neither is the `vfprintf` and `_dtoa_r` machinery behind it. Leaving it to
newlib costs 46,560 bytes. There is no `stderr` on this target, so discarding
the message is the only available behaviour. Note that this stub only silences
the message — the guard still terminates the firmware.

**The two `shake128` defines** repair a gap left by
`MLK_CONFIG_SERIAL_FIPS202_ONLY`, which drops the 4-way SHAKE backend while
mlkem-native's glue still refers to a `shake128ctx` that only that backend
defines. Both names it needs are plain substitutions, so `-D` is enough and no
shim header is required:

```
-Dshake128ctx=OQS_SHA3_shake128_inc_ctx
-Dshake128_release=shake128_inc_ctx_release
```

Dropping `MLK_CONFIG_SERIAL_FIPS202_ONLY` altogether would remove the need for
any substitution, by keeping the 4-way backend that defines `shake128ctx`. That
trades instruction RAM for nothing on a core with no SIMD to exploit it, and
there is no reason to spend it: the two `-D` flags cost nothing.

The build sets three size-related macros. All are mlkem-native / mldsa-native
internals, for which liboqs exposes no equivalent option:

- `MLK_CONFIG_SERIAL_FIPS202_ONLY` and `MLD_CONFIG_SERIAL_FIPS202_ONLY` drop the
  4-way SHAKE backends, which rv32 has no SIMD to exploit.
- `MLD_CONFIG_REDUCE_RAM` materialises ML-DSA's matrix A lazily, saving about
  56 KB of stack. Treat this one as load-bearing rather than an optimisation:
  the instruction-RAM region has no room to give that back.

One flag matters by its **absence**. `MLD_CONFIG_CONTEXT_PARAMETER` must not be
defined. mldsa-native routes every prototype and call site through the
`MLD_CONTEXT_PARAMETERS_N` macros in its `common.h`, which drop the trailing
context argument when the macro is undefined:

```c
/* mldsa-native common.h */
#ifdef MLD_CONFIG_CONTEXT_PARAMETER
#define MLD_CONTEXT_PARAMETERS_3(arg0, arg1, arg2, context) (arg0, arg1, arg2, context)
#else
#define MLD_CONTEXT_PARAMETERS_3(arg0, arg1, arg2, context) (arg0, arg1, arg2)
#endif
```

So today `PQCP_MLDSA_NATIVE_MLDSA44_C_keypair_internal` takes three arguments,
which is what `oqs_mldsa_internal.h` declares — it declares these symbols for
the firmware, and says so in its header comment. Defining the flag adds a fourth
argument **without changing any symbol name**, so the firmware's prototypes
would still link and then pass arguments the callee does not expect. Nothing in
liboqs 0.16.0 sets this macro, in CMake or in the mldsa-native liboqs
integration headers, so omitting it from `ASU_FLAGS` is sufficient. It also has
to be paired with `MLD_CONFIG_CONTEXT_PARAMETER_TYPE` — mldsa-native `#error`s
on one without the other — so only defining both produces the silent mismatch.

### How that assumption is checked

A header comment cannot stop a wrong drop, and neither can the archive: it
carries no DWARF, only fat-LTO GIMPLE, and a C symbol name encodes no argument
list, so nothing in `liboqs.a` distinguishes three arguments from four after the
fact. mlkem-native has the same hazard under `MLK_CONFIG_CONTEXT_PARAMETER`,
which reaches the nine ML-KEM entry points in `oqs_mlkem_internal.h`.

The script therefore checks the source, in a `verifying the firmware contract`
stage that runs after configure and before the build. For each of the six
parameter sets it generates a translation unit holding the upstream declaration
and the firmware contract together, and compiles it with `-fsyntax-only`:

```c
#include "mldsa/src/sign.h"       /* what this build actually declares */
/* ...capture the shared constants, then #undef the names... */
#include "oqs_mldsa_internal.h"   /* what the firmware promises to call */
```

C makes a disagreeing pair a hard error, so a drop built with the context
parameter stops there rather than in a bad ELF:

```
oqs_mldsa_internal.h:99:5: error: conflicting types for
  'PQCP_MLDSA_NATIVE_MLDSA44_C_keypair_internal';
  have 'int(uint8_t *, uint8_t *, const uint8_t *)'
error: contract_mldsa_44: the contract headers do not match this build
```

Two properties are worth preserving if this is ever rewritten. It tests the
declaration the build produced rather than the flags that produced it, so it
still fires if upstream renames the macro or changes its default, where a `grep`
for the flag name would not. And its compiler flags are read back out of CMake's
generated `flags.make` instead of being restated, so the probe cannot drift from
the build it is checking.

The same pass re-derives the constants both contract headers restate — the
`MLD_PREHASH_*` numbering, the seed and digest lengths,
`MLD_DOMAIN_SEPARATION_MAX_BYTES`, and the ML-KEM seed lengths, which
mlkem-native states as multiples of `MLKEM_SYMBYTES` — as `_Static_assert`s. A
silently renumbered `MLD_PREHASH_*` would select a different hash algorithm and
produce signatures that verify nowhere, which is worse than an argument
mismatch:

```
contract_mldsa_44.c:55:1: error: static assertion failed:
  "MLD_PREHASH_SHAKE_256 disagrees with liboqs as configured"
```

The two headers are read from the script's own directory, so this normally needs
no argument. Pass `-C <dir>` when running a copy of the script from elsewhere;
the stage cannot be skipped, only pointed somewhere.

### Notes on liboqs' own build options

Checked against
[CONFIGURE.md](https://github.com/open-quantum-safe/liboqs/blob/main/CONFIGURE.md),
so nobody has to re-derive them:

- Overriding `CMAKE_C_FLAGS_RELEASE` is the documented way to change Release
  flags, not a workaround. Stock `Release` is `-O3 -DNDEBUG`; we deliberately
  use `-O2 -flto -ffat-lto-objects -DNDEBUG` instead.
- `OQS_USE_OPENSSL` and `OQS_DIST_BUILD` both default to **ON**. Leaving either
  at its default breaks this build rather than merely enlarging it.
- `OQS_OPT_TARGET` defaults to `auto`, which normally means `-march=native`.
  It is safe here only because `compiler_opts.cmake` demotes `auto` to
  `generic` when `CMAKE_CROSSCOMPILING` is set — which is one more reason
  `CMAKE_SYSTEM_NAME=Generic` is not optional.
- `CMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY` is what lets CMake configure at
  all. Its compiler probe links a test executable by default, which is
  impossible for a bare-metal target with no startup files or linker script, so
  every probe fails and configure aborts before it reaches liboqs. Building the
  probe as a static library stops at the archive step, which does work.
- `OQS_EMBEDDED_BUILD=ON` selects liboqs' "Embedded environment without a full
  standard library" path: `OQS_USE_PTHREADS` stays off even when CMake finds
  pthreads, `OQS_MEM_strdup` is open-coded with `malloc`/`memcpy`, and
  `OQS_randombytes_system` becomes a stub that refuses to run. Any caller of
  `OQS_randombytes` must therefore install a generator with
  `OQS_randombytes_custom_algorithm()` first; asufw drives the ML-DSA and ML-KEM
  internals directly and supplies its own randomness, so it never reaches that
  path.
- `OQS_MEMOPT_BUILD` sounds applicable but is not: in 0.16.0 it selects
  memory-optimised implementations for MQOM only. There are no ML-KEM or
  ML-DSA memopt variants, so it changes nothing here.
- `CMAKE_BUILD_TYPE=MinSizeRel` (`-Os -DNDEBUG`) is the supported lever if the
  instruction-RAM margin ever runs out. Not used today.

---

## 3. Why the Keccak permutation has to be rolled

liboqs ships XKCP's `opt64` Keccak-p[1600] fully unrolled, on the assumption of
a fast native 64-bit datapath. rv32 has no such datapath, so the unrolling buys
little and costs 56,768 bytes more resident instruction RAM than asufw has. This
code is resident rather than overlaid because SHA3 is called from both the ML-DSA
and ML-KEM overlay images, and `NOCROSSREFS` forbids one overlay from referencing
another, so it cannot live in either.

XKCP is configurable for exactly this, through an `Unrolling` level, but no flag
reaches it. The source hardcodes the choice one line before consuming it:

```c
/* src/common/sha3/xkcp_low/KeccakP-1600/plain-64bits/KeccakP-1600-opt64.c */
#include "KeccakP-1600-opt64-config.h"   /* line 30: defines KeccakP1600_fullUnrolling */
#include "KeccakP-1600-64.macros"        /* line 47 */
#define FullUnrolling                    /* line 48: unconditional */
#include "KeccakP-1600-unrolling.macros" /* line 49: tests defined(FullUnrolling) first */
```

`-DUnrolling=1` loses three ways. The macros file tests
`#if (defined(FullUnrolling))` ahead of its `#elif (Unrolling == N)` chain; the
`#define` on line 48 is unconditional and has no `#ifndef` guard, so it beats
anything the command line defines earlier; and shadowing the `.macros` file via
`-I` fails because it is a quoted include sitting in the same directory as the
`.c`, which a quoted include searches first.

### This is a liboqs vendoring defect, not a knob we are overriding

XKCP itself is configurable here, and dispatches on the config header:

```c
/* XKCP, lib/low/KeccakP-1600/plain-64bits/KeccakP-1600-opt64.c */
#include "KeccakP-1600-64.macros"
#ifdef KeccakP1600_plain64_fullUnrolling
#define FullUnrolling
#else
#define Unrolling KeccakP1600_plain64_unrolling
#endif
#include "KeccakP-1600-unrolling.macros"
```

It also declares the loop counter at every site the rolled path needs one,
under the matching guard:

```c
declareABCDE
#ifndef KeccakP1600_plain64_fullUnrolling
unsigned int i;
#endif
```

liboqs' vendored copy flattened both of those away: the `#ifdef` became an
unconditional `#define FullUnrolling`, and the guarded declarations were
dropped. The evidence is still sitting in the tree —
`KeccakP-1600-opt64-config.h` defines `KeccakP1600_fullUnrolling`, and nothing
anywhere in liboqs reads it, because the `#ifdef` that consumed it is gone. That
dead macro is the vestige of the dispatch.

So the two edits below are not a customisation of upstream behaviour; they
restore the behaviour XKCP already has, which liboqs' copy disabled:

1. `#define FullUnrolling` becomes `#define Unrolling 1`, standing in for the
   `#else` branch liboqs removed.
2. `unsigned int i;` is added after `declareABCDE` at the four sites that lack
   it — `Permute_24rounds`, `Permute_12rounds` and the two `FastLoop_Absorb`
   variants — matching XKCP's guarded declarations. `Permute_Nrounds` already
   has one, which is why only four sites need it.

The script asserts both — that the `#define` was found, and that exactly four
declarations were added — so an upstream layout change fails the build instead of
silently producing a fully unrolled archive.

The real fix belongs in liboqs: guard the `#define` with `#ifndef`, or restore
XKCP's `#ifdef` dispatch so that `KeccakP-1600-opt64-config.h` means something
again. Either would make this build need no source edit at all, and is worth
reporting upstream. Until then the edit is applied at build time, and no copy of
it is carried in this repository.

### What the unrolling levels cost

| Keccak configuration | resident cost | outcome |
|---|---|---|
| `FullUnrolling` (upstream default) | — | does not link, 56,768 bytes over |
| `Unrolling 1` (what ships) | 3,702 | links, 4,448 bytes free |
| `Unrolling 1`, that member at `-Os` | 2,716 | links, 5,434 bytes free |

The `-Os` row is available if the margin ever gets tight: it applies to that one
member only, and costs nothing elsewhere. It is not used today, to keep the whole
archive on one optimisation level.

### Do not size this with `size`

Every archive member is built `-flto -ffat-lto-objects`, so it carries both
machine code and LTO IR. The linker consumes the **IR** and regenerates code with
the firmware's link-time flags, discarding the machine code that `size` reports.
The Keccak member's `.text` is 49,732 bytes at `Unrolling 1` and 212,508 fully
unrolled, and neither predicts the 3,702 above. Only a firmware link measures
this.

---

## 4. Provenance

| Field | Value |
|---|---|
| Repository | `https://github.com/open-quantum-safe/liboqs.git` |
| Tag | `0.16.0` |
| Commit | `5a1a854b0dc9f2141bdc771c555ee60c37950183` |
| Upstream source modified? | One file, at build time only: `KeccakP-1600-opt64.c`, restoring the unrolling dispatch XKCP has and liboqs' vendored copy dropped ([§3](#3-why-the-keccak-permutation-has-to-be-rolled)) |
| ABI / arch | `-march=rv32imac_zicsr_zifencei -mabi=ilp32` |
| Optimisation | `-O2 -flto -ffat-lto-objects` |
| OpenSSL | not used, not linked |
| Parameter sets | ML-KEM-512/768/1024, ML-DSA-44/65/87 |

### The checked-in archive

| Field | Value |
|---|---|
| Produced by | `bash build_liboqs.sh -o liboqs.a`, written here directly |
| Built with | Vitis 2026.2 `riscv64-unknown-elf-gcc` 13.4.0 |
| Members | 74 |
| Defined symbols | 599 |
| ML-DSA / ML-KEM contract symbols | 18 of 18, 9 of 9 |
| Size, checksum | not recorded — differs on every rebuild, see the top of this file |

The three counts above are properties, not fingerprints: they say what the
archive provides, not which run produced it, and no run can be identified from a
rebuilt copy. A reviewer holding a different archive should confirm it satisfies
this table rather than try to match the bytes of the one checked in.

It must be built with the same GCC the firmware uses, because fat-LTO IR is
compiler-version specific. Today that means the Vitis toolchain; a GCC 15.2
archive will not link into a GCC 13.4 firmware.

An earlier archive here had 62 members, because eleven unreachable ones
(`aes*`, `sha2*`, the `*x4` 4-way SHAKE objects, `sig_stfl`) had been deleted by
hand. That pruning was cosmetic — a linker with `--gc-sections` only extracts
members it needs, so it saved a couple of hundred KB on the build host and zero
bytes of firmware. The script does not bother, which is why its output has more
members than that and still produces identical firmware.

---

## 5. Appendix: the same build without the script

`build_liboqs.sh` is a convenience, not the source of truth. Everything it does
is written out here, in the same order, so the archive can be reproduced even if
the script is lost. These steps produce an archive with the same members and
symbols; see the note at the top of this file on why it is not byte-identical.

Start a scratch shell (`bash`), then set the toolchain prefix and a working
directory. `set -e` is not optional: the steps below assert things with bare
`test`, `grep -qx` and `awk` exit codes, and without it a failed assertion
prints nothing and the procedure carries on to produce a wrong archive.

```bash
set -e                                  # makes every check below fatal; see
                                        # the note in 5.2 for why this matters
export CROSS=riscv64-unknown-elf        # or riscv-none-elf for the xPack build
export WORK=/tmp/liboqs_build && mkdir -p $WORK && cd $WORK
```

### 5.1 Clone the pinned source

```bash
git clone --branch 0.16.0 --depth 1 \
    https://github.com/open-quantum-safe/liboqs.git $WORK/src
test "$(git -C $WORK/src rev-parse HEAD)" = 5a1a854b0dc9f2141bdc771c555ee60c37950183
```

Use the tag, not `main`. The only file modified in the clone is the one in
[§5.2](#52-roll-the-keccak-permutation).

### 5.2 Roll the Keccak permutation

Rationale in [§3](#3-why-the-keccak-permutation-has-to-be-rolled). Both guards
matter: if upstream ever moves this code, they stop the procedure instead of
quietly leaving the fully unrolled permutation in place. The `grep -qx` fails if
the `#define` is gone, and the `awk` exits non-zero unless it added exactly four
declarations, in which case the `&&` skips the `mv` and the file is left
untouched.

This is the step that depends on `set -e` from the preamble. Without it, a
missing `#define` makes `grep -qx` return 1 silently, `sed` then matches
nothing, and the `awk` still finds four sites to patch and exits 0 — so the `mv`
commits an edit to a file that was never switched to `Unrolling 1`, and the
build succeeds with a fully unrolled permutation. The script has the same guards
wired to `|| die` instead.

```bash
K=$WORK/src/src/common/sha3/xkcp_low/KeccakP-1600/plain-64bits/KeccakP-1600-opt64.c

# 1. select one round per iteration instead of all 24 inline
grep -qx '#define FullUnrolling' $K
sed -i 's/^#define FullUnrolling$/#define Unrolling 1/' $K

# 2. declare the loop counter the rolled macros need, at the four sites
#    that lack it; Permute_Nrounds already has one
awk '
    /declareABCDE/               { print; pending = 1; next }
    pending && /unsigned int i;/ { pending = 0; print; next }
    pending                      { print "\tunsigned int i;"; added++
                                   pending = 0; print; next }
                                 { print }
    END                          { exit (added == 4 ? 0 : 3) }
' $K > $K.asu && mv $K.asu $K
```

### 5.3 Write the bare-metal `fprintf`

Rationale in [§2](#2-what-the-script-changes-and-why). This file is only
compiled later, in [§5.6](#56-add-the-fprintf-member).

```bash
cat > $WORK/asu_fprintf.c <<'EOF'
#include <stdio.h>

int fprintf(FILE *__restrict stream, const char *__restrict format, ...) {
	(void)stream;
	(void)format;
	return 0;
}
EOF
```

### 5.4 Configure and build

`-G "Unix Makefiles"` is required: steps 5.5 and 5.6 read compile flags out of
the generated `flags.make`, and no other generator emits it.

```bash
ASU_FLAGS="-march=rv32imac_zicsr_zifencei -mabi=ilp32 -ffreestanding \
-DMLK_CONFIG_SERIAL_FIPS202_ONLY -DMLD_CONFIG_SERIAL_FIPS202_ONLY -DMLD_CONFIG_REDUCE_RAM \
-Dshake128ctx=OQS_SHA3_shake128_inc_ctx -Dshake128_release=shake128_inc_ctx_release"

cmake -S $WORK/src -B $WORK/build -G "Unix Makefiles" \
  -DCMAKE_SYSTEM_NAME=Generic \
  -DCMAKE_SYSTEM_PROCESSOR=riscv \
  -DCMAKE_C_COMPILER="$CROSS-gcc" \
  -DCMAKE_AR="$(command -v $CROSS-ar)" \
  -DCMAKE_RANLIB="$(command -v $CROSS-ranlib)" \
  -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_FLAGS="$ASU_FLAGS" \
  -DCMAKE_C_FLAGS_RELEASE="-O2 -flto -ffat-lto-objects -DNDEBUG" \
  -DBUILD_SHARED_LIBS=OFF \
  -DOQS_BUILD_ONLY_LIB=ON \
  -DOQS_DIST_BUILD=OFF \
  -DOQS_USE_OPENSSL=OFF \
  -DOQS_EMBEDDED_BUILD=ON \
  -DOQS_MINIMAL_BUILD="KEM_ml_kem_512;KEM_ml_kem_768;KEM_ml_kem_1024;SIG_ml_dsa_44;SIG_ml_dsa_65;SIG_ml_dsa_87"

cmake --build $WORK/build -j
```

That produces `$WORK/build/lib/liboqs.a` with 73 members.

### 5.5 Check the firmware contract

`oqs_mldsa_internal.h` and `oqs_mlkem_internal.h` declare the native entry points
the firmware calls directly, and those declarations hold only while
`MLD_CONFIG_CONTEXT_PARAMETER` and `MLK_CONFIG_CONTEXT_PARAMETER` stay undefined
([§2](#how-that-assumption-is-checked)). Compile each contract header next to the
declaration this build actually produced, and C rejects the pair if they
disagree.

```bash
H=<embeddedsw>/lib/sw_services/xilasu/src/server   # where the two headers live

for s in 44 65 87; do
  F=$WORK/build/src/sig/ml_dsa/CMakeFiles/ml_dsa_${s}_ref.dir/flags.make
  eval "args=($(sed -n 's/^C_FLAGS = //p; s/^C_INCLUDES = //p' $F))"
  printf '#include "mldsa/src/sign.h"\n#include "oqs_mldsa_internal.h"\n' > $WORK/c$s.c
  $CROSS-gcc "${args[@]}" -I$H -fsyntax-only $WORK/c$s.c
done

for s in 512 768 1024; do
  F=$WORK/build/src/kem/ml_kem/CMakeFiles/ml_kem_${s}_ref.dir/flags.make
  eval "args=($(sed -n 's/^C_FLAGS = //p; s/^C_INCLUDES = //p' $F))"
  printf '#include "mlkem/src/kem.h"\n#include "oqs_mlkem_internal.h"\n' > $WORK/k$s.c
  $CROSS-gcc "${args[@]}" -I$H -fsyntax-only $WORK/k$s.c
done
```

All six must compile. `error: conflicting types for 'PQCP_...'` means the archive
would have linked and then passed arguments the callee does not expect. `eval` is
what splits the `flags.make` line the way make would, keeping the escaped quotes
around `MLD_CONFIG_FILE` intact.

Warnings about a redefined `MLDSA_SEEDBYTES` and its neighbours are expected
here. The script avoids them by capturing each shared value into an enum and
`#undef`-ing the name before including the contract header, which is also how it
compares those constants; the short form above checks argument lists only.

### 5.6 Add the `fprintf` member

It must be compiled with the same flags as the rest of the archive, so recover
them from `flags.make` rather than retyping them.

```bash
A=$WORK/build/lib/liboqs.a
D=$WORK/build/src/common/sha3/xkcp_low/CMakeFiles/xkcp_low_keccakp_1600_plain64.dir
test -f $D/flags.make          # absent unless 5.4 used -G "Unix Makefiles"
KFLAGS=$(sed -n 's/^C_FLAGS = //p' $D/flags.make | head -1)
test -n "$KFLAGS"              # empty would compile the stub with no flags

cd $WORK
BEFORE=$($CROSS-ar t $A | wc -l)
$CROSS-gcc $KFLAGS -c $WORK/asu_fprintf.c -o asu_fprintf.c.obj
$CROSS-ar r $A asu_fprintf.c.obj && $CROSS-ranlib $A
test "$((BEFORE + 1))" = "$($CROSS-ar t $A | wc -l)"
```

The `test` matters: `ar r` appends a new member but silently *replaces* an
existing one, so the count is what distinguishes the two.

### 5.7 Check and install

```bash
# 74 members
$CROSS-ar t $A | wc -l

# 599 defined symbols. 'NF>2' drops the 'member.c.obj:' header lines that nm
# interleaves with the symbols; without it the count is inflated.
$CROSS-nm --defined-only $A | awk 'NF>2{print $NF}' | sort -u | wc -l

# fprintf must be DEFINED here. If it were only referenced, the linker would
# satisfy it from libc.a and drag vfprintf and _dtoa_r in with it.
$CROSS-nm $A | grep -c ' T fprintf$'

# the Keccak entry point must still be defined after the edit in 5.2
$CROSS-nm $A | grep -c ' T KeccakP1600_Permute_24rounds$'

# every symbol the two contract headers declare must resolve: 18 and 9. This
# prints nothing when they all do; anything it prints is an unresolved symbol.
$CROSS-nm --defined-only $A | awk 'NF>2{print $NF}' | sort -u > $WORK/defined.txt
for h in oqs_mldsa_internal.h oqs_mlkem_internal.h; do
  grep -ohE 'PQCP_(MLDSA|MLKEM)_NATIVE_[A-Za-z0-9_]+_C_[A-Za-z0-9_]+' $H/$h \
    | sort -u | comm -23 - $WORK/defined.txt
done

cp $A <embeddedsw>/lib/sw_services/xilasu/src/server/liboqs.a
```

Both `grep -c` lines must print a non-zero count. These are the same two
assertions `build_liboqs.sh` makes before it installs the archive.
