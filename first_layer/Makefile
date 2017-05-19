MODULE_FILE := keydrown.ko

-include config.mk

# Android specific
ADB := adb
MODULE_UPLOAD_PATH := /data/local/tmp/${MODULE_FILE}

# Used time source
TIME_SOURCE ?= register

# Linux (default)
X86_ARCH ?= x86
X86_KDIR ?= /lib/modules/$(shell uname -r)/build
X86_PLUGINS ?= keyboard
X86_PLUGIN_KEYBOARD_DEVICE ?= 8042
X86_PLUGIN_KEYBOARD_GPIO ?= gpio-8042
X86_PLUGIN_KEYBOARD_INTERRUPT_SOFTWARE ?= 49
X86_PLUGIN_KEYBOARD_INTERRUPT_HARDWARE ?= 1

x86: ARCH := ${X86_ARCH}
x86: KDIR := ${X86_KDIR}
x86: PLUGINS := $(X86_PLUGINS)
x86: PLUGIN_KEYBOARD_DEVICE := $(X86_PLUGIN_KEYBOARD_DEVICE)
x86: PLUGIN_KEYBOARD_GPIO := $(X86_PLUGIN_KEYBOARD_GPIO)
x86: PLUGIN_KEYBOARD_INTERRUPT_SOFTWARE := $(X86_PLUGIN_KEYBOARD_INTERRUPT_SOFTWARE)
x86: PLUGIN_KEYBOARD_INTERRUPT_HARDWARE := $(X86_PLUGIN_KEYBOARD_INTERRUPT_HARDWARE)
x86-clean : ARCH := $(X86_ARCH)
x86-clean : PLUGINS := $(X86_PLUGINS)


# Nexus 5 (hammerhead)
HAMMERHEAD_ANDROID_ID := 03117d6fd023fa18
HAMMERHEAD_ARCH := arm
HAMMERHEAD_KDIR := ~/dev/kernel/nexus5_hammerhead
HAMMERHEAD_CCPATH := /opt/android-ndk/toolchains/arm-eabi-4.7/bin/arm-eabi-
HAMMERHEAD_TIME_SOURCE := monotonic
HAMMERHEAD_PLUGINS := touchscreen
HAMMERHEAD_PLUGIN_TOUCHSCREEN_DEVICE := s3350
HAMMERHEAD_PLUGIN_TOUCHSCREEN_MSM_GPIO := gpio-msm-v2
HAMMERHEAD_PLUGIN_TOUCHSCREEN_INTERRUPT_SOFTWARE := 362
HAMMERHEAD_PLUGIN_TOUCHSCREEN_INTERRUPT_HARDWARE := 5

hammerhead : ARCH := $(HAMMERHEAD_ARCH)
hammerhead : KDIR := $(HAMMERHEAD_KDIR)
hammerhead : CCPATH := $(HAMMERHEAD_CCPATH)
hammerhead : TIME_SOURCE := $(HAMMERHEAD_TIME_SOURCE)
hammerhead : PLUGINS := $(HAMMERHEAD_PLUGINS)
hammerhead : PLUGIN_TOUCHSCREEN_DEVICE := $(HAMMERHEAD_PLUGIN_TOUCHSCREEN_DEVICE)
hammerhead : PLUGIN_TOUCHSCREEN_MSM_GPIO := $(HAMMERHEAD_PLUGIN_TOUCHSCREEN_MSM_GPIO)
hammerhead : PLUGIN_TOUCHSCREEN_INTERRUPT_SOFTWARE := $(HAMMERHEAD_PLUGIN_TOUCHSCREEN_INTERRUPT_SOFTWARE)
hammerhead : PLUGIN_TOUCHSCREEN_INTERRUPT_HARDWARE := $(HAMMERHEAD_PLUGIN_TOUCHSCREEN_INTERRUPT_HARDWARE)
hammerhead-clean : ARCH := $(HAMMERHEAD_ARCH)
hammerhead-upload : ANDROID_ID := ${HAMMERHEAD_ANDROID_ID}

# Nexus 5 (oneplus3t)
ONEPLUS3T_ANDROID_ID ?= f9faafa0
ONEPLUS3T_ARCH ?= arm64
ONEPLUS3T_KDIR ?= ~/dev/kernel/android_kernel_oneplus_msm8996
ONEPLUS3T_CCPATH ?= /opt/android-ndk/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-
ONEPLUS3T_TIME_SOURCE ?= monotonic
ONEPLUS3T_PLUGINS ?= touchscreen
ONEPLUS3T_PLUGIN_TOUCHSCREEN_DEVICE ?= s3320
ONEPLUS3T_PLUGIN_TOUCHSCREEN_MSM_GPIO ?= gpio-msm-pinctrl
ONEPLUS3T_PLUGIN_TOUCHSCREEN_INTERRUPT_SOFTWARE ?= 413
ONEPLUS3T_PLUGIN_TOUCHSCREEN_INTERRUPT_HARDWARE ?= 125

oneplus3t : ARCH := $(ONEPLUS3T_ARCH)
oneplus3t : KDIR := $(ONEPLUS3T_KDIR)
oneplus3t : CCPATH := $(ONEPLUS3T_CCPATH)
oneplus3t : TIME_SOURCE := $(ONEPLUS3T_TIME_SOURCE)
oneplus3t : PLUGINS := $(ONEPLUS3T_PLUGINS)
oneplus3t : PLUGIN_TOUCHSCREEN_DEVICE := $(ONEPLUS3T_PLUGIN_TOUCHSCREEN_DEVICE)
oneplus3t : PLUGIN_TOUCHSCREEN_MSM_GPIO := $(ONEPLUS3T_PLUGIN_TOUCHSCREEN_MSM_GPIO)
oneplus3t : PLUGIN_TOUCHSCREEN_INTERRUPT_SOFTWARE := $(ONEPLUS3T_PLUGIN_TOUCHSCREEN_INTERRUPT_SOFTWARE)
oneplus3t : PLUGIN_TOUCHSCREEN_INTERRUPT_HARDWARE := $(ONEPLUS3T_PLUGIN_TOUCHSCREEN_INTERRUPT_HARDWARE)
oneplus3t-clean : ARCH := $(ONEPLUS3T_ARCH)
oneplus3t-upload : ANDROID_ID := ${ONEPLUS3T_ANDROID_ID}


# Kernel module
obj-m := keydrown.o

# Generic sources
keydrown_objs_generic := \
	source/plugin.o \
	source/hook.o \
	source/random.o \
	source/prefetch.o \
	source/timer.o \
	source/interrupts.o \
	main.o

# Timer depending sources
ifeq (${TIME_SOURCE}, register)
keydrown_objs_x86 += \
	source/x86/timing.o

keydrown_objs_arm += \
	source/arm/timing.o

keydrown_objs_arm64 += \
	source/arm64/timing.o
else ifeq (${TIME_SOURCE}, monotonic)
keydrown_objs_generic += \
	source/timing/monotonic.o
else
$(error "Unknown time source given.")
endif

# Plugin specific sources
ifneq (,$(findstring touchscreen, $(PLUGINS)))
keydrown_objs_plugin += \
	source/plugins/touchscreen/plugin.o \
	source/plugins/touchscreen/devices/$(PLUGIN_TOUCHSCREEN_DEVICE).o \
	source/plugins/touchscreen/gpio/${PLUGIN_TOUCHSCREEN_MSM_GPIO}.o

keydrown_cflags_plugin += \
	-DPLUGIN_TOUCHSCREEN_DEVICE=$(PLUGIN_TOUCHSCREEN_DEVICE) \
	-DPLUGIN_TOUCHSCREEN_INTERRUPT_SOFTWARE=$(PLUGIN_TOUCHSCREEN_INTERRUPT_SOFTWARE) \
	-DPLUGIN_TOUCHSCREEN_INTERRUPT_HARDWARE=$(PLUGIN_TOUCHSCREEN_INTERRUPT_HARDWARE)
endif

ifneq (,$(findstring keyboard, $(PLUGINS)))
keydrown_objs_plugin += \
	source/plugins/keyboard/plugin.o \
	source/plugins/keyboard/devices/$(PLUGIN_KEYBOARD_DEVICE).o \
	source/plugins/keyboard/gpio/${PLUGIN_KEYBOARD_GPIO}.o

keydrown_cflags_plugin += \
	-DPLUGIN_KEYBOARD_DEVICE=$(PLUGIN_KEYBOARD_DEVICE) \
	-DPLUGIN_KEYBOARD_INTERRUPT_SOFTWARE=$(PLUGIN_KEYBOARD_INTERRUPT_SOFTWARE) \
	-DPLUGIN_KEYBOARD_INTERRUPT_HARDWARE=$(PLUGIN_KEYBOARD_INTERRUPT_HARDWARE)
endif

# Module sources
keydrown-objs := $(keydrown_objs_generic) $(keydrown_objs_$(ARCH)) $(keydrown_objs_plugin)

# Additional flags
ccflags-y += \
	$(keydrown_cflags_plugin) \
	-I"$(PWD)/include" \
	-I"$(PWD)/include/$(ARCH)" \
	-I"$(PWD)/"

# Build targets
all: x86
x86: build
x86-clean: clean
hammerhead: build
hammerhead-clean: clean
hammerhead-upload: hammerhead upload
oneplus3t: build
oneplus3t-clean: clean
oneplus3t-upload: oneplus3t upload

build:
	@make \
		ARCH=$(ARCH) \
		CROSS_COMPILE=$(CCPATH) \
		TIME_SOURCE=$(TIME_SOURCE) \
		PLUGINS=$(PLUGINS) \
		PLUGIN_TOUCHSCREEN_DEVICE=$(PLUGIN_TOUCHSCREEN_DEVICE) \
		PLUGIN_TOUCHSCREEN_MSM_GPIO=$(PLUGIN_TOUCHSCREEN_MSM_GPIO) \
		PLUGIN_TOUCHSCREEN_INTERRUPT_HARDWARE=$(PLUGIN_TOUCHSCREEN_INTERRUPT_HARDWARE) \
		PLUGIN_TOUCHSCREEN_INTERRUPT_SOFTWARE=$(PLUGIN_TOUCHSCREEN_INTERRUPT_SOFTWARE) \
		PLUGIN_KEYBOARD_INTERRUPT_HARDWARE=$(PLUGIN_KEYBOARD_INTERRUPT_HARDWARE) \
		PLUGIN_KEYBOARD_INTERRUPT_SOFTWARE=$(PLUGIN_KEYBOARD_INTERRUPT_SOFTWARE) \
		PLUGIN_KEYBOARD_DEVICE=$(PLUGIN_KEYBOARD_DEVICE) \
		PLUGIN_KEYBOARD_GPIO=$(PLUGIN_KEYBOARD_GPIO) \
		-C $(KDIR) \
		M=$(PWD) \
		modules

upload:
	@${ADB} \
		-s ${ANDROID_ID} \
		push \
		${MODULE_FILE} \
		${MODULE_UPLOAD_PATH}

deb:
	@./makedeb.sh
	-if test `whoami` != "root"; then echo "\n\nYou need to run this target using fakeroot: fakeroot -u make deb\n"; exit 1; fi
	mkdir -pv keydrown1/etc/init.d
	mkdir -pv keydrown1/usr/share/doc/keydrown1/
	cp keydrown keydrown1/etc/init.d
	mkdir -p keydrown1/DEBIAN
	sed "s/%VERSION%/0.1/" docs/debian-control > keydrown1/DEBIAN/control
	echo "initial version" > keydrown1/usr/share/doc/keydrown1/changelog
	echo "" > keydrown1/usr/share/doc/keydrown1/copyright
	cp docs/postinst keydrown1/DEBIAN/
	chmod +x keydrown1/DEBIAN/postinst
	cp docs/postrm keydrown1/DEBIAN/
	chmod +x keydrown1/DEBIAN/postrm
	gzip -c -9 keydrown1/usr/share/doc/keydrown1/changelog > keydrown1/usr/share/doc/keydrown1/changelog.gz
	rm keydrown1/usr/share/doc/keydrown1/changelog
	chmod 0644 keydrown1/usr/share/doc/keydrown1/copyright
	chmod 0644 keydrown1/usr/share/doc/keydrown1/changelog.gz
	chmod -R 0755 keydrown1/usr
	chown -R root:root keydrown1/
	dpkg-deb --build keydrown1
	rm -rf keydrown1
	-lintian keydrown1.deb
	
install:
	cp keydrown.ko /lib/modules/$(shell uname -r)/kernel/drivers/input
	sed -i '/^keydrown$$/d' /etc/modules
	echo keydrown >> /etc/modules
	depmod
	update-rc.d keydrown remove
	cp keydrown /etc/init.d/
	chmod 755 /etc/init.d/keydrown
	update-rc.d keydrown enable

uninstall:
	-rmmod keydrown
	rm /lib/modules/$(shell uname -r)/kernel/drivers/input/keydrown.ko
	sed -i '/^keydrown$$/d' /etc/modules
	depmod
	rm /etc/init.d/keydrown
	update-rc.d keydrown remove

	
clean:
	@rm -rf \
		*.o \
		*.ko \
		*.mod.c \
		.*.cmd \
		modules.order \
		Module.symvers \
		.tmp_versions \
		$(keydrown_objs_generic) \
		$(keydrown_objs_x86) \
		$(keydrown_objs_arm) \
		$(keydrown-objs) \
		source/*.cmd \
		source/x86/*.cmd \
		source/arm/*.cmd
