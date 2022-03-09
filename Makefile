RACK_DIR ?= ../..

SOURCES += $(wildcard src/*.cpp)

DISTRIBUTABLES += res $(wildcard LICENSE*)

include $(RACK_DIR)/plugin.mk
