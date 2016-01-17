 /* drivers/input/touchscreen/nt11003_touch.c
 *
 * Copyright (C) 2010 - 2011
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#if 1///GTP_ICS_SLOT_REPORT
    #include <linux/input/mt.h>
#endif
//#include <mach/gpio.h>
//#include <plat/gpio-cfg.h>
//#include <plat/gpio-bank-l.h>
//#include <plat/gpio-bank-f.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <../arch/arm/mach-tegra/gpio-names.h>

//#include <linux/syscalls.h>

//#include <linux/reboot.h>

#include <linux/proc_fs.h>
#include "nt11003.h"
#if 0
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/completion.h>
#include <asm/uaccess.h>
#endif
// ????
#define BABBAGE_NT11003_TS_RST1   RK30_PIN4_PC5			/***TEGRA_GPIO_PH6*/
#define BABBAGE_NT11003_TS_INT1    RK30_PIN4_PC2      /***TEGRA_GPIO_PH4 */

#define TP_DBG_NWD() printk("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++%d %s=======\n",__LINE__,__FUNCTION__)
#define INT_PORT BABBAGE_NT11003_TS_INT1
#define TS_INT gpio_to_irq(INT_PORT)
/*******************************************************	
Description:
	Read data from the i2c slave device;
	This operation consisted of 2 i2c_msgs,the first msg used
	to write the operate address,the second msg used to read data.

Parameter:
	client:	i2c device.
	buf[0]:operate address.
	buf[1]~buf[len]:read data buffer.
	len:operate length.
	
return:
	numbers of i2c_msgs to transfer
*********************************************************/
// ????
void hw_reset(void);
static int i2c_read_bytes(struct i2c_client *client, uint8_t *buf, int len)
{
	struct i2c_msg msgs[2];
	int ret=-1;
	int retries = 0;

	msgs[0].flags=!I2C_M_RD;
	msgs[0].addr=client->addr;
	msgs[0].len=1;
	msgs[0].buf=&buf[0];
  msgs[0].scl_rate = 400 * 1000; ///wj
  
	msgs[1].flags=I2C_M_RD;
	msgs[1].addr=client->addr;
	msgs[1].len=len-1;
	msgs[1].buf=&buf[1];
  msgs[1].scl_rate = 400 * 1000;
	while(retries<5)
	{
                                //***
		ret=i2c_transfer(client->adapter,msgs, 2);
		if(ret == 2)break;
		retries++;
	}
	return ret;
}

/*******************************************************	
Description:
	write data to the i2c slave device.

Parameter:
	client:	i2c device.
	buf[0]:operate address.
	buf[1]~buf[len]:write data buffer.
	len:operate length.
	
return:
	numbers of i2c_msgs to transfer.
*********************************************************/
static int i2c_write_bytes(struct i2c_client *client,uint8_t *data,int len)
{
	struct i2c_msg msg;
	int ret=-1;
	int retries = 0;

	msg.flags=!I2C_M_RD;
	msg.addr=client->addr;
	msg.len=len;
	msg.buf=data;		
	msg.scl_rate = 400 * 1000;
	while(retries<5)
	{
		ret=i2c_transfer(client->adapter,&msg, 1);
		if(ret == 1)break;
		retries++;
	}
	return ret;
}

/*******************************************************
Description:
	Novatek touchscreen initialize function.

Parameter:
	ts:	i2c client private struct.
	
return:
	Executive outcomes.0---succeed.
*******************************************************/
// ????
static int nt11003_init_panel(struct nt11003_ts_data *ts)
{
	int ret=-1;
	uint8_t rd_cfg_buf[7] = {0x78,};	//I2C buffer start address = 0x78
	                                  //Read data 1 = X_Resolution high, 
	                                  //Read data 2 = X_Resolution low,
                                    //Read data 3 = Y_Resolution high, 
                                    //Read data 4 = Y_Resolution low,
                                    //Read data 5 = Maximum point number,  
                                    //Read data 6 = Tritter type,
      struct i2c_client *client = ts->client;
									  
	ret=i2c_read_bytes(ts->client, rd_cfg_buf, 7);
	if(ret != 2)
	{
		dev_info(&client->dev, "Read resolution & max_touch_num failed, use default value!\n");
		ts->abs_x_max = 1024;///TOUCH_MAX_HEIGHT;
		ts->abs_y_max = 480;///TOUCH_MAX_WIDTH;
		ts->max_touch_num = MAX_FINGER_NUM;
		ts->int_trigger_type = INT_TRIGGER;
		return 0;
	}
	ts->abs_x_max = (rd_cfg_buf[1]<<8) + rd_cfg_buf[2];
	ts->abs_y_max = (rd_cfg_buf[3]<<8) + rd_cfg_buf[4];
		ts->abs_x_max = 1024;
		ts->abs_y_max = 480;	 
/*** wj add 
	ts->abs_x_max = 1024;///1280;
        ts->abs_y_max = 600;///488;///488;////600;///488;///800;*/

	ts->max_touch_num = rd_cfg_buf[5];
	dev_info(&client->dev, "ts->max_touch_num=%d\n",rd_cfg_buf[5]);
	ts->int_trigger_type = rd_cfg_buf[6]&0x03;
	dev_info(&client->dev, "ts_>int=0x%x\n",ts->int_trigger_type);
	if((!ts->abs_x_max)||(!ts->abs_y_max)||(!ts->max_touch_num))
	{
		dev_info(&client->dev, "Read invalid resolution & max_touch_num, use default value!\n");
		ts->abs_x_max = TOUCH_MAX_HEIGHT;
		ts->abs_y_max = TOUCH_MAX_WIDTH;
		ts->max_touch_num = MAX_FINGER_NUM;
	}
                // ????
	dev_info(&ts->client->dev,"X_MAX = %d,Y_MAX = %d,MAX_TOUCH_NUM = %d\n",ts->abs_x_max,ts->abs_y_max,ts->max_touch_num);
	//wake up mode from green mode
	rd_cfg_buf[0] = 0x7e;
	rd_cfg_buf[1] = 0x00;
	i2c_read_bytes(ts->client, rd_cfg_buf, 2);
	if((rd_cfg_buf[1]&0x0f)==0x05)
	{
		dev_info(&ts->client->dev, "Touchscreen works in INT wake up green mode!\n");
		ts->green_wake_mode = 1;
	}
	else
	{
		dev_info(&ts->client->dev, "Touchscreen works in IIC wake up green mode!\n");
		ts->green_wake_mode = 0;
	}

	msleep(10);
	return 0; 
}

/*******************************************************
Description:
	Read nt11003 touchscreen version function.

Parameter:
	ts:	i2c client private struct.
	
return:
	Executive outcomes.0---succeed.
*******************************************************/

/*******************************************************
Description:
	Novatek touchscreen work function.

Parameter:
	ts:	i2c client private struct.
	
return:
	Executive outcomes.0---succeed.
*******************************************************/
static void nt11003_ts_work_func(struct work_struct *work)
{	
	int ret=-1;
	int tmp = 0;
	// Support 10 points maximum
	uint8_t  point_data[61]={0};//[(1-READ_COOR_ADDR)+1+2+5*MAX_FINGER_NUM+1]={ 0 };  //read address(1byte)+key index(1byte)+point mask(2bytes)+5bytes*MAX_FINGER_NUM+coor checksum(1byte)
	uint8_t  check_sum = 0;
	uint16_t  finger_current = 0;
	uint16_t  finger_bit = 0;
	unsigned int  count = 0, point_count = 0;
	unsigned int position = 0;	
	uint8_t track_id[MAX_FINGER_NUM] = {0};
	unsigned int input_x = 0;
	unsigned int input_y = 0;
	unsigned int input_w = 0;
	unsigned char index = 0;
	unsigned char touch_num = 0,touch_num_make=0;
	
	struct nt11003_ts_data *ts = container_of(work, struct nt11003_ts_data, work);
      struct i2c_client *client = ts->client;
	//point_data[0] = READ_COOR_ADDR;		//read coor address
	ret=i2c_read_bytes(ts->client, point_data,  sizeof(point_data)/sizeof(point_data[0]));

    //***
  	touch_num =  ts->max_touch_num;
  	touch_num_make = ts->max_touch_num;
  	for(index = 0; index < touch_num; index++)
  	{
  		/// dev_info(&client->dev, "aaa point_data[%d]=0x%02X\n", index, point_data[index]);	
  		position = 1 + 6*index;
  		if(point_data[position]&0x3== 0x03)
  		  touch_num_make--;
  		  
	        ///dev_info(&client->dev, "aaa x-position point_data[%d]=0x%02X\n", index, point_data[position]);
	       /// dev_info(&client->dev, "aaa x-position point_data[%d]=0x%02X\n", position, point_data[position]);
	        ///dev_info(&client->dev, "aaa touch_num%d, touch_num_make%d\n", touch_num, touch_num_make);
  	}
  	
	if(touch_num_make > 0)
	{
		for(index=0; index<touch_num; index++)
		{
			position = 1 + 6*index;
			input_x = (unsigned int) (point_data[position+1]<<4) + (unsigned int)( point_data[position+3]>>4);
			input_y = (unsigned int)(point_data[position+2]<<4) + (unsigned int) (point_data[position+3]&0x0f);
			input_w =(unsigned int) (point_data[position+4])+127;		
			//input_x = input_x *SCREEN_MAX_HEIGHT/(TOUCH_MAX_HEIGHT);
			//input_y = input_y *SCREEN_MAX_WIDTH/(TOUCH_MAX_WIDTH);

			//if((input_x > ts->abs_x_max)||(input_y > ts->abs_y_max))continue;
			if(input_x > ts->abs_x_max)continue;

			input_x = 1024 - input_x;
    			input_y = input_y - 65;
    			input_y = input_y + (35 * input_y) / 445;
			input_y = 480 - input_y;

			//dev_info(&client->dev, "ts->abs_x_max: %d, ts->abs_y_max: %d, input_x = %d,input_y = %d, input_w = %d\n", ts->abs_x_max, ts->abs_y_max, input_x, input_y, input_w);
		/***	input_mt_slot(ts->input_dev, track_id[index]);///GTP_ICS_SLOT_REPORT
			input_report_abs(ts->input_dev, ABS_MT_POSITION_X, input_x);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, input_y);			
			input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, input_w);
			input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, input_w);
			input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, track_id[index]);

			input_mt_sync(ts->input_dev); */

 input_mt_slot(ts->input_dev, track_id[index]); 
///input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, false); ///?need to add ?,rk drv have  


    input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, track_id[index]);
    input_report_abs(ts->input_dev, ABS_MT_POSITION_X, input_x);
    input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, input_y);
    input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 128); ///.press
    input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 128);



		}
	}
	else
	{	/***	///input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, false);////
		input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
		input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
		dev_info(&client->dev, "relase event \n");
		input_mt_sync(ts->input_dev);*/

		  input_mt_slot(ts->input_dev, track_id[index]); ///	input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, false);  ?
    			input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, -1);
	}

	#ifdef HAVE_TOUCH_KEY
	dev_info(&client->dev, "HAVE KEY DOWN!0x%x\n",point_data[1]);
	for(count = 0; count < MAX_KEY_NUM; count++)
	{
		input_report_key(ts->input_dev, touch_key_array[count], !!(point_data[1]&(0x01<<count)));	
	}	   
	#endif
	input_sync(ts->input_dev);

#if defined(INT_PORT)
	if(ts->int_trigger_type> 1)
	{
		msleep(POLL_TIME);
		//goto COORDINATE_POLL;
	}
#endif
	goto END_WORK_FUNC;

NO_ACTION:	

#ifdef HAVE_TOUCH_KEY
	//dev_info(&client->dev, KERN_INFO"HAVE KEY DOWN!0x%x\n",point_data[1]);
	for(count = 0; count < MAX_KEY_NUM; count++)
	{
		input_report_key(ts->input_dev, touch_key_array[count], !!(point_data[1]&(0x01<<count)));	
	}
	input_sync(ts->input_dev);	   
#endif
END_WORK_FUNC:
XFER_ERROR:
	if(ts->use_irq)
		enable_irq(ts->client->irq);

}

/*******************************************************
Description:
	Timer interrupt service routine.

Parameter:
	timer:	timer struct pointer.
	
return:
	Timer work mode. HRTIMER_NORESTART---not restart mode
*******************************************************/
static enum hrtimer_restart nt11003_ts_timer_func(struct hrtimer *timer)
{
	struct nt11003_ts_data *ts = container_of(timer, struct nt11003_ts_data, timer);
	queue_work(nt11003_wq, &ts->work);
	hrtimer_start(&ts->timer, ktime_set(0, (POLL_TIME+6)*1000000), HRTIMER_MODE_REL);
	return HRTIMER_NORESTART;
}

/*******************************************************
Description:
	External interrupt service routine.

Parameter:
	irq:	interrupt number.
	dev_id: private data pointer.
	
return:
	irq execute status.
*******************************************************/
static irqreturn_t nt11003_ts_irq_handler(int irq, void *dev_id)
{
	struct nt11003_ts_data *ts = dev_id;
	disable_irq_nosync(ts->client->irq);
	queue_work(nt11003_wq, &ts->work);
	
	return IRQ_HANDLED;
}

/*******************************************************
Description:
	Novatek touchscreen power manage function.

Parameter:
	on:	power status.0---suspend;1---resume.
	
return:
	Executive outcomes.-1---i2c transfer error;0---succeed.
*******************************************************/
static int nt11003_ts_power(struct nt11003_ts_data * ts, int on)
{
	int ret = -1;
	unsigned char i2c_control_buf[2] = {255,  0};		//suspend cmd, (I2C buffer = 255, data = 0)
	int retry = 0;
	struct i2c_client *client = ts->client;
	if(on != 0 && on !=1)
	{
		dev_info(&client->dev, KERN_DEBUG "%s: Cant't support this command.", nt11003_ts_name);
		return -EINVAL;
	}
	
	if(ts != NULL && !ts->use_irq)
		return -2;
	
	if(on == 0)		//suspend
	{ 
		if(ts->green_wake_mode)
		{
			disable_irq(ts->client->irq);
			gpio_direction_output(INT_PORT, 1);
			msleep(5);
			//s3c_gpio_cfgpin(INT_PORT, INT_CFG);
			enable_irq(ts->client->irq);
		}
		while(retry<5)
		{
			ret = i2c_write_bytes(ts->client, i2c_control_buf, 2);
			if(ret == 1)
			{
				dev_info(&client->dev, "Send suspend cmd\n");
				break;
			}
			dev_info(&client->dev, "Send cmd failed!\n");
			retry++;
			msleep(10);
		}
		if(ret > 0)
			ret = 0;
	}
	else if(on == 1)		//resume
	{
		dev_info(&client->dev, KERN_INFO"Int resume\n");
		gpio_direction_output(INT_PORT, 0); 
		msleep(20);
		if(ts->use_irq) 
			gpio_direction_input(INT_PORT);
			//s3c_gpio_cfgpin(INT_PORT, INT_CFG);	//Set IO port as interrupt port	
		else 
			gpio_direction_input(INT_PORT);
			
		hw_reset();	
		msleep(400);

		ret = 0;
	}	 
	return ret;
}

/*******************************************************
Description:
	Novatek debug sysfs cat version function.

Parameter:
	standard sysfs show param.
	
return:
	Executive outcomes. 0---failed.
*******************************************************/

/*******************************************************
Description:
	Novatek debug sysfs cat resolution function.

Parameter:
	standard sysfs show param.
	
return:
	Executive outcomes. 0---failed.
*******************************************************/
static ssize_t nt11003_debug_resolution_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct nt11003_ts_data *ts;
	ts = i2c_get_clientdata(i2c_connect_client_nt11003);
	dev_info(&ts->client->dev,"ABS_X_MAX = %d,ABS_Y_MAX = %d\n",ts->abs_x_max,ts->abs_y_max);
	sprintf(buf,"ABS_X_MAX = %d,ABS_Y_MAX = %d\n",ts->abs_x_max,ts->abs_y_max);

	return strlen(buf);
}
/*******************************************************
Description:
	Novatek debug sysfs cat version function.

Parameter:
	standard sysfs show param.
	
return:
	Executive outcomes. 0---failed.
*******************************************************/
static ssize_t nt11003_debug_diffdata_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	//char diff_data[300];
	unsigned char diff_data[2241] = {00,};
	int ret = -1;
	char diff_data_cmd[2] = {80, 202};
	int i;
	int short_tmp;
	struct nt11003_ts_data *ts;

	disable_irq(TS_INT);
	
	ts = i2c_get_clientdata(i2c_connect_client_nt11003);
	//memset(diff_data, 0, sizeof(diff_data));
	if(ts->green_wake_mode)
	{
		//disable_irq(client->irq);
		gpio_direction_output(INT_PORT, 0);
		msleep(5);
		//zy--s3c_gpio_cfgpin(INT_PORT, INT_CFG);
		//enable_irq(client->irq);
	}
	ret = i2c_write_bytes(ts->client, diff_data_cmd, 2);
	if(ret != 1)
	{
		dev_info(&ts->client->dev, "Write diff data cmd failed!\n");
		enable_irq(TS_INT);
		return 0;
	}

	while(gpio_get_value(INT_PORT));
	ret = i2c_read_bytes(ts->client, diff_data, sizeof(diff_data));
	if(ret != 2)
	{
		dev_info(&ts->client->dev, "Read diff data failed!\n");
		enable_irq(TS_INT);
		return 0;
	}
	for(i=1; i<sizeof(diff_data); i+=2)
	{
		short_tmp = diff_data[i] + (diff_data[i+1]<<8);
		if(short_tmp&0x8000)
			short_tmp -= 65535;
		if(short_tmp == 512)continue;
		sprintf(buf+strlen(buf)," %d",short_tmp);
		//dev_info(&client->dev, " %d\n", short_tmp);
	}
	
	diff_data_cmd[1] = 0;
	ret = i2c_write_bytes(ts->client, diff_data_cmd, 2);
	if(ret != 1)
	{
		dev_info(&ts->client->dev, "Writ``e diff data cmd failed!\n");
		enable_irq(TS_INT);
		return 0;
	}
	enable_irq(TS_INT);
	/*for (i=0; i<1024; i++)
	{
 		sprintf(buf+strlen(buf)," %d",i);
	}*/
	
	return strlen(buf);
}


/*******************************************************
Description:
	Novatek debug sysfs echo calibration function.

Parameter:
	standard sysfs store param.
	
return:
	Executive outcomes..
*******************************************************/
static ssize_t nt11003_debug_calibration_store(struct device *dev,
			struct device_attribute *attr, const char *buf, ssize_t count)
{
	int ret = -1;
	char cal_cmd_buf[] = {110,1};
	struct nt11003_ts_data *ts;

	ts = i2c_get_clientdata(i2c_connect_client_nt11003);
	dev_info(&ts->client->dev,"Begin calibration......\n");
	if((*buf == 10)||(*buf == 49))
	{
		if(ts->green_wake_mode)
		{
                                                // ????
			disable_irq(ts->client->irq);
			gpio_direction_output(INT_PORT, 0);
			msleep(5);
			//zy--s3c_gpio_cfgpin(INT_PORT, INT_CFG);
			enable_irq(ts->client->irq);
		}
		ret = i2c_write_bytes(ts->client,cal_cmd_buf,2);
		if(ret!=1)
		{
			dev_info(&ts->client->dev,"Calibration failed!\n");
			return count;
		}
		else
		{
			dev_info(&ts->client->dev,"Calibration succeed!\n");
		}
	}
	return count;
}

void hw_reset(void)
{		int ret = 0;
		ret = gpio_request(BABBAGE_NT11003_TS_RST1, "K1103 WIFI pwr");
    		if(ret != 0)
   		 {
			gpio_free(BABBAGE_NT11003_TS_RST1);
			ret = gpio_request(BABBAGE_NT11003_TS_RST1, "wK1103 WIFI pwr");
			if(ret != 0)
			printk("gpio req error\n");
    		}

	gpio_direction_output(BABBAGE_NT11003_TS_RST1, 1);
	///gpio_set_value(BABBAGE_NT11003_TS_RST1, 0);
	msleep(10);
	//gpio_set_value(BABBAGE_NT11003_TS_RST1, 1);
	gpio_direction_output(BABBAGE_NT11003_TS_RST1, 1);
		gpio_set_value(BABBAGE_NT11003_TS_RST1, 1);

}

/*******************************************************
Description:
	Novatek touchscreen probe function.

Parameter:
	client:	i2c device struct.
	id:device id.
	
return:
	Executive outcomes. 0---succeed.
*******************************************************/
static int nt11003_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	int retry=0;
	struct nt11003_ts_data *ts;
	char *version_info = NULL;
	//zy---char test_data = 1;
	uint8_t test_data[7] = {0x00,};
	const char irq_table[4] = {IRQ_TYPE_EDGE_RISING,
							   IRQ_TYPE_EDGE_FALLING,
							   IRQ_TYPE_LEVEL_LOW,
							   IRQ_TYPE_LEVEL_HIGH};

	struct nt11003_i2c_rmi_platform_data *pdata;
	dev_info(&client->dev, "Install touch driver.\n");


	ret = gpio_request(BABBAGE_NT11003_TS_RST1, "K1103 WIFI pwr");
    		if(ret != 0)
   		 {
			gpio_free(BABBAGE_NT11003_TS_RST1);
			ret = gpio_request(BABBAGE_NT11003_TS_RST1, "wK1103 WIFI pwr");
			if(ret != 0)
			printk("gpio req error\n");
    		}

	ret = gpio_request(INT_PORT, "K1103 WIFI pwr2");
    		if(ret != 0)
   		 {
			gpio_free(INT_PORT);
			ret = gpio_request(INT_PORT, "wK1103 WIFI pwr2");
			if(ret != 0)
			printk("gpio req error\n");
    		}
		gpio_direction_input(INT_PORT);


	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
	{
		dev_info(&client->dev,  "Must have I2C_FUNC_I2C.\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}
	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (ts == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}
  
#ifdef INT_PORT	
	client->irq=TS_INT;
	if (client->irq)
	{
		//zy--ret = gpio_request(INT_PORT, "TS_INT");
		//gpio_direction_input(INT_PORT);//zy++
		if (ret < 0) 
		{
			dev_info(&client->dev, "Failed to request GPIO:%d, ERRNO:%d\n",(int)INT_PORT,ret);
			goto err_gpio_request_failed;
		}
		//zy--s3c_gpio_setpull(INT_PORT, S3C_GPIO_PULL_UP);
		//zy--s3c_gpio_cfgpin(INT_PORT, INT_CFG);
		dev_info(&client->dev, "ts->int_trigger_type=%d\n",ts->int_trigger_type);
		 ret  = request_irq(client->irq, nt11003_ts_irq_handler ,  irq_table[ts->int_trigger_type],
			client->name, ts);

		if (ret != 0) {
			dev_info(&client->dev, "Cannot allocate ts INT!ERRNO:%d\n", ret);
			gpio_direction_input(INT_PORT);
			gpio_free(INT_PORT);
			goto err_gpio_request_failed;
		}
		else 
		{	
      // ????    
			disable_irq(client->irq);
			ts->use_irq = 1;
			dev_info(&client->dev, "Reques EIRQ %d succesd on GPIO:%d\n",TS_INT,INT_PORT);
		}

		hw_reset();//zy+++++++++++debug     	
	}    //End of "if (client->irq)"
#endif

err_gpio_request_failed:

	i2c_connect_client_nt11003 = client;
	for(retry=0;retry < 30; retry++)
	{
		disable_irq(client->irq);
		//zy--gpio_direction_output(INT_PORT, 0);
		msleep(5);
		//zy--s3c_gpio_cfgpin(INT_PORT, INT_CFG);
		enable_irq(client->irq);
		ret =i2c_read_bytes(client, test_data, 5);
dev_info(&client->dev, "test_data[1]=%d,test_data[2]=%d,test_data[3]=%d,test_data[4]=%d,test_data[5]=%d\n",test_data[1],test_data[2],test_data[3],test_data[4],test_data[5]);
		if (ret > 0)
			break;
		dev_info(&client->dev, "nt11003 i2c test failed!\n");
	}
	if(ret <= 0)
	{
		dev_info(&client->dev,  "I2C communication ERROR!nt11003 touchscreen driver become invalid\n");
		goto err_i2c_failed;
	}	
	INIT_WORK(&ts->work, nt11003_ts_work_func);
	ts->client = client;
	i2c_set_clientdata(client, ts);
	pdata = client->dev.platform_data;
	
	ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) {
		ret = -ENOMEM;
		dev_info(&client->dev, "Failed to allocate input device\n");
		goto err_input_dev_alloc_failed;
	}
#if 1	
	for(retry=0; retry<3; retry++)
	{
		ret=nt11003_init_panel(ts);
		msleep(2);
		if(ret != 0)
			continue;
		else
			break;
	}
	if(ret != 0) {
		ts->bad_data=1;
		goto err_init_godix_ts;
	}
#endif
	ts->input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) ;
	__set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit);                  ///wj add  ,GTP_ICS_SLOT_REPORT
    input_mt_init_slots(ts->input_dev, 10);     // in case of "out of memory"  ///wj add
	///ts->input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	ts->input_dev->absbit[0] = BIT(ABS_X) | BIT(ABS_Y) | BIT(ABS_PRESSURE); 						// absolute coor (x,y)
#ifdef HAVE_TOUCH_KEY
	for(retry = 0; retry < MAX_KEY_NUM; retry++)
	{
		input_set_capability(ts->input_dev,EV_KEY,touch_key_array[retry]);	
	}
#endif


	dev_info(&client->dev, "ts->abs_x_max = %d, ts->abs_y_max = %d \n", ts->abs_x_max, ts->abs_y_max);

	input_set_abs_params(ts->input_dev, ABS_X, 0, ts->abs_x_max, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_Y, 0, ts->abs_y_max, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_PRESSURE, 0, 255, 0, 0);

#ifdef NT11003_MULTI_TOUCH

	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, ts->abs_x_max, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, ts->abs_y_max, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, ts->abs_x_max, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, ts->abs_y_max, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, ts->max_touch_num, 0, 0);
#endif	
                // ????
	sprintf(ts->phys, "input/ts");
	ts->input_dev->name = nt11003_ts_name;
	ts->input_dev->phys = ts->phys;
	ts->input_dev->id.bustype = BUS_I2C;
	ts->input_dev->id.vendor = 0xDEAD;
	ts->input_dev->id.product = 0xBEEF;
	ts->input_dev->id.version = 10427;	//screen firmware version
	
	ret = input_register_device(ts->input_dev);
	if (ret) {
		dev_info(&client->dev, "Probe: Unable to register %s input device\n", ts->input_dev->name);
		goto err_input_register_device_failed;
	}
	ts->bad_data = 0;
		
	if (!ts->use_irq) 
	{
		hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		ts->timer.function = nt11003_ts_timer_func;
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	}
	if(ts->use_irq)
		enable_irq(client->irq);
#if 0
#if defined(INT_PORT)
	if(ts->use_irq) 		
		ts->power = nt11003_ts_power;
#endif	
//zy--
	ret = nt11003_read_version(ts, &version_info);
	if(ret <= 0)
	{
		dev_info(&client->dev, "Read version data failed!\n");
	}
	else
	{
		dev_info(&client->dev, "Novatek TouchScreen Version:%s\n", (version_info+1));
	}
	vfree(version_info);
#endif	
#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = nt11003_ts_early_suspend;
	ts->early_suspend.resume = nt11003_ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif
#if 0
#ifdef CONFIG_TOUCHSCREEN_NT11003_IAP
	nt11003_proc_entry = create_proc_entry("nt11003-update", 0666, NULL);
	if(nt11003_proc_entry == NULL)
	{
		dev_info(&client->dev, "Couldn't create proc entry!\n");
		ret = -ENOMEM;
		goto err_create_proc_entry;
	}
	else
	{
		dev_info(&client->dev, "Create proc entry success!\n");
		nt11003_proc_entry->write_proc = nt11003_update_write;
		nt11003_proc_entry->read_proc = nt11003_update_read;
		//zy--nt11003_proc_entry->owner =THIS_MODULE;
	}
#endif
	nt11003_debug_sysfs_init();
#endif
	dev_info(&client->dev, "Start %s in %s mode\n", 
		ts->input_dev->name, ts->use_irq ? "interrupt" : "polling");
	dev_info(&client->dev,  "Driver Modify Date:2011-06-27\n");
	return 0;

err_init_godix_ts:
	if(ts->use_irq)
	{
		ts->use_irq = 0;
		free_irq(client->irq,ts);
	#ifdef INT_PORT	
		gpio_direction_input(INT_PORT);
		gpio_free(INT_PORT);
	#endif	
	}
	else 
		hrtimer_cancel(&ts->timer);

err_input_register_device_failed:
	input_free_device(ts->input_dev);

err_input_dev_alloc_failed:
	i2c_set_clientdata(client, NULL);
err_i2c_failed:	
	kfree(ts);
err_alloc_data_failed:
err_check_functionality_failed:
err_create_proc_entry:
	return ret;
}


/*******************************************************
Description:
	Novatek touchscreen driver release function.

Parameter:
	client:	i2c device struct.
	
return:
	Executive outcomes. 0---succeed.
*******************************************************/
static int nt11003_ts_remove(struct i2c_client *client)
{
	struct nt11003_ts_data *ts = i2c_get_clientdata(client);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&ts->early_suspend);
#endif
#ifdef CONFIG_TOUCHSCREEN_NT11003_IAP
	remove_proc_entry("nt11003-update", NULL);
#endif
	//nt11003_debug_sysfs_deinit();
	if (ts && ts->use_irq) 
	{
	#ifdef INT_PORT
		gpio_direction_input(INT_PORT);
		gpio_free(INT_PORT);
	#endif	
		free_irq(client->irq, ts);
	}	
	else if(ts)
		hrtimer_cancel(&ts->timer);
	
	dev_notice(&client->dev,"The driver is removing...\n");
	i2c_set_clientdata(client, NULL);
	input_unregister_device(ts->input_dev);
	kfree(ts);
	return 0;
}

static int nt11003_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret;
	struct nt11003_ts_data *ts = i2c_get_clientdata(client);

	if (ts->use_irq)
		disable_irq(client->irq);
	 else
		hrtimer_cancel(&ts->timer);
	  //ret = cancel_work_sync(&ts->work);
	  //if(ret && ts->use_irq)	
		//enable_irq(client->irq);
	if (ts->power) {
		ret = ts->power(ts, 0);
		if (ret < 0)
			dev_info(&client->dev, KERN_ERR "nt11003_ts_resume power off failed\n");
	}
	return 0;
}

static int nt11003_ts_resume(struct i2c_client *client)
{
	int ret;
	struct nt11003_ts_data *ts = i2c_get_clientdata(client);

	if (ts->power) {
		ret = ts->power(ts, 1);
		if (ret < 0)
			dev_info(&client->dev, KERN_ERR "nt11003_ts_resume power on failed\n");
	}

	if (ts->use_irq)
		enable_irq(client->irq);
	 else
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void nt11003_ts_early_suspend(struct early_suspend *h)
{
	struct nt11003_ts_data *ts;
	ts = container_of(h, struct nt11003_ts_data, early_suspend);
	nt11003_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void nt11003_ts_late_resume(struct early_suspend *h)
{
	struct nt11003_ts_data *ts;
	ts = container_of(h, struct nt11003_ts_data, early_suspend);
	nt11003_ts_resume(ts->client);
}
#endif

static const struct i2c_device_id nt11003_ts_id[] = {
	{ NT11003_I2C_NAME, 0 },
	{ }
};

static struct i2c_driver nt11003_ts_driver = {
	.probe		= nt11003_ts_probe,
	.remove		= nt11003_ts_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= nt11003_ts_suspend,
	.resume		= nt11003_ts_resume,
#endif
	.id_table	= nt11003_ts_id,
	.driver = {
		.name	= NT11003_I2C_NAME,
		.owner = THIS_MODULE,
	},
};

/*******************************************************	
Description:
	Driver Install function.
return:
	Executive Outcomes. 0---succeed.
********************************************************/
static int __devinit nt11003_ts_init(void)
{
	int ret;
	
	nt11003_wq = create_workqueue("nt11003_wq");		//create a work queue and worker thread
	if (!nt11003_wq) {
		printk(KERN_ALERT "creat workqueue faiked\n");
		return -ENOMEM;
		
	}
	ret=i2c_add_driver(&nt11003_ts_driver);
	return ret; 
}

/*******************************************************	
Description:
	Driver uninstall function.
return:
	Executive Outcomes. 0---succeed.
********************************************************/
static void __exit nt11003_ts_exit(void)
{
	printk(KERN_ALERT "Touchscreen driver of guitar exited.\n");
	i2c_del_driver(&nt11003_ts_driver);
	if (nt11003_wq)
		destroy_workqueue(nt11003_wq);		//release our work queue
}

late_initcall(nt11003_ts_init);
module_exit(nt11003_ts_exit);

MODULE_DESCRIPTION("Novatek Touchscreen Driver");
MODULE_LICENSE("GPL");
