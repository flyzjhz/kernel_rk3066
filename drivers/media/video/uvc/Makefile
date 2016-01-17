#TARGET=sonix_uvcvideo
#KDIR:=/lib/modules/$(shell uname -r)/build
#PWD:=$(shell pwd)

#CCFLAGS += -I/usr/include/

#$(TARGET)-objs=uvc_driver.o uvc_queue.o uvc_v4l2.o uvc_video.o uvc_ctrl.o \
		  uvc_status.o uvc_isight.o nalu.o
#obj-m:=$(TARGET).o

#default:
#	make $(CCFLAGS) -C $(KDIR) M=$(PWD) modules
#clean:
#	rm -rf *.o *.ko *.mod.* modules.* Module.* .tmp_versions .*.*.cmd

uvcvideo-objs  := uvc_driver.o uvc_queue.o uvc_v4l2.o uvc_video.o uvc_ctrl.o \
          uvc_status.o uvc_isight.o nalu.o
#uvc_driver.o uvc_queue.o uvc_v4l2.o uvc_video.o uvc_ctrl.o \
#          uvc_status.o uvc_isight.o
ifeq ($(CONFIG_MEDIA_CONTROLLER),y)
uvcvideo-objs  += uvc_entity.o
endif
obj-$(CONFIG_USB_VIDEO_CLASS) += uvcvideo.o

