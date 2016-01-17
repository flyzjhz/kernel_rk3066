#ifndef RK30_CAMERA_ONEFRAME_H
#define RK30_CAMERA_ONEFRAME_H

// CIF Reg Offset
#define  CIF_CIF_CTRL                       0x00
#define  CIF_CIF_INTEN                      0x04
#define  CIF_CIF_INTSTAT                    0x08
#define  CIF_CIF_FOR                        0x0c
#define  CIF_CIF_LINE_NUM_ADDR              0x10
#define  CIF_CIF_FRM0_ADDR_Y                0x14
#define  CIF_CIF_FRM0_ADDR_UV               0x18
#define  CIF_CIF_FRM1_ADDR_Y                0x1c
#define  CIF_CIF_FRM1_ADDR_UV               0x20
#define  CIF_CIF_VIR_LINE_WIDTH             0x24
#define  CIF_CIF_SET_SIZE                   0x28
#define  CIF_CIF_SCM_ADDR_Y                 0x2c
#define  CIF_CIF_SCM_ADDR_U                 0x30
#define  CIF_CIF_SCM_ADDR_V                 0x34
#define  CIF_CIF_WB_UP_FILTER               0x38
#define  CIF_CIF_WB_LOW_FILTER              0x3c
#define  CIF_CIF_WBC_CNT                    0x40
#define  CIF_CIF_CROP                       0x44
#define  CIF_CIF_SCL_CTRL                   0x48
#define	 CIF_CIF_SCL_DST                    0x4c
#define	 CIF_CIF_SCL_FCT                    0x50
#define	 CIF_CIF_SCL_VALID_NUM              0x54
#define	 CIF_CIF_LINE_LOOP_CTR              0x58
#define	 CIF_CIF_FRAME_STATUS               0x60
#define	 CIF_CIF_CUR_DST                    0x64
#define	 CIF_CIF_LAST_LINE                  0x68
#define	 CIF_CIF_LAST_PIX                   0x6c

/* buffer for one video frame */
struct rk_camera_buffer
{
    /* common v4l buffer stuff -- must be first */
    struct videobuf_buffer vb;
    enum v4l2_mbus_pixelcode	code;
    int			inwork;
};
enum rk_camera_reg_state
{
	Reg_Invalidate,
	Reg_Validate
};

struct rk_camera_reg
{
	unsigned int cifCtrl;
	unsigned int cifCrop;
	unsigned int cifFs;
	unsigned int cifIntEn;
	unsigned int cifFmt;
    unsigned int cifVirWidth;
    unsigned int cifScale;
//	unsigned int VipCrm;
	enum rk_camera_reg_state Inval;
};
struct rk_camera_work
{
	struct videobuf_buffer *vb;
	struct rk_camera_dev *pcdev;
	struct work_struct work;
    struct list_head queue;
    unsigned int index;    
};
struct rk_camera_frmivalenum
{
    struct v4l2_frmivalenum fival;
    struct rk_camera_frmivalenum *nxt;
};
struct rk_camera_frmivalinfo
{
    struct soc_camera_device *icd;
    struct rk_camera_frmivalenum *fival_list;
};
struct rk_camera_zoominfo
{
    struct semaphore sem;
    struct v4l2_crop a;
    int vir_width;
    int vir_height;
    int zoom_rate;
};
#if CAMERA_VIDEOBUF_ARM_ACCESS
struct rk29_camera_vbinfo
{
    unsigned int phy_addr;
    void __iomem *vir_addr;
    unsigned int size;
};
#endif
struct rk_camera_timer{
	struct rk_camera_dev *pcdev;
	struct hrtimer timer;
    bool istarted;
};
struct rk_cif_clk 
{
    //************must modify start************/
	struct clk *pd_cif;
	struct clk *aclk_cif;
    struct clk *hclk_cif;
    struct clk *cif_clk_in;
    struct clk *cif_clk_out;
	//************must modify end************/

    spinlock_t lock;
    bool on;
};

struct rk_cif_crop
{
    spinlock_t lock;
    struct v4l2_rect c;
    struct v4l2_rect bounds;
};

struct rk_cif_irqinfo
{
    unsigned int irq;
    unsigned long cifirq_idx;
    unsigned long cifirq_normal_idx;
    unsigned long cifirq_abnormal_idx;

    unsigned long dmairq_idx;
    spinlock_t lock;
};

struct rk_camera_dev
{
    struct soc_camera_host	soc_host;    
    struct device		*dev;
    /* RK2827x is only supposed to handle one camera on its Quick Capture
     * interface. If anyone ever builds hardware to enable more than
     * one camera, they will have to modify this driver too */
    struct soc_camera_device *icd;
    void __iomem *base;
    int frame_inval;           /* ddl@rock-chips.com : The first frames is invalidate  */

    unsigned int fps;
    unsigned int last_fps;
    unsigned long frame_interval;
    unsigned int pixfmt;
    //for ipp	
    unsigned int vipmem_phybase;
    void __iomem *vipmem_virbase;
    unsigned int vipmem_size;
    unsigned int vipmem_bsize;
#if CAMERA_VIDEOBUF_ARM_ACCESS    
    struct rk29_camera_vbinfo *vbinfo;
    unsigned int vbinfo_count;
#endif    
    int host_width;
    int host_height;
    int host_left;  //sensor output size ?
    int host_top;
    int hostid;
    int icd_width;
    int icd_height;

    struct rk_cif_crop cropinfo;
    struct rk_cif_irqinfo irqinfo;

    struct rk29camera_platform_data *pdata;
    struct resource		*res;
    struct list_head	capture;
    struct rk_camera_zoominfo zoominfo;

    spinlock_t		lock;

    struct videobuf_buffer	*active;
    struct rk_camera_reg reginfo_suspend;
    struct workqueue_struct *camera_wq;
    struct rk_camera_work *camera_work;
    struct list_head camera_work_queue;
    spinlock_t camera_work_lock;
    unsigned int camera_work_count;
    struct rk_camera_timer fps_timer;
    struct rk_camera_work camera_reinit_work;
    int icd_init;
    rk29_camera_sensor_cb_s icd_cb;
    struct rk_camera_frmivalinfo icd_frmival[2];
    bool timer_get_fps;
    unsigned int reinit_times; 
    struct videobuf_queue *video_vq;
    atomic_t stop_cif;
    struct timeval first_tv;
    
    int chip_id;
};

#endif


