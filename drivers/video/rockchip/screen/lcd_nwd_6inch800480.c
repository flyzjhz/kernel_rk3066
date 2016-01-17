/* This Lcd Driver is HSD070IDW1 write by cst 2009.10.27 */

#ifndef __LCD_AUO__
#define __LCD_AUO__






/* Base */
#define SCREEN_TYPE		SCREEN_RGB
#define LVDS_FORMAT		LVDS_8BIT_1
#define OUT_FACE		OUT_P888
#define DCLK			37486800///33000000
#define LCDC_ACLK       	150000000     //29 lcdc axi DMA ÆµÂÊ

/* Timing */
#define H_PW			3 ////8 //10
#define H_BP			170 ///88 //100
#define H_VD			800 //1024
#define H_FP			95 ///40 //210

#define V_PW			1 ///3 //10
#define V_BP			 28 ///10 //10
#define V_VD			480 //768
#define V_FP			 76 ///32 //18

/* Other */
#define DCLK_POL                0
#define DEN_POL			0
#define VSYNC_POL		0
#define HSYNC_POL		0

#define SWAP_RB			0
#define SWAP_RG			0
#define SWAP_GB			0 


#define LCD_WIDTH       154    //need modify
#define LCD_HEIGHT      85

#endif

