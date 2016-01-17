/*
 *  Touchscreen Linear Scale Adaptor
 *
 *
 * This library is licensed under GPL.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/sysctl.h>
#include <asm/system.h>
#include <mach/hardware.h>

/*
 * sysctl-tuning infrastructure.
 * a[7] use to identify calibrate mode
 */
static struct ts_calibration {
/* Linear scaling and offset parameters for x,y (can include rotation) */
    int a[8];
} cal;

static ctl_table ts_proc_calibration_table[] = {
    {
     .procname = "a0",
     .data = &cal.a[0],
     .maxlen = sizeof(int),
     .mode = 0666,
     .proc_handler = &proc_dointvec,
     },
    {
     .procname = "a1",
     .data = &cal.a[1],
     .maxlen = sizeof(int),
     .mode = 0666,
     .proc_handler = &proc_dointvec,
     },
    {
     .procname = "a2",
     .data = &cal.a[2],
     .maxlen = sizeof(int),
     .mode = 0666,
     .proc_handler = &proc_dointvec,
     },
    {
     .procname = "a3",
     .data = &cal.a[3],
     .maxlen = sizeof(int),
     .mode = 0666,
     .proc_handler = &proc_dointvec,
     },
    {
     .procname = "a4",
     .data = &cal.a[4],
     .maxlen = sizeof(int),
     .mode = 0666,
     .proc_handler = &proc_dointvec,
     },
    {
     .procname = "a5",
     .data = &cal.a[5],
     .maxlen = sizeof(int),
     .mode = 0666,
     .proc_handler = &proc_dointvec,
     },
    {
     .procname = "a6",
     .data = &cal.a[6],
     .maxlen = sizeof(int),
     .mode = 0666,
     .proc_handler = &proc_dointvec,
     },
    {
     .procname = "a7",
     .data = &cal.a[7],
     .maxlen = sizeof(int),
     .mode = 0666,
     .proc_handler = &proc_dointvec,
     },
    {}
};

static ctl_table ts_proc_root[] = {
    {
     .procname = "ts_device",
     .mode = 0555,
     .child = ts_proc_calibration_table,
     },
    {}
};

static ctl_table ts_dev_root[] = {
    {
     .procname = "dev",
     .mode = 0555,
     .child = ts_proc_root,
     },
    {}
};

static struct ctl_table_header *ts_sysctl_header;

int ts_linear_cal_mode(void)
{
    return cal.a[7];
}

int ts_linear_scale(int *x, int *y, int swap_xy)
{
    int xtemp, ytemp;

    /* return in calibration mode */
    if (cal.a[7] == 1) {
        if (swap_xy) {
            int tmp = *x;
            *x = *y;
            *y = tmp;
        }
        return 0;
    }

    xtemp = *x;
    ytemp = *y;

    if (cal.a[6] == 0)
        return -EINVAL;

    *x = (cal.a[2] + cal.a[0] * xtemp + cal.a[1] * ytemp) / cal.a[6];
    *y = (cal.a[5] + cal.a[3] * xtemp + cal.a[4] * ytemp) / cal.a[6];


    //*x = *x * 0x3FFF / 0x320;
    //*y = *y * 0x3FFF / 0x1E0; //by bruce

    if (swap_xy) {
        int tmp = *x;
        *x = *y;
        *y = tmp;
    }

    return 0;
}
EXPORT_SYMBOL(ts_linear_scale);

static int __init ts_linear_init(void)
{
    ts_sysctl_header = register_sysctl_table(ts_dev_root);
    /* Use default values that leave ts numbers
       unchanged after transform */

	cal.a[0] = 1;
	cal.a[1] = 0;
	cal.a[2] = 0;
	cal.a[3] = 0;
	cal.a[4] = 1;
	cal.a[5] = 0;
	cal.a[6] = 1;

	return 0;
}

static void __exit ts_linear_cleanup(void)
{
    unregister_sysctl_table(ts_sysctl_header);
}

module_init(ts_linear_init);
module_exit(ts_linear_cleanup);

MODULE_DESCRIPTION("touch screen linear scaling driver");
MODULE_LICENSE("GPL");
