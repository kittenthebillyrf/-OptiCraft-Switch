.DEFAULT_GOAL := all
.SUFFIXES:
TARGET := OptiCraft
BUILD := build
SOURCES := switch_nro
INCLUDES :=
ARCH := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft
CFLAGS := -g -Wall -Wextra -O2 -ffunction-sections $(ARCH)
CXXFLAGS := $(CFLAGS) -std=gnu++17
ASFLAGS := $(ARCH)
LDFLAGS := -specs=$(DEVKITPRO)/libnx/switch.specs -Wl,--gc-sections
LIBS := -lnx
APP_TITLE := OptiCraft World Prototype
APP_AUTHOR := OptiCraft Switch Port
APP_VERSION := 0.1.0
APP_ICON := assets/opticraft_icon.png
NROFLAGS := --icon=$(APP_ICON) --nacp=$(TARGET).nacp
.PHONY: all clean
all: $(TARGET).nro
include $(DEVKITPRO)/libnx/switch_rules
