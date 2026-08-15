#!/usr/bin/env sh
set -e
make iso disk
exec qemu-system-x86_64 -m 512M -cdrom build/IrOS.iso -drive file=build/IrOS.disk,format=raw,if=ide,index=0,media=disk -boot d -netdev user,id=n0 -device rtl8139,netdev=n0 -display sdl,show-cursor=off,grab-mod=rctrl -serial stdio
