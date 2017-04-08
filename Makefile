#TOP = .
#all: game.img
BOOT := boot.bin
KERNEL := kernel.bin
GAME := game.bin
PROGRAM := program.bin
IMAGE := disk.bin

CC      := gcc
LD      := ld
OBJCOPY := objcopy
DD      := dd
QEMU    := qemu-system-i386
GDB     := gdb

CFLAGS := -Wall -Werror -Wfatal-errors -fno-stack-protector #开启所有警告, 视警告为错误, 第一个错误结束编译
CFLAGS += -MD #生成依赖文件
CFLAGS += -std=gnu11 -m32 -c #编译标准, 目标架构, 只编译
CFLAGS += -I . #头文件搜索目录
CFLAGS += -O0 #不开优化, 方便调试
CFLAGS += -fno-builtin #禁止内置函数
CFLAGS += -ggdb3 #GDB调试信息

QEMU_OPTIONS := -serial stdio
QEMU_OPTIONS += -monitor telnet:127.0.0.1:1111,server,nowait

QEMU_DEBUG_OPTIONS := -S #启动不执行
QEMU_DEBUG_OPTIONS += -s #GDB调试服务器: 127.0.0.1:1234

GDB_OPTIONS := -ex "target remote 127.0.0.1:1234"
GDB_OPTIONS += -ex "symbol $(KERNEL)"

OBJ_DIR        := obj
LIB_DIR        := lib
BOOT_DIR       := boot
KERNEL_DIR     := kernel
GAME_DIR       := game
OBJ_LIB_DIR    := $(OBJ_DIR)/$(LIB_DIR)
OBJ_BOOT_DIR   := $(OBJ_DIR)/$(BOOT_DIR)
OBJ_KERNEL_DIR := $(OBJ_DIR)/$(KERNEL_DIR)
OBJ_GAME_DIR   := $(OBJ_DIR)/$(GAME_DIR)

KERNEL_LD_SCRIPT := $(shell find $(KERNEL_DIR) -name "*.ld")
GAME_LD_SCRIPT	:= $(shell find $(GAME_DIR) -name "*.ld")

LIB_C := $(shell find $(LIB_DIR) -name "*.c")
LIB_O := $(LIB_C:%.c=$(OBJ_DIR)/%.o)

BOOT_S := $(wildcard $(BOOT_DIR)/*.S)
BOOT_C := $(wildcard $(BOOT_DIR)/*.c)
BOOT_O := $(BOOT_S:%.S=$(OBJ_DIR)/%.o)
BOOT_O += $(BOOT_C:%.c=$(OBJ_DIR)/%.o)

GAME_C := $(shell find $(GAME_DIR) -name "*.c")
GAME_O := $(GAME_C:%.c=$(OBJ_DIR)/%.o)

KERNEL_C := $(shell find $(KERNEL_DIR) -name "*.c")
KERNEL_S := $(shell find $(KERNEL_DIR) -name "*.S")
KERNEL_O := $(KERNEL_C:%.c=$(OBJ_DIR)/%.o) 
KERNEL_O += $(KERNEL_S:%.S=$(OBJ_DIR)/%.o)



$(IMAGE): $(BOOT) $(PROGRAM)
	@$(DD) if=/dev/zero of=$(IMAGE) count=10000         > /dev/null # 准备磁盘文件
	@$(DD) if=$(BOOT) of=$(IMAGE) conv=notrunc          > /dev/null # 填充 boot loader
	@$(DD) if=$(PROGRAM) of=$(IMAGE) seek=1 conv=notrunc > /dev/null # 填充 kernel, 跨过 mbr

$(BOOT): $(BOOT_O)
	$(LD) -e boot -Ttext=0x7C00 -m elf_i386 -nostdlib -o $@.out $^
	$(OBJCOPY) --strip-all --only-section=.text --output-target=binary $@.out $@
	@rm $@.out
	perl ./boot/genboot.pl $@
#	ruby ./boot/mbr.rb $@

$(OBJ_BOOT_DIR)/%.o: $(BOOT_DIR)/%.[cS]
	@mkdir -p $(OBJ_BOOT_DIR)
	$(CC) $(CFLAGS) -Os -I ./boot/include $< -o $@

#$(OBJ_BOOT_DIR)/%.o: $(BOOT_DIR)/%.c
#	@mkdir -p $(OBJ_BOOT_DIR)
#	$(CC) $(CFLAGS) -Os -I ./boot/inc $< -o $@

$(PROGRAM): $(KERNEL) $(GAME)
	cat $(KERNEL) $(GAME) > $(PROGRAM)

$(KERNEL): $(KERNEL_LD_SCRIPT)
$(KERNEL): $(KERNEL_O) $(LIB_O)
	$(LD) -m elf_i386 -T $(KERNEL_LD_SCRIPT) -nostdlib -o $@ $^ $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)
	perl ./kernel/genkern.pl $@

$(OBJ_LIB_DIR)/%.o : $(LIB_DIR)/%.c
	@mkdir -p $(OBJ_LIB_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_KERNEL_DIR)/%.o: $(KERNEL_DIR)/%.[cS]
	mkdir -p $(OBJ_DIR)/$(dir $<)
	$(CC) $(CFLAGS) -I ./kernel/include $< -o $@

$(GAME): $(GAME_LD_SCRIPT)
$(GAME): $(GAME_O) $(LIB_O)
	$(LD) -m elf_i386 -T $(GAME_LD_SCRIPT) -nostdlib -o $@ $^ $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)
	$(call git_commit, "compile", $(GITFLAGS))

$(OBJ_GAME_DIR)/%.o: $(GAME_DIR)/%.c
	mkdir -p $(OBJ_DIR)/$(dir $<)
	$(CC) $(CFLAGS) -I ./game/include $< -o $@

DEPS := $(shell find -name "*.d")
-include $(DEPS)
#ASFLAGS := -m32 -MD
#LDFLAGS := -melf_i386
#QEMU 	:= qemu-system-i386

#CFILES 	:= $(shell find game/ lib/ -name "*.c")
#SFILES 	:= $(shell find game/ lib/ -name "*.S")
#OBJS 	:= $(LIB_O) $(GAME_O) $(KERNEL_O)

include config/Makefile.git
include config/Makefile.build

#include boot/Makefile.part
#include kernel/Makefile.part
#include game/Makefile.part

#game.img: bootblock kernel game
#	cat $(BOOT) $(KERNEL) $(GAME) > obj/game.img
#	$(call git_commit, "compile game", $(GITFLAGS))

#game: $(OBJS)
#	@mkdir -p obj/game
#	$(LD) $(LDFLAGS) -e game_init -Ttext 0x00200000 -o obj/game/game $(OBJS)
	

#$(OBJ_LIB_DIR)/%.o : $(LIB_DIR)/%.c
#	@mkdir -p $(OBJ_DIR)/$(dir $<)
#	$(CC) $(CFLAGS) $< -o $@

#$(OBJ_GAME_DIR)/%.o : $(GAME_DIR)/%.c
#	@mkdir -p $(OBJ_DIR)/$(dir $<)
#	$(CC) $(CFLAGS) $< -o $@

#$(OBJ_KERNEL_DIR)/%.o : $(KERNEL_DIR)/%.[cS]
#	@mkdir -p $(OBJ_DIR)/$(dir $<)
#	$(CC) $(CFLAGS) $< -o $@

#-include $(patsubst %.o, %.d, $(OBJS))
#IMAGES	:= $(OBJ_DIR)/game.img
#GDBPORT := $(shell expr `id -u` % 5000 + 25000)
#QEMUOPTS = $(OBJ_DIR)/game.img -serial mon:stdio
#QEMUOPTS += $(shell if $(QEMU) -nographic -help | grep -q '^-D '; then echo '-D qemu.log'; fi)

.PHONY: clean debug gdb submit qemu commit log

gdb:
	$(GDB) $(GDB_OPTIONS)
	$(call git_commit, "run gdb", $(GITFLAGS))

qemu: $(IMAGE)
	$(QEMU) $(QEMU_OPTIONS) $(IMAGE)
	$(call git_commit, "run qemu", $(GITFLAGS))

debug: $(IMAGES)
	$(QEMU) $(QEMU_DEBUG_OPTIONS) $(QEMU_OPTIONS) $(IMAGE)
	$(call git_commit, "debug", $(GITFLAGS))

clean: 
	@rm -rf $(OBJ_DIR) 2> /dev/null
	@rm -rf $(BOOT)    2> /dev/null
	@rm -rf $(KERNEL)  2> /dev/null
	@rm -rf $(GAME)	   2> /dev/null
	@rm -rf $(PROGRAM) 2> /dev/null
	@rm -rf $(IMAGE) 2> /dev/null

submit: clean
	cd .. && tar cvj $(shell pwd | grep -o '[^/]*$$') > $(STU_ID).tar.bz2

commit:
	@git commit --allow-empty

log:
	@git log --author=dancingflower
