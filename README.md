# mrubyz

Experimental mruby VM for Z80.

Currently it skips a lot of the header contents, and only runs a subset of instructions.
It does not support a large range of mruby instructions yet but can run mruby bytecode.

The current focus is Sega Master System but the intention is there to port it to other Z80-based platforms.

There are some sample programs in Ruby and C under the `demo/` directory.

## Videos

There are a couple of very short screen recordings of the demo programs.

### Demo 05 (minimal game)

https://ruby.social/@yujiyokoo/115909334138258782


### Demo 04 (minimal presentation)

https://ruby.social/@yujiyokoo/115909361556444300

## Build

To build the sample programs (for Sega Master System) run,

```
make
```

You should end up with `demo*.sms` files. They are playable on Master System or their emulators.

### TI-84 Plus

There is an experimental build for the TI-84 Plus (the classic Z80 model, not the CE):

```
make ti84_01
```

It produces `build/ti8401.8xp`. Send it to the calculator or an emulator
(for example tilem2), then run `Asm(prgmTI8401)` from the home screen.
It runs the `demo/add.rb` bytecode and shows the result.

This build uses a reduced VM (`-DMRBZ_MINIMAL`: integer load/arithmetic,
move and return only) so it fits the 8811-byte TI-OS limit for `Asm(`
programs.

## Test

Tests are not extensive (yet), but you can run them with:

```
make test
```
