CXX ?= g++
NASM ?= nasm
LD ?= ld

BUILD := build
ISO_ROOT := $(BUILD)/isodir
KERNEL := $(BUILD)/kernel.elf
ISO := $(BUILD)/IrOS.iso

CPP_SOURCES := $(shell find src -name '*.cpp')
CPP_OBJECTS := $(patsubst src/%.cpp,$(BUILD)/%.o,$(CPP_SOURCES))
ASM_SOURCES := src/arch/x86_64/boot.asm src/arch/x86_64/interrupts.asm
ASM_OBJECTS := $(patsubst src/%.asm,$(BUILD)/%.o,$(ASM_SOURCES))

CXXFLAGS := -m64 -std=gnu++17 -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector -fno-pie -fno-pic -fno-threadsafe-statics -fno-builtin -mno-red-zone -mgeneral-regs-only -Wall -Wextra -Wpedantic -O2 -Isrc
LDFLAGS := -m elf_x86_64 -T linker.ld --build-id=none -nostdlib

.PHONY: all iso run check clean
all: iso

$(BUILD)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/%.o: src/%.asm
	@mkdir -p $(dir $@)
	$(NASM) -f elf64 $< -o $@

$(KERNEL): $(ASM_OBJECTS) $(CPP_OBJECTS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(ASM_OBJECTS) $(CPP_OBJECTS)
	grub-file --is-x86-multiboot2 $@

iso: $(KERNEL)
	rm -rf $(ISO_ROOT)
	mkdir -p $(ISO_ROOT)/boot/grub
	cp $(KERNEL) $(ISO_ROOT)/boot/kernel.elf
	cp grub/grub.cfg $(ISO_ROOT)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) $(ISO_ROOT)
	@echo "Built $(ISO)"

check: iso
	@rm -f $(BUILD)/serial.log $(BUILD)/debug.log
	@set +e; timeout 10s qemu-system-x86_64 -m 512M -cdrom $(ISO) -display none -serial file:$(BUILD)/serial.log -debugcon file:$(BUILD)/debug.log -global isa-debugcon.iobase=0xe9 -no-reboot -no-shutdown; status=$$?; set -e; \
	echo "--- debugcon ---"; cat $(BUILD)/debug.log || true; echo; \
	echo "--- serial ---"; cat $(BUILD)/serial.log || true; echo; \
	grep -q "IrOS kernel ready" $(BUILD)/serial.log; \
	grep -q "arch=x86_64" $(BUILD)/serial.log; \
	if [ $$status -ne 0 ] && [ $$status -ne 124 ]; then exit $$status; fi

run: iso
	qemu-system-x86_64 -m 512M -cdrom $(ISO) -boot d -serial stdio

clean:
	rm -rf $(BUILD)
