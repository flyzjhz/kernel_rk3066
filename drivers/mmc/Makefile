#
# Makefile for the kernel mmc device drivers.
#

subdir-ccflags-$(CONFIG_MMC_DEBUG) := -DDEBUG

obj-$(CONFIG_MMC)		+= core/
obj-$(CONFIG_MMC)		+= card/
ifneq ($(CONFIG_PROJECT_D200),y)
obj-$(CONFIG_MMC)		+= host/
endif

ifeq ($(CONFIG_PROJECT_D200),y)
obj-$(CONFIG_MMC)		+= host_d200/
endif
