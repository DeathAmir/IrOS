#!/usr/bin/env sh
set -eu
make iso
exec qemu-system-i386 -m 256M -cdrom build/IrOS.iso -boot d
