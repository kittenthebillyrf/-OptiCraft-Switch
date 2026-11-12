# Adapted from the official devkitPro/libnx application template.
include $(DEVKITPRO)/libnx/switch_rules

TARGET := OptiCraft
BUILD := build
SOURCES := switch_nro
DATA :=
INCLUDES :=
ARCH := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE
CFLAGS := -g -Wall -Wextra -O2 -ffunction-sections $(ARCH)
CXXFLAGS := $(CFLAGS) -std=gnu++17 -fno-rtti -fno-exceptions
ASFLAGS := -g $(ARCH)
LDFLAGS = -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,--gc-sections
LIBS := -lnx
LIBDIRS := $(PORTLIBS) $(LIBNX)

APP_TITLE := OptiCraft World Prototype
APP_AUTHOR := OptiCraft Switch Port
APP_VERSION := 0.1.0
NO_ICON := 1
