/* linux/drivers/char/usb.c

   National Semiconductor SCx200 GPIO driver.  Allows a user space
   process to play with the GPIO pins.

   Copyright (c) 2001,2002 Christer Weinigel <wingel@nano-system.com> */

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/bcd.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <linux/completion.h>
#include <linux/mfd/tps65910.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/types.h>
#include <linux/cdev.h>

#define DRVNAME "SysStatus"
#define IOCTL_ANDROID_SETSYSTEMUSB11 0x80000001 
#define IOCTL_ANDROID_SETSYSTEMUSB20 0x80000002 
#define IOCTL_ANDROID_QUERYSYSTEMUSB 0x80000003

#define IOCTL_ANDROID_RUNTOUCHSCREEN 0x80000010
#define IOCTL_ANDROID_SETTOUCHSCREEN 0x80000011
#define IOCTL_ANDROID_QUERYTOUCHSCREEN 0x80000012
#define IOCTL_ANDROID_SETBOOTLOGO 0x80000021

#define IOCTL_ANDROID_SYSTEMSLEEPTIME 0x81000000
#define IOCTL_ANDROID_GETSTARTUPSDCARDSTATUS 0x81000001
#define IOCTL_ANDROID_GETSTARTUPUDISKSTATUS  0x81000002

static struct platform_device *pdev;

MODULE_AUTHOR("Christer Weinigel <wingel@nano-system.com>");
MODULE_DESCRIPTION("NatSemi/AMD SCx200 GPIO Pin Driver");
MODULE_LICENSE("GPL");
static DEFINE_MUTEX(nvram_mutex);
static int major = 0;		/* default to dynamic major */
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major device number");
static struct kobject *tpa6140_kobj;
#define MAX_PINS 32		/* 64 later, when known ok */
 static int usbsys;

unsigned char SdcardInsertFlag=0;
unsigned char StartupSdcardInsertFlag=0;

unsigned char UdiskInsertFlag=0;
unsigned char StartupUdiskInsertFlag=0;

unsigned char StartupTimes=0;

static struct timer_list SystemStartupTimer;

struct stTouchscreen
{
	unsigned char len;
	unsigned char buf[4];
};

unsigned char mSystemStartFlag=0;
static int usb_open(struct inode *inode, struct file *file)
{
//	printk("\n=====%d== %s====\n",__LINE__,__FUNCTION__ );
	return 0; 
}

static int usb_release(struct inode *inode, struct file *file)
{
//	printk("====%d== %s====\n",__LINE__,__FUNCTION__ );
	return 0;
}
void SystemStartupTimerCallback(unsigned long data )
{
	if(StartupTimes>10)
	{
	    StartupSdcardInsertFlag = 0;
            StartupUdiskInsertFlag = 0;
	    printk("SystemStartupTimerCallback time end");
	}	
	else
	{
	    StartupSdcardInsertFlag = SdcardInsertFlag;
	    StartupUdiskInsertFlag = UdiskInsertFlag;
	    printk("SystemStartupTimerCallback SdcardInsertFlag=%x,UdiskInsertFlag=%x,time%d\n",SdcardInsertFlag,UdiskInsertFlag,StartupTimes);
	    StartupTimes ++;
	    mod_timer(&SystemStartupTimer, jiffies + msecs_to_jiffies(1500));
	}
}
unsigned char SystemEmmcUsbStatus(void)
{
        struct file *fd;
        size_t t;
        unsigned char buff[513];

        loff_t off;

        mm_segment_t old_fs;

        old_fs = get_fs();
        set_fs(KERNEL_DS);
        fd = filp_open("/dev/block/mmcblk0p4", O_RDONLY, 0);       
        off = 0x3ff800;
        t = vfs_read(fd,buff,2, &off);
	vfs_fsync(fd, 0);
	printk("vfs_read %x %x\n",buff[0],buff[1]);
        filp_close(fd, NULL);
	if((buff[0]==0xaa)&&(buff[1]==0x11))
		return 1;
	return 0;
}

extern unsigned char SystemSetUsb11Falg;

unsigned char ReadSystemEmmcUsbStatus(void)
{
	if(mSystemStartFlag==0)
	{
        	if(SystemSetUsb11Falg==0xaa)
        	{
                	printk("System Usb11...\n");
                	return 1;
        	}
	}
	else
	{
		if(SystemEmmcUsbStatus()==1)
		{
			printk("start System Usb11...\n");
                        return 1;	
		}
		
	}
        printk("System Usb20...\n");
        return 0;
}
EXPORT_SYMBOL(ReadSystemEmmcUsbStatus);
void SetSystemUsb11(void)
{
        struct file *fd;
        size_t t;
        unsigned char buff[513];

        loff_t off;

        mm_segment_t old_fs;

        old_fs = get_fs();
        set_fs(KERNEL_DS);
        fd = filp_open("/dev/block/mmcblk0p4", O_RDWR, 0);
        off = 0x3ff800;

        buff[0]=0xaa;
        buff[1]=0x11;
        t = vfs_write(fd,buff,2, &off);
        vfs_fsync(fd, 0);
        printk("vfs_write %x %x",buff[0],buff[1]);

        filp_close(fd, NULL);
}

void SetSystemUsb20(void)
{
        struct file *fd;
        size_t t;
        unsigned char buff[513];

        loff_t off;

        mm_segment_t old_fs;

        old_fs = get_fs();
        set_fs(KERNEL_DS);
        fd = filp_open("/dev/block/mmcblk0p4", O_RDWR, 0);
        off = 0x3ff800;
        
	buff[0]=0;
        buff[1]=0;
        t = vfs_write(fd,buff,2, &off);
        vfs_fsync(fd, 0);
	filp_close(fd, NULL);
        printk("vfs_write %x %x",buff[0],buff[1]);
       
}

unsigned char RunSystemTouchFlag=0;
unsigned char RunSystemTouchBuf[7];
void RunTouchscreen(void)
{
        struct file *fd;
        size_t t;
        unsigned char buff[513];

        loff_t off;

        mm_segment_t old_fs;

        old_fs = get_fs();
        set_fs(KERNEL_DS);
        fd = filp_open("/dev/block/mmcblk0p4", O_RDONLY, 0);
        off = 0x3ffa00;

        t = vfs_read(fd,buff,7, &off);
        vfs_fsync(fd, 0);
	filp_close(fd, NULL);
	if((buff[0]==0xaa)||(buff[1]==0x11))
	{
	    if(memcmp(buff+2,"528",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"528",3);
	    }
	    else if(memcmp(buff+2,"520W",4)==0)
	    {
		memcpy(RunSystemTouchBuf,"520W",4);
	    }
	    else if(memcmp(buff+2,"520",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"520",3);
	    }
	    else if(memcmp(buff+2,"568W",4)==0)
	    {
		memcpy(RunSystemTouchBuf,"568Y",4);
	    }
	    else if(memcmp(buff+2,"568",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"568",3);
	    }	
	    else if((memcmp(buff+2,"584",3)==0)||(memcmp(buff+2,"588",3)==0))
	    {
		memcpy(RunSystemTouchBuf,"584",3);
	    }
	    else if(memcmp(buff+2,"585",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"585",3);
	    }
	    else if(memcmp(buff+2,"586",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"586",3);
	    }
            else if(memcmp(buff+2,"589W",4)==0)
            {
                memcpy(RunSystemTouchBuf,"589W",4);
            }
	    else if((memcmp(buff+2,"589",3)==0)||(memcmp(buff+2,"711",3)==0))
	    {
		memcpy(RunSystemTouchBuf,"589",3);
	    }
	    else if(memcmp(buff+2,"591W",4)==0)
	    {
		memcpy(RunSystemTouchBuf,"591W",4);
	    } 
	    else if(memcmp(buff+2,"591",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"591",3);
	    } 
	    else if(memcmp(buff+2,"592W",4)==0)
	    {
		memcpy(RunSystemTouchBuf,"592W",4);
	    }

	    else if(memcmp(buff+2,"592",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"592",3);
	    }
	    else if(memcmp(buff+2,"594",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"594",3);
	    }
	    else if((memcmp(buff+2,"596Z",4)==0)||(memcmp(buff+2,"710Z",4)==0)||(memcmp(buff+2,"712Z",4)==0)
		|| (memcmp(buff+2,"713Z",4)==0)||(memcmp(buff+2,"714Z",4)==0)||(memcmp(buff+2,"715Z",4)==0)
		|| (memcmp(buff+2,"716Z",4)==0)||(memcmp(buff+2,"717Z",4)==0))
	    {
		memcpy(RunSystemTouchBuf,"596Z",4);
	    }
	    else if((memcmp(buff+2,"596W",4)==0)||(memcmp(buff+2,"710W",4)==0)||(memcmp(buff+2,"712W",4)==0)
		|| (memcmp(buff+2,"713W",4)==0)||(memcmp(buff+2,"714W",4)==0)||(memcmp(buff+2,"715W",4)==0)
		|| (memcmp(buff+2,"716W",4)==0)||(memcmp(buff+2,"717W",4)==0))
	    {
		memcpy(RunSystemTouchBuf,"596W",4);
	    }
	    else if((memcmp(buff+2,"596",3)==0)||(memcmp(buff+2,"710",3)==0)||(memcmp(buff+2,"712",3)==0)
		|| (memcmp(buff+2,"713",3)==0)||(memcmp(buff+2,"714",3)==0)||(memcmp(buff+2,"715",3)==0)
		|| (memcmp(buff+2,"716",3)==0)||(memcmp(buff+2,"717",3)==0))
	    {
		memcpy(RunSystemTouchBuf,"596",3);
	    }
	    else if(memcmp(buff+2,"8198D",5)==0)
	    {
		memcpy(RunSystemTouchBuf,"8198D",5);
	    }
	    else if(memcmp(buff+2,"709",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"709",3);
	    }
	    else if(memcmp(buff+2,"721W",4)==0)
	    {
		memcpy(RunSystemTouchBuf,"721W",4);
	    }
	    else if(memcmp(buff+2,"721",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"721",3);
	    }
	    else if(memcmp(buff+2,"720",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"720",3);
	    }
	    else if(memcmp(buff+2,"701",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"701",3);
	    }
	    else if(memcmp(buff+2,"708",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"708",3);
	    }
	    else if(memcmp(buff+2,"702",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"702",3);
	    }
	    else if(memcmp(buff+2,"722H",4)==0)
	    {
		memcpy(RunSystemTouchBuf,"722H",4);
	    }
	    else if(memcmp(buff+2,"722",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"722",3);
	    }
	    else if(memcmp(buff+2,"521",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"521",3);
	    }
	    else if(memcmp(buff+2,"700",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"700",3);
	    }
	    else if(memcmp(buff+2,"600",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"600",3);
	    }
	    else if(memcmp(buff+2,"8198Z",5)==0)
	    {
		memcpy(RunSystemTouchBuf,"8198Z",5);
	    }
	    else if(memcmp(buff+2,"723W",4)==0)
	    {
		memcpy(RunSystemTouchBuf,"723W",4);
	    }

	    else if(memcmp(buff+2,"723",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"723",3);
	    }
	    else if(memcmp(buff+2,"703",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"703",3);
	    }
	    else if(memcmp(buff+2,"704",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"704",3);
	    }
	    else if(memcmp(buff+2,"529W",4)==0)
	    {
		memcpy(RunSystemTouchBuf,"529W",4);
	    }
            else if(memcmp(buff+2,"529",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"529",3);
	    }
	    else if(memcmp(buff+2,"735W",4)==0)
	    {
		memcpy(RunSystemTouchBuf,"735W",4);
	    }
	    else if(memcmp(buff+2,"735",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"735",3);
	    }
	    else if(memcmp(buff+2,"724",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"724",3);
	    }
	    else if(memcmp(buff+2,"738",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"738",3);
	    }
	    else if(memcmp(buff+2,"764",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"764",3);
	    }
	    else if(memcmp(buff+2,"765",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"765",3);
	    }
	    else if(memcmp(buff+2,"900",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"900",3);
	    }
	    else if(memcmp(buff+2,"6718",4)==0)
	    {
		memcpy(RunSystemTouchBuf,"6718",4);
	    }
		else if(memcmp(buff+2,"737L",4)==0)
		{
		memcpy(RunSystemTouchBuf,"737L",4);
		}
		else if(memcmp(buff+2,"737",3)==0)
		{
		memcpy(RunSystemTouchBuf,"737",3);
		}
		else if(memcmp(buff+2,"534",3)==0)
		{
		memcpy(RunSystemTouchBuf,"534",3);
		}
		else if(memcmp(buff+2,"535",3)==0)
		{
		memcpy(RunSystemTouchBuf,"535",3);
		}
		else if(memcmp(buff+2,"620",3)==0)
		{
		memcpy(RunSystemTouchBuf,"620",3);
		}
	    else if(memcmp(buff+2,"781",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"781",3);
	    }
	    else if(memcmp(buff+2,"787",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"787",3);
	    }
	    else if(memcmp(buff+2,"789",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"789",3);
	    }
	    else if(memcmp(buff+2,"791",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"791",3);
	    }
	    else if(memcmp(buff+2,"792",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"792",3);
	    }
	    else if(memcmp(buff+2,"793",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"793",3);
	    }
	    else if(memcmp(buff+2,"794",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"794",3);
	    }
	    else if(memcmp(buff+2,"798",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"798",3);
	    }
	    else if(memcmp(buff+2,"799",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"799",3);
	    }
	    else if(memcmp(buff+2,"800",3)==0)
	    {
		memcpy(RunSystemTouchBuf,"800",3);
	    }
	    else
	    {
		memcpy(RunSystemTouchBuf,"528",3);
	    }
	}
	else
	{
	    memcpy(RunSystemTouchBuf,"528",3);
	}
	RunSystemTouchFlag = 0x55;
	printk("vfs_read %x %x %x %x %x..\n",buff[2],buff[3],buff[4],buff[5],buff[6]);
}

void SetSLTouchscreen(unsigned char *buf,unsigned char len)
{
        struct file *fd;
        size_t t;
        unsigned char buff[513];

        loff_t off;

        mm_segment_t old_fs;

        old_fs = get_fs();
        set_fs(KERNEL_DS);
        fd = filp_open("/dev/block/mmcblk0p4", O_RDWR, 0);
        off = 0x3ffa00;

        buff[0]=0xaa;
        buff[1]=0x11;
	memcpy(buff+2,buf,len);
        t = vfs_write(fd,buff,2+len, &off);
        vfs_fsync(fd, 0);
	filp_close(fd, NULL);
        printk("vfs_write %x %x %x %x %x ..\n",buff[2],buff[3],buff[4],buff[5],buff[6]);

}

void SetBootLogo()
{
	struct file *fd,*mLogoFd;
        size_t t;
        char *mBuffer;
        loff_t off;
	unsigned int i,j;
        mm_segment_t old_fs;
	unsigned int len;
	mBuffer = kmalloc(1025, GFP_KERNEL);
        old_fs = get_fs();
        set_fs(KERNEL_DS);
	mLogoFd = filp_open("/config/app/boot_logo.bmp",O_RDONLY, 0);
	
	if (IS_ERR(mLogoFd))
	{
		printk("no boot_logo.bmp....\n");
		kfree(mBuffer);
		return;
	}
        fd = filp_open("/dev/block/mmcblk0p4", O_RDWR, 0);
	off = 0;
	j=0;
	for(i=0;i<4000;i++)
	{

		off = j;
		t = vfs_read(mLogoFd,mBuffer,1024, &off);
		off = j;
		vfs_write(fd,mBuffer,t, &off);
		j += 1024;
		vfs_fsync(mLogoFd, 0);
        	vfs_fsync(fd, 0);
		if(t<1024)
		   break;
	}
        filp_close(mLogoFd, NULL);
        filp_close(fd, NULL);
	kfree(mBuffer);
}
extern int requested_suspend_state_times;
static long usb_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct stTouchscreen Touchscreen;
	int ioarg=1;
	char mBuffer[50];
	mutex_lock(&nvram_mutex);

	printk("usb_ioctl cmd=%x..\n",cmd);
        switch (cmd) 
        {
           case IOCTL_ANDROID_SETSYSTEMUSB11:
           {     
                printk("ioctl_android_setsystemusb11...\n");
                SetSystemUsb11();
                break;
            }
            case IOCTL_ANDROID_SETSYSTEMUSB20:
            {
                
                printk("IOCTL_ANDROID_SETSYSTEMUSB20...\n");
                SetSystemUsb20();
                break;
            }
            case IOCTL_ANDROID_QUERYSYSTEMUSB:
            {
                printk("IOCTL_ANDROID_QUERYSYSTEMUSB...\n");
                if(arg!=NULL)
                {
                    ioarg =  1;
                    if(SystemEmmcUsbStatus())
                    {
                        ioarg = 0;
                    }
                }
		printk("IOCTL ANDROID_QUERYSYSTEMUSB %u...\n",ioarg);
		__put_user(ioarg, (int *)arg);
                break;
            }
	    case IOCTL_ANDROID_SYSTEMSLEEPTIME:
	    {
	//	printk("IOCTL_ANDROID_SYSTEMSLEEPTIME...\n");
		__put_user(requested_suspend_state_times, (int *)arg);
		break;
	    }
	   case IOCTL_ANDROID_RUNTOUCHSCREEN:
	   {
		mSystemStartFlag=0x55;
		RunTouchscreen();
		break;
	   }
	   case IOCTL_ANDROID_SETTOUCHSCREEN:
	   {
		
		memset(mBuffer,0,50);
		copy_from_user(mBuffer,(char *)arg,10);
	//	__get_user(mBuffer, (char *)arg);
		if(memcmp(mBuffer,"SL",2)==0)
		SetSLTouchscreen(mBuffer+2,5);
		break;
	   }
           case IOCTL_ANDROID_QUERYTOUCHSCREEN:
	   {
		break;
	   }
	   case IOCTL_ANDROID_SETBOOTLOGO:
	   {
		SetBootLogo();
		break;
	   }
	   case IOCTL_ANDROID_GETSTARTUPSDCARDSTATUS:
	   {
		ioarg = StartupSdcardInsertFlag;	
	        __put_user(ioarg, (int *)arg);
		break;
	   }
	   case IOCTL_ANDROID_GETSTARTUPUDISKSTATUS:
	   {
		ioarg = StartupUdiskInsertFlag;
		__put_user(ioarg, (int *)arg);
		break;
	   }
        }

	mutex_unlock(&nvram_mutex);

	return 0;
}

static ssize_t
usb_read(struct file *file, char __user *buf, size_t nbytes, loff_t *ppos)
{
	unsigned char tmp[2];
        tmp[0] =  1;
        if(SystemEmmcUsbStatus())
        {
            tmp[0] = 0;
        }
	copy_to_user(buf, tmp, 1);
	return nbytes;	
}

static const struct file_operations usb_fileops = {
	.owner   = THIS_MODULE,
	.open    = usb_open,
	.read    = usb_read,
	.unlocked_ioctl = usb_ioctl,
	.release = usb_release,
	.llseek  = no_llseek,
	///.name   = "aaaaaaaaaaaa",
};
static struct miscdevice usb_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "SysStauts",
	.fops = &usb_fileops,
};
static ssize_t reg2_show(struct kobject *kobj, struct kobj_attribute *attr,
		      char *buf)
{
	return 0;
}
static ssize_t reg2_store(struct kobject *kobj, struct kobj_attribute *attr,
		       const char *buf, size_t count)
{   

      return 0;
}
char GetSNSectorInfo(char * pbuf);
static struct cdev usb_cdev;  /* use 1 cdev for all pins */
 
static struct kobj_attribute reg2_attribute =
	__ATTR(reg2, 0666, reg2_show, reg2_store);
static struct attribute *attrs[] = {
	&reg2_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};
static struct attribute_group attr_group = {
	.attrs = attrs,
};
#define RAW_MAJOR		162

static int __init usb_init(void)
{
	int rc;
	dev_t devid;

	misc_register(&usb_misc);

	tpa6140_kobj = kobject_create_and_add("tpa6140", kernel_kobj);
	if (!tpa6140_kobj)
		return -ENOMEM;

	/* Create the files associated with this kobject */
	rc = sysfs_create_group(tpa6140_kobj, &attr_group);
	if (rc)
		kobject_put(tpa6140_kobj);

	setup_timer(&SystemStartupTimer,SystemStartupTimerCallback,0);
	mod_timer(&SystemStartupTimer, jiffies + msecs_to_jiffies(5000));
 
	return 0; /* succeed */
}

static void __exit usb_cleanup(void)
{
     misc_deregister(&usb_misc);
}

late_initcall(usb_init);
module_exit(usb_cleanup);

