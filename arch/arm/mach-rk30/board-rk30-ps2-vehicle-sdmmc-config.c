/*****************************************************************************************
 * arch/arm/mach-rkxx/baord-xxx-sdmmc-config.c
 *
 * Copyright (C) 2013 ROCKCHIP, Inc.
 *
 * Description: 
 *     define the gpio for SD-MMC-SDIO-Wifi functions  according to your own projects.
 *
 * Author: Michael Xie
 *         15 Jan,2013
 * E-mail: xbw@rock-chips.com
 *
 ******************************************************************************************/

/*
** If you select the macro of CONFIG_SDMMC0_RK29_WRITE_PROTECT, You must define the following values.
** Otherwise, there is no need to define the following values。
*/
//#define SDMMC0_WRITE_PROTECT_PIN	            RK30_PIN3_PB2	//According to your own project to set the value of write-protect-pin.
//#define SDMMC0_WRITE_PROTECT_ENABLE_VALUE     GPIO_HIGH

/*
** If you select the macro of CONFIG_SDMMC1_RK29_WRITE_PROTECT, You must define the following values.
** Otherwise, there is no need to define the following values。
*/
//#define SDMMC1_WRITE_PROTECT_PIN	            RK30_PIN3_PB3	//According to your own project to set the value of write-protect-pin.
//#define SDMMC1_WRITE_PROTECT_ENABLE_VALUE     GPIO_HIGH
    
/*
** If you select the macro of CONFIG_RK29_SDIO_IRQ_FROM_GPIO, You must define the following values.
** Otherwise, there is no need to define the following values。
*/
//#define RK29SDK_WIFI_SDIO_CARD_INT         RK30_PIN3_PD2


/*
* define sdcard PowerEn-pin
*/
#define RK29SDK_SD_CARD_PWR_EN                  RK30_PIN3_PA7
#define RK29SDK_SD_CARD_PWR_EN_LEVEL            GPIO_LOW 
#define RK29SDK_MMC1_CARD_PWR_EN                  RK30_PIN3_PD0
#define RK29SDK_MMC1_CARD_PWR_EN_LEVEL            GPIO_LOW 
int rk31sdk_get_sdmmc0_pin_io_voltage(void)
{
    int voltage;
#define RK31SDK_SET_SDMMC0_PIN_VOLTAGE

    /**************************************************************************************
    **  Please tell me how much voltage of your SDMMC0-pin in your project. 
    **
    **     例如: 有的项目，它的SDMMC0所在的RK主控的IO组，想用1.8V, 而卡本身用3.3V, 
    **  而中间通过个电平转换.那么，您此时，应该设置下面的voltage值为 1.8V(即1800mv)
    ***************************************************************************************/
    voltage = 3300;  //default the voltage 3300mv. 

    return voltage;
}

/*
* define the card-detect-pin.
*/
#define RK29SDK_SD_CARD_DETECT_N                RK30_PIN3_PB6  //According to your own project to set the value of card-detect-pin.
#define RK29SDK_SD_CARD_INSERT_LEVEL             GPIO_LOW       // set the voltage of insert-card. Please pay attention to the default setting.

#define RK29SDK_MMC1_CARD_DETECT_N              RK30_PIN3_PC6  //According to your own project to set the value of card-detect-pin.
#define RK29SDK_MMC1_CARD_INSERT_LEVEL            GPIO_LOW       // set the voltage of insert-card. Please pay attention to the default setting.
//due to no define the NEW-IMOUX-API,so define the macro again
#define RK29SDK_SD_CARD_DETECT_PIN_NAME         GPIO3B6_SDMMC0DETECTN_NAME
#define RK29SDK_SD_CARD_DETECT_IOMUX_FGPIO      GPIO3B_GPIO3B6
#define RK29SDK_SD_CARD_DETECT_IOMUX_FMUX       GPIO3B_SDMMC0_DETECT_N

#define RK29SDK_MMC1_CARD_DETECT_PIN_NAME       GPIO3C6_SDMMC1DETECTN_NAME
#define RK29SDK_MMC1_CARD_DETECT_IOMUX_FGPIO    GPIO3C_GPIO3C6
#define RK29SDK_MMC1_CARD_DETECT_IOMUX_FMUX     GPIO3C_SDMMC1_DETECT_N

//wake up host gpio
#define RK30SDK_WIFI_GPIO_WIFI_INT_B                INVALID_GPIO
#define RK30SDK_WIFI_GPIO_WIFI_INT_B_ENABLE_VALUE   GPIO_HIGH

/*
* Define wifi module's power and reset gpio, and gpio sensitive level.
* Please set the value according to your own project.
*
* Well, you just own engineering module to set the value in the corresponding module branch.
* Otherwise, you do not define this macro, eliminate it.
*
*/          
#if defined(CONFIG_RTL8192CU) || defined(CONFIG_RTL8188EU) 
    #define RK30SDK_WIFI_GPIO_POWER_N               RK30_PIN3_PD0            
    #define RK30SDK_WIFI_GPIO_POWER_ENABLE_VALUE    GPIO_LOW//GPIO_HIGH        
    
#elif defined(CONFIG_BCM4330) || defined(CONFIG_BCM4329) || defined(CONFIG_RKWIFI) || defined(CONFIG_RTL8723BS)
	#define RK30SDK_WIFI_GPIO_POWER_N               RK30_PIN3_PD0                 
    #define RK30SDK_WIFI_GPIO_POWER_ENABLE_VALUE    GPIO_HIGH

    #define RK30SDK_WIFI_GPIO_RESET_N               RK30_PIN2_PA7
    #define RK30SDK_WIFI_GPIO_RESET_ENABLE_VALUE    GPIO_HIGH 

#elif defined(CONFIG_MT5931_MT6622) || defined(CONFIG_MT5931)

	#ifdef  CONFIG_MACH_RK3168_LR097 
    	#define RK30SDK_WIFI_GPIO_POWER_N               RK30_PIN3_PD0 
    	#define RK30SDK_WIFI_GPIO_POWER_ENABLE_VALUE    GPIO_HIGH

    	//#define RK30SDK_WIFI_GPIO_RESET_N 	            RK30_PIN3_PD1
    	//#define RK30SDK_WIFI_GPIO_RESET_ENABLE_VALUE    GPIO_HIGH

	#else
    	#define RK30SDK_WIFI_GPIO_POWER_N               RK30_PIN0_PA5
    	#define RK30SDK_WIFI_GPIO_POWER_ENABLE_VALUE    GPIO_HIGH

    	#define RK30SDK_WIFI_GPIO_RESET_N               RK30_PIN3_PD1
    	#define RK30SDK_WIFI_GPIO_RESET_ENABLE_VALUE    GPIO_HIGH	
	#endif

#elif defined(CONFIG_ESP8089)
	#define RK30SDK_WIFI_GPIO_POWER_N               RK30_PIN3_PD0
	#define RK30SDK_WIFI_GPIO_POWER_ENABLE_VALUE    GPIO_HIGH

#elif defined(CONFIG_MT6620)
    #define COMBO_MODULE_MT6620_CDT    0  // to control antsel2,antsel3 and gps_lan foot when using AcSip or Cdtech chip. 
	                                      //- 1--use Cdtech chip; 0--unuse CDT chip

    //power, PMU_EN
    #define RK30SDK_WIFI_GPIO_POWER_N                   RK30_PIN0_PB5//INVALID_GPIO//RK30_PIN3_PC7            
    #define RK30SDK_WIFI_GPIO_POWER_ENABLE_VALUE        GPIO_HIGH        
    //reset, DAIRST,SYSRST_B
    #define RK30SDK_WIFI_GPIO_RESET_N                   RK30_PIN3_PD0//RK30_PIN3_PD1
    #define RK30SDK_WIFI_GPIO_RESET_ENABLE_VALUE        GPIO_HIGH 
    //VDDIO
    //#define RK30SDK_WIFI_GPIO_VCCIO_WL                  RK30_PIN0_PD2 //You do not get control of the foot, and you do not need to define the macro 
    //#define RK30SDK_WIFI_GPIO_VCCIO_WL_ENABLE_VALUE     GPIO_HIGH
    //WIFI_INT_B
    #define RK30SDK_WIFI_GPIO_WIFI_INT_B                RK30_PIN3_PD2
    #define RK30SDK_WIFI_GPIO_WIFI_INT_B_ENABLE_VALUE   GPIO_HIGH 
    //BGF_INT_B
    #define RK30SDK_WIFI_GPIO_BGF_INT_B                 RK30_PIN0_PA5
    #define RK30SDK_WIFI_GPIO_BGF_INT_B_ENABLE_VALUE    GPIO_HIGH 
    //GPS_SYNC
    #define RK30SDK_WIFI_GPIO_GPS_SYNC                  INVALID_GPIO//RK30_PIN3_PD1
    #define RK30SDK_WIFI_GPIO_GPS_SYNC_ENABLE_VALUE     GPIO_HIGH 

    #if COMBO_MODULE_MT6620_CDT
    //ANTSEL2
    #define RK30SDK_WIFI_GPIO_ANTSEL2                   RK30_PIN4_PD4
    #define RK30SDK_WIFI_GPIO_ANTSEL2_ENABLE_VALUE      GPIO_LOW    //use 6620 in CDT chip, LOW--work; High--no work.
    //ANTSEL3
    #define RK30SDK_WIFI_GPIO_ANTSEL3                   RK30_PIN4_PD3
    #define RK30SDK_WIFI_GPIO_ANTSEL3_ENABLE_VALUE      GPIO_HIGH    //use 6620 in CDT chip, High--work; Low--no work..
    //GPS_LAN
    #define RK30SDK_WIFI_GPIO_GPS_LAN                   RK30_PIN4_PD6
    #define RK30SDK_WIFI_GPIO_GPS_LAN_ENABLE_VALUE      GPIO_HIGH    //use 6620 in CDT chip, High--work; Low--no work..
    #endif // #if COMBO_MODULE_MT6620_CDT--#endif
#endif 

#ifndef RK30SDK_WIFI_GPIO_WIFI_INT_B
#define RK30SDK_WIFI_GPIO_WIFI_INT_B              RK30_PIN3_PD2
#define RK30SDK_WIFI_GPIO_WIFI_INT_B_ENABLE_VALUE   GPIO_HIGH 
#endif

int rk31sdk_get_sdio_wifi_voltage(void)
{
    int voltage;
    
    /******************************************************************************
    **  Please tell me how much wifi-module uses voltage in your project.  
    ******************************************************************************/
#if defined(CONFIG_BCM4330) || defined(CONFIG_BCM4329) || defined(CONFIG_RKWIFI)
    voltage = 1800 ; //power 1800mV
    
#elif defined(CONFIG_MT5931_MT6622)||defined(CONFIG_MT5931)
    voltage = 1800 ; //power 1800V
#elif defined(CONFIG_ESP8089)
	voltage = 3000 ; //power 3000V
#elif defined(CONFIG_MT6620) 
    voltage = 2800 ; //power 2800V
#elif defined(CONFIG_RDA5990)||defined(CONFIG_RTL8723AS) || defined(CONFIG_RTL8723BS) 
    voltage = 3300 ; //power 3300V
#else
    //default, sdio use 3.0V
    voltage = 3000 ; //power 3000V
#endif

    return voltage;
}






///wj add ,raw in  board-rk30-sdk-sdmmc.c ,for usb-wifi (may not used this)
int rk29sdk_wifi_power(int on)
{
    return 0;
}
EXPORT_SYMBOL(rk29sdk_wifi_power);
   


///static int rk29sdk_wifi_cd = 0;   // wifi virtual 'card detect' status 
static void (*wifi_status_cb)(int card_present, void *dev_id);
static void *wifi_status_cb_devid;
static int rk29sdk_wifi_reset_state;
static int rk29sdk_wifi_reset(int on)
{
        pr_info("%s: %d\n", __func__, on);
        //mdelay(100);
        rk29sdk_wifi_reset_state = on;
        return 0;
}
/***
int rk29sdk_wifi_set_carddetect(int val)
{
        pr_info("%s:%d\n", __func__, val);
        rk29sdk_wifi_cd = val;
        if (wifi_status_cb){
                wifi_status_cb(val, wifi_status_cb_devid);
        }else {
                pr_warning("%s, nobody to notify\n", __func__);
        }
        return 0;
}
EXPORT_SYMBOL(rk29sdk_wifi_set_carddetect);*/

#include <linux/etherdevice.h>
u8 wifi_custom_mac_addr[6] = {0,0,0,0,0,0};
extern char GetSNSectorInfo(char * pbuf);
int rk29sdk_wifi_mac_addr(unsigned char *buf)
{
       char mac_buf[20] = {0};
    printk("rk29sdk_wifi_mac_addr.\n");

    // from vflash
    if(is_zero_ether_addr(wifi_custom_mac_addr)) {
       int i;
       char *tempBuf = kmalloc(512, GFP_KERNEL);
       if(tempBuf) {
           GetSNSectorInfo(tempBuf);
           for (i = 506; i <= 511; i++)
    wifi_custom_mac_addr[i-506] = tempBuf[i];
           kfree(tempBuf);
       } else {
           return -1;
       }
    }

    sprintf(mac_buf,"%02x:%02x:%02x:%02x:%02x:%02x",wifi_custom_mac_addr[0],wifi_custom_mac_addr[1],
    wifi_custom_mac_addr[2],wifi_custom_mac_addr[3],wifi_custom_mac_addr[4],wifi_custom_mac_addr[5]);
    printk("falsh wifi_custom_mac_addr=[%s]\n", mac_buf);

    if (is_valid_ether_addr(wifi_custom_mac_addr)) {
	if (2 == (wifi_custom_mac_addr[0] & 0x0F)) {
            printk("This mac address come into conflict with the address of direct, ignored...\n");
            return -1;
        }
    } else {
        printk("This mac address is not valid, ignored...\n");
        return -1;
    }

#if defined(CONFIG_RKWIFI)
    memcpy(buf, wifi_custom_mac_addr, 6);
#else
 memcpy(buf, mac_buf, strlen(mac_buf));//realtek's wifi use this branch
#endif
    return 0;
}
EXPORT_SYMBOL(rk29sdk_wifi_mac_addr);

static struct resource resources[] = {
	{
		.start = RK30SDK_WIFI_GPIO_WIFI_INT_B,
		.flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL | IORESOURCE_IRQ_SHAREABLE,
		.name = "bcmdhd_wlan_irq",
	},
};
#if defined(CONFIG_WIFI_CONTROL_FUNC)
static struct wifi_platform_data rk29sdk_wifi_control = {
        .set_power = rk29sdk_wifi_power,
        .set_reset = rk29sdk_wifi_reset,
        .set_carddetect = rk29sdk_wifi_set_carddetect,
        .mem_prealloc   = rk29sdk_mem_prealloc,
		.get_mac_addr   = rk29sdk_wifi_mac_addr,
};

static struct platform_device rk29sdk_wifi_device = {
        .name = "bcmdhd_wlan",
        .id = 1,
        .num_resources = ARRAY_SIZE(resources),
 .resource = resources,
        .dev = {
                .platform_data = &rk29sdk_wifi_control,
         },
};

#elif defined(CONFIG_WIFI_COMBO_MODULE_CONTROL_FUNC)
///wj del 

#endif


/***************************************************wj from xxx-sdmmc.c************************
int rk29sdk_wifi_mac_addr(unsigned char *buf)
{
       char mac_buf[20] = {0};
    printk("rk29sdk_wifi_mac_addr.\n");

   // from vflash
   if(is_zero_ether_addr(wifi_custom_mac_addr)) {
      int i;
      char *tempBuf = kmalloc(512, GFP_KERNEL);
       if(tempBuf) {
           GetSNSectorInfo(tempBuf);
           for (i = 506; i <= 511; i++)
               wifi_custom_mac_addr[i-506] = tempBuf[i];
              kfree(tempBuf);
       } else {
           return -1;
       }
    }

    sprintf(mac_buf,"%02x:%02x:%02x:%02x:%02x:%02x",wifi_custom_mac_addr[0],wifi_custom_mac_addr[1],
   wifi_custom_mac_addr[2],wifi_custom_mac_addr[3],wifi_custom_mac_addr[4],wifi_custom_mac_addr[5]);
 printk("falsh wifi_custom_mac_addr=[%s]\n", mac_buf);

    if (is_valid_ether_addr(wifi_custom_mac_addr)) {
		if (2 == (wifi_custom_mac_addr[0] & 0x0F)) {
            printk("This mac address come into conflict with the address of direct, ignored...\n");
           return -1;
       }
   } else {
       printk("This mac address is not valid, ignored...\n"); 
       return -1;    }
#if defined(CONFIG_RKWIFI)
   memcpy(buf, wifi_custom_mac_addr, 6);
#else
   memcpy(buf, mac_buf, strlen(mac_buf));//realtek's wifi use this branch
#endif
   return 0;
}
EXPORT_SYMBOL(rk29sdk_wifi_mac_addr);****/
