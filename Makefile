# tessera build. Produces BOOTX64.EFI (PE32+) via ld, no mingw required.
CC      ?= gcc
LD      ?= ld
RAW_PAYLOAD ?= 1
CFLAGS  := -DTESS_RAW_PAYLOAD=$(RAW_PAYLOAD) -std=c11 -Wall -Wextra -Werror -ffreestanding -fno-stack-protector \
           -fpic -mno-red-zone -nostdlib -Iinclude
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
	$(LD) -m i386pep --subsystem 10 --dynamicbase --nxcompat -e efi_entry -o $@ $(OBJS)
	objcopy -R .comment $@

clean:
	rm -rf $(BUILD)

# Boot the image under QEMU with OVMF and check the serial log (needs qemu and OVMF).
qemu-test: all
	sh tests/qemu_boot.sh

.PHONY: all check clean qemu-test

# Host tests: link every unit object with the test driver. Units are freestanding, so
# only the test program uses the hosted C library.
TEST_OBJS := $(filter-out build/boot/efi_entry/%,$(OBJS))
build/test_modload: tests/host/test_modload.c $(OBJS)
	$(CC) -std=c11 -Wall -Wextra -Werror -O1 -Iinclude -o $@ $< $(TEST_OBJS)

build/test_payload: tests/host/test_payload.c $(OBJS)
	$(CC) -std=c11 -Wall -Wextra -Werror -O1 -Iinclude -o $@ $< $(TEST_OBJS)

build/test_crash_auth: tests/host/test_crash_auth.c $(OBJS)
	$(CC) -std=c11 -Wall -Wextra -Werror -O1 -Iinclude -o $@ $< $(TEST_OBJS)

build/test_trap: tests/host/test_trap.c $(OBJS)
	$(CC) -std=c11 -Wall -Wextra -Werror -O1 -Iinclude -o $@ $< $(TEST_OBJS)

build/test_sched: tests/host/test_sched.c $(OBJS)
	$(CC) -std=c11 -Wall -Wextra -Werror -O1 -Iinclude -o $@ $< $(TEST_OBJS)

build/test_panic: tests/host/test_panic.c $(OBJS)
	$(CC) -std=c11 -Wall -Wextra -Werror -O1 -Iinclude -o $@ $< $(TEST_OBJS)

build/test_shell: tests/host/test_shell.c $(OBJS)
	$(CC) -std=c11 -Wall -Wextra -Werror -O1 -Iinclude -o $@ $< $(TEST_OBJS)

build/test_morse: tests/host/test_morse.c $(OBJS)
	$(CC) -std=c11 -Wall -Wextra -Werror -O1 -Iinclude -o $@ $< $(TEST_OBJS)

build/test_initrd: tests/host/test_initrd.c $(OBJS)
	$(CC) -std=c11 -Wall -Wextra -Werror -O1 -Iinclude -o $@ $< $(TEST_OBJS)

build/test_ram: tests/host/test_ram.c $(OBJS)
	$(CC) -std=c11 -Wall -Wextra -Werror -O1 -Iinclude -o $@ $< $(TEST_OBJS)





test: build/test_initrd build/test_modload build/test_payload build/test_ram build/test_morse build/test_crash_auth build/test_shell build/test_panic build/test_sched build/test_trap
	./build/test_initrd
	./build/test_modload
	./build/test_payload
	./build/test_ram
	./build/test_morse
	./build/test_crash_auth
	./build/test_shell
	./build/test_panic
	./build/test_sched
	./build/test_trap

.PHONY: test
