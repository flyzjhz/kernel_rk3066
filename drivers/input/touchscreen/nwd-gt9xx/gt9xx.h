/* drivers/input/touchscreen/gt9xx.h
 * 
 * 2010 - 2013 Goodix Technology.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be a reference 
 * to you, when you are integrating the GOODiX's CTP IC into your system, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 * 
 */

#ifndef _GOODIX_GT9XX_H_
#define _GOODIX_GT9XX_H_

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


//***************************PART1:ON/OFF define*******************************


#define GTP_CUSTOM_CFG        0
#define GTP_CHANGE_X2Y        0
#define GTP_DRIVER_SEND_CFG   1


#define GTP_HAVE_TOUCH_KEY    1

#define GTP_POWER_CTRL_SLEEP  0
#define GTP_ICS_SLOT_REPORT   1 
#define GTP_AUTO_UPDATE       0    // auto update fw by .bin file as default
#define GTP_HEADER_FW_UPDATE  0    // auto update fw by gtp_default_FW in gt9xx_firmware.h, function together with GTP_AUTO_UPDATE
#define GTP_AUTO_UPDATE_CFG   0    // auto update config by .cfg file, function together with GTP_AUTO_UPDATE

#define GTP_COMPATIBLE_MODE   0    // compatible with GT9XXF

#define GTP_CREATE_WR_NODE    1
#define GTP_ESD_PROTECT       0    // esd protection with a cycle of 2 seconds
#define GTP_WITH_PEN          0

#define GTP_SLIDE_WAKEUP      0
#define GTP_DBL_CLK_WAKEUP    0    // double-click wakeup, function together with GTP_SLIDE_WAKEUP

#define GTP_DEBUG_ON          0
#define GTP_DEBUG_ARRAY_ON    0
#define GTP_DEBUG_FUNC_ON     0


#if GTP_COMPATIBLE_MODE
typedef enum
{
    CHIP_TYPE_GT9  = 0,
    CHIP_TYPE_GT9F = 1,
} CHIP_TYPE_T;
#endif

struct nwdkey{
	u8 short_keyvalue;  //0 meaning no short touch key
	u8 long_keyvalue;   //0 meaning no long touch key
	short key_x;
	short key_y;
	u8 key_range;
	u8 report_status;
	u32 down_jiffies;
	u32 total_jiffies;
	
};

struct nwd_ts_panel{
	const char *name;
	char *config;
	u16 index;
	
	u16 pixel_x;
	u16 pixel_y;

	u16 l_x;
	u16 up_y;
	u16 r_x;
	u16 down_y;

	s8 keyindex;
	u8 int_trigger;
	struct nwdkey key[16];
	
	void (*mintomax)(struct nwd_ts_panel *panel, int *x, int *y);
};


struct goodix_ts_data {
    spinlock_t irq_lock;
    struct i2c_client *client;
    struct input_dev  *input_dev;
    struct hrtimer timer;
    struct work_struct  work;
    struct early_suspend early_suspend;
    struct nwd_ts_panel *nwd_ts_panel;
    s32 irq_is_disable;
    s32 use_irq;
    u16 abs_x_max;
    u16 abs_y_max;
    u8  max_touch_num;
    u8  int_trigger_type;
    u8  green_wake_mode;
    u8  enter_update;
    u8  gtp_is_suspend;
    u8  gtp_rawdiff_mode;
    u8  gtp_cfg_len;
    u8  fixed_cfg;
    u8  fw_error;
    u8  pnl_init_error;
    
#if GTP_ESD_PROTECT
    spinlock_t esd_lock;
    u8  esd_running;
    s32 clk_tick_cnt;
#endif

#if GTP_COMPATIBLE_MODE
    u16 bak_ref_len;
    s32 ref_chk_fs_times;
    s32 clk_chk_fs_times;
    CHIP_TYPE_T chip_type;
    u8 rqst_processing;
    u8 is_950;
#endif
    
};

struct gtp_event{
    u8 touch_id;
    u8 down_jiffies;
    u8 total_jiffies;
    u8 keyevent;
    u8 keystate;
}; 

extern u16 show_len;
extern u16 total_len;
extern int project_id;


//*************************** PART2:TODO define **********************************
// STEP_1(REQUIRED): Define Configuration Information Group(s)
// Sensor_ID Map:
/* sensor_opt1 sensor_opt2 Sensor_ID
    GND         GND         0 
    VDDIO       GND         1 
    NC          GND         2 
    GND         NC/300K     3 
    VDDIO       NC/300K     4 
    NC          NC/300K     5 
*/
// TODO: define your own default or for Sensor_ID == 0 config here. 
// The predefined one is just a sample config, which is not suitable for your tp in most cases.

#define CTP_CFG_GROUP1 {0x40,0x00,0x04,0x58,0x02,0x05,0x4D,0x00,0x02,0x0B,0x28,0x0F,0x5F,0x4B,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x90,0x30,0xCC,0x3E,0x3B,0x0C,0x08,0x00,0x00,0x00,0x02,0x02,0x1D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2D,0x64,0x94,0xC5,0x02,0x07,0x00,0x00,0x00,0x9B,0x30,0x00,0x85,0x39,0x00,0x74,0x43,0x00,0x67,0x4E,0x00,0x5B,0x5C,0x00,0x5A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1D,0x1C,0x1B,0x1A,0x19,0x18,0x17,0x16,0x15,0x14,0x11,0x10,0x0F,0x0E,0x0D,0x0C,0x09,0x08,0x07,0x06,0x05,0x04,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2A,0x29,0x28,0x27,0x26,0x25,0x24,0x23,0x22,0x21,0x20,0x1F,0x1E,0x1C,0x1B,0x19,0x14,0x13,0x12,0x11,0x10,0x0F,0x0E,0x0D,0x0C,0x0A,0x08,0x07,0x06,0x04,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x8B,0x01}

// TODO: define your config for Sensor_ID == 1 here, if needed
#define CTP_CFG_GROUP2 {\
    }

// TODO: define your config for Sensor_ID == 2 here, if needed
#define CTP_CFG_GROUP3 {\
    }

// TODO: define your config for Sensor_ID == 3 here, if needed
#define CTP_CFG_GROUP4 {\
    }

// TODO: define your config for Sensor_ID == 4 here, if needed
#define CTP_CFG_GROUP5 {\
    }

// TODO: define your config for Sensor_ID == 5 here, if needed
#define CTP_CFG_GROUP6 {\
    }

// STEP_2(REQUIRED): Customize your I/O ports & I/O operations
#define GTP_RST_PORT    RK30_PIN4_PC5 ///RK30_PIN4_PD0 ///S5PV210_GPJ3(6)
#define GTP_INT_PORT    RK30_PIN4_PC2 ///S5PV210_GPH1(3)
#define GTP_INT_IRQ     gpio_to_irq(GTP_INT_PORT)
///#define GTP_INT_CFG     S3C_GPIO_SFN(0xF)

#define GTP_GPIO_AS_INPUT(pin)          do{\
                                            gpio_direction_input(pin);\
                                            gpio_pull_updown(pin,0);\
                                        }while(0)
#define GTP_GPIO_AS_INT(pin)            do{\
                                            GTP_GPIO_AS_INPUT(pin);\
                                        }while(0)
#define GTP_GPIO_GET_VALUE(pin)         gpio_get_value(pin)
#define GTP_GPIO_OUTPUT(pin,level)      gpio_direction_output(pin,level)
#define GTP_GPIO_REQUEST(pin, label)    gpio_request(pin, label)
#define GTP_GPIO_FREE(pin)              gpio_free(pin)
#define GTP_IRQ_TAB                     {IRQ_TYPE_EDGE_RISING, IRQ_TYPE_EDGE_FALLING, IRQ_TYPE_LEVEL_LOW, IRQ_TYPE_LEVEL_HIGH}

// STEP_3(optional): Specify your special config info if needed

//warn : 中断触发的配置放到了结构体nwd_ts_panel里面配置，如果这个指针为空，才用这里的默认配置
#if GTP_CUSTOM_CFG
  #define GTP_MAX_HEIGHT   800
  #define GTP_MAX_WIDTH    480
  #define GTP_INT_TRIGGER  0            // 0: Rising 1: Falling
#else
  #define GTP_MAX_HEIGHT   1024
  #define GTP_MAX_WIDTH    600
  #define GTP_INT_TRIGGER  1
#endif
#define GTP_MAX_TOUCH         5

#define   KEY_VOLUMEUP_NWD   248
#define   KEY_VOLUMEDOWN_NWD 249
#define   KEY_POWEROFF_NWD   251
#define   KEY_DISPLAYTOGGLE_NWD  250
#define   NAVI_NWD  		254
#define   KEY_MENU			139
#define   KEY_SETTING		242 
#define   KEY_ALLAPP		243
#define   KEY_SWITCH_MOD	244
#define   KEY_DISPLAY_NWD	253

// STEP_4(optional): If keys are available and reported as keys, config your key info here                             
#if GTP_HAVE_TOUCH_KEY
   // #define GTP_KEY_TAB  {KEY_MENU, KEY_HOME, KEY_BACK, KEY_VOLUMEDOWN, KEY_VOLUMEUP, KEY_PWR_SCREEN}

// KEY_CAMERA   212   //camera 360
// KEY_F1      59   //mic
// KEY_F2      60    //radio 
// KEY_F3      61  //A/C
// KEY_F4      62   //空调风向
// KEY_F5      63   //车内温度+
// KEY_F6      64   //车内温度-
// KEY_F7      65   //风量加大
// KEY_F8      66   //风量减小
// KEY_F9      67   //出气
// KEY_F10     68   //车内循环
// KEY_F11     69   //A/C off


#define KEY_NWD_MIC   		224   //mic
#define KEY_NWD_RADIO 		225   //radio
#define KEY_NWD_AC_ON		226   //AC ON
#define KEY_NWD_AC_OFF 		227   //AC OFF
#define KEY_NWD_WIND_DIRECTION 	228   //空调风向
#define KEY_NWD_TEM_UP		229   //车内温度+
#define KEY_NWD_TEM_DOWN   	230   //车内温度-
#define KEY_NWD_WIND_UP		231   //风量加大
#define KEY_NWD_WIND_DOWN 	232   //风量减小
#define KEY_NWD_WIND_OUT   	233   //出气
#define KEY_NWD_WIND_CIRCLE	234   //车内循环
#define KEY_NWD_CAMERA		235   //车内循环


#if defined(CONFIG_JILI_HONGJING)
#define GTP_KEY_TAB  {KEY_NWD_CAMERA, KEY_NWD_MIC, KEY_NWD_RADIO, KEY_NWD_AC_ON, KEY_NWD_AC_OFF, KEY_NWD_TEM_UP, KEY_NWD_TEM_DOWN, KEY_NWD_WIND_DIRECTION, KEY_NWD_WIND_UP, KEY_NWD_WIND_DOWN, KEY_NWD_WIND_OUT, KEY_NWD_WIND_CIRCLE, KEY_HOME, KEY_VOLUMEDOWN_NWD, KEY_VOLUMEUP_NWD, KEY_DISPLAYTOGGLE_NWD, KEY_POWEROFF_NWD, KEY_CAMERA}
#else
#define GTP_KEY_TAB  {KEY_F11, KEY_F12, KEY_BACK, KEY_HOME, KEY_VOLUMEDOWN_NWD, KEY_VOLUMEUP_NWD, KEY_DISPLAYTOGGLE_NWD, KEY_POWEROFF_NWD, NAVI_NWD, KEY_MENU, KEY_SETTING, KEY_ALLAPP, KEY_INSERT}
#endif
#endif
//***************************PART3:OTHER define*********************************
#define GTP_DRIVER_VERSION    "V2.0<2013/08/28>"
#define GTP_I2C_NAME          "Goodix-TS"
#define GTP_POLL_TIME         10    
#define GTP_ADDR_LENGTH       2
#define GTP_CONFIG_MIN_LENGTH 186
#define GTP_CONFIG_MAX_LENGTH 240
#define FAIL                  0
#define SUCCESS               1
#define SWITCH_OFF            0
#define SWITCH_ON             1

//******************** For GT9XXF Start **********************//
#define GTP_REG_BAK_REF                 0x99D0
#define GTP_REG_MAIN_CLK                0x8020
#define GTP_REG_CHIP_TYPE               0x8000
#define GTP_REG_HAVE_KEY                0x804E
#define GTP_REG_MATRIX_DRVNUM           0x8069     
#define GTP_REG_MATRIX_SENNUM           0x806A

#define GTP_FL_FW_BURN              0x00
#define GTP_FL_ESD_RECOVERY         0x01
#define GTP_FL_READ_REPAIR          0x02

#define GTP_BAK_REF_SEND                0
#define GTP_BAK_REF_STORE               1
#define CFG_LOC_DRVA_NUM                29
#define CFG_LOC_DRVB_NUM                30
#define CFG_LOC_SENS_NUM                31

#define GTP_CHK_FW_MAX                  40
#define GTP_CHK_FS_MNT_MAX              300
#define GTP_BAK_REF_PATH                "/data/gtp_ref.bin"
#define GTP_MAIN_CLK_PATH               "/data/gtp_clk.bin"
#define GTP_RQST_CONFIG                 0x01
#define GTP_RQST_BAK_REF                0x02
#define GTP_RQST_RESET                  0x03
#define GTP_RQST_MAIN_CLOCK             0x04
#define GTP_RQST_RESPONDED              0x00
#define GTP_RQST_IDLE                   0xFF

//******************** For GT9XXF End **********************//
// Registers define
#define GTP_READ_COOR_ADDR    0x814E
#define GTP_REG_SLEEP         0x8040
#define GTP_REG_SENSOR_ID     0x814A
#define GTP_REG_CONFIG_DATA   0x8047
#define GTP_REG_VERSION       0x8140

#define RESOLUTION_LOC        3
#define TRIGGER_LOC           8

#define CFG_GROUP_LEN(p_cfg_grp)  (sizeof(p_cfg_grp) / sizeof(p_cfg_grp[0]))
// Log define
#define GTP_INFO(fmt,arg...)           printk("<<-GTP-INFO->> "fmt"\n",##arg)
#define GTP_ERROR(fmt,arg...)          printk("<<-GTP-ERROR->> "fmt"\n",##arg)
#define GTP_DEBUG(fmt,arg...)          do{\
                                         if(GTP_DEBUG_ON)\
                                         printk("<<-GTP-DEBUG->> [%d]"fmt"\n",__LINE__, ##arg);\
                                       }while(0)
#define GTP_DEBUG_ARRAY(array, num)    do{\
                                         s32 i;\
                                         u8* a = array;\
                                         if(GTP_DEBUG_ARRAY_ON)\
                                         {\
                                            printk("<<-GTP-DEBUG-ARRAY->>\n");\
                                            for (i = 0; i < (num); i++)\
                                            {\
                                                printk("%02x   ", (a)[i]);\
                                                if ((i + 1 ) %10 == 0)\
                                                {\
                                                    printk("\n");\
                                                }\
                                            }\
                                            printk("\n");\
                                        }\
                                       }while(0)
#define GTP_DEBUG_FUNC()               do{\
                                         if(GTP_DEBUG_FUNC_ON)\
                                         printk("<<-GTP-FUNC->> Func:%s@Line:%d\n",__func__,__LINE__);\
                                       }while(0)
#define GTP_SWAP(x, y)                 do{\
                                         typeof(x) z = x;\
                                         x = y;\
                                         y = z;\
                                       }while (0)




//*****************************End of Part III********************************



#endif /* _GOODIX_GT9XX_H_ */
