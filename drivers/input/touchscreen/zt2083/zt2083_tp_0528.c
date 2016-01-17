/*
 * author: os-huangfujun
 * date:   2014-11-25
 * 
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/input.h>
#include <linux/cdev.h>
#include <linux/uaccess.h> /*copy_from_user */
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <mach/gpio.h>
#include <plat/board.h>
#include "zt2083_tp.h"

#define TP_SCREEN_MAX_X         800
#define TP_SCREEN_MAX_Y         480

#define TP_MIN_XC       0
#define TP_MAX_XC       0x3FFF//4095
#define TP_MIN_YC       0
#define TP_MAX_YC       0x3FFF//4095
#ifdef ZT2803
unsigned int cmd_x = 0x80;
unsigned int cmd_y = 0x90;
#else
unsigned int cmd_x = 0xc0;
unsigned int cmd_y = 0xd0;

#endif

static struct timer_list gtp_key_timer;

extern int ts_linear_scale(int *x, int *y, int swap_xy);

void cali_limit_bounder(int *keyintchx, int *keyintchy)
{
    // Calibration the value of X & Y
    ts_linear_scale(keyintchx, keyintchy, 0);
}

void platform_iic_reg_write(struct i2c_client *client, unsigned char cmd)
{
	struct i2c_msg msg;
	struct i2c_adapter *adapter = client->adapter;

	if (NULL==adapter){
		printk("%s: adapter==NULL\n", __FUNCTION__);
		return ;
	}

	msg.addr  = client->addr;
	msg.flags = 0x00;
	msg.len   = 1;
	msg.buf   = &cmd;
	msg.scl_rate = 100 * 1000;    // for Rockchip
	msg.udelay   = 5;

	i2c_transfer(adapter, &msg, 1);
}

int platform_iic_reg_read(struct i2c_client *client, unsigned char *buf)
{
	struct i2c_msg msg;
	unsigned int ret = -1;
	struct i2c_adapter *adapter = client->adapter;

	if (NULL==adapter){
		printk("%s: adapter==NULL\n", __FUNCTION__);
		return -1;
	}

	msg.addr  = client->addr;
	msg.flags = 0x01;
	msg.len   = 2;
	msg.buf   = buf;
	msg.scl_rate = 100 * 1000;    // for Rockchip
	msg.udelay   = 5;

	ret = i2c_transfer(adapter, &msg, 1);

	return ret;
}

#define FILTER_LIMIT 10
static int Touch_Pen_Filtering(int *px, int *py)
{
	int RetVal = 1;
	static int count = 0;
	static int x[2], y[2];
	int TmpX, TmpY;
	int dx, dy;

	count++;

	if (count > 2)
	{
		// apply filtering rule
		count = 2;

		// average between x,y[0] and *px,y
		TmpX = (x[0] + *px) / 2;
		TmpY = (y[0] + *py) / 2;

		// difference between x,y[1] and TmpX,Y
		dx = (x[1] > TmpX) ? (x[1] - TmpX) : (TmpX - x[1]);
		dy = (y[1] > TmpY) ? (y[1] - TmpY) : (TmpY - y[1]);

		if ((dx > FILTER_LIMIT) || (dy > FILTER_LIMIT))
		{

			// Invalid pen sample
			*px = x[1];
			*py = y[1]; // previous valid sample
			RetVal = 0;
			count = 0;

		}
		else
		{
			// Valid pen sample
			x[0] = x[1]; y[0] = y[1];
			x[1] = *px; y[1] = *py; // reserve pen samples
			RetVal = 1;
		}

	}
	else
	{ // till 2 samples, no filtering rule

		x[0] = x[1]; y[0] = y[1];
		x[1] = *px; y[1] = *py; // reserve pen samples

		RetVal = 0;	// <- TRUE jylee 2003.03.04
	}

	return RetVal;
}

static void key_timer(unsigned long data)
{
	struct zt2803_ts *zt2803_ts = (struct zt2803_ts *)data;
	input_report_key(zt2803_ts->input_dev, BTN_TOUCH, 0);
	input_report_abs(zt2803_ts->input_dev, ABS_PRESSURE, 0);
	input_sync(zt2803_ts->input_dev);
		
}


extern unsigned char RunSystemTouchBuf[7];

void report_value(struct zt2803_ts *zt2803_ts)
{
	int ret = 0;
	printk("1111--->  x: %-4d, y: %-4d \n", zt2803_ts->x_rp, zt2803_ts->y_rp);
	if(zt2803_ts->x_rp > 3970 || 
		zt2803_ts->x_rp  < 180 || 
		zt2803_ts->y_rp > 3800 || 
		zt2803_ts->y_rp < 290)
		return;
#if 0
	zt2803_ts->x_rp = zt2803_ts->x_rp - 180;
	zt2803_ts->x_rp = 800 * zt2803_ts->x_rp / 3790;

	zt2803_ts->y_rp = zt2803_ts->y_rp - 290;
	zt2803_ts->y_rp = 480 * zt2803_ts->y_rp / 3510;	
#else
        zt2803_ts->x_rp = zt2803_ts->x_rp - 50;
	zt2803_ts->x_rp = 800 * zt2803_ts->x_rp / 3920;

	zt2803_ts->y_rp = zt2803_ts->y_rp - 180;
	zt2803_ts->y_rp = 480 * zt2803_ts->y_rp / 3620;	
#endif

	if(memcmp(RunSystemTouchBuf,"620",3)==0)
	{
	if(zt2803_ts->x_rp<260)
	{
		zt2803_ts->x_rp = (zt2803_ts->x_rp * 1025)/1000;
		if(zt2803_ts->x_rp>260) zt2803_ts->x_rp = 260;

	}
	else if(zt2803_ts->x_rp<540)
	{
		zt2803_ts->x_rp = (zt2803_ts->x_rp * 1030)/1000;
		if(zt2803_ts->x_rp>540) zt2803_ts->x_rp = 540;

	}
	else
	{
		zt2803_ts->x_rp = (zt2803_ts->x_rp * 1035)/1000;
		if(zt2803_ts->x_rp>800) zt2803_ts->x_rp = 800;
	}

	if(zt2803_ts->y_rp<160)
	{
		zt2803_ts->y_rp = (zt2803_ts->y_rp * 1040)/1000;
		if(zt2803_ts->y_rp>160) zt2803_ts->y_rp = 160;

	}
	else if(zt2803_ts->y_rp<320)
	{
		zt2803_ts->y_rp = (zt2803_ts->y_rp * 1050)/1000;
		if(zt2803_ts->y_rp>320) zt2803_ts->y_rp = 320;

	}
	else
	{		
		zt2803_ts->y_rp = (zt2803_ts->y_rp * 1060)/1000;
		if(zt2803_ts->y_rp>480) zt2803_ts->y_rp = 480;

	}
	}

	ret = Touch_Pen_Filtering(&zt2803_ts->x_rp, &zt2803_ts->y_rp);
	if(!ret)
		return;

	printk("3333--->  x: %-4d, y: %-4d \n", zt2803_ts->x_rp, zt2803_ts->y_rp);
	//cali_limit_bounder(&zt2803_ts->x_rp, &zt2803_ts->y_rp);
	//printk("after cali ----->  x: %-4d, y: %-4d \n", zt2803_ts->x_rp, zt2803_ts->y_rp);
	input_report_abs(zt2803_ts->input_dev, ABS_X, zt2803_ts->x_rp);
	input_report_abs(zt2803_ts->input_dev, ABS_Y, zt2803_ts->y_rp);
	input_report_key(zt2803_ts->input_dev, BTN_TOUCH, 1);
	input_report_abs(zt2803_ts->input_dev, ABS_PRESSURE, 255);
	input_sync(zt2803_ts->input_dev);

/*
	switch(zt2803_ts->keystate)
	{
		case PEN_DOWN:
		case PEN_REPEAT:
			break;
		case PEN_UP:
			input_report_key(zt2803_ts->input_dev, BTN_TOUCH, 0);
			input_report_abs(zt2803_ts->input_dev, ABS_PRESSURE, 0);
			input_sync(zt2803_ts->input_dev);
			break;
		default:
			;
	}
*/

        mod_timer(&gtp_key_timer, jiffies + msecs_to_jiffies(35));
}

/*
 * sort val array from the min to the max,
 * cut off the min val and max val,
 * then average the last part,
 * return the average.
 */
int middle_value_fiter(int *val, int n)
{
	int i;
	int j;
	int temp    = 0;
	int ret_val = 0;

	for(i=1; i<n; i++)
	{
		for(j=0; j<i; j++)
		{
			if(val[j] > val[i])
			{
				temp   = val[j];
				val[j] = val[i];
				val[i] = temp;
			}
		}
	}


	if(n >= 3)
	{
		for(i=1; i<n-1; i++)
		{
			ret_val += val[i];
		}

		ret_val = ret_val / (n-2);
	}

	return ret_val;
}

int diff_val(int x1, int x2)
{
	int diff;

	diff = (x1>x2) ? (x1-x2) : (x2-x1);
	return diff;
}

void data_process_func(struct zt2803_ts *zt2803_ts, int data_x, int data_y)
{
	int i;
	int diffx;
	int diffy;
	int get_val = 0;
	static int pen_count     = 0;
	static int data_x_tmp_bk = 0;
	static int data_y_tmp_bk = 0;
	static int data_x_tmp[DATA_NUM] = {0};
	static int data_y_tmp[DATA_NUM] = {0};


	get_val = gpio_get_value(CTP_INT_PIN);
	if(0 == get_val)
	{
		if(pen_count >= DATA_NUM)
		{
			pen_count = 0;
		}
		else
		{
			data_x_tmp[pen_count] = data_x;
			data_y_tmp[pen_count] = data_y;
			pen_count++;
			return ;
		}
#if 0
		diffx = diff_val( (data_x_tmp[0]+data_x_tmp[2])/2, data_x_tmp[1] );
		diffy = diff_val( (data_y_tmp[0]+data_y_tmp[2])/2, data_y_tmp[1] );

		if(diffx > PEN_OFFSET || diffy > PEN_OFFSET)
		{
			data_x = data_x_tmp_bk;
			data_y = data_y_tmp_bk;
		}
		else if( diff_val(data_x_tmp_bk, data_x) < PEN_OFFSET>>1 ||
				 diff_val(data_y_tmp_bk, data_y) < PEN_OFFSET>>1 )
		{
			data_x = data_x_tmp_bk;
			data_y = data_y_tmp_bk;
		}
		else
		{
			data_x = data_x_tmp[1];
			data_y = data_y_tmp[1];
			data_x_tmp_bk = data_x;
			data_y_tmp_bk = data_y;
		}
#else
		data_x=middle_value_fiter(data_x_tmp,DATA_NUM);
		data_y=middle_value_fiter(data_y_tmp,DATA_NUM);
#endif
		zt2803_ts->keystate = PEN_REPEAT;
	}
	else
	{
		zt2803_ts->keystate = PEN_UP;
		zt2803_ts->use_irq = 0;
		pen_count = 0;

		for(i=0; i<DATA_NUM; i++)
		{
			data_x_tmp[i] = 0;
			data_y_tmp[i] = 0;
		}
	}

	zt2803_ts->x_rp = data_x;
	zt2803_ts->y_rp = data_y;

	report_value(zt2803_ts);
}

static void zt2803_ts_timer_func(unsigned long data)
{
	int ret    = 0;
	int temp   = 0;
	int data_x = 0;
	int data_y = 0;
	unsigned char buf_x[2];
	unsigned char buf_y[2];
	struct zt2803_ts *zt2803_ts = (struct zt2803_ts *)data;

	/* Activate x+ x-, and get x value */
	platform_iic_reg_write(zt2803_ts->client, cmd_x);
	ret = platform_iic_reg_read(zt2803_ts->client, buf_x);

	/* Activate y+ y-, and get y value */
	platform_iic_reg_write(zt2803_ts->client, cmd_y);
	ret = platform_iic_reg_read(zt2803_ts->client, buf_y);

	/* write back to init 0x00, or else can not generate irq pulse */
	platform_iic_reg_write(zt2803_ts->client, 0x00);

	/* merge data bytes read x and y analog value */
	data_x = (unsigned int)buf_x[0]<<4 | buf_x[1]>>4;
	data_y = (unsigned int)buf_y[0]<<4 | buf_y[1]>>4;

	if(data_x == 0 || data_y == 4095)
	{
		enable_irq(zt2803_ts->client->irq);
		return;
	}

	/* change between x and y */
	temp   = data_y;
	data_y = data_x;
	data_x = 4096 - temp;
#if 0
	printk("data_x:%-4d   ", data_x);
	printk("data_y:%-4d \n", data_y);
#endif

	data_process_func(zt2803_ts, data_x, data_y);

	if(zt2803_ts->use_irq == 1)
	{
		mod_timer(&zt2803_ts->timer, jiffies + msecs_to_jiffies(8) );
	}
	else
	{
		enable_irq(zt2803_ts->client->irq);
	}
}


static irqreturn_t zt2803_ts_irq_handler(int irq, void *dev_id)
{
	struct zt2803_ts *zt2803_ts = dev_id;
	disable_irq_nosync(zt2803_ts->client->irq);

	//queue_work(zt2803_ts->wq, &zt2803_ts->work);
	zt2803_ts->use_irq = 1;
	mod_timer(&zt2803_ts->timer, jiffies + msecs_to_jiffies(15) );

	return IRQ_HANDLED;
}

static int zt2083_tp_probe(struct i2c_client *client, const struct i2c_device_id *did)
{
	int ret = -1;
	struct zt2803_ts *zt2803_ts;


	printk(KERN_INFO "-----%s------ \n", __FUNCTION__);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
	{
		printk(KERN_ERR "I2C check functionality failed.\n");
		return -ENODEV;
	}

	zt2803_ts = kzalloc(sizeof(struct zt2803_ts), GFP_KERNEL);
	if (zt2803_ts == NULL)
	{
		printk(KERN_ERR "Alloc GFP_KERNEL memory failed.\n");
		return -ENOMEM;
	}
	memset(zt2803_ts, 0, sizeof(struct zt2803_ts));

	//zt2803_ts->wq = create_singlethread_workqueue("zt2803_ts_wq");
	//INIT_WORK(&zt2803_ts->work, zt2803_ts_work_func);
	zt2803_ts->client = client;
	spin_lock_init(&zt2803_ts->irq_lock);

	i2c_set_clientdata(client, zt2803_ts);

	ret = gpio_request(CTP_INT_PIN, "ZT2803_INT_PIN");
	if(ret < 0)
	{
		printk(KERN_ERR "CTP_INT_PIN request fail.\n");
		goto tp_gpio_request_fail;
	}
	gpio_direction_input(CTP_INT_PIN);
	gpio_pull_updown(CTP_INT_PIN, 0);
	zt2803_ts->irq = gpio_to_irq(CTP_INT_PIN);
	zt2803_ts->client->irq = zt2803_ts->irq;

	zt2803_ts->input_dev = input_allocate_device();
	if(zt2803_ts->input_dev == NULL)
	{
		printk(KERN_ERR "zt2803_ts allocate input device fail.\n");
		goto tp_allocate_input_fail;
	}

	zt2803_ts->input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) ;
	zt2803_ts->input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	__set_bit(INPUT_PROP_DIRECT, zt2803_ts->input_dev->propbit);

	input_set_abs_params(zt2803_ts->input_dev, ABS_X, 0, 800, 0, 0);
	input_set_abs_params(zt2803_ts->input_dev, ABS_Y, 0, 480, 0, 0);
	input_set_abs_params(zt2803_ts->input_dev, ABS_PRESSURE, 0, 255, 0, 0);
	input_set_abs_params(zt2803_ts->input_dev, ABS_TOOL_WIDTH, 0, 255, 0, 0);
	input_set_abs_params(zt2803_ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(zt2803_ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);

    	ret = input_register_device(zt2803_ts->input_dev);
	if(ret < 0)
	{
		printk(KERN_ERR "zt2803_ts input dev register fail.\n");
		goto tp_input_dev_register_fail;
	}

	ret  = request_irq(zt2803_ts->client->irq, 
			zt2803_ts_irq_handler,
			IRQ_TYPE_LEVEL_LOW,
			"zt2803_ts_irq",
			zt2803_ts);
	if(ret < 0)
	{
		printk(KERN_ERR "zt2803_ts request irq fail.\n");
		goto tp_irq_request_fail;
	}

	init_timer(&zt2803_ts->timer);    
	zt2803_ts->timer.expires = jiffies + msecs_to_jiffies(15);
	zt2803_ts->timer.function = &zt2803_ts_timer_func;
	zt2803_ts->timer.data = (unsigned long)zt2803_ts;

    	init_timer(&gtp_key_timer);
    	gtp_key_timer.expires = jiffies + 10;
    	gtp_key_timer.function = key_timer;
	gtp_key_timer.data = (unsigned long)zt2803_ts;

	printk(KERN_INFO "%s\n", __FUNCTION__);
	zt2083_sysdev_debug_init(&client->dev);

	return 0;

tp_irq_request_fail:
	input_unregister_device(zt2803_ts->input_dev);
tp_input_dev_register_fail:
	input_free_device(zt2803_ts->input_dev);
tp_allocate_input_fail:
	gpio_free(CTP_INT_PIN);
tp_gpio_request_fail:
	kfree(zt2803_ts);

	return ret;
}

static int zt2083_tp_remove(struct i2c_client *client)
{
	//printk(KERN_INFO "-----%s--d_wq2---- \n", __FUNCTION__);
	zt2083_sysdev_debug_exit(&client->dev);
	return 0;
}

static const struct i2c_device_id	zt2083_tp_id[] =
{
	{ "zt2083_tp", 0 },
	{  },
};

static struct i2c_driver	zt2083_tp_driver = 
{
	.driver   = 
	{
		.name = "zt2083_tp",
		.owner= THIS_MODULE,
	},
	.probe    = zt2083_tp_probe,
	.remove   = zt2083_tp_remove,
	.id_table = zt2083_tp_id,
};

static int __init zt2083_tp_module_init(void)
{
	//printk(KERN_INFO "-----%s------ \n", __FUNCTION__);
	return i2c_add_driver(&zt2083_tp_driver);
}

static void __exit zt2083_tp_module_exit(void)
{
	//printk(KERN_INFO "-----%s------ \n", __FUNCTION__);
	i2c_del_driver(&zt2083_tp_driver);
}

module_init(zt2083_tp_module_init);
module_exit(zt2083_tp_module_exit);

MODULE_DESCRIPTION("ZILLTEK ZT2083 i2c touch screen");
MODULE_AUTHOR("Alex <os-huangfujun@nowada.com>");
MODULE_LICENSE("GPL v2");

