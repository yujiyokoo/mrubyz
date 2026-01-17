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

## Test

Tests are not extensive (yet), but you can run them with:

```
make test
```
