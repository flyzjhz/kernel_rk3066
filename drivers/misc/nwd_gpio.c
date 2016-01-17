/*
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */


#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/string.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/delay.h>

#if defined(CONFIG_ZHUOYI_BMW520_YUANCHEPING)
#define AUDIO_MIC_SW1  RK30_PIN4_PB7
#define AUDIO_MIC_SW2  RK30_PIN6_PA1
#define RESET_3G	RK30_PIN4_PD5
#define ON_OFF_3G	RK30_PIN0_PC6
#elif defined(CONFIG_JILI_HONGJING_XIONGMAO)
#define POWER_3G	RK30_PIN4_PD5
#define RESET_3G	RK30_PIN0_PC6
#elif defined(CONFIG_SHENGHUALELV)
#define AUDIO_BT_SW  RK30_PIN6_PA1
#elif defined(CONFIG_SL_AUDIQ3)//SL_AudiQ3
#define AUDIO_MIC_SW  RK30_PIN6_PA7
#else
#define AUDIO_BT_SW  RK30_PIN6_PA1
#define AUDIO_MIC_SW1  RK30_PIN2_PB5
#define AUDIO_MIC_SW2  RK30_PIN2_PB6

#define USBHOST_POWER RK30_PIN0_PC5 
#define USB_OTG_POWER RK30_PIN6_PA0 
#define SDIO_WIFI_POWER RK30_PIN3_PA7
#endif

#define AUDIO_SW_DEBUG  

#ifdef  AUDIO_SW_DEBUG
    #define dbg_func(fmt, ...)              \
        do {                                \
            printk(                         \
                " (%d) %s: " fmt, task_pid_nr(current),     \
                         __func__ , ##__VA_ARGS__);          \
            } while(0)
#else
    #define dbg_func(fmt, ...)
#endif

/*switch bt audio channel or system audio channel, 1 -> bt; 1 -> 0*/



/*audio channel state*/
struct gpio_index{
};
struct oh_base{
    char audio_bt;
    char audio_system;
    char switch_3g;
    char mic1;
    char mic2;
};

static ssize_t oh_get_audio(struct device *dev, struct device_attribute *attr, char *buf)
{
        struct platform_device *pdev = to_platform_device(dev);
        struct oh_base *oh = platform_get_drvdata(pdev);
        return sprintf(buf, "%d", oh->switch_3g);
}

static ssize_t oh_set_audio(struct device *dev,
                struct device_attribute *attr, const char *buf, size_t count)
{
        struct platform_device *pdev = to_platform_device(dev);
        struct oh_base *oh = platform_get_drvdata(pdev);

#if defined(CONFIG_ZHUOYI_BMW520_YUANCHEPING)
    if(!strncmp(buf, "bt_enable", 9)){
            dbg_func("bt_enable\n");
        gpio_set_value(AUDIO_MIC_SW1, 1);
        gpio_set_value(AUDIO_MIC_SW2, 0);
        oh->audio_bt = 1;
        oh->audio_system = 0;
    }

    if(!strncmp(buf, "system_enable", 13)){
            dbg_func("system_enable\n");

        gpio_set_value(AUDIO_MIC_SW1, 0);
        gpio_set_value(AUDIO_MIC_SW2, 1);

        oh->audio_bt = 0;
        oh->audio_system = 1;
    }

    if(!strncmp(buf, "3g_onoff", 8)){
            dbg_func("system_enable\n");

        gpio_set_value(ON_OFF_3G, 0);
	msleep(4000);
        gpio_set_value(ON_OFF_3G, 1);

    }

    if(!strncmp(buf, "3g_reset", 8)){

        dbg_func("3g reset\n");
        gpio_set_value(RESET_3G, 1);
	msleep(150);
        gpio_set_value(RESET_3G, 0);

    }

    if(!strncmp(buf, "3g_off", 6)){

        dbg_func("3g_off\n");
	gpio_set_value(RESET_3G, 1);
        oh->switch_3g = 0;
    }

    if(!strncmp(buf, "3g_on", 5)){

        dbg_func("3g_on\n");
	gpio_set_value(RESET_3G, 0);
        oh->switch_3g = 1;

    }

#elif defined(CONFIG_JILI_HONGJING_XIONGMAO)
    if(!strncmp(buf, "3gon", 4)){
        dbg_func("3g_enable\n");
        gpio_set_value(POWER_3G, 1);
    }

    if(!strncmp(buf, "3goff", 5)){
        dbg_func("3g_disable\n");
        gpio_set_value(POWER_3G, 0);
    }

    if(!strncmp(buf, "3g_reseton", 10)){
        dbg_func("3g reset set hight\n");
        gpio_set_value(RESET_3G, 1);
    }
    if(!strncmp(buf, "3g_resetoff", 11)){
        dbg_func("3g reset set low\n");
        gpio_set_value(RESET_3G, 0);
    }

#elif defined(CONFIG_SHENGHUALELV)
    if(!strncmp(buf, "bt_enable", 9)){
            dbg_func("bt_enable\n");
        gpio_set_value(AUDIO_BT_SW, 0);
        oh->audio_bt = 1;
        oh->audio_system = 0;
    }

    if(!strncmp(buf, "system_enable", 13)){
            dbg_func("system_enable\n");
        gpio_set_value(AUDIO_BT_SW, 1);
        oh->audio_bt = 0;
        oh->audio_system = 1;
    }

#elif defined(CONFIG_SL_AUDIQ3)
	if(!strncmp(buf, "bt_enable", 9)){
		dbg_func("bt_enable\n");
		gpio_set_value(AUDIO_MIC_SW, 0);
		oh->audio_bt = 1;
		oh->audio_system = 0;
	}
	if(!strncmp(buf, "system_enable", 13)){
		dbg_func("system_enable\n");

		gpio_set_value(AUDIO_MIC_SW, 1);

		oh->audio_bt = 0;
		oh->audio_system = 1;
	}

#else
    if(!strncmp(buf, "bt_enable", 9)){
            dbg_func("bt_enable\n");
        gpio_set_value(AUDIO_BT_SW, 0);
        oh->audio_bt = 1;
        oh->audio_system = 0;
    }

    if(!strncmp(buf, "system_enable", 13)){
            dbg_func("system_enable\n");
        gpio_set_value(AUDIO_BT_SW, 1);
        oh->audio_bt = 0;
        oh->audio_system = 1;
    }

    if(!strncmp(buf, "hoston", 6)){
            dbg_func("host on\n");
        gpio_set_value(USBHOST_POWER, 1);
    }

    if(!strncmp(buf, "hostoff", 7)){
            dbg_func("host off\n");
        gpio_set_value(USBHOST_POWER, 0);
    }

    if(!strncmp(buf, "mic1_high", 9)){
            dbg_func("mic1_high\n");
        gpio_set_value(AUDIO_MIC_SW1, 1);
        oh->mic1 = 1;
    }

    if(!strncmp(buf, "mic1_low", 8)){
            dbg_func("mic1_low\n");
        gpio_set_value(AUDIO_MIC_SW1, 0);
        oh->mic1 = 0;
    }

    if(!strncmp(buf, "mic2_high", 9)){
            dbg_func("mic2_high\n");
        gpio_set_value(AUDIO_MIC_SW2, 1);
        oh->mic2 = 1;
    }

    if(!strncmp(buf, "mic2_low", 8)){
            dbg_func("mic2_low\n");
        gpio_set_value(AUDIO_MIC_SW2, 0);
        oh->mic2 = 0;
    }
#endif

    return count;
}

/**************************************************************/

static DEVICE_ATTR(audioincall, S_IWUGO | S_IRUGO, oh_get_audio, oh_set_audio);

static struct attribute *nwd_gpio_attrs[] = {
        &dev_attr_audioincall.attr,
        NULL
};
static const struct attribute_group nwd_gpio_files = {
        .attrs  = nwd_gpio_attrs,
};

static int __init oh_sysfs_probe(struct platform_device *pdev)
{
        struct oh_base *oh;
        int err;

        dbg_func("****oh audio switch*****\n");

        oh = kzalloc(sizeof(struct oh_base), GFP_KERNEL);
        if (!oh) {
                err = -ENOMEM;
                goto fail_no_mem;
        }

    oh->audio_bt = 0;
    oh->audio_system = 1;
    oh->switch_3g= 1;
    oh->mic1 = 0;
    oh->mic1 = 1;

/*default channel is system channel*/
#if defined(CONFIG_ZHUOYI_BMW520_YUANCHEPING)
    gpio_request(AUDIO_MIC_SW1, "audio_MIC_switch1");
    gpio_direction_output(AUDIO_MIC_SW1, 0);
    gpio_set_value(AUDIO_MIC_SW1, 0);

    gpio_request(AUDIO_MIC_SW2, "audio_MIC_switch2");
    gpio_direction_output(AUDIO_MIC_SW2, 1);
    gpio_set_value(AUDIO_MIC_SW2, 1);

    gpio_request(ON_OFF_3G, "3g on/off");
    gpio_direction_output(ON_OFF_3G, 1);
    gpio_set_value(ON_OFF_3G, 1);

    gpio_request(RESET_3G, "3g reset pin");
    gpio_direction_output(RESET_3G, 1);
    gpio_set_value(RESET_3G, 1);
    msleep(500);//150
    gpio_direction_output(RESET_3G, 0);
    gpio_set_value(RESET_3G, 0);

#elif defined(CONFIG_QINWEN_BMW520_YUANCHEPING)
    gpio_request(AUDIO_BT_SW, "audio_bt_switch");
    gpio_direction_output(AUDIO_BT_SW, 1);
    gpio_set_value(AUDIO_BT_SW, 1);

    gpio_request(AUDIO_MIC_SW1, "audio_MIC_switch1");
    gpio_direction_output(AUDIO_MIC_SW1, 0);
    gpio_set_value(AUDIO_MIC_SW1, 0);

    gpio_request(AUDIO_MIC_SW2, "audio_MIC_switch2");
    gpio_direction_output(AUDIO_MIC_SW2, 1);
    gpio_set_value(AUDIO_MIC_SW2, 1);
#elif defined(CONFIG_JILI_HONGJING_XIONGMAO) 
    gpio_request(POWER_3G, "3g_power");
    gpio_direction_output(POWER_3G, 0);
    gpio_set_value(POWER_3G, 0);

    gpio_request(RESET_3G, "3g reset pin");
    gpio_direction_output(RESET_3G, 1);
    gpio_set_value(RESET_3G, 1);
    msleep(200);
    gpio_direction_output(RESET_3G, 0);
    gpio_set_value(RESET_3G, 0);
#elif defined(CONFIG_SL_AUDIQ3)
	gpio_request(AUDIO_MIC_SW, "audio_MIC_switch");
	gpio_direction_output(AUDIO_MIC_SW, 1);
	gpio_set_value(AUDIO_MIC_SW, 1);
#else 
    gpio_request(AUDIO_BT_SW, "audio_bt_switch");
    gpio_direction_output(AUDIO_BT_SW, 1);
    gpio_set_value(AUDIO_BT_SW, 1);

#endif

    platform_set_drvdata(pdev, oh);

        err = sysfs_create_group(&pdev->dev.kobj, &nwd_gpio_files);
        if (err)
                goto fail_no_sysfs;
        return 0;

fail_no_sysfs:
        kfree(oh);
fail_no_mem:
        return err;
}

static int __exit oh_sysfs_remove(struct platform_device *pdev)
{
        struct oh_base *oh = platform_get_drvdata(pdev);

        platform_set_drvdata(pdev, NULL);
        sysfs_remove_group(&pdev->dev.kobj, &nwd_gpio_files);
        kfree(oh);

        return 0;
}

static struct platform_driver nwd_gpio_driver = {
        .driver         = {
                .name   = "oh_audio_sw",
                .owner  = THIS_MODULE,
        },
        .remove         = __exit_p(oh_sysfs_remove),
};

static int __init oh_sysfs_init(void)
{
        return platform_driver_probe(&nwd_gpio_driver, oh_sysfs_probe);
}

static void __exit oh_sysfs_exit(void)
{
        platform_driver_unregister(&nwd_gpio_driver);
}

module_init(oh_sysfs_init);
module_exit(oh_sysfs_exit);

MODULE_DESCRIPTION("nwd_gpio");
MODULE_LICENSE("GPL");


                     




