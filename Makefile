CXX ?= g++
NASM ?= nasm
LD ?= ld

BUILD := build
ISO_ROOT := $(BUILD)/isodir
KERNEL := $(BUILD)/kernel.elf
ISO := $(BUILD)/IrOS.iso

CXXFLAGS := -m32 -std=gnu++17 -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector -fno-pie -fno-threadsafe-statics -nostdlib -Wall -Wextra -Wpedantic -O2 -Isrc
LDFLAGS := -m elf_i386 -T linker.ld --build-id=none

.PHONY: all iso run clean check

all: iso

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/boot.o: src/boot.asm | $(BUILD)
	$(NASM) -f elf32 $< -o $@

$(BUILD)/kernel.o: src/kernel.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(KERNEL): $(BUILD)/boot.o $(BUILD)/kernel.o linker.ld
	$(LD) $(LDFLAGS) -o $@ $(BUILD)/boot.o $(BUILD)/kernel.o
	grub-file --is-x86-multiboot $@

iso: $(KERNEL)
	rm -rf $(ISO_ROOT)
	mkdir -p $(ISO_ROOT)/boot/grub
	cp $(KERNEL) $(ISO_ROOT)/boot/kernel.elf
	cp grub/grub.cfg $(ISO_ROOT)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) $(ISO_ROOT) >/dev/null 2>&1
	@echo "Built $(ISO)"

check: iso
	@rm -f $(BUILD)/serial.log
	@set +e; timeout 6s qemu-system-i386 -m 256M -cdrom $(ISO) -display none -serial file:$(BUILD)/serial.log -no-reboot -no-shutdown; status=$$?; set -e; \
	cat $(BUILD)/serial.log; \
	grep -q "IrOS kernel ready" $(BUILD)/serial.log; \
	if [ $$status -ne 0 ] && [ $$status -ne 124 ]; then exit $$status; fi

run: iso
	qemu-system-i386 -m 256M -cdrom $(ISO) -boot d

clean:
	rm -rf $(BUILD)
