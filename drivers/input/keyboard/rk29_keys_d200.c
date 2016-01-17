/*
 * Driver for keys on GPIO lines capable of generating interrupts.
 *
 * Copyright 2005 Phil Blundell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/adc.h>
#include <linux/earlysuspend.h>
#include <asm/gpio.h>
#include <mach/board.h>
#include <plat/key.h>


#include <mach/pmu.h>
#include <mach/board.h>
#include <mach/system.h>
#include <mach/sram.h>
#include <mach/gpio.h>
#include <mach/iomux.h>
#include <mach/cru.h>
#include <mach/ddr.h>
#include <mach/debug_uart.h>
#include <plat/efuse.h>
#include <plat/cpu.h>

#include <mach/board.h>
#include <mach/hardware.h>
#include <mach/io.h>
#include <mach/gpio.h>
#include <mach/iomux.h>

struct early_suspend  gpio_sus;

#define grf_readl(offset)	readl_relaxed(RK30_GRF_BASE + offset)
#define grf_writel(v, offset)	do { writel_relaxed(v, RK30_GRF_BASE + offset); dsb(); } while (0)

#define EMPTY_ADVALUE					950
#define DRIFT_ADVALUE					70
#define INVALID_ADVALUE 				-1
#define EV_MENU					KEY_MENU ///KEY_F1
#define KEY_LONG_PRESS 0x251
#define KEY_APP_NWD  253 ///hbs indoor tast key   ,
#if 0
#define key_dbg(bdata, format, arg...)		\
	dev_printk(KERN_INFO , &bdata->input->dev , format , ## arg)
#else
#define key_dbg(bdata, format, arg...)	
#endif

struct rk29_button_data {
	int state;
	int long_press_count;
	struct rk29_keys_button *button;
	struct input_dev *input;
	struct timer_list timer;
        struct rk29_keys_drvdata *ddata;
};

struct rk29_keys_drvdata {
	int nbuttons;
	int result;
	bool in_suspend;	/* Flag to indicate if we're suspending/resuming */
	struct input_dev *input;
	struct adc_client *client;
	struct timer_list timer;
	struct rk29_button_data data[0];
};

static struct input_dev *input_dev;
struct rk29_keys_Arrary {
	char keyArrary[20];
};

static ssize_t rk29key_set(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct rk29_keys_platform_data *pdata = dev_get_platdata(dev);
	int i,j,start,end;
	char rk29keyArrary[400];
	struct rk29_keys_Arrary Arrary[]={
            /*    {
                        .keyArrary = {"menu"},
                },
                {
                        .keyArrary = {"home"},
                },
                {
                        .keyArrary = {"home"},
                },*/
           /*     {
                        .keyArrary = {"sensor"},
                },
                {
                        .keyArrary = {"play"},
                },
                {
                        .keyArrary = {"vol+"},
                },
                {
                        .keyArrary = {"vol-"},
                },*/
    		{
                        .keyArrary = {"play"},
                },
        }; 
	char *p;
	  
	for(i=0;i<1;i++)
	{
		
		p = strstr(buf,Arrary[i].keyArrary);
		if(p==0)
              {
                   dev_dbg(dev," rk29key_set p == 0 error ...............\n");
                   continue;
              }
		start = strcspn(p,":");
		
		if(i<6)
			end = strcspn(p,",");
		else
			end = strcspn(p,"}");
	
		memset(rk29keyArrary,0,sizeof(rk29keyArrary));
		
		strncpy(rk29keyArrary,p+start+1,end-start-1);
							 		
		for(j=0;j<1;j++)
		{		
			if(strcmp(pdata->buttons[j].desc,Arrary[i].keyArrary)==0)
			{
				if(strcmp(rk29keyArrary,"MENU")==0)
					pdata->buttons[j].code = EV_MENU;
				else if(strcmp(rk29keyArrary,"HOME")==0)
					pdata->buttons[j].code = KEY_HOME;
				else if(strcmp(rk29keyArrary,"ESC")==0)
					pdata->buttons[j].code = KEY_BACK;
				else if(strcmp(rk29keyArrary,"sensor")==0)
					pdata->buttons[j].code = KEY_CAMERA;
				else if(strcmp(rk29keyArrary,"PLAY")==0)
					pdata->buttons[j].code = KEY_POWER;
				else if(strcmp(rk29keyArrary,"VOLUP")==0)
					pdata->buttons[j].code = KEY_VOLUMEUP;
				else if(strcmp(rk29keyArrary,"VOLDOWN")==0)
					pdata->buttons[j].code = KEY_VOLUMEDOWN;
				else
				     continue;
		 	}

		}
			
   	}

	for(i=0;i<1;i++)
		dev_dbg(dev, "desc=%s, code=%d\n",pdata->buttons[i].desc,pdata->buttons[i].code);
	return 0; 

}

static DEVICE_ATTR(rk29key,0660, NULL, rk29key_set);

void rk29_send_power_key(int state)
{
	if (!input_dev)
		return;
	if(state)
	{
		input_report_key(input_dev, KEY_POWER, 1);
		input_sync(input_dev);
	}
	else
	{
		input_report_key(input_dev, KEY_POWER, 0);
		input_sync(input_dev);
	}
}

void rk28_send_wakeup_key(void)
{
	if (!input_dev)
		return;

	input_report_key(input_dev, KEY_WAKEUP, 1);
	input_sync(input_dev);
	input_report_key(input_dev, KEY_WAKEUP, 0);
	input_sync(input_dev);
}
extern void report_poweroffkey(unsigned char on);
extern void report_mute_key(unsigned char on);
extern unsigned char is_long_pressed = 0;
EXPORT_SYMBOL(is_long_pressed);
unsigned long      long_pressed_time = 0;
static unsigned  char   is_suspend=0;
static void keys_long_press_timer(unsigned long _data)
{
	int state;
	struct rk29_button_data *bdata = (struct rk29_button_data *)_data;
	struct rk29_keys_button *button = bdata->button;
	struct input_dev *input = bdata->input;
	unsigned int type = EV_KEY;
	///printk("=========%d=======%s=\n",__LINE__,__FUNCTION__);
	if(button->gpio != INVALID_GPIO )
		state = !!((gpio_get_value(button->gpio) ? 1 : 0) ^ button->active_low);
	else
		state = !!button->adc_state;
	if(state) {	///printk("=========%d=======%s=\n",__LINE__,__FUNCTION__);
		if(bdata->long_press_count != 0) {
			if(bdata->long_press_count % (LONG_PRESS_COUNT+ONE_SEC_COUNT) == 0)
			{
				key_dbg(bdata, "%skey[%s]: report ev[%d] state[0]\n", 
					(button->gpio == INVALID_GPIO)?"ad":"io", button->desc, button->code_long_press);
				printk("=====llllllllllllllllllllllllllllllllllllll====%d=======%s=\n",__LINE__,__FUNCTION__);
				///input_event(input, type, button->code_long_press, 0);
				/***input_event(input, type, KEY_LONG_PRESS, 0);
				input_sync(input);*/
				report_poweroffkey(0);
			}
			else if(bdata->long_press_count%LONG_PRESS_COUNT == 0) {
				key_dbg(bdata, "%skey[%s]: report ev[%d] state[1]\n", 
					(button->gpio == INVALID_GPIO)?"ad":"io", button->desc, button->code_long_press);
						printk("=====llllllllllllllllllllllllllllllllllllll====%d=======%s=\n",__LINE__,__FUNCTION__);
				///input_event(input, type, button->code_long_press, 1);
				/***input_event(input, type, KEY_LONG_PRESS, 1);
				input_sync(input);*/
				report_poweroffkey(1);report_poweroffkey(0);is_long_pressed = 1;  long_pressed_time = jiffies ;
			}
		}
		bdata->long_press_count++;
		mod_timer(&bdata->timer,
				jiffies + msecs_to_jiffies(DEFAULT_DEBOUNCE_INTERVAL));
	}
	else {printk("=========%d=======%s=\n",__LINE__,__FUNCTION__);
		if(bdata->long_press_count <= LONG_PRESS_COUNT) {
			bdata->long_press_count = 0;
			key_dbg(bdata, "%skey[%s]: report ev[%d] state[1], report ev[%d] state[0]\n", 
					(button->gpio == INVALID_GPIO)?"ad":"io", button->desc, button->code, button->code);
				printk("=====ssssssssssssssssssssssssssssssss====%d=======%s=\n",__LINE__,__FUNCTION__);
			/***input_event(input, type, button->code, 1);
			input_sync(input);
			input_event(input, type, button->code, 0);
			input_sync(input);*/
			if(is_long_pressed == 1){
			    if( jiffies_to_msecs(jiffies - long_pressed_time) > 1300)
			    {
				report_mute_key(1);
				report_mute_key(0);is_long_pressed = 0;
			    }
			}else{
				///if(ignore_wakeup_report%2==1)
				if(is_suspend==1){is_suspend=0;}
				else{
					input_event(input, type, button->code, 1);
					input_sync(input);
					input_event(input, type, button->code, 0);
					input_sync(input);
				}	
			}				
		}
		else if(bdata->state != state) {
			key_dbg(bdata, "%skey[%s]: report ev[%d] state[0]\n", 
			(button->gpio == INVALID_GPIO)?"ad":"io", button->desc, button->code_long_press);
			printk("=====ssssssssssssssssssssssssssssssss====%d=======%s=\n",__LINE__,__FUNCTION__);
			/****/input_event(input, type, button->code_long_press, 0);
			input_sync(input);
			report_mute_key(0);
		}
	}
	bdata->state = state;
}
static void keys_timer(unsigned long _data)
{
	int state;
	struct rk29_button_data *bdata = (struct rk29_button_data *)_data;
	struct rk29_keys_button *button = bdata->button;
	struct input_dev *input = bdata->input;
	unsigned int type = EV_KEY;
	printk("=========%d=======%s=\n",__LINE__,__FUNCTION__);
	if(button->gpio != INVALID_GPIO)
		state = !!((gpio_get_value(button->gpio) ? 1 : 0) ^ button->active_low);
	else
		state = !!button->adc_state;
	if(bdata->state != state) {
		bdata->state = state;
		key_dbg(bdata, "%skey[%s]: report ev[%d] state[%d]\n", 
			(button->gpio == INVALID_GPIO)?"ad":"io", button->desc, button->code, bdata->state);
		input_event(input, type, button->code, bdata->state);
		input_sync(input);
	}
	if(state)
		mod_timer(&bdata->timer,
			jiffies + msecs_to_jiffies(DEFAULT_DEBOUNCE_INTERVAL));
}
struct input_dev * input_dev_kp;
home_key_to1()
{
	input_event(input_dev_kp, EV_KEY, KEY_HOME, 1);
	input_sync(input_dev_kp);
	
}
task_key_to1()
{
	
	input_event(input_dev_kp, EV_KEY, KEY_APP_NWD, 1);
	input_sync(input_dev_kp);
	
}
short_key_to1()
{
	input_event(input_dev_kp, EV_KEY, 250, 1);
	input_sync(input_dev_kp);
	
}
long_key_to1()
{
	input_event(input_dev_kp, EV_KEY, 251, 1);
	input_sync(input_dev_kp);
	
}
home_key_to0()
{
	input_event(input_dev_kp, EV_KEY, KEY_HOME, 0);
	///input_event(input_dev_kp, EV_KEY, KEY_APP_NWD, 0);
	input_sync(input_dev_kp);
	
}
task_key_to0()
{

	input_event(input_dev_kp, EV_KEY, KEY_APP_NWD, 0);
	input_sync(input_dev_kp);
	
}
menu_key_to1()
{
	input_event(input_dev_kp, EV_KEY, 82, 1);
	input_sync(input_dev_kp);
	
}
menu_key_to0()
{
	input_event(input_dev_kp, EV_KEY, 82, 0);
	input_sync(input_dev_kp);
	
}
static irqreturn_t keys_isr(int irq, void *dev_id)
{
	struct rk29_button_data *bdata = dev_id;
	struct rk29_keys_button *button = bdata->button;
	struct input_dev *input = bdata->input;
	unsigned int type = EV_KEY;
	BUG_ON(irq != gpio_to_irq(button->gpio));
	printk("=========%d=======%s=\n",__LINE__,__FUNCTION__);
        if(button->wakeup == 1 && bdata->ddata->in_suspend == true){
		bdata->state = 1;
		key_dbg(bdata, "wakeup: %skey[%s]: report ev[%d] state[%d]\n", 
			(button->gpio == INVALID_GPIO)?"ad":"io", button->desc, button->code, bdata->state);
		input_event(input, type, button->code, bdata->state);
		input_sync(input);
        }
	bdata->long_press_count = 0;
	mod_timer(&bdata->timer,
				jiffies + msecs_to_jiffies(DEFAULT_DEBOUNCE_INTERVAL));
	return IRQ_HANDLED;
}
struct input_dev * input_dev_kp;



static void keys_adc_callback(struct adc_client *client, void *client_param, int result)
{
	struct rk29_keys_drvdata *ddata = (struct rk29_keys_drvdata *)client_param;
	int i;
	if(result > INVALID_ADVALUE && result < EMPTY_ADVALUE)
		ddata->result = result;
	for (i = 0; i < ddata->nbuttons; i++) {
		struct rk29_button_data *bdata = &ddata->data[i];
		struct rk29_keys_button *button = bdata->button;
		if(!button->adc_value)
			continue;
		if(result < button->adc_value + DRIFT_ADVALUE &&
			result > button->adc_value - DRIFT_ADVALUE)
			button->adc_state = 1;
		else
			button->adc_state = 0;
		if(bdata->state != button->adc_state)
			mod_timer(&bdata->timer,
				jiffies + msecs_to_jiffies(DEFAULT_DEBOUNCE_INTERVAL));
	}
	return;
}

static void keys_adc_timer(unsigned long _data)
{
	struct rk29_keys_drvdata *ddata = (struct rk29_keys_drvdata *)_data;

	if (!ddata->in_suspend)
		adc_async_read(ddata->client);
	mod_timer(&ddata->timer, jiffies + msecs_to_jiffies(ADC_SAMPLE_TIME));
}

static ssize_t adc_value_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct rk29_keys_drvdata *ddata = dev_get_drvdata(dev);
	
	return sprintf(buf, "adc_value: %d\n", ddata->result);
}

static DEVICE_ATTR(get_adc_value, S_IRUGO | S_IWUSR, adc_value_show, NULL);


static void gpio_sus_early_suspend(struct early_suspend *h)
{
	int ret;
	printk("%s  %d==============\n",__FUNCTION__,__LINE__);
	/***rk30_mux_api_set(GPIO4D3_SMCDATA11_TRACEDATA11_NAME, GPIO4D_GPIO4D3); ///RST
	ret = gpio_request(RK30_PIN4_PD3, "led  pin");
	if (ret != 0) {
			gpio_free(RK30_PIN4_PD3);
			ret = gpio_request(RK30_PIN4_PD3, "led pin");
			if (ret != 0) {
				printk("led gpio_request error\n");
				return -EIO;
			}
		}
	
	///rk30_mux_api_set(GPIO2C3_LCDC1DATA19_SPI1CLK_HSADCDATA0_NAME,GPIO2C_GPIO2C3);
	gpio_direction_output(RK30_PIN4_PD3,0);
	gpio_set_value(RK30_PIN4_PD3, 0);	*/
                 ///LED RK30_PIN4_PD3  ,must have this
	grf_writel( 0x08000800, GRF_GPIO4H_DIR);
	grf_writel( 0x08000800, GRF_GPIO4H_DO);  //set gpio6_b1 output low
	grf_writel(0x08000800 , GRF_GPIO4H_EN);
#if 0
///6A0  lcd_en
	grf_writel( 0x00010001, GRF_GPIO6L_DIR);
	grf_writel(  0x00010001, GRF_GPIO6L_DO);  //set gpio6_b1 output low
	grf_writel( 0x00010001 , GRF_GPIO6L_EN);

///1a0 1a1 uart0   1a4 1a5  uart1   3d5  3d6
	grf_writel( 0x00030003, GRF_GPIO1L_DIR);
	grf_writel( 0x00030003, GRF_GPIO1L_DO);  //set gpio6_b1 output low
	grf_writel( 0x00030003 , GRF_GPIO1L_EN);

	grf_writel( 0x00300030, GRF_GPIO1L_DIR);
	grf_writel( 0x00300030, GRF_GPIO1L_DO);  //set gpio6_b1 output low
	grf_writel( 0x00300030 , GRF_GPIO1L_EN);

	grf_writel( 0x60006000, GRF_GPIO3H_DIR);
	grf_writel( 0x60006000, GRF_GPIO3H_DO);  //set gpio6_b1 output low
	grf_writel( 0x60006000, GRF_GPIO3H_EN);
#endif
///6a0  lcd_en,out high
		grf_writel( 0x00010001, GRF_GPIO6L_DIR);
	grf_writel(  0x00010001, GRF_GPIO6L_DO);  // 
	grf_writel( 0x00010001 , GRF_GPIO6L_EN);

///1a0 1a1 uart0   1a4 1a5  uart1   3d5  3d6,input 
	grf_writel( 0x00030000, GRF_GPIO1L_DIR);
	grf_writel( 0x00030003, GRF_GPIO1L_DO);  //set gpio6_b1 output low
	grf_writel( 0x00030003 , GRF_GPIO1L_EN);

	grf_writel( 0x00300000, GRF_GPIO1L_DIR);
	grf_writel( 0x00300030, GRF_GPIO1L_DO);  //set gpio6_b1 output low
	grf_writel( 0x00300030 , GRF_GPIO1L_EN);

	grf_writel( 0x60000000, GRF_GPIO3H_DIR);
	grf_writel( 0x60006000, GRF_GPIO3H_DO);  //set gpio6_b1 output low
	grf_writel( 0x60006000, GRF_GPIO3H_EN);

/***
	////GPIO3A0 3A1
	///grf_writel( 0xffff0000, GRF_GPIO3L_DIR);
	rk30_mux_api_set(GPIO3A0_I2C2SDA_NAME,0);
	rk30_mux_api_set(GPIO3A1_I2C2SCL_NAME,0);

	grf_writel( 0x00030003, GRF_GPIO3L_DIR);
grf_writel( 0xffff0000, GRF_GPIO3L_DIR);

grf_writel( 0xffffffff, GRF_GPIO1L_PULL);
grf_writel( 0xffffffff, GRF_GPIO1H_PULL);
grf_writel( 0xffffffff, GRF_GPIO0L_PULL);
grf_writel( 0xffffffff, GRF_GPIO0H_PULL);
grf_writel( 0xffffffff, GRF_GPIO2L_PULL);
grf_writel( 0xffffffff, GRF_GPIO2H_PULL);
grf_writel( 0xffffffff, GRF_GPIO3L_PULL);
grf_writel( 0xffffffff, GRF_GPIO3H_PULL);
grf_writel( 0xffffffff, GRF_GPIO4L_PULL);
grf_writel( 0xffffffff, GRF_GPIO4H_PULL);
	grf_writel( 0x00030003, GRF_GPIO3L_DO);  //set gpio6_b1 output low
	grf_writel( 0x00030003 , GRF_GPIO3L_EN);


	 */
#if 0
	rk30_mux_api_set(GPIO3A4_I2C4SDA_NAME,0);
	rk30_mux_api_set(GPIO3A5_I2C4SCL_NAME,0);
grf_writel( 0x00300030, GRF_GPIO3L_DIR);
grf_writel( 0x00300030, GRF_GPIO3L_DO);
grf_writel( 0x00300030, GRF_GPIO3L_EN);
#endif

}

static void gpio_sus_late_resume(struct early_suspend *h)
{

	grf_writel( 0x08000800, GRF_GPIO4H_DIR);
	grf_writel( 0x08000800, GRF_GPIO4H_DO);  //set gpio6_b1 output low
	grf_writel(0x08000000 , GRF_GPIO4H_EN);
///6a0  lcd_en
	grf_writel( 0x00010001, GRF_GPIO6L_DIR);
	grf_writel( 0x00010001, GRF_GPIO6L_DO);  
	grf_writel( 0x00010000 , GRF_GPIO6L_EN);

///1a0 1a1 uart0   1a4 1a5  uart1   3d5  3d6	
	grf_writel( 0x00030000, GRF_GPIO1L_DIR);
	grf_writel( 0x00030003, GRF_GPIO1L_DO);  //set gpio6_b1 output low
	grf_writel( 0x00030000 , GRF_GPIO1L_EN);

	grf_writel( 0x00300000, GRF_GPIO1L_DIR);
	grf_writel( 0x00300030, GRF_GPIO1L_DO);  //set gpio6_b1 output low
	grf_writel( 0x00300000 , GRF_GPIO1L_EN);

	grf_writel( 0x60000000, GRF_GPIO3H_DIR);
	grf_writel( 0x60006000, GRF_GPIO3H_DO);  //set gpio6_b1 output low
	grf_writel( 0x60000000, GRF_GPIO3H_EN);


#if 0
////GPIO3A0 3A1
	///grf_writel( 0x00030003, GRF_GPIO3L_IOMUX);
	grf_writel( 0x00030003, GRF_GPIO3L_DIR);
	grf_writel( 0x00030003, GRF_GPIO3L_DO);  //set gpio6_b1 output low
	grf_writel( 0x00030000 , GRF_GPIO3L_EN);
#endif
	int ret = 0;
	rk30_mux_api_set(GPIO2B5_LCDC1DATA13_SMCADDR17_HSADCDATA8_NAME,0);
	ret = gpio_request(RK30_PIN2_PB5, "led  pin");
	if (ret != 0) {
			gpio_free(RK30_PIN2_PB5);
			ret = gpio_request(RK30_PIN2_PB5, "led pin");
			if (ret != 0) {
				printk("led gpio_request error\n");
				return -EIO;
			}
		}
	
	///rk30_mux_api_set(GPIO2C3_LCDC1DATA19_SPI1CLK_HSADCDATA0_NAME,GPIO2C_GPIO2C3);
	gpio_direction_output(RK30_PIN2_PB5,0);
	gpio_set_value(RK30_PIN2_PB5, 0);

}
static int __devinit keys_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rk29_keys_platform_data *pdata = dev_get_platdata(dev);
	struct rk29_keys_drvdata *ddata;
	struct input_dev *input;
	int i, error = 0;
	int wakeup = 0;
	/***printk("===========\n\n\n\n\n\n\n\n",__FUNCTION__,__LINE__);

	grf_writel( 0x00ff0000 ,GRF_GPIO2L_EN);	  grf_writel( 0xffff0000 ,GRF_GPIO2L_EN);
	grf_writel( 0xff000000 ,GRF_GPIO2H_EN);
	grf_writel( 0xffff0000 ,GRF_GPIO3H_EN);
	grf_writel( 0xff000000,GRF_GPIO4H_EN);*/

	if(!pdata) 
		return -EINVAL;
#define  ANDROID_EARLY_SUSPEND_LEVEL_BLANK_SCREEN  50 
	gpio_sus.level = ANDROID_EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 20;
	gpio_sus.suspend = gpio_sus_early_suspend;
	gpio_sus.resume = gpio_sus_late_resume;
	register_early_suspend(&gpio_sus);	



	ddata = kzalloc(sizeof(struct rk29_keys_drvdata) +
			pdata->nbuttons * sizeof(struct rk29_button_data),
			GFP_KERNEL);
	input = input_allocate_device();
	if (!ddata || !input) {
		error = -ENOMEM;printk("===========\n\n\n\n\n\n\n\n",__FUNCTION__,__LINE__);
		goto fail0;
	}
        input_dev_kp = input ;
	platform_set_drvdata(pdev, ddata);

	input->name = pdev->name;
	input->phys = "gpio-keys/input0";
	input->dev.parent = dev;

	input->id.bustype = BUS_HOST;
	input->id.vendor = 0x0001;
	input->id.product = 0x0001;
	input->id.version = 0x0100;

	///int irq = gpio_to_irq( RK30_PIN6_PA0); ///wj add
	///enable_irq_wake(irq);		///wj add

#if 1
	/* Enable auto repeat feature of Linux input subsystem */
	if (pdata->rep)
		__set_bit(EV_REP, input->evbit);
	ddata->nbuttons = pdata->nbuttons;
	ddata->input = input;

	for (i = 0; i < pdata->nbuttons; i++) {
		struct rk29_keys_button *button = &pdata->buttons[i];
		struct rk29_button_data *bdata = &ddata->data[i];

		bdata->input = input;
		bdata->button = button;
                bdata->ddata = ddata;
		button->code_long_press = KEY_LONG_PRESS;
		//if (button->code_long_press)
		if (1)
			setup_timer(&bdata->timer,
			    	keys_long_press_timer, (unsigned long)bdata);
		else if (button->code)
			setup_timer(&bdata->timer,
			    	keys_timer, (unsigned long)bdata);

		if (button->wakeup)
			wakeup = 1;

		input_set_capability(input, EV_KEY, button->code);

		input_set_capability(input, EV_KEY, button->code_long_press);
		input_set_capability(input, EV_KEY, KEY_APP_NWD);
		///input_set_capability(input, EV_KEY, KEY_CAMERA);
	};

	if (pdata->chn >= 0) {
		setup_timer(&ddata->timer, keys_adc_timer, (unsigned long)ddata);
		ddata->client = adc_register(pdata->chn, keys_adc_callback, (void *)ddata);
		if (!ddata->client) {
			error = -EINVAL;printk("===========\n\n\n\n\n\n\n\n",__FUNCTION__,__LINE__);
			goto fail1;
		}
		mod_timer(&ddata->timer, jiffies + msecs_to_jiffies(100));
	}

	for (i = 0; i < pdata->nbuttons; i++) {
		struct rk29_keys_button *button = &pdata->buttons[i];
		struct rk29_button_data *bdata = &ddata->data[i];
		int irq;

		if(button->gpio != INVALID_GPIO) {
			error = gpio_request(button->gpio, button->desc ?: "keys");
			if (error < 0) {
				pr_err("gpio-keys: failed to request GPIO %d,"
					" error %d\n", button->gpio, error);
				printk("===========\n\n\n\n\n\n\n\n",__FUNCTION__,__LINE__);
					gpio_free(button->gpio);
				error = gpio_request(button->gpio, button->desc ?: "keys");
				if (error < 0){printk("===========\n\n\n\n\n\n\n\n",__FUNCTION__,__LINE__);
				goto fail2;
				}
			}

			error = gpio_direction_input(button->gpio);
			if (error < 0) {
				pr_err("gpio-keys: failed to configure input"
					" direction for GPIO %d, error %d\n",
					button->gpio, error);
				gpio_free(button->gpio);printk("===========\n\n\n\n\n\n\n\n",__FUNCTION__,__LINE__);
				goto fail2;
			}

			irq = gpio_to_irq(button->gpio);
			if (irq < 0) {
				error = irq;
				pr_err("gpio-keys: Unable to get irq number"
					" for GPIO %d, error %d\n",
					button->gpio, error);
				gpio_free(button->gpio);printk("===========\n\n\n\n\n\n\n\n",__FUNCTION__,__LINE__);
				goto fail2;
			}
			if(strcmp(button->desc,"menu")==0) {
			  nwd();
			  irq_set_irq_wake( irq, 1);
			}
			error = request_irq(irq, keys_isr,
					    (button->active_low)?IRQF_TRIGGER_FALLING : IRQF_TRIGGER_RISING,
					    button->desc ? button->desc : "keys",
					    bdata);
			if (error) {
				pr_err("gpio-keys: Unable to claim irq %d; error %d\n",
					irq, error);
				gpio_free(button->gpio);printk("===========\n\n\n\n\n\n\n\n",__FUNCTION__,__LINE__);
				goto fail2;
			}
		}
	}
#endif
	input_set_capability(input, EV_KEY, KEY_WAKEUP);

	error = input_register_device(input);
	if (error) {
		pr_err("gpio-keys: Unable to register input device, "
			"error: %d\n", error);printk("===========\n\n\n\n\n\n\n\n",__FUNCTION__,__LINE__);
		goto fail2;
	}

	device_init_wakeup(dev, wakeup);
	error = device_create_file(dev, &dev_attr_get_adc_value);

	error = device_create_file(dev, &dev_attr_rk29key);
	if(error )
	{
		pr_err("failed to create key file error: %d\n", error);
	}


	input_dev = input;	printk("===========\n\n\n\n\n\n\n\n",__FUNCTION__,__LINE__);
	return error;

 fail2:
	while (--i >= 0) {
		free_irq(gpio_to_irq(pdata->buttons[i].gpio), &ddata->data[i]);
		del_timer_sync(&ddata->data[i].timer);
		gpio_free(pdata->buttons[i].gpio);
	}
	if(pdata->chn >= 0 && ddata->client);
		adc_unregister(ddata->client);
	if(pdata->chn >= 0)
	        del_timer_sync(&ddata->timer);
 fail1:
 	platform_set_drvdata(pdev, NULL);
 fail0:
	input_free_device(input);
	kfree(ddata);

	return error;
}

static int __devexit keys_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rk29_keys_platform_data *pdata = dev_get_platdata(dev);
	struct rk29_keys_drvdata *ddata = dev_get_drvdata(dev);
	struct input_dev *input = ddata->input;
	int i;

	input_dev = NULL;
	device_init_wakeup(dev, 0);

	for (i = 0; i < pdata->nbuttons; i++) {
		int irq = gpio_to_irq(pdata->buttons[i].gpio);
		free_irq(irq, &ddata->data[i]);
		del_timer_sync(&ddata->data[i].timer);
		gpio_free(pdata->buttons[i].gpio);
	}
	if(pdata->chn >= 0 && ddata->client);
		adc_unregister(ddata->client);
	input_unregister_device(input);

	return 0;
}


void	d200_wifi_pwr(int on);
void disable_ldo(void);
#ifdef CONFIG_PM
static int keys_suspend(struct device *dev)
{
	struct rk29_keys_platform_data *pdata = dev_get_platdata(dev);
	struct rk29_keys_drvdata *ddata = dev_get_drvdata(dev);
	int i;printk("===%d %s===\n",__LINE__,__FUNCTION__);
	///d200_wifi_pwr(0);///printk("===%d %s===\n",__LINE__,__FUNCTION__);
	///disable_ldo();printk("===%d %s===\n",__LINE__,__FUNCTION__);
	ddata->in_suspend = true;
	is_suspend = 1;
	int irq2 = gpio_to_irq(RK30_PIN6_PA0);
	enable_irq_wake(RK30_PIN6_PA0);

	if (device_may_wakeup(dev)) {
		for (i = 0; i < pdata->nbuttons; i++) {
			struct rk29_keys_button *button = &pdata->buttons[i];
			if (button->wakeup) {
				int irq = gpio_to_irq(button->gpio);
				enable_irq_wake(irq);
			}
		}
	}

	///int irq = gpio_to_irq( RK30_PIN6_PA0); ///wj add
	////enable_irq_wake(irq);		///wj add

	return 0;
}

static int keys_resume(struct device *dev)
{
	struct rk29_keys_platform_data *pdata = dev_get_platdata(dev);
	struct rk29_keys_drvdata *ddata = dev_get_drvdata(dev);
	int i;
	 
	if (device_may_wakeup(dev)) {
		for (i = 0; i < pdata->nbuttons; i++) {
			struct rk29_keys_button *button = &pdata->buttons[i];
			if (button->wakeup) {
				int irq = gpio_to_irq(button->gpio);
				disable_irq_wake(irq);
			}
		}
		preempt_disable();
		if (local_softirq_pending())
			do_softirq(); // for call resend_irqs, which may call keys_isr
		preempt_enable_no_resched();
	}

	ddata->in_suspend = false;

	return 0;
}

static const struct dev_pm_ops keys_pm_ops = {
	.suspend	= keys_suspend,
	.resume		= keys_resume,
};
#endif

static struct platform_driver keys_device_driver = {
	.probe		= keys_probe,
	.remove		= __devexit_p(keys_remove),
	.driver		= {
		.name	= "rk29-keypad",
		.owner	= THIS_MODULE,
#ifdef CONFIG_PM
		.pm	= &keys_pm_ops,
#endif
	}
};

static int __init keys_init(void)
{
	return platform_driver_register(&keys_device_driver);
}

static void __exit keys_exit(void)
{
	platform_driver_unregister(&keys_device_driver);
}

module_init(keys_init);
module_exit(keys_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Phil Blundell <pb@handhelds.org>");
MODULE_DESCRIPTION("Keyboard driver for CPU GPIOs");
MODULE_ALIAS("platform:gpio-keys");
