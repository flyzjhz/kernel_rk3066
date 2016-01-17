#ifndef __VOUT_CH7026__
#define __VOUT_CH7026__


#include <mach/gpio.h> 
#include "n867a_user_config.h"

//#define GPIO_BASE					SIRFSOC_GPIO_VA_BASE

#define CH7026_MAJOR				200
#define CH7026_ADJ_MAJOR			201

// ADJUST
#define SATURATION					0x2F
#define CONTRAST					0x30
#define BRIGHTNESS					0x31
#define ROTATION					0x2D
#define FLIP						0x2D
#define TEXT_EH						0x32
#define HUE						0x2E
#define VP_H						0x33
#define VP_L						0x34
#define HP_H						0x35
#define HP_L						0x36

//-- Output -------------
#define HAO_H						0x1B
#define HAO_L						0x1C
#define HTO_H						0x1B
#define HTO_L						0x1D
#define HOO_H						0x1E
#define HOO_L						0x1F
#define HWO_H						0x1E
#define HWO_L						0x20

#define VAO_H						0x21
#define VAO_L						0x22
#define VTO_H						0x21
#define VTO_L						0x23
#define VOO_H						0x24
#define VOO_L						0x25
#define VWO_H						0x24
#define VWO_L						0x26

//-- Input -----------
#define HAI_H						0x0F
#define HAI_L						0x10
#define HTI_H						0x0F
#define HTI_L						0x11
#define HO_H						0x12
#define HO_L						0x13
#define HW_H						0x12
#define HW_L						0x14

#define VAI_H						0x15
#define VAI_L						0x16
#define VTI_H						0x15
#define VTI_L						0x17
#define VO_H						0x18
#define VO_L						0x19
#define VW_H						0x18
#define VW_L						0x1A


int iic_reg_read(unsigned char reg_addr);
void iic_reg_write(unsigned char reg_addr, unsigned char data);
int platform_iic_reg_read(struct i2c_client *client, unsigned char reg_addr);
void platform_iic_reg_write(struct i2c_client *client, unsigned char reg_addr, unsigned char val);
void vout_ch7026_platform_reset(void);
int  vout_ch7026_platform_init(void);
void vout_ch7026_platform_exit(void);

#endif
