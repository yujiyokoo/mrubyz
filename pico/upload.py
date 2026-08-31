#!/usr/bin/env python3
# T11: compile a Ruby file with mruby 3.x mrbc and upload the .mrb bytecode to
# the Pico over USB serial using the T9 framing (4-byte LE length + raw bytes).
# Prints whatever the device sends back (e.g. "returned: 5").
#
# Usage:  ./upload.py [--source] PROG.rb [device]
#   default: host-compile with mrbc, send .mrb bytecode (mrubyz_pico firmware).
#   --source: send raw Ruby SOURCE, let the device compile it on-chip
#             (mrubyz_ondevice firmware, T13). No mrbc needed.
#   device defaults to the first /dev/cu.usbmodem*.
#   MRBC env var overrides the compiler (must be mruby 3.x -> RITE "0300").
#
# Stdlib only: no pyserial. macOS USB CDC ignores baud; we just need raw mode.
import glob
import os
import select
import struct
import subprocess
import sys
import termios
import time
import tty

MRBC = os.environ.get("MRBC", "/usr/local/bin/mrbc")


def find_device():
    m = sorted(glob.glob("/dev/cu.usbmodem*"))
    return m[0] if m else None


def compile_rb(path):
    mrb = path.rsplit(".", 1)[0] + ".mrb"
    subprocess.run([MRBC, "-o", mrb, path], check=True)
    with open(mrb, "rb") as f:
        code = f.read()
    if code[:8] != b"RITE0300":
        sys.exit("error: %s is not RITE0300 (mrubyz requires mruby 3.x). "
                 "Set MRBC to an mruby 3.x mrbc." % mrb)
    return code


def main():
    args = sys.argv[1:]
    source_mode = False
    if args and args[0] == "--source":
        source_mode = True
        args = args[1:]
    if not args:
        sys.exit("usage: upload.py [--source] PROG.rb [device]")
    rb = args[0]
    dev = args[1] if len(args) > 1 else find_device()
    if not dev:
        sys.exit("no /dev/cu.usbmodem* found -- is the Pico plugged in?")

    if source_mode:
        with open(rb, "rb") as f:
            code = f.read()               # raw Ruby source; device compiles it
        print("sending source %s: %d bytes -> %s" % (rb, len(code), dev))
    else:
        code = compile_rb(rb)             # host-compile to .mrb bytecode
        print("compiled %s: %d bytes -> %s" % (rb, len(code), dev))

    fd = os.open(dev, os.O_RDWR | os.O_NOCTTY)
    try:
        tty.setraw(fd)
        os.write(fd, struct.pack("<I", len(code)))  # 4-byte LE length
        os.write(fd, code)                            # raw .mrb

        deadline = time.time() + 2.0
        buf = b""
        while time.time() < deadline:
            r, _, _ = select.select([fd], [], [], 0.2)
            if r:
                buf += os.read(fd, 256)
        sys.stdout.write(buf.decode(errors="replace"))
        if not buf:
            print("(no reply -- device may not have the T10 receive loop yet)")
    finally:
        os.close(fd)


if __name__ == "__main__":
    main()
