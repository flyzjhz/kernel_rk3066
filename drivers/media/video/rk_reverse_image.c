#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/platform_device.h>
#include <linux/kthread.h>
#include <linux/fb.h>
#include <linux/rk_fb.h>
#include <mach/gpio.h>
#include <media/soc_camera.h>
#if defined(CONFIG_ARCH_RKPX2) || defined(CONFIG_ARCH_RK30)||defined(CONFIG_ARCH_RK3188)
#include <mach/rk30_camera.h>
#endif
#include <plat/efuse.h>
#if (defined(CONFIG_ARCH_RK2928) || defined(CONFIG_ARCH_RK3026))
#include <mach/rk2928_camera.h>
#endif
#include "rk30_camera_oneframe.h"
#include "rk_reverse_image.h"

///#define DEBUG_DEFAUlT_REVERSE
#define DEBUG
#ifdef DEBUG
#define DBG(format, ...) \
		printk(KERN_INFO "rk_reverse: " format "\n", ## __VA_ARGS__)
#else
#define DBG(format, ...)
#endif

#define __OPEN__   1
#define __CLOSE__  0

#define GPIO_HIGH 1
#define GPIO_LOW  0

#define GPIO_REARVIEW INVALID_GPIO//RK30_PIN3_PC6//RK30_PIN4_PC1
#define GPIO_VAULE GPIO_LOW 

#define PORT_LCDC 0
#define PORT_CIF 0
#define __NUM 1

#define write_cif_reg(base,addr,val)  __raw_writel(val, addr+(base))
#define read_cif_reg(base,addr) __raw_readl(addr+(base))

static int rk_reverse_check_mode(void);
static void rk_reverse_get_size(int *width,int *height);
void rk_reverse_next_frame(void);
static void rk_reverse_cif_fmt(void);
void rk_reverse_image_mode(void);
void rk_normal_display_mode(void);
EXPORT_SYMBOL(rk_normal_display_mode);
EXPORT_SYMBOL(rk_reverse_image_mode);
static void rk_reverse_wait_driver(void);
static unsigned long rk_reverse_get_buf(void);

static int debug_boot_into_reverse(cur_state);
static void rk_sync_cif(void);
static void test_init_lcdc(void);
static void test_init_cif(void);
static void init_all(void);

struct soc_camera_device * reverse_icd = NULL;
struct rk_camera_dev *reverse_pcdev = NULL;

char *VIR_ADDR;
char *lcdc_vit;


/*
static void cif_read_all_reg(void);
static void cif_read_all_reg(){
	struct rk_camera_dev *pcdev = reverse_pcdev; 
		int i =0;

		while(1){

                        printk("reg:%02x, value:%08x \r\n",i*0x04,read_cif_reg(pcdev->base,i*0x04));
                        if(i*0x04 <= 0x6c)
                                i++;
                        else
                                break;
                }
}
*/
int state_nwd_cvbs=1;
static int rk_reverse_check_mode(){
	static int cur_state = -1,old_state = !GPIO_VAULE,num = 0;
	static int tst_cvbs=1;

	if(GPIO_REARVIEW == INVALID_GPIO)
		;///return -1;
	///if(tst_cvbs <4) 
		cur_state = state_nwd_cvbs;///gpio_get_value(GPIO_REARVIEW);
	///else
		///cur_state = 0;
	///tst_cvbs++;


	
	if(cur_state != old_state){
		if(num++>__NUM){
			old_state = cur_state;
			num = 0;
			return ((cur_state==GPIO_VAULE)?1:0);
		}
	}
	return ((cur_state==GPIO_VAULE)?2:-1);
}


static unsigned long rk_reverse_get_buf(){
	struct fb_info *info = registered_fb[1];

	return info->fix.smem_start;
}

static void rk_reverse_get_size(int *width,int *height){
	struct rk_camera_dev *pcdev = reverse_pcdev; 
	int val = 0;

	val = read_cif_reg(pcdev->base,CIF_CIF_SET_SIZE);
	*width = val&0xffff;
	*height = val>>16;
}

 void rk_reverse_next_frame(){
	struct rk_camera_dev *pcdev = reverse_pcdev; 
	int val = 0;

	val = read_cif_reg(pcdev->base,CIF_CIF_FRAME_STATUS);
	
	if(val&0x1){
		write_cif_reg(pcdev->base,CIF_CIF_FRAME_STATUS,val&0xfffe);
	}
}
EXPORT_SYMBOL(rk_reverse_next_frame);
static void rk_reverse_cif_addr(int addr,int width,int height){
	struct rk_camera_dev *pcdev = reverse_pcdev;
 
	write_cif_reg(pcdev->base,CIF_CIF_FRM0_ADDR_Y,addr);
	write_cif_reg(pcdev->base,CIF_CIF_FRM0_ADDR_UV,(addr+width*height));
}

static void rk_reverse_cif_interrupt(int state){
	struct rk_camera_dev *pcdev = reverse_pcdev;
 
	write_cif_reg(pcdev->base,CIF_CIF_INTEN,0);
}

static void rk_reverse_cif_capture(int state){
	struct rk_camera_dev *pcdev = reverse_pcdev;
	int val = 0;
 
	val = read_cif_reg(pcdev->base,CIF_CIF_CTRL);
	if(state == 1)
		write_cif_reg(pcdev->base,CIF_CIF_CTRL,val|0x01);
	else
		write_cif_reg(pcdev->base,CIF_CIF_CTRL,val&(~0x01));	
}

static void rk_reverse_set_fb(int state){
	struct fb_info *info_win0 = registered_fb[1];
        struct fb_info *info_win1 = registered_fb[0];
        struct rk_lcdc_device_driver * dev_drv_win0 = (struct rk_lcdc_device_driver * )info_win0->par;
        struct rk_lcdc_device_driver * dev_drv_win1 = (struct rk_lcdc_device_driver * )info_win1->par;
        int layer_id = 0;

	if(state == 1){
		//dev_drv_win1->open(dev_drv_win1,1,0);
		layer_id = dev_drv_win1->fb_get_layer(dev_drv_win1,info_win1->fix.id);
    		dev_drv_win1->layer_par[layer_id]->state = 1;
		info_win0->fbops->fb_open(info_win0,1);
		dev_drv_win1->open(dev_drv_win1,1,0);
		info_win0->fbops->fb_ioctl(info_win0,RK_FBIOSET_CONFIG_DONE,0);
	}else{
        	layer_id = dev_drv_win1->fb_get_layer(dev_drv_win1,info_win1->fix.id);
        	dev_drv_win1->layer_par[layer_id]->state = 0;
        	dev_drv_win1->open(dev_drv_win1,1,1);
        	dev_drv_win0->open(dev_drv_win0,0,0);
		info_win1->fbops->fb_ioctl(info_win1,RK_FBIOSET_CONFIG_DONE,0);
	}
}

static void rk_reverse_set_pan(unsigned long addr,int width,int height){
	struct fb_info *info = registered_fb[1];
	struct fb_var_screeninfo *var = &info->var;

	var->xres_virtual = width;
	var->yres_virtual = height;
	var->xres = width;
	var->yres = height;
	//var->width = width;
	//var->height = height;
	//var->xoffset = 0;
	//var->yoffset = 0;
	//printk("rk_reverse_set_pan : %d , %d \r\n",var_1->width,var_1->height);
//	var->grayscale = (he_x<<8) | (he_y<<20) ;
	var->nonstd =(var->xoffset<<8)|(var->yoffset)|HAL_PIXEL_FORMAT_YCrCb_NV12;

	info->fbops->fb_ioctl(info,RK_FBIOSET_YUV_ADDR_Y,addr);
	info->fbops->fb_ioctl(info,RK_FBIOSET_YUV_ADDR_UV,(addr+(width*height)));
	info->fbops->fb_set_par(info);
	info->fbops->fb_pan_display(var,info);

	info->fbops->fb_ioctl(info,RK_FBIOSET_CONFIG_DONE,0);
}

static int rk_reverse_sensor_clk(int state){
	struct rk_camera_dev *pcdev = reverse_pcdev; 

	if(pcdev->pdata->sensor_mclk == NULL)
		return -1;

	if(state == 0){
		pcdev->pdata->sensor_mclk(0,0,0); ///wj 
		///pcdev->pdata->sensor_mclk(33,0,0);
	}else{
		pcdev->pdata->sensor_mclk(33,1,24000000);	
	}
	return 1;
}

static void rk_reverse_cif_swith(int state){
	struct rk_camera_dev *pcdev = reverse_pcdev; 
	struct soc_camera_device *icd= reverse_icd;
	
	if(state)
		pcdev->soc_host.ops->add(icd);	
	else
		pcdev->soc_host.ops->remove(pcdev->icd);		
}

static void rk_reverse_cif_fmt(){
	struct rk_camera_dev *pcdev = reverse_pcdev; 
	struct soc_camera_device *icd= pcdev->icd;
	struct v4l2_format format;
//	struct v4l2_control sctrl;

//	pcdev->soc_host.ops->add(reverse_icd);
//	pcdev->soc_host.ops->init_videobuf(&he_icd->vb_vidq, he_icd);
//	sctrl.id = 10094861;
//	sctrl.value = 100;
//	pcdev->soc_host.ops->set_ctrl(he_icd,&sctrl);
	
	format.type= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width = 10000;
	format.fmt.pix.height = 10000;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
	format.fmt.pix.field = V4L2_FIELD_NONE;
	format.fmt.pix.colorspace = 0;
//	format.fmt.pix.priv = 1515912958;
//	format.fmt.pix.sizeimage = 460800;

	pcdev->soc_host.ops->try_fmt(icd,&format);	
	pcdev->soc_host.ops->set_fmt(icd,&format);
	pcdev->soc_host.ops->try_fmt(icd,&format);	
	pcdev->soc_host.ops->set_fmt(icd,&format);	
//	pcdev->soc_host.ops->set_bus_param(he_icd,0);
//	cif_read_all_reg();	
}

  void rk_reverse_image_mode(){
	unsigned long addr = 0;
	int ret = 0,width = 0,height = 0;
	
	addr = rk_reverse_get_buf();

	//cif
	ret = rk_reverse_sensor_clk(__OPEN__);
	rk_reverse_cif_fmt();	
	rk_reverse_get_size(&width,&height);
	rk_reverse_cif_addr(addr,width,height);
	rk_reverse_cif_interrupt(__CLOSE__);
	rk_reverse_cif_capture(__OPEN__);			
	
	//rk_fb
	rk_reverse_set_fb(__OPEN__);
	rk_reverse_set_pan(addr,width,height);

}

 void rk_normal_display_mode(){
	rk_reverse_set_fb(__CLOSE__);
	rk_reverse_cif_capture(__CLOSE__);
	rk_reverse_sensor_clk(__CLOSE__);
}
	
static void rk_reverse_wait_driver(){
	while(reverse_icd == NULL || reverse_pcdev == NULL)
		udelay(5);
	
	rk_reverse_cif_swith(__OPEN__);
}

static void addr_test(int addr1,int addr2,int addr3,int addr4){
	write_cif_reg(VIR_ADDR,0x14,addr1);
	write_cif_reg(VIR_ADDR,0x18,addr2);

	write_cif_reg(lcdc_vit,0x28,addr3);
	write_cif_reg(lcdc_vit,0x2c,addr4);
	
}

static void rk_sync_cif(){
	int val = 0;
	int i = 0;
	while(1){
		val = read_cif_reg(VIR_ADDR,CIF_CIF_FRAME_STATUS);
	
		if(val&0x1){
			if(i == 0){
				i = 1;
				addr_test(0x9a000000,0x9a040b00,0x99000000,0x99040b00);
			}else{
				i = 0;
				addr_test(0x99000000,0x99040b00,0x9a000000,0x9a040b00);

			}
	
			write_cif_reg(VIR_ADDR,CIF_CIF_FRAME_STATUS,val&0xfe);
		}
		mdelay(30);
	}
}


static void test_init_lcdc(){    
	int value[512] = {
0xfac68800,0x00000041,0x80000280,0x00000000
,0x00000020,0x00711c08,0x00000000,0x00000000
,0x00000000,0x00000000,0x9a000000,0x9a040b00
,0x00000000,0x00000000,0x00000090,0x01cb023f
,0x025703ff,0x001a00c8,0x0c440900,0x06220480
,0x00000000,0x9b4b0000,0x9b4b0000,0x00000400
,0x025703ff,0x025703ff,0x001a00c8,0x10001000
,0x10001000,0x00000000,0x00000000,0x00000140
,0x00ef013f,0x000a000a,0x00000000,0x000a000a
,0x00000000,0x00000000,0x00000000,0x05400064
,0x00c804c8,0x027b0003,0x001a0272,0x00000000
,0x00000000,0x00000041,0x00000000,0x00000000
,0x00000000,0x00000000
};
	int addr = 0,i = 0;

	lcdc_vit = ioremap(0x1010c000,0x400);

	while(1){
                addr = i*0x04;

		write_cif_reg(lcdc_vit,addr,value[i]);

                if(addr == 0xb0)
                        break;
                else
                        i++;
        }
	write_cif_reg(lcdc_vit,0xc0,0x01);


	printk("read-lcdc : %08x \r\n",read_cif_reg(lcdc_vit,0x04));

}

static void test_init_cif(){
	int i =0,addr=0,value=0;
	int reg[1024]= {
0x0000f001,0x00000000,0x0000024b,0x0001020c
,0x00000000,0x9a000000,0x9a040b00,0x00000000
,0x00000000,0x00000240,0x01cc0240,0x00000000
,0x00000000,0x00000000,0x00000000,0x00000000
,0x00000000,0x00160048,0x00000010,0x00000000
,0x20002000,0x00000000,0x00000000,0x00000000
,0x00000000,0x00034840,0x000001cc,0x00000240
,0x00000000,0x00000000,0x00000000,0x00000000
,0x00000000,0x00000000,0x00000000,0x00000000
,0x00000000,0x00000000,0x00000000,0x00000000
,0x00000000,0x00000000,0x00000000,0x00000000
,0x00000000,0x00000000,0x00000000,0x00000000
,0x00000000,0x00000000
};
	
	 while(1){
                addr = i*0x04;
                value= reg[i];
		write_cif_reg(VIR_ADDR,addr,value);
                i++;
                if(addr == 0x6c)
                        break;
	printk("read-cif : %02x , %08x \r\n",addr,read_cif_reg(VIR_ADDR,0x00));
        }
	//printk("read-cif : %08x \r\n",read_cif_reg(VIR_ADDR,0x00));
}

static void init_all(){

	VIR_ADDR = ioremap(0x10108000,0x70);
	rk_reverse_image_mode();
	test_init_lcdc();

	test_init_cif();

	rk_sync_cif();
	while(1){mdelay(5);}
}

static int debug_boot_into_reverse(cur_state){
	static int i = 0;
	int state = -1;
		
	if(cur_state == 2){
		if(i++>150)
			return 0;

		return cur_state;
	}
	else{
		if(i--<0){
			cur_state++;
			return cur_state;
		}
	}
		
	return -1; 
}

int rk_reverse_image_main(void *arg){
	int state= 0;

	rk_reverse_wait_driver();
	///gpio_direction_input(GPIO_REARVIEW);

	 while(1)
	{	
#ifdef DEBUG_DEFAUlT_REVERSE
		nwd();
		state = debug_boot_into_reverse(state);
#else
		state = rk_reverse_check_mode();///nwd(); ///this
#endif
   	  ///state = state_nwd_cvbs ; ///
		if(state == 0){ ///nwd();
			 rk_normal_display_mode();
		}
		else if(state == 1){ ///nwd();
			 rk_reverse_image_mode();
		}
		else if(state == 2){///nwd();
			 rk_reverse_next_frame();
		}
		///rk_normal_display_mode();
		mdelay(15);
	}
	
	return 0;
}


