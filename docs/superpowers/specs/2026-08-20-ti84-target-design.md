# TI-84 Plus Target — Design

Date: 2026-08-20
Status: approved design, not yet implemented

## Goal

Add a TI-84 Plus build target to mrubyz. The target builds one tiny test
program. The program runs the existing `demo/add.rb` mruby bytecode in the
mrubyz VM and shows the result (6) on the calculator screen.

This proves that the VM runs on a second Z80 platform. It does not add
features to the VM.

## Decisions

| Topic | Decision | Reason |
|---|---|---|
| Model | TI-84 Plus (Z80), not the CE (eZ80) | z88dk supports it with `+ti83p` (the TI-83 Plus family target). Same toolchain as the SMS build. |
| Test program | Run `demo/add.rb` bytecode and print the result | Proves the VM, not only the toolchain. Needs no new Ruby code. |
| Launch method | Plain TI-OS, run with `Asm(prgmNAME)` | No shell is needed. z88dk supports it with `-subtype=asm`. |
| Verification | TI-84 Plus emulator (for example tilem2) | Fast feedback. The user supplies the ROM image. |
| Build structure | One new demo file and one new Makefile target | Smallest change. Follows the existing pattern. |

## Components

### 1. `demo/ti84_01.c` (new)

The only new C code. It does not include `sms.h`.

`main` does these steps in order:

1. Initialize a small static heap: `static uint8_t heap[2048];` then
   `mallinit()` and `sbrk(heap, sizeof(heap))`.
2. Print a short banner with `printf`.
3. Call `mrbz_vm_run(&vm, &v, bytecode)` on the `add.rb` bytecode.
4. Print the returned number with `printf`.
5. Wait for a key press with the z88dk console call `fgetc_cons()` (or
   `getk()`), then return to TI-OS. Do not use any SMS helper such as
   `wait_vblank_noint`.

The file defines one stub only: `uint8_t bytecode_at(uint8_t i)`.
`mrubyz.c` uses it unconditionally but declares it only inside
`#ifdef __SMS__`, so the non-SMS build needs a declaration and a definition.

The file must NOT define `show_logo`, `restore_font`, `pal1`, or `pal2`.
`mrubyz.c` already defines them in its `#ifndef __SMS__` block (lines
54-143). Copying those stubs from `demo01.c` causes duplicate-symbol link
errors.

### 2. Makefile target `ti84_01` (new)

```
TI84_CFLAGS = +ti83p -subtype=asm -vn -m -DAMALLOC -DMRBZ_MINIMAL

ti84_01: demo/ti84_01.c mrubyz.c demo/add.ruby.c | $(BUILDDIR)
	$(ZCC) $(TI84_CFLAGS) demo/ti84_01.c mrubyz.c demo/add.ruby.c -o $(BUILDDIR)/ti8401 -create-app
```

`-create-app` makes a `.8xp` file in `build/`. The TI-OS program name comes
from the output name; it must be 8 characters or fewer, letters and digits
only. The implementation confirms the exact name that z88dk produces.

The target is not in `DEMOS`. `make` alone still builds only the SMS demos.
`make ti84_01` builds the calculator program. `make clean` already removes
everything in `build/`, so it needs no change.

### 3. `mrubyz.c` — minimal VM build flag `MRBZ_MINIMAL`

The VM already has a non-SMS path behind `#ifdef __SMS__`. The first
`+ti83p` build linked with no gaps, but measured 21,910 bytes — about 2.5x
the 8811-byte `Asm(` limit (VM code alone ~15.4 KB). Size-optimization flags
saved only 128 bytes. So the TI build compiles a reduced VM.

A new compile-time flag `MRBZ_MINIMAL` keeps only what `add.rb` needs:

| Kept | Compiled out under `MRBZ_MINIMAL` |
|---|---|
| irep/header parsing (`parse_ireps`, `parse_irep_record`) | `call_builtin` and all SMS builtins |
| `OP_NOP`, `OP_MOVE`, `OP_LOADI*`, `OP_LOADINEG`, `OP_LOADI16` | sound/music (`bgm_*`, `sfx_*`, `start_*`, `play_end_sfx`) |
| `OP_ADD`, `OP_ADDI`, `OP_SUB`, `OP_SUBI`, `OP_MUL`, `OP_DIV` | text rendering (`text_scroll`, `emit_text_code`, `nametable_row`) |
| `OP_RETURN`, `OP_STOP` | method dispatch (`op_send`, `op_ssend`, `op_enter`, `op_def`, `op_method`, `op_tclass`, `dispatch_*`, `find_method`) |
| the `mrbz_vm_run` loop and its `default:` error path | arrays, strings, `getidx`, `eq`, `gt`, `jmp*`, `loadnil/t/f` |

Rules:
- The flag is additive: without `-DMRBZ_MINIMAL` the file compiles exactly
  as before. The SMS demos and `make test` do not set it and must not change.
- Compiled-out opcodes fall to the existing `default:` case in
  `mrbz_vm_run`, which reports an unknown instruction.
- The cuts are `#ifndef MRBZ_MINIMAL` blocks around whole functions and
  whole `case` groups. No logic edits inside kept functions.
- If parsing code references compiled-out helpers (for example pool or
  symbol handling), keep those helpers; correctness of parsing comes first.

Estimated result: well under 8811 bytes (the C library is only ~1.3 KB).

## Heap design

`demo01.c` registers 4 KB of heap at the address of a 4-byte variable
(`long _heap; sbrk(&_heap, 4096);`). The linker reserves only 4 bytes, so
the other 4092 bytes are unowned memory after BSS. On the SMS this works by
luck of the memory layout. On the TI-84 Plus the memory after the program
belongs to TI-OS, so the same trick corrupts the OS and can clear RAM.

The new file uses a real static array. The linker then reserves the full
region, and it counts toward the program size. 2 KB is the starting size. It
is enough for the `add.rb` program, and it fits the small RAM budget.

## Size risk — the go/no-go gate

TI-OS refuses `Asm(` programs above about 8.8 KB (8811 bytes). The SMS
`demo01` code binary is about 23 KB, but most of that is SMS graphics code
and font data that the TI-84 build does not include.

The size spike was done on 2026-08-20: full VM = 21,910 bytes raw
(code 15,412 + clib 1,304 + rodata ~2,000 + BSS 2,054). Over the limit.
Size flags: 21,782 bytes. Decision (user, 2026-08-20): build a minimal VM
(component 3) and use the size to pick the launch method:

| Minimal build size | Launch method |
|---|---|
| under 8811 bytes | plain TI-OS `Asm(` program (`-subtype=asm`) — preferred |
| 8811 to ~16 KB | single-page Flash app (`-subtype=app_one_page`, z88dk signs it) — fallback, needs a new decision |
| over 16 KB | stop; the C runtime, not the VM, is the problem |

Research notes: z88dk's `ti8xk` appmake signs `.8xk` files itself; shells
that "skip" the 8811 limit are not documented reliably and the 84+ has RAM
execution protection, so they are not a path.

## Error handling

The VM has `ASSERT` and `debug_out`, but they are no-ops unless the build
sets `-DDEBUG`. The size spike compiles without `-DDEBUG` first, because
debug output adds code size. If the program misbehaves, a second build with
`-DDEBUG` added to `TI84_CFLAGS` is the diagnostic step. No new error
handling is needed for the test program. If the build
fails because of a missing symbol in the non-SMS path of `mrubyz.c`, the
fix is a new stub behind the existing `#ifndef __SMS__` block.

## Testing

1. `make ti84_01` completes in the z88dk Docker image with no errors.
2. `build/*.8xp` exists and is under 8811 bytes.
3. Load the `.8xp` into a TI-84 Plus emulator and run `Asm(prgmNAME)`.
   Success: the screen shows the banner and the number 6.
4. `make` and `make test` still pass. The SMS build is untouched.

Step 3 needs the user's ROM image and emulator. The implementer reports the
build size and asks the user to confirm the screen output.

## Out of scope

- Any TI-84 Plus CE (eZ80) support.
- A multi-platform Makefile restructure.
- Fixing the `_heap` trick in the existing SMS demos. It is a known latent
  issue, left for a separate cleanup.
- Adding TI-84 builds to `make all` or to the test suite.
