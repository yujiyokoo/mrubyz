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
| Model | TI-84 Plus (Z80), not the CE (eZ80) | z88dk supports it with `+ti8x`. Same toolchain as the SMS build. |
| Test program | Run `demo/add.rb` bytecode and print the result | Proves the VM, not only the toolchain. Needs no new Ruby code. |
| Launch method | Plain TI-OS, run with `Asm(prgmNAME)` | No shell is needed. z88dk supports it directly. |
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
5. Wait for a key press, then return to TI-OS.

The file defines the same stub functions as `demo01.c`: `show_logo`,
`restore_font`, and `bytecode_at`. The linker needs them because
`mrubyz.c` references them.

### 2. Makefile target `ti84_01` (new)

```
TI84_CFLAGS = +ti8x -vn -m -DAMALLOC

ti84_01: demo/ti84_01.c mrubyz.c demo/add.ruby.c | $(BUILDDIR)
	$(ZCC) $(TI84_CFLAGS) demo/ti84_01.c mrubyz.c demo/add.ruby.c -o $(BUILDDIR)/ti84_01 -create-app
```

`-create-app` makes a `.8xp` file in `build/`. The TI-OS program name comes
from the output name; it must be 8 characters or fewer, letters and digits
only. The implementation confirms the exact name that z88dk produces.

The target is not in `DEMOS`. `make` alone still builds only the SMS demos.
`make ti84_01` builds the calculator program. `make clean` removes the new
build files.

### 3. `mrubyz.c` (no planned change)

The VM already has a non-SMS path behind `#ifdef __SMS__`, with stub
functions for the SMS-only builtins. If the `+ti8x` compile shows a gap in
that path, we fix only that gap. We do not change SMS behaviour.

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

The first implementation step is a size spike:

1. Compile the three files for `+ti8x`.
2. Read the output size from the `.8xp` file and the map file.
3. If the size is under the limit, continue.
4. If the size is over the limit, try z88dk size-optimization flags
   (for example `-SO3 --opt-code-size`), then report the numbers and stop
   for a decision before more work.

## Error handling

The VM already has `ASSERT`, which prints a message and exits on a failed
check. No new error handling is needed for the test program. If the build
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
