BLOCKSDS ?= /opt/wonderful/thirdparty/blocksds/core
NAME := ftl-ds
GAME_TITLE := FTL DS Campaign
GAME_SUBTITLE := Base game demake
GAME_AUTHOR := Unofficial fan demake
GAME_ICON := generated/launcher_icon.gif
SOURCEDIRS := source
INCLUDEDIRS := generated
NITROFSDIR := nitrofiles
LIBS := -lmm9 -lnds9
LIBDIRS := $(BLOCKSDS)/libs/maxmod
include $(BLOCKSDS)/sys/default_makefiles/rom_arm9/Makefile

# Repackage when launcher artwork or title settings change.
$(ROM): $(GAME_ICON) Makefile

# Audio imports must trigger a new embedded NitroFS image.
$(ROM): $(wildcard nitrofiles/audio/*.pcm)
