#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

export TARGET := star-drag
export TOPDIR := $(CURDIR)

GAME_ICON :=

# v12: music is EMBEDDED in the ARM9 ELF again (audio/music_aN.bin via
# bin2o) — the v11 NitroFS path failed silently on NooDS. The nitro/
# directory is kept on disk as the v11 record but is NOT linked into
# the ROM anymore.
NITRO_FILES :=

include $(DEVKITARM)/ds_rules

.PHONY: checkarm7 checkarm9 clean

all: checkarm7 checkarm9 $(TARGET).nds

checkarm7:
	$(MAKE) -C arm7

checkarm9:
	$(MAKE) -C arm9

$(TARGET).nds : $(NITRO_FILES) arm7/$(TARGET).elf arm9/$(TARGET).elf
	ndstool	-c $(TARGET).nds -7 arm7/$(TARGET).elf -9 arm9/$(TARGET).elf \
	-b $(GAME_ICON) "Star Drag;Touch-and-drag shooter;retroromrunner" \
	$(_ADDFILES)

arm7/$(TARGET).elf:
	$(MAKE) -C arm7

arm9/$(TARGET).elf:
	$(MAKE) -C arm9

clean:
	$(MAKE) -C arm9 clean
	$(MAKE) -C arm7 clean
	rm -f $(TARGET).nds
