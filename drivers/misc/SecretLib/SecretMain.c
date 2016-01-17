#include "windows.h"
extern BOOL AT_Boot_Authenticate(unsigned int RdPassWord ,void *pData );
extern int	bPassSecretIC;

static char uboot_uid[16] = {0};
struct i2c_client *atsha204a_client = NULL;
struct timer_list atsha204a_timer;
extern void kernel_restart(char *cmd);
BOOL Boot_Auth(char *uid)
{
	//===================================================
	// COPR	 :NOWADA
	// AUTHOR:Peter.Lee
	// DATE	 :2010/03/25
	//
	// DESCRIPTION:
	// Read UUID.
	//===================================================
	//volatile int 	i;
	//RETAILMSG(1,(TEXT("\n BspEbootPreBootMonitor is start:")));
	//if( !AT_Boot_Authenticate( 0x090401,v_pDriverGlobals->g_BootConf.OSConf.UUID) )
	int i;  
    if( !AT_Boot_Authenticate( 0x090401, uid))
	{
		RETAILMSG(1,(TEXT("\n Boot Authenticate is Error,CPU stop to run!!!!!!\r\n")));
		return FALSE;
	}
#if 0
	RETAILMSG(1,(TEXT("\n  Device ID Info:")));
	for(i=0;i<16;i++)
	{
		if ((i&7) ==0)
		{
			RETAILMSG(1,(TEXT("\n        ")));
		}
		RETAILMSG(1,(TEXT("0x%x,"),v_pDriverGlobals->g_BootConf.OSConf.UUID[i]));
	}
	RETAILMSG(1,(TEXT("\r\n")));
#endif
	bPassSecretIC ++;

	return TRUE;
}


static int atsha204a_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
  
	struct i2c_msg msgs[2];
	int i;

	printk(KERN_INFO "-----%s------ \n", __FUNCTION__);

	atsha204a_client = client;

	if (false == Boot_Auth(uboot_uid)) {
			printk("atsha204 password fail, reboot system after 5 seconds !!!!!\n");
			msleep(5);
			kernel_restart(NULL);
	} else {
		printk("atsha204 Boot_Auth ok\n");
	}

	return 0;
}

static int atsha204a_remove(struct i2c_client *client)
{
        printk(KERN_INFO "-----%s------- \n", __FUNCTION__);
        return 0;
}


static const struct i2c_device_id atsha204a_id[] =
{
        { "atsha204a", 0 },
        {  },
};

static struct i2c_driver atsha204a_dirver =
{
        .driver =
        {
                .name = "atsha204a",
        },
        .probe = atsha204a_probe,
        .remove = atsha204a_remove,
        .id_table = atsha204a_id,
};

static int __init atsha204a_module_init(void)
{
        printk(KERN_INFO ">--------------%s----------------- \n", __FUNCTION__);
        return i2c_add_driver(&atsha204a_dirver);
}

static void __exit atsha204a_module_exit(void)
{
        printk(KERN_INFO ">--------------%s----------------- \n", __FUNCTION__);
        i2c_del_driver(&atsha204a_dirver);
}

module_init(atsha204a_module_init);
module_exit(atsha204a_module_exit);

MODULE_DESCRIPTION("nwd-secretlib");
MODULE_AUTHOR("nwd");
MODULE_LICENSE("GPL");


