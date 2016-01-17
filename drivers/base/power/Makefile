obj-$(CONFIG_PM)	+= sysfs.o generic_ops.o
ifneq ($(CONFIG_PROJECT_D200),y)
obj-$(CONFIG_PM_SLEEP)	+= main.o wakeup.o
endif
ifeq  ($(CONFIG_PROJECT_D200),y)
obj-$(CONFIG_PM_SLEEP)	+= main_d200.o wakeup.o
endif
obj-$(CONFIG_PM_RUNTIME)	+= runtime.o
obj-$(CONFIG_PM_TRACE_RTC)	+= trace.o
obj-$(CONFIG_PM_OPP)	+= opp.o
obj-$(CONFIG_HAVE_CLK)	+= clock_ops.o

ccflags-$(CONFIG_DEBUG_DRIVER) := -DDEBUG
