CXX ?= g++
NASM ?= nasm
LD ?= ld
BUILD := build
ISO_ROOT := $(BUILD)/isodir
KERNEL := $(BUILD)/kernel.elf
ISO := $(BUILD)/IrOS.iso
DISK := $(BUILD)/IrOS.disk
FONT_TTF := $(BUILD)/Vazir.ttf
FONT_HEADER := $(BUILD)/generated/vazir_bitmap.hpp
FONT_URL := https://raw.githubusercontent.com/rastikerdar/vazirmatn/master/fonts/ttf/Vazirmatn-Regular.ttf
CPP_SOURCES := $(shell find src -name '*.cpp')
CPP_OBJECTS := $(patsubst src/%.cpp,$(BUILD)/%.o,$(CPP_SOURCES))
ASM_SOURCES := src/arch/x86_64/boot.asm src/arch/x86_64/interrupts.asm
ASM_OBJECTS := $(patsubst src/%.asm,$(BUILD)/%.o,$(ASM_SOURCES))
CXXFLAGS := -m64 -std=gnu++17 -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector -fno-pie -fno-pic -fno-threadsafe-statics -fno-builtin -mno-red-zone -mgeneral-regs-only -Wall -Wextra -Wpedantic -O2 -Isrc -I$(BUILD)/generated
LDFLAGS := -m elf_x86_64 -T linker.ld --build-id=none -nostdlib
.PHONY: all iso disk run check clean font-assets
all: iso disk
font-assets:
	@mkdir -p $(BUILD)/generated
	@rm -f $(FONT_HEADER)
	@if command -v curl >/dev/null 2>&1 && python3 -c 'import PIL' >/dev/null 2>&1; then \
	  if curl -L --fail --silent --show-error "$(FONT_URL)" -o "$(FONT_TTF)"; then \
	    python3 tools/fontpack.py "$(FONT_TTF)" "$(FONT_HEADER)" || rm -f "$(FONT_HEADER)"; \
	  fi; \
	fi
$(CPP_OBJECTS): | font-assets
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
disk:
	@mkdir -p $(BUILD)
	@test -f $(DISK) || truncate -s 8M $(DISK)
check: iso disk
	@rm -f $(BUILD)/serial-first.log $(BUILD)/serial.log $(BUILD)/debug.log
	@set +e; timeout 7s qemu-system-x86_64 -m 512M -cdrom $(ISO) -drive file=$(DISK),format=raw,if=ide,index=0,media=disk -netdev user,id=n0 -device rtl8139,netdev=n0 -display none -serial file:$(BUILD)/serial-first.log -debugcon file:$(BUILD)/debug.log -global isa-debugcon.iobase=0xe9 -no-reboot -no-shutdown; first=$$?; set -e; \
	echo "--- first boot serial ---"; cat $(BUILD)/serial-first.log || true; \
	grep -q "IrOS kernel ready" $(BUILD)/serial-first.log; grep -q "IrOS runtime stable" $(BUILD)/serial-first.log; grep -q "icmp=ok" $(BUILD)/serial-first.log; grep -q "font=Vazirmatn-Regular.ttf" $(BUILD)/serial-first.log; \
	set +e; timeout 7s qemu-system-x86_64 -m 512M -cdrom $(ISO) -drive file=$(DISK),format=raw,if=ide,index=0,media=disk -netdev user,id=n0 -device rtl8139,netdev=n0 -display none -serial file:$(BUILD)/serial.log -no-reboot -no-shutdown; second=$$?; set -e; \
	echo "--- second boot serial ---"; cat $(BUILD)/serial.log || true; \
	grep -q "IrOS kernel ready" $(BUILD)/serial.log; grep -q "arch=x86_64" $(BUILD)/serial.log; grep -q "IrOS runtime stable" $(BUILD)/serial.log; grep -q "IrFS persistence restored" $(BUILD)/serial.log; grep -q "net=rtl8139" $(BUILD)/serial.log; grep -q "icmp=ok" $(BUILD)/serial.log; grep -q "font=Vazirmatn-Regular.ttf" $(BUILD)/serial.log; \
	if grep -q "IrOS exception" $(BUILD)/serial.log; then echo "Kernel exception detected"; exit 1; fi; \
	if [ $$first -ne 0 ] && [ $$first -ne 124 ]; then exit $$first; fi; if [ $$second -ne 0 ] && [ $$second -ne 124 ]; then exit $$second; fi
run: iso disk
	qemu-system-x86_64 -m 512M -cdrom $(ISO) -drive file=$(DISK),format=raw,if=ide,index=0,media=disk -boot d -netdev user,id=n0 -device rtl8139,netdev=n0 -display sdl,show-cursor=off,grab-mod=rctrl -serial stdio
clean:
	rm -rf $(BUILD)
