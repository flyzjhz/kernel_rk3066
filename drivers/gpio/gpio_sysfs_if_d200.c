#include <linux/module.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>#include <linux/types.h>

#include <linux/vmalloc.h>
#include <asm/io.h>
#include <asm/sizes.h>
#include <mach/iomux.h>
#include <mach/gpio.h> 
#include <linux/delay.h>

#include <linux/wakelock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <mach/board.h>



#include <linux/module.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/i2c/pca954x.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <asm/uaccess.h> 
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <mach/gpio.h>
#include <linux/earlysuspend.h>

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
#define grf_readl(offset)	readl_relaxed(RK30_GRF_BASE + offset)
#define grf_writel(v, offset)	do { writel_relaxed(v, RK30_GRF_BASE + offset); dsb(); } while (0)




#define GPIO_HW_TST 1
int tp_tst = 2;
void init_nwd_tps2546(void)
{
    int ret;
    rk30_mux_api_set(GPIO6A6_TESTCLOCKOUT_NAME,0);
    rk30_mux_api_set(GPIO6A0_TESTCLOCKOUT_NAME,0);
    rk30_mux_api_set(GPIO4C3_SMCDATA3_TRACEDATA3_NAME,GPIO4C_GPIO4C3);
//USBPWR_EN  GPIO6_A0 H
	ret = gpio_request(RK30_PIN6_PA0, "K1103 WIFI pwr");
    		if(ret != 0)
   		 {
			gpio_free(RK30_PIN6_PA0);
			ret = gpio_request(RK30_PIN6_PA0, "wK1103 WIFI pwr");
			if(ret != 0)
			printk("gpio req error\n");
    		}

			gpio_direction_output(RK30_PIN6_PA0,1);
			gpio_set_value(RK30_PIN6_PA0, 1);

    
//CLT1 GPIO4_C3    H

    ret = gpio_request(RK30_PIN4_PC3, "USBPWR_EN");
    if(ret != 0)
    {
	gpio_free(RK30_PIN4_PC3);
  	ret = gpio_request(RK30_PIN4_PC3, "USBPWR_EN");
	    if(ret != 0)
	printk("tps2546 USBPWR_EN error\n");
    }
 ///   else
    {
	gpio_direction_output(RK30_PIN4_PC3, 1);
	gpio_set_value(RK30_PIN4_PC3, 1);
    }

//USB-SEL GPIO6_A6  L

ret = gpio_request(RK30_PIN6_PA6, "K1103 WIFI pwr");
    		if(ret != 0)
   		 {
			gpio_free(RK30_PIN6_PA6);
			ret = gpio_request(RK30_PIN6_PA6, "wK1103 WIFI pwr");
			if(ret != 0)
			printk("gpio req error\n");
    		}

			gpio_direction_output(RK30_PIN6_PA6,0);
			gpio_set_value(RK30_PIN6_PA6, 0);


}
#ifdef GPIO_HW_TST
#define printk_gpio printk
///#define GPIO_TST(name)  		      
int gpio_setting_tst(int gpio,const char *buf,char* name)
{
		int ret;
		printk_gpio("===%d %s===GPIOCAOCAO:%s\n",__LINE__,__FUNCTION__,buf);
		ret = gpio_request(gpio, name);
    		if(ret != 0)
   		 {
			gpio_free(gpio);
			ret = gpio_request(gpio, name);
			if(ret != 0)
			printk("gpio req error\n");
    		}

		if(strncmp(buf+8,"inp",3)==0)	{
			gpio_direction_input(gpio);
			printk_gpio("===%d %s===\n",__LINE__,__FUNCTION__);		
		}

		if(strncmp(buf+8,"hig",3)==0)	{
			gpio_direction_output(gpio,1);
			gpio_set_value(gpio, 1);
			printk_gpio("===%d %s===\n",__LINE__,__FUNCTION__);
		}

		if(strncmp(buf+8,"low",3)==0)	{
			gpio_direction_output(gpio,1);
			gpio_set_value(gpio, 0);
			printk_gpio("===%d %s===\n",__LINE__,__FUNCTION__);
		}
}

 int gpio;
#endif
static ssize_t tps2546_nwd_write(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)

{
    int ret = 0;int cnt = 0;
    int new_state = simple_strtoul(buf, NULL, 16);
#ifdef CONFIG_CHARGE_TPS2546    
    printk("[%s] statue change: %d\n", __func__, new_state);
 
    switch(new_state)
    {
    case 0:
	printk("===%d %s===\n",__LINE__,__FUNCTION__);
//USBPWR_EN  GPIO6_A0 H
	gpio_direction_output(RK30_PIN6_PA0, 1);
	gpio_set_value(RK30_PIN6_PA0, 1);

//CLT1 GPIO4_C3    H
        gpio_direction_output(RK30_PIN4_PC3, 1);
        gpio_set_value(RK30_PIN4_PC3, 1);
//USB-SEL GPIO6_A6  L
   	gpio_direction_output(RK30_PIN6_PA6, 0);
   	gpio_set_value(RK30_PIN6_PA6, 0);
        break;
    case 1:
	printk("===%d %s===\n",__LINE__,__FUNCTION__);
//USBPWR_EN  GPIO6_A0 H
	gpio_direction_output(RK30_PIN6_PA0, 1);
	gpio_set_value(RK30_PIN6_PA0, 1);

//CLT1 GPIO4_C3    L
        gpio_direction_output(RK30_PIN4_PC3, 0);
        gpio_set_value(RK30_PIN4_PC3, 0);
//USB-SEL GPIO6_A6  L
   	gpio_direction_output(RK30_PIN6_PA6, 0);
   	gpio_set_value(RK30_PIN6_PA6, 0);
	break;
    case 2:
	printk("===%d %s===\n",__LINE__,__FUNCTION__);
//USBPWR_EN  GPIO6_A0 H
	gpio_direction_output(RK30_PIN6_PA0, 1);
	gpio_set_value(RK30_PIN6_PA0, 1);

//CLT1 GPIO4_C3    H
        gpio_direction_output(RK30_PIN4_PC3, 0);
        gpio_set_value(RK30_PIN4_PC3,0);
//USB-SEL GPIO6_A6  L
   	gpio_direction_output(RK30_PIN6_PA6, 1);
   	gpio_set_value(RK30_PIN6_PA6, 1);
        break;
    default:
        ret = -1;
        printk("[%s] Invalid new status: %d\n", __func__, new_state);
        break;
    }
#endif
    return count;///ret;
}
ssize_t state_store(struct kobject *kobj, struct kobj_attribute *attr,
			   const char *buf, size_t n);
void rk28_send_wakeup_key(void);
static ssize_t tps2546_nwd_read(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{	
	return 0;
}
///6B4  4D6  6B1  6A0  0C5 4C4 4C6  4C7 6B2  2C5  2C6  2C3   6A2 1B3  3D5  3D6  4C1  4C0  3D0   4D2  3A2  3A3  4D1 4D0    3A7  2A0 2A1  2A2  2C0 2B6 6A5
///6A6  6B3  4C3  6A7   0C7  
///6B4  6B1   6B2   6B3  6A5 6A0 6A2 6A6   4D6   0C5 4C4 4C6  4C7   2C5  2C6  2C3     1B3  3D5  3D6  4C1  4C0  3D0   4D2  3A2  3A3  4D1 4D0    3A7  2A0 2A1  2A2  2C0 2B6 
///  4C3  6A7   0C7  
void wm8999_wr(int reg,int val);
static int foo_reg;
static int foo_val;
static ssize_t gpio_nwd_write2(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)

{
    int ret = 0;int cnt = 0;
  ///  int new_state = simple_strtoul(buf, NULL, 16);
sscanf(buf, "%x", &foo_val);
///wm8999_wr(foo_reg,foo_val);printk("===%d %s===foo_reg:%x foo_val:%x\n",__LINE__,__FUNCTION__,foo_reg,foo_val);
return 4;
}

static ssize_t gpio_nwd_write(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)

{
    int ret = 0;int cnt = 0;
    int new_state = simple_strtoul(buf, NULL, 16);
/**
sscanf(buf, "%x", &foo_reg);printk("===%d %s===foo_reg:%x\n",__LINE__,__FUNCTION__,foo_reg);
return 2;*/
#ifdef GPIO_HW_TST
	 char req_name[7];
  	if(strncmp(buf,"GPIO2C2",7)==0)
	{	printk_gpio("===%d %s===\n",__LINE__,__FUNCTION__);
		gpio = RK30_PIN2_PC2;	  
		if( strlen(buf) == 7 ) 
		{printk_gpio("===%d %s===\n",__LINE__,__FUNCTION__);goto end_gpio;}
		printk_gpio("===%d %s===\n",__LINE__,__FUNCTION__);
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO2C2_LCDC1DATA18_SMCBLSN1_HSADCDATA5_NAME,0);		   
	/***	ret = gpio_request(gpio, "gpio2c2");
    		if(ret != 0)
   		 {
			gpio_free(gpio);
			ret = gpio_request(gpio, "gpio2c2");
			if(ret != 0)
			printk("gpio req error\n");
    		}

	  	if(strncmp(buf+8,"inp",3)==0)	
			gpio_direction_input(gpio);
		if(strncmp(buf+8,"hig",3)==0)	{
			gpio_direction_output(gpio,1);
			gpio_set_value(gpio, 1);
		}
		if(strncmp(buf+8,"low",3)==0)	{
			gpio_direction_output(gpio,1);
			gpio_set_value(gpio, 0);
		}*/
		ret = gpio_setting_tst(  gpio,buf,req_name);	
		////}    
	}
/***   rk30_mux_api_set(GPIO0D7_PWM3_NAME, GPIO0D_GPIO0D7); ///RST
        ret = gpio_request(RK30_PIN0_PD7, "od7 caocao ,logic not stable");
        if (ret != 0) {
                        gpio_free(RK30_PIN0_PD7);
                        ret = gpio_request(RK30_PIN0_PD7, "CACOACOAOCled pin");
                        if (ret != 0) {
                                printk("led gpio_request error\n");
                                return -EIO;
                        }
                }

        gpio_direction_output(RK30_PIN0_PD7,0);
        gpio_set_value(RK30_PIN0_PD7, 1);
*/

/**/  
	else if(strncmp(buf,"GPIO1A6",7)==0) ///wj 20151-09
	{	gpio = RK30_PIN1_PA6;	///if( strlen(buf) == 7 ) goto end_gpio;
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO1A6_UART1CTSN_SPI0RXD_NAME ,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}
	else if(strncmp(buf,"GPIO4D6",7)==0)
	{	gpio = RK30_PIN4_PD6;	///if( strlen(buf) == 7 ) goto end_gpio;
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO4D6_SMCDATA14_TRACEDATA14_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO0C5",7)==0)
	{	gpio = RK30_PIN0_PC5;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO0C5_I2S12CHSDO_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}
else if(strncmp(buf,"GPIO4C4",7)==0)
	{	gpio = RK30_PIN4_PC4;	///if( strlen(buf) == 7 ) goto end_gpio;
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO4C4_SMCDATA4_TRACEDATA4_NAME,0);printk_gpio("===%d %s===\n",__LINE__,__FUNCTION__);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO4C6",7)==0)
	{	gpio = RK30_PIN4_PC6;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO4C6_SMCDATA6_TRACEDATA6_NAME,0); printk_gpio("===%d %s===\n",__LINE__,__FUNCTION__);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}
else if(strncmp(buf,"GPIO4C7",7)==0)
	{	gpio = RK30_PIN4_PC7;	///if( strlen(buf) == 7 ) goto end_gpio;
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO4C7_SMCDATA7_TRACEDATA7_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO2C3",7)==0)
	{	gpio = RK30_PIN2_PC3;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set( GPIO2C3_LCDC1DATA19_SPI1CLK_HSADCDATA0_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO2C5",7)==0)
	{	gpio = RK30_PIN2_PC5;	///if( strlen(buf) == 7 ) goto end_gpio;
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO2C5_LCDC1DATA21_SPI1TXD_HSADCDATA2_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO2C6",7)==0)
	{	gpio = RK30_PIN2_PC6;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO2C6_LCDC1DATA22_SPI1RXD_HSADCDATA3_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);

	}else if(strncmp(buf,"GPIO1B3",7)==0)
	{	gpio = RK30_PIN1_PB3;	///if( strlen(buf) == 7 ) goto end_gpio;
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO1B3_CIF0CLKOUT_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO3D5",7)==0)
	{	gpio = RK30_PIN3_PD5;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO3D5_UART3CTSN_NAME	,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO3D6",7)==0)
	{	gpio = RK30_PIN3_PD6;	///if( strlen(buf) == 7 ) goto end_gpio;
		strncpy(req_name,buf,7);
		rk30_mux_api_set( GPIO3D6_UART3RTSN_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO4C0",7)==0)
	{	gpio = RK30_PIN4_PC0;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO4C0_SMCDATA0_TRACEDATA0_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO4C1",7)==0)
	{	gpio = RK30_PIN4_PC1;	///if( strlen(buf) == 7 ) goto end_gpio;
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO4C1_SMCDATA1_TRACEDATA1_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO3D0",7)==0)
	{	gpio = RK30_PIN3_PD0;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO3D0_SDMMC1PWREN_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO4D2",7)==0)
	{	gpio = RK30_PIN4_PD2;	///if( strlen(buf) == 7 ) goto end_gpio;
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO4D2_SMCDATA10_TRACEDATA10_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO3A2",7)==0)
	{	gpio = RK30_PIN3_PA2;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO3A2_I2C3SDA_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO3A3",7)==0)
	{	gpio = RK30_PIN3_PA3;	///if( strlen(buf) == 7 ) goto end_gpio;
		strncpy(req_name,buf,7);
		rk30_mux_api_set( GPIO3A3_I2C3SCL_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO4D0",7)==0)
	{	gpio = RK30_PIN4_PD0;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO4D0_SMCDATA8_TRACEDATA8_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO4D1",7)==0)
	{	gpio = RK30_PIN4_PD1;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO4D1_SMCDATA9_TRACEDATA9_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO3A7",7)==0)
	{	gpio = RK30_PIN3_PA7;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO3A7_SDMMC0PWREN_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO2A0",7)==0)
	{	gpio = RK30_PIN2_PA0;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set( GPIO2A0_LCDC1DATA0_SMCADDR4_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO2B6",7)==0)
	{	gpio = RK30_PIN2_PB6;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set( GPIO2B6_LCDC1DATA14_SMCADDR18_TSSYNC_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO2C0",7)==0)
	{	gpio = RK30_PIN2_PC0;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO2C0_LCDCDATA16_GPSCLK_HSADCCLKOUT_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO2A2",7)==0)
	{	gpio = RK30_PIN2_PA2;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO2A2_LCDCDATA2_SMCADDR6_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO2A1",7)==0)
	{	gpio = RK30_PIN2_PA1;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO2A1_LCDC1DATA1_SMCADDR5_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}


else if(strncmp(buf,"GPIO6B1",7)==0)
	{	gpio = RK30_PIN6_PB1;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO6B1_TESTCLOCKOUT_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO6B2",7)==0)
	{	gpio = RK30_PIN6_PB2;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO6B2_TESTCLOCKOUT_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}
else if(strncmp(buf,"GPIO6B3",7)==0)
	{	gpio = RK30_PIN6_PB3;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO6B3_TESTCLOCKOUT_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO6B4",7)==0)
	{	gpio = RK30_PIN6_PB4;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO6B4_TESTCLOCKOUT_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}
	else if(strncmp(buf,"GPIO6B0",7)==0) ////d200,lcd en 2015-1-30
	{	gpio = RK30_PIN6_PB0;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO6B0_TESTCLOCKOUT_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}
else if(strncmp(buf,"GPIO6A0",7)==0)
	{	gpio = RK30_PIN6_PA0;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO6A0_TESTCLOCKOUT_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO6A2",7)==0)
	{	gpio = RK30_PIN6_PA2;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO6A2_TESTCLOCKOUT_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}
else if(strncmp(buf,"GPIO6A5",7)==0)
	{	gpio = RK30_PIN6_PA5;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO6A5_TESTCLOCKOUT_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO6A6",7)==0)
	{	gpio = RK30_PIN6_PA6;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO6A6_TESTCLOCKOUT_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}

else if(strncmp(buf,"GPIOOC3",7)==0)
	{	gpio = RK30_PIN0_PC3;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO0C3_I2S12CHLRCKTX_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}









	else if(strncmp(buf,"GPIO2C1",7)==0)
	{	gpio = RK30_PIN2_PC1;	///if( strlen(buf) == 7 ) goto end_gpio;
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO2C1_LCDC1DATA17_SMCBLSN0_HSADCDATA6_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO2B7",7)==0)
	{	gpio = RK30_PIN2_PB7;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO2B7_LCDC1DATA15_SMCADDR19_HSADCDATA7_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO2B5",7)==0)
	{	gpio = RK30_PIN2_PB5;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO2B5_LCDC1DATA13_SMCADDR17_HSADCDATA8_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO2C4",7)==0) ///
	{	gpio = RK30_PIN2_PC4;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO2C4_LCDC1DATA20_SPI1CSN0_HSADCDATA1_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO2C7",7)==0)
	{	gpio = RK30_PIN2_PC7;	 
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO2C7_LCDC1DATA23_SPI1CSN1_HSADCDATA4_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO4C5",7)==0)///
	{	gpio = RK30_PIN4_PC5;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO4C5_SMCDATA5_TRACEDATA5_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO4C2",7)==0)
	{	gpio = RK30_PIN4_PC2;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO4C2_SMCDATA2_TRACEDATA2_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO3A1",7)==0)  ///
	{	gpio = RK30_PIN3_PA1;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO3A1_I2C2SCL_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO3A0",7)==0)
	{	gpio = RK30_PIN3_PA0;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO3A0_I2C2SDA_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO4B7",7)==0)///
	{	gpio = RK30_PIN4_PB7;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO4B7_SPI0CSN1_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO6A1",7)==0)
	{	gpio = RK30_PIN6_PA1;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO6A1_TESTCLOCKOUT_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);  ////FOLLWOING IS CAM
	}else if(strncmp(buf,"GPIO1B1",7)==0)
	{	gpio = RK30_PIN1_PB1;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO1B1_UART2SOUT_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO1B0",7)==0)
	{	gpio = RK30_PIN1_PB0;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO1B0_UART2SIN_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO3D4",7)==0)
	{	gpio = RK30_PIN3_PD4;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO3D4_UART3SOUT_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO3D3",7)==0)
	{	gpio = RK30_PIN3_PD3;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO3D3_UART3SIN_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO1A5",7)==0)
	{	gpio = RK30_PIN1_PA5;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO1A5_UART1SOUT_SPI0CLK_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO1A4",7)==0)
	{	gpio = RK30_PIN1_PA4;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO1A4_UART1SIN_SPI0CSN0_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO6A5",7)==0)
	{	gpio = RK30_PIN6_PA5;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO6A5_TESTCLOCKOUT_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO6A7",7)==0)
	{	gpio = RK30_PIN6_PA7;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO6A7_TESTCLOCKOUT_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO3B6",7)==0)
	{	gpio = RK30_PIN3_PB6;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO3B6_SDMMC0DETECTN_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO0A4",7)==0)                 ///add for v2
	{	gpio = RK30_PIN0_PA4;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO0A4_PWM1_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}else if(strncmp(buf,"GPIO0D6",7)==0)
	{	gpio = RK30_PIN0_PD6;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO0D6_PWM2_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}



else if(strncmp(buf,"GPIO3C7",7)==0)
	{	gpio = RK30_PIN3_PC7;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO3C7_SDMMC1WRITEPRT_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}
else if(strncmp(buf,"GPIO1A1",7)==0)
	{	gpio = RK30_PIN1_PA1;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO1A1_UART0SOUT_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}
else if(strncmp(buf,"GPIO1A0",7)==0)
	{	gpio = RK30_PIN1_PA0;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO1A0_UART0SIN_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}
else if(strncmp(buf,"GPIO0C6",7)==0)
	{	gpio = RK30_PIN0_PC6;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO0C6_TRACECLK_SMCADDR2_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}
	else if(strncmp(buf,"GPIO0C7",7)==0)
	{	gpio = RK30_PIN0_PC7;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO0C7_TRACECTL_SMCADDR3_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}
	else if(strncmp(buf,"GPIO4C3",7)==0)
	{	gpio = RK30_PIN4_PC3;	
		strncpy(req_name,buf,7);
		rk30_mux_api_set(GPIO4C3_SMCDATA3_TRACEDATA3_NAME,0);
		ret = gpio_setting_tst(  gpio,buf,req_name);
	}

#endif
end_gpio:
    return ret;///ret;
}
 void read_vol(void);
static ssize_t gpio_nwd_read(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{	
	printk("====%d=========\n",__LINE__);
	read_vol();
#ifdef GPIO_HW_TST
	int val;	
	val = gpio_get_value(gpio);
	return sprintf(buf, "%d\n", val);
#endif 
}
#define LOGO_CUSTOMIZE 				1
static ssize_t gpio_nwd_read2(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{	
#if  LOGO_CUSTOMIZE
 	///char pCalTest[1024 *600*2] = {0xf0}; ///1280
	printk("====%d=========\n",__LINE__);
  	struct file *fd;   loff_t   off= 0; 
		struct file *fd2;  
 	fd = filp_open("/dev/block/mmcblk0p4",   O_RDWR, 0666);	
 	//////////fd = filp_open("/data/wj.txt",   O_RDWR, 0666);
fd2 = filp_open("/data/logo.bmp",   O_RDWR, 0666);
        ///fd2 = filp_open("/config/app/logo.bmp",   O_RDWR, 0666);		
 	///fd = filp_open("/dev/block/mmcblk0p4",   O_RDWR, 0);	
  mm_segment_t old_fs; 
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    int vsync=0;

    for(;vsync<600;vsync++)
    {
	u8 temp[1024*3];
	loff_t pos = 0 +1024*3*vsync;
	///temp[2] = 0; 
        if (IS_ERR(fd))
        {    printk("Open update file(%d) error!", __LINE__);
            return -1;
        }

    if (IS_ERR(fd2))
        {
            printk("Open update file(%d) error!", __LINE__);
            return -1;
        }

	///printk("====%d=========\n",__LINE__);
/***
        if ((IS_ERR(fd)))
        {	printk("====%d=========\n",__LINE__);
           /// GTP_ERROR("Failed to Create file: %s for fw_header!", UPDATE_FILE_PATH_2);
            return -1;
        } */
		///printk("====%d=========\n",__LINE__);
 /// fd->f_op->llseek(fd, 0, SEEK_SET);	printk("====%d======%d===\n",__LINE__,&fd->f_pos);
    ///  fd->f_op->write(fd, pCalTest, 1280*800*2, &fd->f_pos);	printk("====%d=========\n",__LINE__);
///vfs_write(fd, pCalTest, 1280*800*2 ,  off);
///vfs_read(fd, pCalTest, 10 ,  &off);///printk("===%d=%s=========\n",__LINE__,pCalTest);

vfs_read(fd2, temp, 1024*3, &pos);
pos = 0 +1024*3*vsync;
////vfs_write(fd, "wangjinggrgr", 12, &pos);
 vfs_write(fd, temp, 1024*3, &pos);
if(vsync==599){ temp[0] = 0;temp[1] = 0;temp[2] = 0;temp[3] = 0;temp[4] = 0;printk("%02x %02x %02x %02x %02x\n",temp[0],temp[1],temp[2],temp[3],temp[4]);
 vfs_read(fd, temp, 1024*3, &pos);printk("%02x %02x %02x %02x %02x\n",temp[0],temp[1],temp[2],temp[3],temp[4]);


pos = 0x82b50;
vfs_read(fd2, temp, 1024*3, &pos);
temp[0] = 0;temp[1] = 0;temp[2] = 0;temp[3] = 0;temp[4] = 0;printk("%02x %02x %02x %02x %02x\n",temp[0],temp[1],temp[2],temp[3],temp[4]);
 vfs_read(fd, temp, 1024*3, &pos);printk("%02x %02x %02x %02x %02x\n",temp[0],temp[1],temp[2],temp[3],temp[4]);


}
///pos = 0;
///vfs_read(fd, temp, 12, &pos);
///printk("===%d=wr-read:%c=%c==%c=%c====%c=%c==%c=%c==%c=%c==%c=%c=\n",__LINE__,temp[0],temp[1],temp[2],temp[3],temp[4],temp[5],temp[6],temp[7],temp[8],temp[9],temp[10],temp[11],temp[12]);
   
}



 	set_fs(old_fs);
        filp_close(fd, NULL);
printk("====%d=====endaa===vsync:%d=\n",__LINE__,vsync);return 0;
#endif

	/// read_vol();
#ifdef GPIO_HW_TST
	int val;	
	val = gpio_get_value(gpio);
	return sprintf(buf, "%d\n", val);
#endif 
}
static struct kobj_attribute tps2546_attribute =
	__ATTR(tps2546, 0666, tps2546_nwd_read, tps2546_nwd_write);
static struct kobj_attribute gpio_attribute =
	__ATTR(gpio, 0666, gpio_nwd_read, gpio_nwd_write);
static struct kobj_attribute gpio_attribute2 =
	__ATTR(gpio2, 0666, gpio_nwd_read2, gpio_nwd_write2);
/*
 * Create a group of attributes so that we can create and destroy them all
 * at once.
 */
static struct attribute *attrs[] = {
	&tps2546_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};
static struct attribute *attrs_gpio[] = {
	&gpio_attribute.attr,
&gpio_attribute2.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};
static struct attribute_group attr_group = {
	.attrs = attrs,
};
static struct attribute_group attr_gpio_group = {
	.attrs = attrs_gpio,
};
static struct kobject *gpio_kobj;
static struct kobject *gpiotest_kobj;
static struct timer_list clk_timer_4lcdlogo_modified;
extern int   clk_disable_unused(void);


static struct timer_list led2_timer;
unsigned char TimeCount=0;
unsigned long wm8988_hot=0;
unsigned char FristStartFlag=0; 
#if defined (CONFIG_WATCHDOG)
void rk29_wdt_keepalive(void);
#endif 
wm8988_out1_pwrdn();
void led2_timer_callback(unsigned long data )
{	
	static int prev_onoff = 1; int ret;///prev_onoff =0;
#if 0 
        ret = gpio_request(RK30_PIN6_PB0, "lcd pwr en pin-d200");
        if (ret != 0) {
                        gpio_free(RK30_PIN6_PB0);
                        ret = gpio_request(RK30_PIN6_PB0,"lcd pwr en pin2");
                        if (ret != 0) {
                                printk("lcd pwr engpio_request error\n");
                                return -EIO;
                        }
                }

        gpio_direction_output(RK30_PIN6_PB0,1);
        gpio_set_value(RK30_PIN6_PB0, 0);
        gpio_free(RK30_PIN6_PB0);
#endif 

        gpio_set_value(RK30_PIN6_PB0, 0); ///wj add

	gpio_set_value(RK30_PIN4_PD3, prev_onoff);prev_onoff = !prev_onoff;
	mod_timer(&led2_timer, jiffies + msecs_to_jiffies(prev_onoff==1?300:1500));
#if defined (CONFIG_WATCHDOG)
	rk29_wdt_keepalive();
#endif 
	if(TimeCount>8)
	{  
	   FristStartFlag = 0x55;
	}
	else
	{ 
	  /// if(TimeCount==8) 
	      ///	wm8988_out1_pwrdn(); ///wj
	   TimeCount ++;
	}

	  if(wm8988_hot++ > 35 && wm8988_hot%10==0) { 
		wm8988_out1_pwrdn(); ///wj
//		printk("========================================================hot============================================\n");
	}
///mod_timer(&led2_timer, jiffies + msecs_to_jiffies(prev_onoff==1?1500:1500));
/***
	rk30_mux_api_set(GPIO3A7_SDMMC0PWREN_NAME,0);
		///ret = gpio_setting_tst(RK30_PIN3_PA7,"GPIO3A7 high","K1103 WIFI pwr");
	ret = gpio_request(RK30_PIN3_PA7, "K1103 WIFI pwr");
    		if(ret != 0)
   		 {
			gpio_free(RK30_PIN3_PA7);
			ret = gpio_request(RK30_PIN3_PA7, "wK1103 WIFI pwr");
			if(ret != 0)
			printk("gpio req error\n");
    		}

			gpio_direction_output(RK30_PIN3_PA7,1);
			gpio_set_value(RK30_PIN3_PA7, 1);*/
}
extern unsigned char wifi_on_flag;
void wifi_power_on(unsigned char onoff)
{	int ret;
///suoling wifi power
	 rk30_mux_api_set(GPIO3A7_SDMMC0PWREN_NAME,0);
	///gpio_setting_tst( RK30_PIN3_PA7,"GPIO3A7 high","hw tstout");mdelay(10);
	
	/****/
	printk_gpio("===%d %s===on:%d \n",__LINE__,__FUNCTION__,onoff);
ret = gpio_request(RK30_PIN3_PA7, "WIFIHAOBANGSHOU pwr");
    	if(ret != 0)
   	{
			gpio_free(RK30_PIN3_PA7);
			ret = gpio_request(RK30_PIN3_PA7, "wifi pwr");
			if(ret != 0)
			;///printk("gpio req error\n");
	}
	if(onoff)
        {		;///printk("===%d %s===\n",__LINE__,__FUNCTION__);
			gpio_direction_output(RK30_PIN3_PA7,1);		
			gpio_set_value(RK30_PIN3_PA7, 1);
			wifi_on_flag = 1;
	}
else 
	{		;///printk("===%d %s===\n",__LINE__,__FUNCTION__);
			gpio_direction_output(RK30_PIN3_PA7,1);		
			gpio_set_value(RK30_PIN3_PA7, 0);
			wifi_on_flag = 0;
	}
}
EXPORT_SYMBOL(wifi_power_on);
void wifi_power_sleep(unsigned char onoff) //wifi wake ,and wifi power en in the arch/arm/mach-rk30 is not the exact gpio
{	int ret;
///suoling wifi power
	 rk30_mux_api_set(GPIO3A6_SDMMC0RSTNOUT_NAME,0);
	///gpio_setting_tst( RK30_PIN3_PA7,"GPIO3A7 high","hw tstout");mdelay(10);
	
	/****/
	printk_gpio("===%d %s===on:%d \n",__LINE__,__FUNCTION__,onoff);
ret = gpio_request(RK30_PIN3_PA6, "WIFIHAOBANGSHOU sleeeep");
    	if(ret != 0)
   	{
			gpio_free(RK30_PIN3_PA6);
			ret = gpio_request(RK30_PIN3_PA6, "wifi pwrss");
			if(ret != 0)
			;///printk("gpio req error\n");
	}
	if(onoff)
  {		;///printk("===%d %s===\n",__LINE__,__FUNCTION__);
			gpio_direction_output(RK30_PIN3_PA6,1);		
			gpio_set_value(RK30_PIN3_PA6, 1);
		///	wifi_on_flag = 1;
	}
else 
	{		;///printk("===%d %s===\n",__LINE__,__FUNCTION__);
			gpio_direction_output(RK30_PIN3_PA6,1);		
			gpio_set_value(RK30_PIN3_PA6, 0);
			///wifi_on_flag = 0;
	}
}
EXPORT_SYMBOL(wifi_power_sleep);
void wifi_power_on2(unsigned char onoff)
{	int ret;
///suoling wifi power
	 rk30_mux_api_set(GPIO3A7_SDMMC0PWREN_NAME,0);
	///gpio_setting_tst( RK30_PIN3_PA7,"GPIO3A7 high","hw tstout");mdelay(10);
	
	/****/
	///printk_gpio("===%d %s===\n",__LINE__,__FUNCTION__);
ret = gpio_request(RK30_PIN3_PA7, "WIFIHAOBANGSHOU pwr");
    	if(ret != 0)
   	{
			gpio_free(RK30_PIN3_PA7);
			ret = gpio_request(RK30_PIN3_PA7, "wifi pwr");
			if(ret != 0)
			;///printk("gpio req error\n");
	}
	if(onoff){
///printk("===%d %s===\n",__LINE__,__FUNCTION__);
			gpio_direction_output(RK30_PIN3_PA7,1);		
			gpio_set_value(RK30_PIN3_PA7, 1);
		///	wifi_on_flag = 1;
	}
else 
	{		///printk("===%d %s===\n",__LINE__,__FUNCTION__);
			gpio_direction_output(RK30_PIN3_PA7,1);		
			gpio_set_value(RK30_PIN3_PA7, 0);
		///	wifi_on_flag = 0;
	}
}
EXPORT_SYMBOL(wifi_power_on2);

#define	CONFIG_NWD_K1103 1
///void sp2815_rst(void);
void d200_lcd_pwron(void)
{           int ret;
		rk30_mux_api_set(GPIO6B0_TESTCLOCKOUT_NAME,0); ///2015-1-30
	ret = gpio_request(RK30_PIN6_PB0, "lcd pwr en");
	if (ret != 0) {
			gpio_free(RK30_PIN6_PB0);
			ret = gpio_request(RK30_PIN6_PB0, "lcd pwr en pin22");
			if (ret != 0) {
				printk("led gpio_request error\n");
			///	return -EIO;
			}
		}

		gpio_direction_output(RK30_PIN6_PB0, 1);
		gpio_set_value(RK30_PIN6_PB0, 0);

}
EXPORT_SYMBOL(d200_lcd_pwron);
static int __init gpio_nwd_init(void)
{
	int retval,ret;
	grf_writel( 0x00ff0000 ,GRF_GPIO2L_EN);	  grf_writel( 0xffff0000 ,GRF_GPIO2L_EN);
	grf_writel( 0xff000000 ,GRF_GPIO2H_EN);
	grf_writel( 0xffff0000 ,GRF_GPIO3H_EN);
	grf_writel( 0xff000000,GRF_GPIO4H_EN);
////wj 1-26,for 6.5v dahuo,then reboot ,then bot high 
        grf_writel( 0x20002000, GRF_GPIO2L_DIR);
          grf_writel( 0x20002000, GRF_GPIO2L_DO);  //set gpio6_b1 output low
         /// grf_writel( 0x20002000, GRF_GPIO2L_EN);
         grf_writel( 0x20000000, GRF_GPIO2L_EN);


	rk30_mux_api_set(GPIO2B5_LCDC1DATA13_SMCADDR17_HSADCDATA8_NAME,0);
	ret = gpio_request(RK30_PIN2_PB5, "sleep display  pin when boot");
	if (ret != 0) {
			gpio_free(RK30_PIN2_PB5);
			ret = gpio_request(RK30_PIN2_PB5, "status  pin when boot");
			if (ret != 0) {
				printk("led gpio_request error\n");
			///	return -EIO;
			}
		}

		gpio_direction_output(RK30_PIN2_PB5, 1);
		gpio_set_value(RK30_PIN2_PB5, 1);


	rk30_mux_api_set(GPIO2B5_LCDC1DATA13_SMCADDR17_HSADCDATA8_NAME,0);
	ret = gpio_request(RK30_PIN2_PB5, "sleep display  pin22");
	if (ret != 0) {
			gpio_free(RK30_PIN2_PB5);
			ret = gpio_request(RK30_PIN2_PB5, "status  pin22");
			if (ret != 0) {
				printk("led gpio_request error\n");
			///	return -EIO;
			}
		}

		gpio_direction_output(RK30_PIN2_PB5, 1);
		gpio_set_value(RK30_PIN2_PB5, 1);


		rk30_mux_api_set(GPIO6B0_TESTCLOCKOUT_NAME,0); ///2015-1-30
	ret = gpio_request(RK30_PIN6_PB0, "lcd pwr en");
	if (ret != 0) {
			gpio_free(RK30_PIN6_PB0);
			ret = gpio_request(RK30_PIN6_PB0, "lcd pwr en pin22");
			if (ret != 0) {
				printk("led gpio_request error\n");
			///	return -EIO;
			}
		}

		gpio_direction_output(RK30_PIN6_PB0, 1);
		gpio_set_value(RK30_PIN6_PB0, 0);
/***
	grf_writel( 0xffff0000 ,GRF_GPIO0H_EN);	grf_writel( 0xffff0000 ,GRF_GPIO0L_EN);
	grf_writel( 0xffff0000 ,GRF_GPIO1H_EN);	grf_writel( 0xffff0000 ,GRF_GPIO1L_EN);
	grf_writel( 0xffff0000 ,GRF_GPIO2H_EN);	grf_writel( 0xffff0000 ,GRF_GPIO2L_EN);
	grf_writel( 0xffff0000 ,GRF_GPIO3H_EN);	grf_writel( 0xffff0000 ,GRF_GPIO3L_EN);
	grf_writel( 0xffff0000 ,GRF_GPIO4H_EN);	grf_writel( 0xffff0000 ,GRF_GPIO4L_EN);*/

	rk30_mux_api_set(GPIO6A1_TESTCLOCKOUT_NAME,0);
	///gpio_setting_tst(  RK30_PIN6_PA1,"GPIO6A1 low","WIFI PWR ,HBS indoor");mdelay(10);   ///116
#ifdef CONFIG_CHARGE_TPS2546
	init_nwd_tps2546();
#endif
	///led
	rk30_mux_api_set(GPIO4D3_SMCDATA11_TRACEDATA11_NAME, GPIO4D_GPIO4D3); ///RST
	ret = gpio_request(RK30_PIN4_PD3, "led  pin");
	if (ret != 0) {
			gpio_free(RK30_PIN4_PD3);
			ret = gpio_request(RK30_PIN4_PD3, "led pin");
			if (ret != 0) {
				printk("led gpio_request error\n");
				return -EIO;
			}
		}
		gpio_direction_output(RK30_PIN4_PD3, 0);
		gpio_set_value(RK30_PIN4_PD3, 0);
	setup_timer( &led2_timer, led2_timer_callback, 0 );
	mod_timer(&led2_timer, jiffies + msecs_to_jiffies(1000));
	/*
	 * Create a simple kobject with the name of "kobject_example",
	 * located under /sys/kernel/
	 *
	 * As this is a simple directory, no uevent will be sent to
	 * userspace.  That is why this function should not be used for
	 * any type of dynamic kobjects, where the name and number are
	 * not known ahead of time.
	 */
	gpio_kobj = kobject_create_and_add("tps2546_gpio", kernel_kobj);
	if (!gpio_kobj)
		return -ENOMEM;

	/* Create the files associated with this kobject */
	retval = sysfs_create_group(gpio_kobj, &attr_group);
	if (retval)
		kobject_put(gpio_kobj);

	gpiotest_kobj = kobject_create_and_add("nwdtest_gpio", kernel_kobj);
	if (!gpiotest_kobj)
		return -ENOMEM;

	/* Create the files associated with this kobject */
	retval = sysfs_create_group(gpiotest_kobj, &attr_gpio_group);
	if (retval)
		kobject_put(gpiotest_kobj);
#if 0///defined(CONFIG_HBS)
	s32 gpio_val1,gpio_val2;
	rk30_mux_api_set(GPIO4B7_SPI0CSN1_NAME,0);
	gpio_setting_tst(  RK30_PIN4_PB7,"GPIO4B7 high","hw tstout");mdelay(10);
	rk30_mux_api_set(GPIO6A1_TESTCLOCKOUT_NAME,0);
	gpio_setting_tst(  RK30_PIN6_PA1,"GPIO6A1 input","hw tstin");mdelay(10);
	gpio_val1 = gpio_get_value(RK30_PIN6_PA1);


	///rk30_mux_api_set(GPIO4B7_SPI0CSN1_NAME,0);
	gpio_setting_tst(  RK30_PIN4_PB7,"GPIO4B7 low","hw tstout2");mdelay(10);
	rk30_mux_api_set(GPIO6A1_TESTCLOCKOUT_NAME,0);
	///gpio_setting_tst(  RK30_PIN6_PA1,"GPIO6A1 input","hw tstin2");mdelay(10);
	gpio_val2 = gpio_get_value(RK30_PIN6_PA1);
	if(gpio_val1==1  && gpio_val2==0)
		tp_tst = 1;
	else 	tp_tst = 0;
	printk("=%d===\n",tp_tst);
#endif 
#if  1 ///defined(CONFIG_SUOLING)
	s32 gpio_val1,gpio_val2,gpio_val3;

	rk30_mux_api_set(GPIO4C4_SMCDATA4_TRACEDATA4_NAME,0);
	gpio_setting_tst(  RK30_PIN4_PC4,"GPIO4C4 high","hw tstout");mdelay(10);
	rk30_mux_api_set(GPIO4C6_SMCDATA6_TRACEDATA6_NAME,0);
	gpio_setting_tst(  RK30_PIN4_PC6,"GPIO4C6 input","hw tstin");mdelay(10);
	gpio_val1 = gpio_get_value(RK30_PIN4_PC6);

	///rk30_mux_api_set(GPIO4B7_SPI0CSN1_NAME,0);
	gpio_setting_tst(  RK30_PIN4_PC4,"GPIO4C4 low","hw tstout2");mdelay(10);
	///rk30_mux_api_set(GPIO6A1_TESTCLOCKOUT_NAME,0);
	///gpio_setting_tst(  RK30_PIN6_PA1,"GPIO6A1 input","hw tstin2");mdelay(10);
	gpio_val2 = gpio_get_value(RK30_PIN4_PC6);
	gpio_setting_tst(  RK30_PIN4_PC4,"GPIO4C4 high","hw tstout");mdelay(10);
	gpio_val3 = gpio_get_value(RK30_PIN4_PC6);

	if(gpio_val1==1  && gpio_val2==0&&gpio_val3==1)
		tp_tst = 1;
	else 	tp_tst = 0;
	printk("\n\n\n\n\n\n\n\n\n\n=caocaocaoa====%d===\n",tp_tst);


#endif 

/***
	///if(tp_tst == 0)
			rk30_mux_api_set(GPIO6A1_TESTCLOCKOUT_NAME,0);
	gpio_setting_tst(  RK30_PIN6_PA1,"GPIO6A1 high","wifi pwr");mdelay(10);
	 gpio_set_value(RK30_PIN6_PA1,1);*/
	 
	 
	 	rk30_mux_api_set(GPIO3B6_SDMMC0DETECTN_NAME,0);
	gpio_setting_tst(  RK30_PIN3_PB6,"GPIO3B6 high","hw tstout");mdelay(10);
	
	
		printk_gpio("===%d %s===\n",__LINE__,__FUNCTION__);
		ret = gpio_request(RK30_PIN3_PB6, "WIFIHAOBANGSHOU CS");
    		if(ret != 0)
   		 {
			gpio_free(RK30_PIN3_PB6);
			ret = gpio_request(RK30_PIN3_PB6, "wifi cs");
			if(ret != 0)
			printk("gpio req error\n");
    		}

			gpio_direction_output(RK30_PIN3_PB6,0);
			gpio_set_value(RK30_PIN3_PB6, 0);
			printk_gpio("===%d %s===\n",__LINE__,__FUNCTION__);
	
	

///K1103 bt pwr en  ,wifi wr en
	rk30_mux_api_set(GPIO2C0_LCDCDATA16_GPSCLK_HSADCCLKOUT_NAME,0);
		ret = gpio_setting_tst(RK30_PIN2_PC0,"GPIO2C0 high","K1103 BT pwr");///mdelay(3000);

///d200 ,tp pwr en ,2015-1-10
		rk30_mux_api_set(GPIO2A1_LCDC1DATA1_SMCADDR5_NAME,0);
		ret = gpio_setting_tst(RK30_PIN2_PA1,"GPIO2A1 high","K1103 TP pwr");///mdelay(3000);
	/***	ret = gpio_setting_tst(RK30_PIN2_PC0,"GPIO2C0 low","K1103 BT pwr");mdelay(3000);
		ret = gpio_setting_tst(RK30_PIN2_PC0,"GPIO2C0 high","K1103 BT pwr");mdelay(3000);
		ret = gpio_setting_tst(RK30_PIN2_PC0,"GPIO2C0 low","K1103 BT pwr");mdelay(3000);

		ret = gpio_setting_tst(RK30_PIN2_PC0,"GPIO2C0 high","K1103 BT pwr");mdelay(3000);
		ret = gpio_setting_tst(RK30_PIN2_PC0,"GPIO2C0 low","K1103 BT pwr");mdelay(3000);

		ret = gpio_setting_tst(RK30_PIN2_PC0,"GPIO2C0 high","K1103 BT pwr");mdelay(3000);
		///ret = gpio_setting_tst(RK30_PIN2_PC0,"GPIO2C0 low","K1103 BT pwr");mdelay(3000);*/
#ifdef CONFIG_NWD_K1103

	rk30_mux_api_set(GPIO3A7_SDMMC0PWREN_NAME,0);
		///ret = gpio_setting_tst(RK30_PIN3_PA7,"GPIO3A7 high","K1103 WIFI pwr");
	ret = gpio_request(RK30_PIN3_PA7, "K1103 WIFI pwr");
    		if(ret != 0)
   		 {
			gpio_free(RK30_PIN3_PA7);
			ret = gpio_request(RK30_PIN3_PA7, "wK1103 WIFI pwr");
			if(ret != 0)
			printk("gpio req error\n");
    		}

			gpio_direction_output(RK30_PIN3_PA7,1);
			gpio_set_value(RK30_PIN3_PA7, 0);mdelay(2000);
			gpio_set_value(RK30_PIN3_PA7, 1);printk("=1 \n");/*** mdelay(8000);gpio_set_value(RK30_PIN3_PA7, 0);printk("=0 \n");mdelay(8000);
			gpio_set_value(RK30_PIN3_PA7, 1);printk("=1 \n");mdelay(8000);gpio_set_value(RK30_PIN3_PA7, 0);printk("=0 \n");mdelay(8000);
			///gpio_set_value(RK30_PIN3_PA7, 1);printk("=1 \n");mdelay(5000);gpio_set_value(RK30_PIN3_PA7, 0);printk("=0 \n");mdelay(5000);
			gpio_set_value(RK30_PIN3_PA7, 1);printk("=1 \n");///mdelay(5000);///gpio_set_value(RK30_PIN3_PA7, 0);mdelay(3000);*/
		///	gpio_set_value(RK30_PIN3_PA7, 0);mdelay(2);
		///	gpio_set_value(RK30_PIN3_PA7, 1);printk("=1 \n");
#endif 
	rk30_mux_api_set(GPIO3D0_SDMMC1PWREN_NAME,0);
	gpio_setting_tst(  RK30_PIN3_PD0,"GPIO3D0 high","sd pwrEN");mdelay(10);///d200
	
	rk30_mux_api_set(GPIO1A6_UART1CTSN_SPI0RXD_NAME ,0);
	gpio_setting_tst(  RK30_PIN1_PA6,"GPIO1A6 high","gps pwrEN");mdelay(10);///d200
	
	rk30_mux_api_set(GPIO3D0_SDMMC1PWREN_NAME,0);
	gpio_setting_tst(  RK30_PIN3_PD0,"GPIO3D0 high","sd pwrEN");mdelay(10);

	rk30_mux_api_set(GPIO0D6_PWM2_NAME,0);
	gpio_setting_tst( RK30_PIN0_PD6,"GPIO0D6 high","hdmi pwr hbs_indoor");mdelay(10);///pin 126

    ///CAM PWRDN
	rk30_mux_api_set(GPIO6A5_TESTCLOCKOUT_NAME,0);
    ///CAM PWR
	rk30_mux_api_set(GPIO6A7_TESTCLOCKOUT_NAME,0);
	gpio_setting_tst(RK30_PIN6_PA7,"GPIO0D6 high","CAM pwr, hbs_indoor");mdelay(10);

	gpio_setting_tst(  RK30_PIN6_PA1,"GPIO6A1 high","WIFI PWR ,HBS indoor");	 gpio_set_value(RK30_PIN6_PA1,1);///mdelay(10);
	///sp2815_rst();
///hbs indoor bt pwr en 
	rk30_mux_api_set(GPIO2C4_LCDC1DATA20_SPI1CSN0_HSADCDATA1_NAME,0);
	gpio_setting_tst(RK30_PIN2_PC4,"GPIO2C4 high","BT pwr, hbs_indoor");///mdelay(10);  ///123
///hbs indoor BL pwr en -L ACTIVE 
	rk30_mux_api_set(GPIO0A4_PWM1_NAME,0);
	gpio_setting_tst(RK30_PIN0_PA4,"GPIO0A4 low","BL pwr, hbs_indoor");///mdelay(10);  ///125

	rk30_mux_api_set(GPIO2B5_LCDC1DATA13_SMCADDR17_HSADCDATA8_NAME,0); ///d200 51,sleep indcate gpio
	gpio_setting_tst(RK30_PIN2_PB5,"GPIO2B5 hign","sleep indcate gpio");///mdelay(10);  ///125
	return retval;  
}

static void __exit gpio_nwd_exit(void)
{
	kobject_put(gpio_kobj);
}

///core_initcall(gpio_nwd_init);
subsys_initcall(gpio_nwd_init);
module_exit(gpio_nwd_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Greg Kroah-Hartman <greg@kroah.com>");
