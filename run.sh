#!/usr/bin/env bash
set -euo pipefail
qemu-system-x86_64 -m 512M -cdrom build/IrOS.iso -boot d -serial stdio
