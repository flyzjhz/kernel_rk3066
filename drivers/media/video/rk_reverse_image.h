#ifndef SOC_REARVIEW_H
#define SOC_REARVIEW_H

struct picsize {
	unsigned int width;
	unsigned int height;
};

extern struct soc_camera_device *reverse_icd;
extern struct rk_camera_dev *reverse_pcdev;

int rk_reverse_image_main(void *arg);

#endif
