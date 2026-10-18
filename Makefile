# tessera build. Produces BOOTX64.EFI (PE32+) via ld, no mingw required.
CC      ?= gcc
LD      ?= ld
CFLAGS  := -std=c11 -Wall -Wextra -Werror -ffreestanding -fno-stack-protector \
           -fno-pic -mno-red-zone -nostdlib -Iinclude
UNITS   := $(wildcard src/*/*/*.c)
OBJS    := $(patsubst src/%.c,build/%.o,$(UNITS))
BUILD   := build

all: check $(BUILD)/BOOTX64.EFI

check:
	python3 tools/check_layout.py

$(BUILD)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/BOOTX64.EFI: $(OBJS)
	$(LD) -m i386pep --subsystem 10 -e efi_entry -o $@ $(OBJS)

clean:
	rm -rf $(BUILD)

.PHONY: all check clean
