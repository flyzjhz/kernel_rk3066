#include <linux/module.h>
#include <linux/termios.h>
#include <linux/tty.h>
#include <linux/device.h>
#include <linux/kfifo.h>
#include <linux/tty_flip.h>
#include <linux/timer.h>
#include <linux/serial.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/rfkill.h>
#include <linux/fs.h>
#include <linux/dmapool.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <mach/gpio.h>
#include <linux/earlysuspend.h>
#include <linux/ctype.h>
#include <linux/serio.h>
#include "mcu_uart.h"

/* local variables */
static struct tty_driver *tty_drv;
static struct mcu_device *saved_mcu_dev;
static int mcu_debug = 1;
static unsigned char mcu_filter_buf[256];
static int mcu_filter_len;
static struct workqueue_struct *mcu_wq;
static struct timer_list seriotouch_key_timer;

typedef enum
{
    TS_KEY_DOWM = 1,
    TS_KEY_REPEAT,
    TS_KEY_RELEASE
}KEYSTATE;

static int keystate = TS_KEY_RELEASE;

static int msr ;
static int mcr ;

#define PEN_DOWN    1
#define PEN_RELEASE 0

#define DEBUG_MCU_FUNC		0x01
#define DEBUG_MCU_DATA		0x02
#define DEBUG_MCU_IRQ		0x04
#define DEBUG_MCU_MSG		0x08
#define DEBUG_MCU_RX_FILTER 	0x10
#define DEBUG_MCU_TX_FILTER 	0x20

#ifdef MCU_DEBUG
#define dbg_mcu_func(fmt, ...)		if((mcu_debug&DEBUG_MCU_FUNC) == DEBUG_MCU_FUNC)printk(fmt, ##__VA_ARGS__)
#define dbg_mcu_data(fmt, ...)		if((mcu_debug&DEBUG_MCU_DATA) == DEBUG_MCU_DATA)printk(fmt, ##__VA_ARGS__)
#define dbg_mcu_irq(fmt, ...)		if((mcu_debug&DEBUG_MCU_IRQ) == DEBUG_MCU_IRQ)printk(fmt, ##__VA_ARGS__)
#define dbg_mcu_msg(fmt, ...)		if((mcu_debug&DEBUG_MCU_MSG) == DEBUG_MCU_MSG)printk(fmt, ##__VA_ARGS__)
#define dbg_mcu_filter(fmt, ...)	if((mcu_debug&DEBUG_MCU_RX_FILTER) == DEBUG_MCU_RX_FILTER || (mcu_debug&DEBUG_MCU_TX_FILTER) == DEBUG_MCU_TX_FILTER)printk(fmt, ##__VA_ARGS__)
#else
#define dbg_mcu_func(fmt, ...)
#define dbg_mcu_data(fmt, ...)
#define dbg_mcu_irq(fmt, ...)
#define dbg_mcu_msg(fmt, ...)
#define dbg_mcu_filter(fmt, ...)	
#endif

/*mcu operations*/

static int mcu_data_sum(unsigned char *data, int len)
{
	int i;
	int data_sum = 0;

	for(i=0; i< len; i++)
	{
		data_sum += data[i];
	}

	data_sum &= 0xff;
	return data_sum;
}

static void write_data_to_mcu(struct mcu_device *port, unsigned char *data, int len)
{
	int i;
	if( port== NULL)
		return ;

	dbg_mcu_func("><%s><%d><\n", __func__, __LINE__);
	for(i=0; i<len; i++)
	{
		port->serio->write(port->serio,data[i]);
#ifdef MCU_DEBUG
		if((i%16) == 0) dbg_mcu_data("\n");
		dbg_mcu_data("0x%02x ", data[i]);
#endif
	}
	dbg_mcu_data("\n");
}

static void send_mcu_kernel_ready(struct mcu_device *port)
{
	unsigned char sbuf[7];
	int nsend;		//resend number of times
 
	sbuf[0] = 0xF0;		//head code
	sbuf[1] = 4;		//length
	sbuf[2] = 0x13;		//type
	sbuf[3] = 0x04;		//data type
	sbuf[4] = 0x02;		//send from; 2 arm to mcu,  1 mcu to arm
	sbuf[5] = 0x01;		//data1;  kernel ready
	sbuf[6] = sbuf[1] + sbuf[2] + sbuf[3] + sbuf[4] + sbuf[5];

	dbg_mcu_func("><%s><\n", __func__);
	for(nsend=0; nsend<3; nsend++)
	{
		spin_lock_irqsave(&port->write_lock, port->flags);
		write_data_to_mcu(port, sbuf, 7);
		spin_unlock_irqrestore(&port->write_lock, port->flags);
	}
}
//要注意，现在can是接到arm上面的，现在的rx_frame_len是固定长度的,现在这个port->rx_buf[2]是数据而不是长度
static void mcu_work_rx_start(struct mcu_device *port)
{
	dbg_mcu_irq("><%s><%d><\n", __func__, __LINE__);
	if((port->rx_frame_len == 0) && (kfifo_len(&port->rx_fifo)>3) )
	{
		kfifo_out(&port->rx_fifo, port->rx_buf, 3);
		if(port->rx_buf[0] == 0x2e)
		{
			port->rx_frame_len = port->rx_buf[2];
			if( port->rx_frame_len+3 > MCU_MAX_BUF_SIZE)
			{
				dbg_mcu_msg("><%s><error>< length > max_buf, len:%d><\n", __func__, port->rx_frame_len+3);
				port->rx_frame_len = 0;
			}
		}
		else
		{
			dbg_mcu_msg("><%s><head data error, data: 0x%02x, 0x%02x><\n", __func__, port->rx_buf[0], port->rx_buf[1] );
			port->rx_buf[0] = port->rx_buf[1];
			if(port->rx_buf[0] == 0x2e)
			{
				kfifo_get(&port->rx_fifo, &port->rx_buf[2]);
				port->rx_frame_len = port->rx_buf[2];
				if( port->rx_frame_len+3 > MCU_MAX_BUF_SIZE)
				{
					dbg_mcu_msg("><%s><error>< length > max_buf, len:%d><\n", __func__, port->rx_frame_len+3);
					port->rx_frame_len = 0;
				}
			}
		}
	}

	if((kfifo_len(&port->rx_fifo) > port->rx_frame_len) && (port->rx_frame_len != 0) )
	{
		queue_work(mcu_wq, &port->rx_work);
	}

}

static void serio_key_timer(unsigned long data)
{
	struct mcu_device *port = (struct mcu_device *)data;
	if(keystate != TS_KEY_RELEASE)
	{
		input_report_key(port->input_dev, BTN_TOUCH, PEN_RELEASE);
		input_sync(port->input_dev);
		//printk("tup\n");
	}
}

static void mcu_work_rx_func(struct work_struct *work)
{
	struct mcu_device *port = container_of(work, struct mcu_device, rx_work);
	int checksum;
	int i;

	dbg_mcu_func("><%s><%d><fifo data len: %d><rx_frame_len:%d><\n", __func__, __LINE__, kfifo_len(&port->rx_fifo), port->rx_frame_len + 3);

	if( kfifo_len(&port->rx_fifo) >= port->rx_frame_len+1)		//data[0-n] + sum
	{
		kfifo_out(&port->rx_fifo, &port->rx_buf[2], port->rx_frame_len+1);
#ifdef MCU_DEBUG
		if((mcu_debug&DEBUG_MCU_DATA) == DEBUG_MCU_DATA)
		{
			printk("><uart frame data from mcu><\n");
			for(i=0; i<port->rx_frame_len+3; i++)
			{
				printk("0x%02x ", port->rx_buf[i]);
				if((i%16) == 15) printk("\n");
			}
			printk("\n");
		}
#endif
		if((mcu_debug&DEBUG_MCU_RX_FILTER) == DEBUG_MCU_RX_FILTER)
		{
			if( port->rx_frame_len+3 == mcu_filter_len)
			{
				if( !memcmp(port->rx_buf, mcu_filter_buf, mcu_filter_len) )
				{
					dbg_mcu_filter("><%s><find filter data><\n", __func__);
				}
			}
		}
		if(!((port->rx_buf[0] == 0x2e) && (port->rx_buf[1] == 0x05)))	//to virtual uart
		{
			//printk("0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",port->rx_buf[0],port->rx_buf[1],port->rx_buf[2],port->rx_buf[3],port->rx_buf[4],port->rx_buf[5],port->rx_buf[6]);
			if(port->tty)
			{
				dbg_mcu_msg("<uart data to virtual uart><count:0x%02x><\n", port->rx_frame_len + 3);
				tty_insert_flip_string(port->tty, port->rx_buf, port->rx_frame_len + 3);	//frame data len = head + length + data[rx_frame_len] + sum
				tty_flip_buffer_push (port->tty);
			}
			else
			{
				dbg_mcu_msg("><port->tty is NULL><\n");
			}
		}
		else	//touch	
		{
			switch(port->rx_buf[1])
			{
				case 0x05:	
					//printk("x:%d, y:%d\n",  800 - (port->rx_buf[2] << 8 | (port->rx_buf[3])), 480 - (port->rx_buf[4] << 8 | (port->rx_buf[5])));
					if(port->rx_buf[2] || port->rx_buf[3] || port->rx_buf[4] || port->rx_buf[5])					
					{
				 		keystate = TS_KEY_DOWM;
						input_report_abs(port->input_dev, ABS_X, 800 - (port->rx_buf[2] << 8 | (port->rx_buf[3])));
						input_report_abs(port->input_dev, ABS_Y, 480 - (port->rx_buf[4] << 8 | (port->rx_buf[5])));
						input_report_key(port->input_dev, BTN_TOUCH, PEN_DOWN);
        					input_report_abs(port->input_dev, ABS_PRESSURE, 255);
						input_sync(port->input_dev);
						//printk("down\n");
					}
					else
					{
				 		keystate = TS_KEY_RELEASE;
        					input_report_key(port->input_dev, BTN_TOUCH, PEN_RELEASE);
        					input_report_abs(port->input_dev, ABS_PRESSURE, 0);
        					input_sync(port->input_dev);
						//printk("up\n");
					}
       					//mod_timer(&seriotouch_key_timer, jiffies + msecs_to_jiffies(200)); 
					break;
				case 0x3:
				default:
					dbg_mcu_msg("><uart2iic><error type><type: 0x%02x><\n", port->rx_buf[3]);
					break;
			}
		}

		port->rx_frame_len = 0;
	}
	else
	{
		return ;
	}

	dbg_mcu_func("><%s><%d><fifo data len: %d><\n", __func__, __LINE__, kfifo_len(&port->rx_fifo));
	mcu_work_rx_start(port);
}

static void mcu_work_tx_start(struct mcu_device *port)
{
	dbg_mcu_irq("><%s><%d><\n", __func__, __LINE__);
	if((port->tx_frame_len == 0) && (kfifo_len(&port->tx_fifo)>2) )
	{
		kfifo_out(&port->tx_fifo, port->tx_buf, 2);
		if(port->tx_buf[0] == 0xf0)
		{
			port->tx_frame_len = port->tx_buf[1];
			if(port->tx_frame_len+3 > MCU_MAX_BUF_SIZE)
			{
				dbg_mcu_msg("><%s><error>< length > max_buf, len:%d><\n", __func__, port->tx_frame_len+3);
				port->tx_frame_len = 0;
			}
		}
		else
		{
			dbg_mcu_msg("><%s><head data error, data: 0x%02x, 0x%02x><\n", __func__, port->tx_buf[0], port->tx_buf[1] );
			port->tx_buf[0] = port->tx_buf[1];
			if(port->tx_buf[0] == 0xf0)
			{
				kfifo_get(&port->tx_fifo, &port->tx_buf[1]);
				port->tx_frame_len = port->tx_buf[1];
				if(port->tx_frame_len+3 > MCU_MAX_BUF_SIZE)
				{
					dbg_mcu_msg("><%s><error>< length > max_buf, len:%d><\n", __func__, port->tx_frame_len+3);
					port->tx_frame_len = 0;
				}
			}
		}
	}

	if((kfifo_len(&port->tx_fifo) > port->tx_frame_len) && (port->tx_frame_len != 0) )
	{
		queue_delayed_work(mcu_wq, &port->tx_work, msecs_to_jiffies(5));
	}

}

static void mcu_work_tx_func(struct work_struct *work)
{
	struct mcu_device *port = container_of(work, struct mcu_device, tx_work.work);
	int i;

	dbg_mcu_func("><%s><%d><fifo data len: %d><rx_frame_len:%d><\n", __func__, __LINE__, kfifo_len(&port->tx_fifo), port->tx_frame_len + 3);
	if( kfifo_len(&port->tx_fifo) >= port->tx_frame_len+1)		//data[0-n] + sum
	{
		kfifo_out(&port->tx_fifo, &port->tx_buf[2], port->tx_frame_len+1);
#ifdef MCU_DEBUG
		if((mcu_debug&DEBUG_MCU_DATA) == DEBUG_MCU_DATA)
		{
			printk("><uart frame data to mcu, len: %d><\n", port->tx_frame_len+3);
			for(i=0; i<port->tx_frame_len+3; i++)
			{
				printk("0x%02x ", port->tx_buf[i]);
				if((i%16) == 15) printk("\n");
			}
			printk("\n");
		}
#endif
		if((mcu_debug&DEBUG_MCU_TX_FILTER) == DEBUG_MCU_TX_FILTER)
		{
			if( port->tx_frame_len+3 == mcu_filter_len)
			{
				if( !memcmp(port->tx_buf, mcu_filter_buf, mcu_filter_len) )
				{
					dbg_mcu_filter("><%s><find filter data><\n", __func__);
				}
			}
		}
		spin_lock_irqsave(&port->write_lock, port->flags);
		write_data_to_mcu( port, port->tx_buf, port->tx_frame_len+3);
		spin_unlock_irqrestore(&port->write_lock, port->flags);
		port->tx_frame_len = 0;
	}
	else
	{
		return ;
	}

	dbg_mcu_func("><%s><%d><fifo data len: %d><\n", __func__, __LINE__, kfifo_len(&port->tx_fifo));
	mcu_work_tx_start(port);
}
/* char/tty operations */

static int mcu_tiocmget(struct tty_struct *tty, struct file *filp)
{
	unsigned int value = 0;

	dbg_mcu_func("><%s><%d><\n", __func__, __LINE__);
	return value;
}

static int mcu_tiocmset(struct tty_struct *tty, struct file *filp,
			    unsigned int set, unsigned int clear)
{
        int cur_mcr = mcr;
        int cur_msr = msr;

	dbg_mcu_func("><%s><%d><\n", __func__, __LINE__);
        if (set&TIOCM_DTR)
        {
                cur_mcr |= TIOCM_DTR ;
        }
        if(clear&TIOCM_DTR)
        {
                cur_mcr &= ~TIOCM_DTR ;
        }
        if (set&TIOCM_RTS)
        {
                cur_mcr |= TIOCM_RTS ;
        }
        if(clear&TIOCM_RTS)
        {
                cur_mcr &= ~TIOCM_RTS ;
        }

        mcr = cur_mcr ;
        msr = cur_msr ;


	return 0;
}

static int mcu_open(struct tty_struct *tty, struct file *filp)
{
	tty->driver_data = saved_mcu_dev;
	saved_mcu_dev->tty = tty;

	saved_mcu_dev->count++ ;

	dbg_mcu_func("><%s><%d><\n", __func__, __LINE__);
	return 0;
}

static void mcu_close(struct tty_struct *tty, struct file *filp)
{
	struct mcu_device *mcu_dev = tty->driver_data;
	dbg_mcu_func("><%s><%d><\n", __func__, __LINE__);
	mcu_dev->count-- ;
	if(mcu_dev->count == 0)
		mcu_dev->tty = NULL;
}

static int mcu_write(struct tty_struct *tty, const unsigned char *buf,
			 int count)
{
	struct mcu_device *port;
	int i;

	dbg_mcu_func("><%s><%d><count:%d><\n", __func__, __LINE__, count);

        if (tty) {
                port = tty->driver_data;
                if(port)
                {
			if( MCU_MAX_FIFO_SIZE - kfifo_len(&port->tx_fifo) >= count )
			{
				kfifo_in(&port->tx_fifo, buf, count);
				mcu_work_tx_start(port);
			}
			else
			{
				dbg_mcu_msg("><%s><error : tx_fifo flow><\n", __func__);
			}
                }
		else
		{
			dbg_mcu_msg("><%s><line: %d><mcu_device is NULL><\n", __func__, __LINE__);
		}
    	}
	else
	{
		dbg_mcu_msg("><%s><line :%d><tty is NULL><\n", __func__, __LINE__);
	}

	return count;
}

static int mcu_write_room(struct tty_struct *tty)
{
	struct mcu_device *port = tty->driver_data;
	dbg_mcu_func("><%s><%d><\n", __func__, __LINE__);
	return MCU_MAX_FIFO_SIZE - kfifo_len(&port->tx_fifo);
}

static int mcu_chars_in_buffer(struct tty_struct *tty)
{
	struct mcu_device *port = tty->driver_data;
	dbg_mcu_func("><%s><%d><\n", __func__, __LINE__);
	return kfifo_len(&port->tx_fifo);
}

static void mcu_hangup(struct tty_struct *tty)
{
//	struct mcu_device *mcu_dev = tty->driver_data;
	dbg_mcu_func("><%s><%d><\n", __func__, __LINE__);
//	tty_port_hangup(&mcu_dev->tty_port);
//	tty_hangup(tty);
}


#ifdef MCU_DEBUG
/*
 *	sysfs
 */
static ssize_t ready_store(struct device *dev, struct device_attribute *attr, const char * buf, size_t count)
{
	send_mcu_kernel_ready(saved_mcu_dev);
        return strnlen(buf, PAGE_SIZE);
}

static DEVICE_ATTR(ready, S_IRUGO|S_IWUSR, NULL, ready_store);

static ssize_t debug_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	sprintf(buf, "bit0, printf debug func\n");
	sprintf(buf, "%sbit1, printf debug data\n", buf);
	sprintf(buf, "%sbit2, printf debug data for irq\n", buf);
	sprintf(buf, "%sbit3, printf debug msg\n", buf);
	sprintf(buf, "%sbit4, rx filter\n", buf);
	sprintf(buf, "%sbit5, tx filter\n", buf);
        return sprintf(buf,"%scurnt debug: 0x%02x\n", buf, mcu_debug);
}

static ssize_t debug_store(struct device *dev, struct device_attribute *attr, const char * buf, size_t count)
{
	sscanf(buf, "%x", &mcu_debug);
        return strnlen(buf, PAGE_SIZE);
}

static DEVICE_ATTR(debug, S_IRUGO|S_IWUSR, debug_show, debug_store);

static ssize_t filter_show(struct device *dev,struct device_attribute *attr, char *buf)
{
	int i;
	sprintf(buf, "filter data\n");
	for(i=0; i< mcu_filter_len; i++)
	{
		sprintf(buf, "%s0x%02x ", buf,mcu_filter_buf[i]);
	}
        return sprintf(buf,"%s\nlen:%d\n", buf, mcu_filter_len);
}

#define TOLOWER(x) ((x) | 0x20)
static ssize_t filter_store(struct device *dev, struct device_attribute *attr, const char * buf, size_t count)
{
	char *cp = buf;
	int i = 0;

	memset(mcu_filter_buf, 0, 256);
	mcu_filter_len = 0;

	while (isxdigit(*cp)) {
                unsigned char value;

                value = isdigit(*cp) ? *cp - '0' : TOLOWER(*cp) - 'a' + 10;
                if (value >= 16)
                        break;
                mcu_filter_buf[i] = (value & 0xf) << 4;
                cp++;
		if(isxdigit(*cp))
		{
			value = isdigit(*cp) ? *cp - '0' : TOLOWER(*cp) - 'a' + 10;
			if (value >= 16)
				break;
			mcu_filter_buf[i] |= (value & 0xf);
			cp++;
		}
		i++;
        }

	mcu_filter_len = i;

        return strnlen(buf, PAGE_SIZE);
}

static DEVICE_ATTR(filter, S_IRUGO|S_IWUSR, filter_show, filter_store);

static struct attribute *mcu_serio_attrs[] = {
        &dev_attr_ready.attr,
        &dev_attr_debug.attr,
	&dev_attr_filter.attr,
        NULL
};

static struct attribute_group mcu_serio_group = {
        .name = NULL,
        .attrs = mcu_serio_attrs,
};
#endif

static const struct tty_operations mcu_serial_ops = {
	.open = mcu_open,
	.close = mcu_close,
	.write = mcu_write,
	.hangup = mcu_hangup,
	.write_room = mcu_write_room,
	.chars_in_buffer = mcu_chars_in_buffer,
	.tiocmget = mcu_tiocmget,
	.tiocmset = mcu_tiocmset,
};

static irqreturn_t mcu_interrupt(struct serio *serio,
                unsigned char data, unsigned int flags)
{
    struct mcu_device *port = serio_get_drvdata(serio);

	dbg_mcu_irq("><%s><%d><data:0x%02x><\n", __func__, __LINE__, data);
	if( port == NULL)
        	return IRQ_HANDLED;


	if( MCU_MAX_FIFO_SIZE - kfifo_len(&port->rx_fifo) > 0 )
	{
		kfifo_put(&port->rx_fifo, &data);
	}
	else
	{
		dbg_mcu_msg("><%s><error : rx_fifo flow><\n", __func__);
	}

	mcu_work_rx_start(port);

        return IRQ_HANDLED;
}


static void mcu_disconnect(struct serio *serio)
{
        struct mcu_device *port = serio_get_drvdata(serio);

	dbg_mcu_func("><%s><%d><\n", __func__, __LINE__);

    	tty_unregister_device(tty_drv, 0);
	tty_unregister_driver(tty_drv);
	serio_close(serio);
	serio_set_drvdata(serio, NULL);
	kfifo_free(&port->rx_fifo);
	kfifo_free(&port->tx_fifo);
	kfree(port);
	saved_mcu_dev = NULL;
}

static struct input_dev  *input_dev;

static int mcu_connect(struct serio *serio, struct serio_driver *drv)
{
	struct mcu_device *port;
	int err;

	dbg_mcu_func("><%s><%d><\n", __func__, __LINE__);
	port = kzalloc(sizeof(struct mcu_device), GFP_KERNEL);
	if(!port)
	{
		printk("kzalloc is fail\n");
		return -ENOMEM;
	}
	err = kfifo_alloc(&port->rx_fifo, MCU_MAX_FIFO_SIZE, GFP_KERNEL);
	if (err) {
    	printk(KERN_ERR "error kfifo_alloc\n");
		goto alloc_rx_fifo_err;
	}

	err = kfifo_alloc(&port->tx_fifo, MCU_MAX_FIFO_SIZE, GFP_KERNEL);
	if (err) {
		printk(KERN_ERR "error kfifo_alloc\n");
		goto alloc_tx_fifo_err;
		return err;
	}
	port->rx_frame_len = 0;
	port->tx_frame_len = 0;

	port->input_dev = input_dev;
	port->serio = serio;
	saved_mcu_dev = port;
	port->count = 0;

	port->serio = serio;
	serio_set_drvdata(serio, port);

	err = serio_open(serio, drv);
	if (err)
	{
		goto serio_open_err;
	}
	tty_drv = alloc_tty_driver(1);
	if (!tty_drv) {
		pr_err("%s: alloc_tty_driver failed", "ttymcu");
		err = -ENOMEM;
		goto alloc_tty_driver_err;
	}


	tty_drv->name = "ttymcu";
	tty_drv->major = TTY_MAJOR;
	tty_drv->magic = TTY_DRIVER_MAGIC;
	tty_drv->owner = THIS_MODULE;
	tty_drv->driver_name = "mcutty";
	tty_drv->minor_start = 0;
	tty_drv->num = 1;
	tty_drv->type = TTY_DRIVER_TYPE_SERIAL;
	tty_drv->subtype = SERIAL_TYPE_NORMAL;
	tty_drv->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
	tty_drv->init_termios = tty_std_termios;
	tty_drv->init_termios.c_cflag = B38400 | CS8 | CREAD | HUPCL | CLOCAL;
	tty_drv->init_termios.c_ispeed = 38400;
	tty_drv->init_termios.c_ospeed = 38400;

	tty_set_operations(tty_drv, &mcu_serial_ops);

	err = tty_register_driver(tty_drv);
	if (err) {
		pr_err("%s: tty_register_driver failed(%d)",
			"ttymcu", err);
		goto tty_register_driver_err;
	}

        port->tty_dev = tty_register_device(tty_drv, 0, NULL);
        if (IS_ERR(port->tty_dev)) {
                dev_dbg(&port->serio->dev,
                        "%s: registering tty device failed", __func__);
                err = PTR_ERR(port->tty_dev);
		goto tty_register_device_err;
        }
#ifdef MCU_DEBUG
        err = sysfs_create_group(&port->serio->dev.kobj, &mcu_serio_group);
        if (err) {
            printk(KERN_ERR "Can't create sysfs attrs for vty-server@%X\n", err);
			goto sysfs_err;
        }
#endif
	init_timer(&seriotouch_key_timer);
	seriotouch_key_timer.expires = jiffies + 10;
	seriotouch_key_timer.function = serio_key_timer;
	seriotouch_key_timer.data = (unsigned long)port;


	INIT_WORK(&port->rx_work, mcu_work_rx_func);
	INIT_DELAYED_WORK(&port->tx_work, mcu_work_tx_func);

	flush_workqueue(mcu_wq);

	dbg_mcu_func("><%s><%d><\n", __func__, __LINE__);

    return 0;

#ifdef MCU_DEBUG
sysfs_err:
		
#endif

tty_register_device_err:
	tty_unregister_driver(tty_drv);
tty_register_driver_err:
	put_tty_driver(tty_drv);
alloc_tty_driver_err:
	serio_close(serio);
serio_open_err:
	serio_set_drvdata(serio, NULL);
	kfifo_free(&port->tx_fifo);
alloc_tx_fifo_err:
	kfifo_free(&port->rx_fifo);
alloc_rx_fifo_err:
	kfree(port);	
        return err;
}

/*
 * The serio driver structure.
 */

static struct serio_device_id mcu_serio_ids[] = {
        {
                .type   = SERIO_RS232,
                .proto  = SERIO_NWDMCU,
                .id     = SERIO_ANY,
                .extra  = SERIO_ANY,
        },
        { 0 }
};

MODULE_DEVICE_TABLE(serio, mcu_serio_ids);
static struct serio_driver mcu_drv = {
        .driver         = {
                .name   = "mcu",
		.owner = THIS_MODULE,
        },
        .description    = "mcu uart to i2c",
        .id_table       = mcu_serio_ids,
        .interrupt      = mcu_interrupt,
        .connect        = mcu_connect,
        .disconnect     = mcu_disconnect,
};

/*
 * The functions for inserting/removing us as a module.
 */
static int __init mcu_init(void)
{
	int ret;
	char phys[32];

	input_dev = input_allocate_device();
	if (!input_dev) {
	    return -ENOMEM;
	}

	input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) ;
	input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	__set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
	input_set_abs_params(input_dev, ABS_X, 0, 800, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0, 480, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, 255, 0, 0);
	input_set_abs_params(input_dev, ABS_TOOL_WIDTH, 0, 255, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);

#if 0
	sprintf(phys, "input/ts");
	input_dev->name = "Nowada Serial TouchScreen";
	input_dev->phys = phys;
	input_dev->id.bustype = BUS_RS232;
	input_dev->id.vendor = SERIO_NWDMCU;
	input_dev->id.product = 0;
	input_dev->id.version = 0x0100;
	input_dev->dev.parent = NULL;

	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) | BIT_MASK(EV_SYN);
	input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	__set_bit(INPUT_PROP_DIRECT, input_dev->propbit);

	input_set_abs_params(input_dev, ABS_X, 0, 800, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0, 480, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, 1, 0, 0);
#endif

	ret= input_register_device(input_dev);
	if (ret)
	    goto input_register_device_err;

	mcu_wq = create_singlethread_workqueue("mcu_wq");
	if (!mcu_wq)
    	{
		printk("Creat workqueue failed.");
        	ret = -ENOMEM;
		goto mcu_wq_err;
    	}

    	ret = serio_register_driver(&mcu_drv);
	if(ret < 0)
	{
        	printk("serio register faild.ret = %d\n",ret);
		goto serio_register_driver_err;
	}

	printk("mcu uart init ... ...\n");
	return ret;
serio_register_driver_err:
	destroy_workqueue(mcu_wq);
mcu_wq_err:
	input_unregister_device(input_dev);
	input_dev = NULL;
input_register_device_err:
	input_free_device(input_dev);	
	return ret;
}

static void __exit mcu_exit(void)
{
	//uart2iic_del_i2c();
        serio_unregister_driver(&mcu_drv);
        if (mcu_wq)
        {
                destroy_workqueue(mcu_wq);
        }

}

module_init(mcu_init);
module_exit(mcu_exit);

MODULE_AUTHOR("Nowada");
MODULE_DESCRIPTION("mcu uart2iic driver");
MODULE_LICENSE("GPL");
MODULE_INFO(Version, "0.1");

