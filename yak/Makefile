# SPDX-License-Identifier: BSD-3-Clause
# Copyright(c) 2010-2014 Intel Corporation

# binary name
APP = out

# all source are stored in SRCS-y
SRCS-y := main.c tap.c port.c

# Build using pkg-config variables if possible
$(shell pkg-config --exists libdpdk)
ifeq ($(.SHELLSTATUS),0)

all: shared
.PHONY: shared static
shared: build/$(APP)-shared
	ln -sf $(APP)-shared build/$(APP)
static: build/$(APP)-static
	ln -sf $(APP)-static build/$(APP)

PC_FILE := $(shell pkg-config --path libdpdk)
CFLAGS += -O3 $(shell pkg-config --cflags libdpdk)
LDFLAGS_SHARED = $(shell pkg-config --libs libdpdk)
LDFLAGS_STATIC = -Wl,-Bstatic $(shell pkg-config --static --libs libdpdk)

build/$(APP)-shared: $(SRCS-y) Makefile $(PC_FILE) | build
	$(CC) $(CFLAGS) $(SRCS-y) -o $@ $(LDFLAGS) $(LDFLAGS_SHARED)

build/$(APP)-static: $(SRCS-y) Makefile $(PC_FILE) | build
	$(CC) $(CFLAGS) $(SRCS-y) -o $@ $(LDFLAGS) $(LDFLAGS_STATIC)

build:
	@mkdir -p $@

.PHONY: clean
clean:
	rm -f build/$(APP) build/$(APP)-static build/$(APP)-shared
	rmdir --ignore-fail-on-non-empty build

else # Build using legacy build system

ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

# Default target, can be overridden by command line or environment
RTE_TARGET ?= x86_64-native-linuxapp-gcc

include $(RTE_SDK)/mk/rte.vars.mk

CFLAGS += $(WERROR_FLAGS)

# workaround for a gcc bug with noreturn attribute
# http://gcc.gnu.org/bugzilla/show_bug.cgi?id=12603
ifeq ($(CONFIG_RTE_TOOLCHAIN_GCC),y)
CFLAGS_main.o += -Wno-return-type
endif

EXTRA_CFLAGS += -O3 -g -Wfatal-errors

include $(RTE_SDK)/mk/rte.extapp.mk
endif

run:
	sudo ./build/out \
		--vdev=net_tap0,iface=dpdk0

netns:
	sudo ip link set ge-0-0-0 up
	sudo ip addr add 10.0.0.1/24 dev ge-0-0-0
	sudo ip link add link ge-0-0-0 name ge-0-0-0.10 type vlan id 10
	sudo ip link set ge-0-0-0.10 up
	sudo ip addr add 10.0.10.1/24 dev ge-0-0-0.10
	sudo ip link set dpdk0 netns ns0
	sudo ip netns exec ns0 ip link set dpdk0 up
	sudo ip netns exec ns0 ip addr add 10.0.0.2/24 dev dpdk0
	sudo ip netns exec ns0 ip link add link dpdk0 name dpdk0.10 type vlan id 10
	sudo ip netns exec ns0 ip link set dpdk0.10 up
	sudo ip netns exec ns0 ip addr add 10.0.10.2/24 dev dpdk0.10

