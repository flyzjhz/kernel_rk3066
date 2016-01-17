/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/byteorder/generic.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif 
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/rtpm_prio.h>

#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/dma-mapping.h>

#include <cust_eint.h>
#include <linux/jiffies.h>

#include "tpd.h"
#ifndef TPD_NO_GPIO 
#include "cust_gpio_usage.h"
#endif
#include "tpd_custom_tangoc32.h"

#define MAX_TRANSACTION_LENGTH	8
#define I2C_MASTER_CLOCK	400
#define FW_FRAME_LEN		143

#define SLAVE_ADDR		(0x5c<<1)
#define BOOTLOADER_ADDR		(0x5d<<1)

#ifndef I2C_MAJOR
#define I2C_MAJOR 		125
#endif
#define I2C_MINORS 		256

#define CALIBRATION_FLAG	1
#define BOOTLOADER		7
#define RESET_TP		9

#define ENABLE_IRQ		10
#define DISABLE_IRQ		11
#define BOOTLOADER_STU		16
#define POINTS_VALUE		20	

#define TPD_MAX_POINTS          5
#define TPD_MAX_KEYS          	8
#define TPD_POINT_INFO_LEN	5

#define TPD_POWER_MODE_REG	0x33
#define TPD_FW_CRC_REG		62
#define TPD_TOUCH_INFO_REG_BASE	0
///wj add
struct tpd_driver_t
{
        char *tpd_device_name;
        int (*tpd_local_init)(void);
         void (*suspend)(struct early_suspend *h);
         void (*resume)(struct early_suspend *h);
         int tpd_have_button;
};
int tpd_load_status; 
///wj add
static u8 *I2CDMABuf_va = NULL;
static dma_addr_t I2CDMABuf_pa = 0;
static u8 g_buffer[32];

extern struct tpd_device *tpd;

static int tpd_flag = 0;
static int tpd_halt=0;
static int i2c_clock = 400;
static struct task_struct *thread = NULL;
static DECLARE_WAIT_QUEUE_HEAD(waiter);

#ifdef TPD_HAVE_BUTTON 
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif

static void tpd_eint_interrupt_handler(void);
static int touch_event_handler(void *unused);
static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int tpd_i2c_remove(struct i2c_client *client);
extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
                                     kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
                                     kal_bool auto_umask);

static struct i2c_client *i2c_client = NULL;
static const struct i2c_device_id tpd_i2c_id[] = {{"tangoc32_tpd",0},{}};
static unsigned short force[] = {TPD_I2C_NUMBER, SLAVE_ADDR, I2C_CLIENT_END, I2C_CLIENT_END};
static const unsigned short * const forces[] = { force, NULL };
static struct i2c_client_address_data addr_data = { .forces = forces,};
static struct i2c_driver tpd_i2c_driver =
{                       
    .probe = tpd_i2c_probe,                                   
    .remove = tpd_i2c_remove,                           
    .detect = tpd_i2c_detect,                           
    .driver.name = "tangoc32_tpd", 
    .id_table = tpd_i2c_id,                             
    .address_data = &addr_data,                        
}; 

/* proc file system */
static unsigned char status_reg = 0;
volatile int irq_flag;

struct i2c_dev {
    struct list_head list;
    struct i2c_adapter *adap;
    struct device *dev;
};

static struct class *i2c_dev_class;
static LIST_HEAD(i2c_dev_list);
static DEFINE_SPINLOCK( i2c_dev_list_lock);

static void return_i2c_dev(struct i2c_dev *i2c_dev)
{
    spin_lock(&i2c_dev_list_lock);
    list_del(&i2c_dev->list);
    spin_unlock(&i2c_dev_list_lock);
    kfree(i2c_dev);
}

static struct i2c_dev *i2c_dev_get_by_minor(unsigned index)
{
    struct i2c_dev *i2c_dev;
    i2c_dev = NULL;

    spin_lock(&i2c_dev_list_lock);
    list_for_each_entry(i2c_dev, &i2c_dev_list, list)
    {
	if (i2c_dev->adap->nr == index)
	    goto found;
    }
    i2c_dev = NULL;
    found: spin_unlock(&i2c_dev_list_lock);
    return i2c_dev;
}

static struct i2c_dev *get_free_i2c_dev(struct i2c_adapter *adap)
{
    struct i2c_dev *i2c_dev;

    if (adap->nr >= I2C_MINORS) {
	printk(KERN_ERR "i2c-dev: Out of device minors (%d)\n",
			adap->nr);
	return ERR_PTR(-ENODEV);
    }

    i2c_dev = kzalloc(sizeof(*i2c_dev), GFP_KERNEL);
    if (!i2c_dev)
	return ERR_PTR(-ENOMEM);

    i2c_dev->adap = adap;

    spin_lock(&i2c_dev_list_lock);
    list_add_tail(&i2c_dev->list, &i2c_dev_list);
    spin_unlock(&i2c_dev_list_lock);

    return i2c_dev;
}
/*end of proc file system */

static void tangoc32_reset(void)
{
    gpio_set_value(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
   /// mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	rk30_mux_api_set(GPIO4C5_SMCDATA5_TRACEDATA5_NAME, GPIO4C_GPIO4C5); ///RST
    gpio_direction_output(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    gpio_set_value(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
    mdelay(10);  
    gpio_set_value(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
}

static void tangoc32_irq_enable(void)
{
    mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
}

static void tangoc32_irq_disable(void)
{
    mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
}

static int attb_read_val(void)
{
    int ret;
///    mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_GPIO);
	rk30_mux_api_set(GPIO4C2_SMCDATA2_TRACEDATA2_NAME, GPIO4C_GPIO4C2); ///INT
    ///ret = mt_get_gpio_in(GPIO_CTP_EINT_PIN);
    ret = gpio_get_value(GPIO_CTP_EINT_PIN);
 ///   mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
	rk30_mux_api_set(GPIO4C2_SMCDATA2_TRACEDATA2_NAME, GPIO4C_GPIO4C2); ///INT
    return ret;
}
/***
static int i2c_read_bytes( struct i2c_client *client, u8 *buf, int len )
{
    int i, ret = 0;

    if (len < 8) {
	client->addr &= I2C_MASK_FLAG;
	ret = i2c_master_recv(client, buf, len);
	if (ret != len) {

	    return ret;
	}
    } else {
	client->addr = (client->addr & I2C_MASK_FLAG) | I2C_DMA_FLAG;
	ret = i2c_master_recv(client, (char*)I2CDMABuf_pa, len);
	if (ret != len) {

	    return ret;
	}

	for (i = 0; i < len; i++) {
		buf[i] = I2CDMABuf_va[i];
	}
    }

    return 0;
}

static int i2c_write_bytes( struct i2c_client *client, u8 *buf, int len )
{
    int i, ret = 0;
    if (len < 8) {
	client->addr &= I2C_MASK_FLAG;
	ret = i2c_master_send(client, buf, len);
	if (ret != len) {

	    return ret;
	}
    } else {
	for (i = 0; i < len; i++) {
		I2CDMABuf_va[i] = buf[i];
	}
	client->addr = (client->addr & I2C_MASK_FLAG) | I2C_DMA_FLAG;
	ret = i2c_master_send(client, (const char*)I2CDMABuf_pa, len);
	if (ret != len) {

	    return ret;
	}
    }

    return 0;
}
*/
static int i2c_read_bytes(struct i2c_client *client, u8 *buf, s32 len)
{
    struct i2c_msg msgs[2];
    s32 ret=-1;
    s32 retries = 0;

    GTP_DEBUG_FUNC();

    msgs[0].flags = !I2C_M_RD;
    msgs[0].addr  = client->addr;
    msgs[0].len   = GTP_ADDR_LENGTH;
    msgs[0].buf   = &buf[0];            ///buf[0] buf[1] is reg addr
    msgs[0].scl_rate = 100 * 1000;    // for Rockchip
    msgs[0].udelay = 5;
    msgs[1].flags = I2C_M_RD;
    msgs[1].addr  = client->addr;
    msgs[1].len   = len - GTP_ADDR_LENGTH;
    msgs[1].buf   = &buf[GTP_ADDR_LENGTH];
    msgs[1].scl_rate = 100 * 1000;
    msgs[1].udelay = 5;

    while(retries < 5)
    {
        ret = i2c_transfer(client->adapter, msgs, 2);
        if(ret == 2)break;
        retries++;
    }
    if((retries >= 5))
    {
    #if GTP_SLIDE_WAKEUP
 // reset chip would quit doze mode
        if (DOZE_ENABLED == doze_status)
        {
            return ret;
        }
    #endif
        printk("I2C rd-communication timeout, resetting chip...");
       /// gtp_reset_guitar(client, 10);
    }
    return ret;
}
static int i2c_write_bytes(struct i2c_client *client,u8 *buf,s32 len)
{
    struct i2c_msg msg;
    s32 ret = -1;
    s32 retries = 0;

    GTP_DEBUG_FUNC();

    msg.flags = !I2C_M_RD;
    msg.addr  = client->addr;
    msg.len   = len;
    msg.buf   = buf;
    msg.scl_rate = 100 * 1000;    // for Rockchip
    msg.udelay = 5;
    while(retries < 5)
   {
        ret = i2c_transfer(client->adapter, &msg, 1);
        if (ret == 1)break;
        retries++;
    }
    if((retries >= 5))
    {
    #if GTP_SLIDE_WAKEUP
        if (DOZE_ENABLED == doze_status)
        {
            return ret;
        }
    #endif
        printk("I2C wr-communication timeout, resetting chip...");
       /// gtp_reset_guitar(client, 10);
    }
    return ret;
}

static int tpd_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info)
{
    strcpy(info->type, "tangoc32_tpd");
    return 0;
}

static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{             
    struct device *dev;
    struct i2c_dev *i2c_dev;
    int err = 0;
    char *crash = 0;
    unsigned char tpd_fw_crc[2] = { 0 }, wrbuf[1] = { TPD_FW_CRC_REG };

    if(TPD_POWER_SOURCE != MT65XX_POWER_NONE)
    {
	hwPowerDown(TPD_POWER_SOURCE,"TP");
	hwPowerOn(TPD_POWER_SOURCE,VOL_2800,"TP");
	msleep(50);	
    }	
	
    I2CDMABuf_va = dma_alloc_coherent(NULL, 4096, &I2CDMABuf_pa, GFP_KERNEL);
    if (!I2CDMABuf_va) {
	dev_err(&client->dev, "dma_alloc_coherent failed\n");
	goto exit_dma_coherent;
    }

    i2c_client = client;


if (TOUCH_RESET_PIN != INVALID_GPIO) {
		ret = gpio_request(TOUCH_RESET_PIN, "goodix reset pin");
		if (ret != 0) {
			gpio_free(TOUCH_RESET_PIN);
			ret = gpio_request(TOUCH_RESET_PIN, "goodix reset pin");
			if (ret != 0) {
				printk("goodix gpio_request error\n");
				return -EIO;
			}
		}
    //set tangoc32_tpd rst pin
    ///mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	rk30_mux_api_set(GPIO4C5_SMCDATA5_TRACEDATA5_NAME, GPIO4C_GPIO4C5); ///RST
    gpio_direction_output(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    gpio_set_value(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
    msleep(80);

    #if 1
    i2c_write_bytes(client, wrbuf, 1);
    err = i2c_read_bytes( client, tpd_fw_crc, 2);
    if (err) {
       printk( " Fail to read FW CRC info %d\n", err );
    } else {
        printk( " FW CRC :  %x %x\n", tpd_fw_crc[0], tpd_fw_crc[1] );
    }
    #endif
    // Create proc file system
    /*********************************Bee-0928-TOP****************************************/
    i2c_dev = get_free_i2c_dev(client->adapter);
    if (IS_ERR(i2c_dev)) {
	err = PTR_ERR(i2c_dev);
	return err;
    }

    dev = device_create(i2c_dev_class, &client->adapter->dev, MKDEV(I2C_MAJOR,
		client->adapter->nr), NULL, "pixcir_i2c_ts%d", 0);
    if (IS_ERR(dev)) {
	err = PTR_ERR(dev);
	return err;
    }
    /*********************************Bee-0928-BOTTOM****************************************/
    //End of  Create proc file system
    thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
    if (IS_ERR(thread)) {         
        err = PTR_ERR(thread);
        printk( " Failed to create kernel thread: %d\n", err);
        *crash = 0;
        return err;
    }
    // set INT mode
   /// mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
	rk30_mux_api_set(GPIO4C2_SMCDATA2_TRACEDATA2_NAME, GPIO4C_GPIO4C2); ///INT
    ///gpio_direction_output(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
	gpio_direction_input(GPIO_CTP_EINT_PIN);

   /// mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_DISABLE);
    //mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
    msleep(50);
    ///mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
   /// mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
    //mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler, 1);
    mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_POLARITY_HIGH, tpd_eint_interrupt_handler, 1);
    ////mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
    irq_flag = 1;

    tpd_load_status = 1;
    return 0;

exit_dma_coherent:
    dma_free_coherent(NULL, 4096, I2CDMABuf_va, I2CDMABuf_pa);
    I2CDMABuf_va = NULL;
    I2CDMABuf_pa = 0; 
}

static void tpd_eint_interrupt_handler(void)
{ 
    ///TPD_DEBUG_PRINT_INT;
    tpd_flag=1;
    wake_up_interruptible(&waiter);
}

static int tpd_i2c_remove(struct i2c_client *client)
{
    int error;
    struct i2c_dev *i2c_dev;
/****
    mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
    irq_flag = 0;

    dma_free_coherent(NULL, 4096, I2CDMABuf_va, I2CDMABuf_pa);
    I2CDMABuf_va = NULL;
    I2CDMABuf_pa = 0;

    i2c_dev = get_free_i2c_dev(client->adapter);
    if (IS_ERR(i2c_dev)) {
	error = PTR_ERR(i2c_dev);
	return error;
    }

    return_i2c_dev(i2c_dev);
    device_destroy(i2c_dev_class, MKDEV(I2C_MAJOR, client->adapter->nr));
*/
    return 0;
}

static void tpd_down(int id, int x, int y, int size)
{
    input_report_abs(tpd->dev, ABS_PRESSURE, 128);
    input_report_key(tpd->dev, BTN_TOUCH, 1);
    input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 128);
    input_report_abs(tpd->dev, ABS_MT_WIDTH_MAJOR, 128);
    input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
    input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
    input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, id);
    input_mt_sync(tpd->dev);
    //TPD_DMESG("[mtk-tpd] tpd_down : id=%d x=%d y=%d\n", id, x, y);
    TPD_EM_PRINT(x, y, x, y, size, 1);
}

static void tpd_up(int id, int x, int y)
{
    input_report_abs(tpd->dev, ABS_PRESSURE, 0);
    input_report_key(tpd->dev, BTN_TOUCH, 0);
    input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
    input_report_abs(tpd->dev, ABS_MT_WIDTH_MAJOR, 0);
    input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
    input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
    input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, id);
    input_mt_sync(tpd->dev);
    TPD_EM_PRINT(x, y, x, y, 0, 0);
    //TPD_DEBUG_PRINT_POINT( x, y, 0 );
   // TPD_DMESG("[mtk-tpd] tpd_up   : id=%d x=%d y=%d\n", id, x, y);
}

static int touch_event_handler(void *unused)
{
    struct sched_param param = { .sched_priority = RTPM_PRIO_TPD }; 
    int x, y, id, size, finger_num = 0;
    static u8 buffer[32], wrbuf[1] = { TPD_TOUCH_INFO_REG_BASE };
    static u8 id_mask = 0;
    u8 cur_mask;
    int idx;
    static int x_history[TPD_MAX_POINTS];
    static int y_history[TPD_MAX_POINTS];
    u8 *p;
    static u8 key_status,key_status_history;

    sched_setscheduler(current, SCHED_RR, &param); 

    do {
        set_current_state(TASK_INTERRUPTIBLE);

        while (tpd_halt) {
            tpd_flag = 0;
            msleep(20);
        }

        wait_event_interruptible(waiter, tpd_flag != 0);
        tpd_flag = 0;
        TPD_DEBUG_SET_TIME;
        set_current_state(TASK_RUNNING); 

	i2c_write_bytes(i2c_client, wrbuf, 1);
        i2c_read_bytes( i2c_client, buffer, 32);
                
        finger_num = buffer[0] & 0x07;
	//iTPD_DEBUG(" [mtk-tpd] finger_num = %d\n", finger_num);
	key_status = buffer[1];

#ifdef TPD_HAVE_BUTTON 
	if((key_status != 0) || (key_status_history != 0)) {
	    //key button
	    for(idx = 0; idx < TPD_MAX_KEYS; idx++) {
		if (key_status&(0x01<<idx)) {
			tpd_down(1,tpd_keys_dim_local[idx][0], tpd_keys_dim_local[idx][1],100);
		}
	    }

	    for (idx = 0; idx < TPD_MAX_KEYS; idx++) {
		if(((key_status&(0x01<<idx)) == 0x00)&& ((key_status_history&(0x01<<idx)) != 0x00))
			tpd_up(1,tpd_keys_dim_local[idx][0], tpd_keys_dim_local[idx][1]);
	    }	
	       
	    if (tpd != NULL && tpd->dev != NULL)
		input_sync(tpd->dev);

	    key_status_history = key_status;	
		continue;
	}
#endif
        
	cur_mask = 0;
	for (idx=0; idx<finger_num; idx++) {
	    u8 *ptr = &buffer[idx*TPD_POINT_INFO_LEN+2];
	    id = ptr[4]&0x07;

	    if (id < TPD_MAX_POINTS) {
		x = ptr[0] + (((int)ptr[1]) << 8);
		y = ptr[2] + (((int)ptr[3]) << 8);
		size = 28;

		tpd_down(id, x, y, size);

		cur_mask |= (1 << id);
		x_history[id] = x;
		y_history[id] = y;
	    } else
		TPD_DEBUG("Invalid id %d\n", id );
	}

	if (cur_mask != id_mask) {
	    u8 diff = cur_mask^id_mask;
	    idx = 0;

	    while (diff) {
		if (((diff & 0x01) == 1) &&
		((cur_mask >> idx) & 0x01) == 0) {
                    // check if key release
		    tpd_up(idx, x_history[idx], y_history[idx]);                    
		}

		diff = (diff >> 1);
		idx++;
            }
	    id_mask = cur_mask;
        }

	if (tpd != NULL && tpd->dev != NULL)
	    input_sync(tpd->dev);

	//for pixcir debug please keep it here.
	memcpy(g_buffer, buffer, sizeof(buffer));

    } while (!kthread_should_stop()); 

    return 0;
}

/*************************************Bee-0928****************************************/


/*************************************Bee-0928****************************************/
/*                        	     pixcir_ioctl                                    */
/*************************************Bee-0928****************************************/


/***********************************Bee-0928****************************************/
/*                        	  pixcir_read                                      */
/***********************************Bee-0928****************************************/




/***********************************Bee-0928****************************************/
/*                        	  pixcir_write                                     */
/***********************************Bee-0928****************************************/


/***********************************Bee-0928****************************************/
/*                        	  pixcir_release                                   */
/***********************************Bee-0928****************************************/


/*********************************Bee-0928-TOP****************************************/

/*********************************Bee-0928-BOTTOM****************************************/

static int tpd_local_init(void) 
{

    if (i2c_add_driver(&tpd_i2c_driver) != 0) {
        TPD_DMESG("unable to add i2c driver.\n");
        return -1;
    }

    if(tpd_load_status == 0) {
    	TPD_DMESG("add error touch panel driver.\n");
    	i2c_del_driver(&tpd_i2c_driver);
    	return -1;
    }

#ifdef TPD_HAVE_BUTTON     
    tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
#endif   
  
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))    
    TPD_DO_WARP = 1;
    memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT*4);
    memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT*4);
#endif

#if 0
#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
    memcpy(tpd_calmat, tpd_def_calmat_local, 8*4);
    memcpy(tpd_def_calmat, tpd_def_calmat_local, 8*4);	
#endif  
#endif

    TPD_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);  
    tpd_type_cap = 1;

    return 0;
}



static struct tpd_driver_t tpd_device_driver =
{
    .tpd_device_name = "TangoC32",
    .tpd_local_init = tpd_local_init,
 ///   .suspend = tpd_suspend,
 ///   .resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
    .tpd_have_button = 1,
#else
    .tpd_have_button = 0,
#endif		
};

/* called when loaded into kernel */
static int __init tpd_driver_init(void)

    int ret;
    printk("MediaTek tangoc32 touch panel driver init\n");

    /*********************************Bee-0928-TOP****************************************/

    /********************************Bee-0928-BOTTOM******************************************/

  /****  if ( tpd_driver_add(&tpd_device_driver) < 0)
	TPD_DMESG("add generic driver failed\n");*/
	tpd_local_init();

    return 0;
}

/* should never be called */
static void __exit tpd_driver_exit(void)
{
   /// TPD_DMESG("MediaTek tangoc32 touch panel driver exit\n");
    //input_unregister_device(tpd->dev);
   /// tpd_driver_remove(&tpd_device_driver);
}

module_init(tpd_driver_init);
module_exit(tpd_driver_exit);

