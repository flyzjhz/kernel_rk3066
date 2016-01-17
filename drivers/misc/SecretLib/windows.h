/****************************************************************************
 *   FileName    : windows.h
 *   Description :
 ****************************************************************************
*
*   Copyright (c) Nowada, Inc.
*   ALL RIGHTS RESERVED
*
*	Created: 2014-08-19
*  	Author: Rocky Pan
****************************************************************************/

#ifndef __WINDOWS_H__
#define __WINDOWS_H__

//#include <common.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <mach/gpio.h>
#include <linux/earlysuspend.h> 
#include <linux/hrtimer.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <mach/iomux.h>
#include <linux/skbuff.h>
#include <linux/mm.h>
#include <linux/slab.h>
//#include "../../i2c/busses/i2c-rk30.h"

#define BOOT_AUTH	1

typedef unsigned char	UINT8;
typedef unsigned short	UINT16;
typedef unsigned int	UINT32;

typedef unsigned char	BYTE;
typedef unsigned int	DWORD;

#define BOOL	bool
#define TRUE	true
#define FALSE	false

#define usWait(us)			udelay(us)
#define A6				0
#define READ_REGISTER_ULONG(reg)	readl(reg)
#define WRITE_REGISTER_ULONG(reg, val)	writel(val, reg)
//#define WRITE_BITFIELD(x...)

//#define DEBUGMSG(x...)
#define KITLOutputDebugString(fmt, args...) printk(fmt, ##args)
#define L
#define TEXT(str) str
#define RETAILMSG(level, fmt, args...) KITLOutputDebugString fmt, ##args

#endif /*__WINDOWS_H__*/
