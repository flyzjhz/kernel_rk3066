#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/workqueue.h>
#include "zt2083_tp.h"


extern unsigned int cmd_x;

static struct class *ts_class = NULL;
static int calibration_time =15;
static unsigned char calibration_flag = 0;


static ssize_t touch_mode_show(struct class *cls,struct class_attribute *attr, char *_buf)
{
	printk(">>>>>>>added by liqing touch_mode_show!\n");
	if(calibration_flag == 1)
	{
		//calibration_flag = 0;
		//enable_irq(irq_flag);
		printk("hjc----return successful\n");
		//return sprintf(_buf,"failed");//for test
		return sprintf(_buf,"successful");      
	}
	else
	{
		calibration_flag = 0;
		if(--calibration_time <= 0){
			calibration_time=15;
			return sprintf(_buf,"successful");
		}else{
			if(calibration_time>16)
				calibration_time=15;
			calibration_flag = 1;
			printk("return calibration\n");
			return sprintf(_buf,"calibration");
		}
	}                               
}

static ssize_t touch_mode_store(struct class *cls,struct class_attribute *attr, char *_buf,size_t _count)
{
	int ret = 0;
	//struct goodix_ts_data *ts;
	//char cal_cmd_buf[] = {110,1};
	calibration_flag = 0;
	//ts = i2c_get_clientdata(i2c_connect_client);
	//if(!strncmp(_buf,"tp_cal" , strlen("tp_cal")))//hjc for test 
	{
		printk("TP Calibration is start!!! \n");
		//ret = i2c_write_bytes(ts->client,cal_cmd_buf,2);
		//if(ret!=1)
		{
			mdelay(5000);
			calibration_flag = 1;   
			//dev_info(&ts->client->dev,"Calibration failed!\n");
		}
		//else
		//{
		//calibration_flag = 1; 
		//dev_info(&ts->client->dev,"Calibration succeed!\n");
		//}
		//disable_irq(irq_flag);
		//touchplus_calibrate( g_tsd->client);       //hjc 02-10
	}
	return _count;
	//return 0;
}   

ssize_t cmdval_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "0x%x \n", cmd_x);
}

ssize_t cmdval_store(struct device *dev, struct device_attribute *attr, const char * buf, size_t count)
{
	unsigned int val ;

	sscanf(buf, "0x%x", &val);
	cmd_x = val;
	//printk(KERN_INFO "cmdval\n");

	return strnlen(buf, 4096);
}

static DEVICE_ATTR(cmdval, 0644, cmdval_show, cmdval_store);
static CLASS_ATTR(touchcalibration, 0666, touch_mode_show, touch_mode_store);

static struct attribute *zt2083_attrs[] = {
	&dev_attr_cmdval.attr, 
	NULL 
};

static struct attribute_group zt2083_group = {
	.name = NULL,
	.attrs = zt2083_attrs,
};

int zt2083_sysdev_debug_init(struct device *zt2083_dev)
{
	int retval;

	/* add sys debug */
	retval = sysfs_create_group(&zt2083_dev->kobj, &zt2083_group);
	if(retval) 
	{
		printk(KERN_ERR "Can't create sysfs attrs @%x\n", retval);
		return retval;
	}

	ts_class = class_create(THIS_MODULE, "ts_class");
   	retval =  class_create_file(ts_class, &class_attr_touchcalibration);
    if(retval)
    {
       printk("Fail to creat class touchcalibration.\n");
    }

	return 0;
}

void zt2083_sysdev_debug_exit(struct device *zt2083_dev)
{
	sysfs_remove_group(&zt2083_dev->kobj, &zt2083_group);
	class_destroy(ts_class);
	class_remove_file(ts_class, &class_attr_touchcalibration);
}

