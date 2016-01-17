#ifndef __ZT2083_TP_H__
#define __ZT2083_TP_H__


#define CTP_INT_PIN		RK30_PIN4_PC2
#define PEN_OFFSET		30
#define DATA_NUM		4

enum {PEN_DOWN=0, PEN_UP, PEN_REPEAT};

struct zt2803_ts 
{
	struct i2c_client *client;
	struct input_dev  *input_dev;
	struct work_struct  work;
	struct timer_list timer;
	struct workqueue_struct *wq;

	unsigned int x_rp;
	unsigned int y_rp;
	unsigned int abs_x_max;
	unsigned int abs_y_max;
	int irq;
	spinlock_t irq_lock;
	int use_irq;

	int keystate;
};

int zt2083_sysdev_debug_init(struct device *zt2083_dev);
void zt2083_sysdev_debug_exit(struct device *zt2083_dev);
#endif //__ZT2083_TP_H__
