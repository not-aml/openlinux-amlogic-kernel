/*
 * Amlogic M1 & M2
 * frame buffer driver  -------bt656 & 601 input
 * Copyright (C) 2010 Amlogic, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the named License,
 * or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/amports/amstream.h>
#include <linux/amports/ptsserv.h>
#include <linux/amports/canvas.h>
#include <linux/workqueue.h>
#include <linux/dma-mapping.h>
#include <asm/delay.h>
#include <asm/atomic.h>
#include <mach/am_regs.h>
#include "tvin_global.h"
#include "bt656_601_in.h"
#include "vdin_regs.h"


#define DEVICE_NAME "amvdec_656in"
#define MODULE_NAME "amvdec_656in"
#define BT656IN_IRQ_NAME "amvdec_656in-irq"

//#define HANDLE_BT656IN_IRQ

//#define BT656IN_COUNT 			32
//#define BT656IN_ANCI_DATA_SIZE     	0x4000
#define BR656IN_NTSC_FLAG		0x10


/* Per-device (per-bank) structure */
//typedef struct bt656_in_dev_s {
    /* ... */
//    struct cdev         cdev;             /* The cdev structure */
    //wait_queue_head_t   wait_queue;            /* wait queues */
//}am656in_dev_t;

//static struct am656in_dev_t am656in_dev_;
typedef struct {
    unsigned        pbufAddr;
    unsigned        pbufSize;

    unsigned        pin_mux_reg1;   //for bt656 input
    unsigned        pin_mux_mask1;

    unsigned        pin_mux_reg2;   //for bt601 input
    unsigned        pin_mux_mask2;

    unsigned        pin_mux_reg3;   //for camera input
    unsigned        pin_mux_mask3;

    unsigned        active_pixel;
    unsigned        active_line;

//below macro defined is from tvin_global.h, they maybe not exact.
//input_mode is TVIN_SIG_FMT_NULL: disable 656in/601/camera decode;
//input_mode is TVIN_SIG_FMT_COMPONENT_576I_50D000 or TVIN_SIG_FMT_COMPONENT_576I_50D000, NTSC or PAL input(interlace mode): CLOCK + D0~D7(with SAV + EAV )
//              TVIN_SIG_FMT_COMPONENT_576I_50D000:     656--PAL ;
//              TVIN_SIG_FMT_COMPONENT_480I_59D940:     656--NTSC   ccir656 input
//input_mode is TVIN_SIG_FMT_HDMI_1440x576I_50Hz or TVIN_SIG_FMT_HDMI_1440x480I_60Hz,  NTSC or PAL input(interlace mode): CLOCK + D0~D7 + HSYNC + VSYNC + FID
//              TVIN_SIG_FMT_HDMI_1440x576I_50Hz:       601--PAL ;
//              TVIN_SIG_FMT_HDMI_1440x480I_60Hz:6      601--NTSC   ccir656 input
//input_mode is others,    CAMERA input(progressive mode): CLOCK + D0~D7 + HREF + VSYNC
//              TVIN_SIG_FMT_VGA_640X480P_60D000:640x480 camera inout(progressive)
//              TVIN_SIG_FMT_VGA_800X600P_60D317:800x600 camera inout(progressive)
//              TVIN_SIG_FMT_VGA_1024X768P_60D004:1024x768 camera inout(progressive)
//              .....
    tvin_sig_format_t   input_mode;

    unsigned char       rd_canvas_index;
    unsigned char       wr_canvas_index;
    unsigned char       buff_flag[BT656IN_VF_POOL_SIZE];


}am656in_t;

am656in_t am656in_dec_info = {
        .pbufAddr = 0x81000000,
        .pbufSize = 0x1000000,
        .pin_mux_reg1 = 0,      //for bt656 input
        .pin_mux_mask1 = 0,
        .pin_mux_reg2 = 0,      //for bt656 input
        .pin_mux_mask2 = 0,
        .pin_mux_reg3 = 0,      //for camera input
        .pin_mux_mask3 = 0,
        .active_pixel = 720,
        .active_line = 288,
        .input_mode = TVIN_SIG_FMT_NULL,     //disable 656in/601/camera decode;
        .rd_canvas_index = 0xff - (BT656IN_VF_POOL_SIZE + 2),
        .wr_canvas_index =  BT656IN_VF_POOL_SIZE,
        .buff_flag = {  VIDTYPE_INTERLACE_BOTTOM,
                        VIDTYPE_INTERLACE_TOP,
                        VIDTYPE_INTERLACE_BOTTOM,
                        VIDTYPE_INTERLACE_TOP,
                        VIDTYPE_INTERLACE_BOTTOM,
                        VIDTYPE_INTERLACE_TOP,
                        VIDTYPE_INTERLACE_BOTTOM,
                        VIDTYPE_INTERLACE_TOP},
    };


#ifdef HANDLE_BT656IN_IRQ
static const char bt656in_dec_id[] = "bt656in-dev";
#endif

//static dev_t am656in_id;
//static struct class *am656in_class;
//static struct device *am656in_dev;


//below macro defined is from tvin_global.h, they maybe not exact.
//input_mode is TVIN_SIG_FMT_NULL: disable 656in/601/camera decode;
//input_mode is TVIN_SIG_FMT_COMPONENT_576I_50D000 or TVIN_SIG_FMT_COMPONENT_576I_50D000, NTSC or PAL input(interlace mode): CLOCK + D0~D7(with SAV + EAV )
//              TVIN_SIG_FMT_COMPONENT_576I_50D000:     656--PAL ;
//              TVIN_SIG_FMT_COMPONENT_480I_59D940:     656--NTSC   ccir656 input
//input_mode is TVIN_SIG_FMT_HDMI_1440x576I_50Hz or TVIN_SIG_FMT_HDMI_1440x480I_60Hz,  NTSC or PAL input(interlace mode): CLOCK + D0~D7 + HSYNC + VSYNC + FID
//              TVIN_SIG_FMT_HDMI_1440x576I_50Hz:       601--PAL ;
//              TVIN_SIG_FMT_HDMI_1440x480I_60Hz:6      601--NTSC   ccir656 input
//input_mode is others,    CAMERA input(progressive mode): CLOCK + D0~D7 + HREF + VSYNC
//              TVIN_SIG_FMT_VGA_640X480P_60D000:640x480 camera inout(progressive)
//              TVIN_SIG_FMT_VGA_800X600P_60D317:800x600 camera inout(progressive)
//              TVIN_SIG_FMT_VGA_1024X768P_60D004:1024x768 camera inout(progressive)
//              .....
static void init_656in_dec_parameter(tvin_sig_format_t input_mode)
{
    am656in_dec_info.input_mode = input_mode;
    switch(input_mode)
    {
        case TVIN_SIG_FMT_COMPONENT_576I_50D000:     //656--PAL(interlace)
            am656in_dec_info.active_pixel = 720;
            am656in_dec_info.active_line = 288;
            pr_dbg("PAL input mode is selected for bt656, \n");
            break;

        case TVIN_SIG_FMT_COMPONENT_480I_59D940:     //656--NTSC(interlace)
            am656in_dec_info.active_pixel = 720;
            am656in_dec_info.active_line = 240;
            pr_dbg("NTSC input mode is selected for bt656, \n");
            break;

        case TVIN_SIG_FMT_HDMI_1440x576I_50Hz:     //601--PAL(interlace)
            am656in_dec_info.active_pixel = 720;
            am656in_dec_info.active_line = 288;
            pr_dbg("PAL input mode is selected for bt601, \n");
            break;

        case TVIN_SIG_FMT_HDMI_1440x480I_60Hz:     //601--NTSC(interlace)
            am656in_dec_info.active_pixel = 720;
            am656in_dec_info.active_line = 240;
            pr_dbg("NTSC input mode is selected for bt601, \n");
            break;

        case TVIN_SIG_FMT_VGA_640X480P_60D000:     //640x480 camera inout(progressive)
            am656in_dec_info.active_pixel = 640;
            am656in_dec_info.active_line = 480;
            pr_dbg("640x480 input mode is selected for camera, \n");
            break;

        case TVIN_SIG_FMT_VGA_800X600P_60D317:     //800x600 camera inout(progressive)
            am656in_dec_info.active_pixel = 800;
            am656in_dec_info.active_line = 600;
            pr_dbg("800x600 input mode is selected for camera, \n");
            break;

        case TVIN_SIG_FMT_VGA_1024X768P_60D004:     //1024x768 camera inout(progressive)
            am656in_dec_info.active_pixel = 1024;
            am656in_dec_info.active_line = 768;
            pr_dbg("1024x768 input mode is selected for camera, \n");
            break;

        case TVIN_SIG_FMT_NULL:
            pr_dbg("bt656_601 input decode is not start, do nothing \n");
            break;

        default:
            pr_dbg("bt656_601 input mode is not defined, do nothing \n");
            break;

    }

    return;
}


static inline u32 bt656_index2canvas(u32 index)
{
    const u32 canvas_tab[BT656IN_VF_POOL_SIZE] = {
            VDIN_START_CANVAS,
            VDIN_START_CANVAS + 1,
            VDIN_START_CANVAS + 2,
            VDIN_START_CANVAS + 3,
            VDIN_START_CANVAS + 4,
            VDIN_START_CANVAS + 5,
            VDIN_START_CANVAS + 6,
            VDIN_START_CANVAS + 7,
    };
    if(index < BT656IN_VF_POOL_SIZE)
        return canvas_tab[index];
    else
        return 0xff;
}

static inline u32 camera_index2canvas(u32 index)
{
    const u32 canvas_tab[CAMERA_IN_VF_POOL_SIZE] = {
            VDIN_START_CANVAS,
            VDIN_START_CANVAS + 1,
            VDIN_START_CANVAS + 2,
            VDIN_START_CANVAS + 3,
            VDIN_START_CANVAS + 4,
            VDIN_START_CANVAS + 5,
            VDIN_START_CANVAS + 6,
            VDIN_START_CANVAS + 7,
    };
    if(index < CAMERA_IN_VF_POOL_SIZE)
        return canvas_tab[index];
    else
        return 0xff;
}


//NTSC or PAL input(interlace mode): CLOCK + D0~D7(with SAV + EAV )
static void reset_bt656in_dec(void)
{
		unsigned temp_data;
		// reset BT656in module.
		temp_data = READ_CBUS_REG(BT_CTRL);
		temp_data |= ( 1 << BT_SOFT_RESET );
		WRITE_CBUS_REG(BT_CTRL, temp_data);

		temp_data = READ_CBUS_REG(BT_CTRL);
		temp_data &= ~( 1 << BT_SOFT_RESET );
		WRITE_CBUS_REG(BT_CTRL, temp_data);

        WRITE_CBUS_REG(BT_FIELDSADR, (4 << 16) | 4);	// field 0/1 start lcnt: default value
// configuration the BT PORT control
// For standaREAD_CBUS_REG bt656 in stream, there's no HSYNC VSYNC pins.
// So we don't need to configure the port.
		WRITE_CBUS_REG(BT_PORT_CTRL, 1 << BT_D8B);	// data itself is 8 bits.

		WRITE_CBUS_REG(BT_SWAP_CTRL,	( 4 << 0 ) |        //POS_Y1_IN
						( 5 << 4 ) |        //POS_Cr0_IN
 						( 6 << 8 ) |        //POS_Y0_IN
						( 7 << 12 ));       //POS_CB0_IN

        WRITE_CBUS_REG(BT_LINECTRL , 0)  ;
// ANCI is the field blanking data, like close caption. If it connected to digital camara interface, the jpeg bitstream also use this ANCI FIFO.
		WRITE_CBUS_REG(BT_ANCISADR, am656in_dec_info.pbufAddr);
		WRITE_CBUS_REG(BT_ANCIEADR, am656in_dec_info.pbufAddr + BT656IN_ANCI_DATA_SIZE);

		WRITE_CBUS_REG(BT_AFIFO_CTRL,	(1 <<31) |     // load start and end address to afifo.
						(1 << 6) |     // fill _en;
						(1 << 3)) ;     // urgent


    		WRITE_CBUS_REG(BT_INT_CTRL ,   // (1 << 5) |    //ancififo done int.
//						(1 << 4) |    //SOF interrupt enable.
//						(1 << 3) |      //EOF interrupt enable.
						(1 << 1)); // |      //input overflow interrupt enable.
//						(1 << 0));      //bt656 controller error interrupt enable.

		WRITE_CBUS_REG(BT_ERR_CNT, (626 << 16) | (1760));

		if(am656in_dec_info.input_mode  == TVIN_SIG_FMT_COMPONENT_576I_50D000 ) //input is PAL
		{
				WRITE_CBUS_REG(BT_VBIEND, 	22 | (22 << 16));		//field 0/1 VBI last line number
				WRITE_CBUS_REG(BT_VIDEOSTART, 	23 | (23 << 16));	//Line number of the first video start line in field 0/1.
				WRITE_CBUS_REG(BT_VIDEOEND , 	312 |          //  Line number of the last video line in field 1. added video end for avoid overflow.
								(312 <<16));					// Line number of the last video line in field 0
				WRITE_CBUS_REG(BT_CTRL ,	(0 << BT_UPDATE_ST_SEL) |  //Update bt656 status register when end of frame.
								(1 << BT_COLOR_REPEAT) | //Repeated the color data when do 4:2:2 -> 4:4:4 data transfer.
								(1 << BT_AUTO_FMT ) |			//use haREAD_CBUS_REGware to check the PAL/NTSC format input format if it's standaREAD_CBUS_REG BT656 input format.
								(1 << BT_MODE_BIT     ) | // BT656 standaREAD_CBUS_REG interface.
								(1 << BT_EN_BIT       ) |    // enable BT moduale.
								(1 << BT_REF_MODE_BIT ) |    // timing reference is from bit stream.
								(1 << BT_CLK27_SEL_BIT) |    // use external xclk27.
								(1 << BT_XCLK27_EN_BIT)) ;    // xclk27 is input.
                WRITE_CBUS_REG(VDIN_WR_V_START_END, 287 |     //v end
                                        (0 << 16) );   // v start

		}
		else if(am656in_dec_info.input_mode  == TVIN_SIG_FMT_COMPONENT_480I_59D940) //input is PAL	//input is NTSC
		{
				WRITE_CBUS_REG(BT_VBIEND, 	21 | (21 << 16));		//field 0/1 VBI last line number
				WRITE_CBUS_REG(BT_VIDEOSTART, 	18 | (18 << 16));	//Line number of the first video start line in field 0/1.
				WRITE_CBUS_REG(BT_VIDEOEND , 	257 |          //  Line number of the last video line in field 1. added video end for avoid overflow.
								(257 <<16));					// Line number of the last video line in field 0
				WRITE_CBUS_REG(BT_CTRL ,	(0 << BT_UPDATE_ST_SEL) |  //Update bt656 status register when end of frame.
								(1 << BT_COLOR_REPEAT) | //Repeated the color data when do 4:2:2 -> 4:4:4 data transfer.
								(1 << BT_AUTO_FMT ) |		//use haREAD_CBUS_REGware to check the PAL/NTSC format input format if it's standaREAD_CBUS_REG BT656 input format.
								(1 << BT_MODE_BIT     ) | // BT656 standaREAD_CBUS_REG interface.
								(1 << BT_EN_BIT       ) |    // enable BT moduale.
								(1 << BT_REF_MODE_BIT ) |    // timing reference is from bit stream.
								(1 << BT_CLK27_SEL_BIT) |    // use external xclk27.
								(1 << BT_XCLK27_EN_BIT) |		// xclk27 is input.
								(1 << BT_FMT_MODE_BIT));   //input format is NTSC
                WRITE_CBUS_REG(VDIN_WR_V_START_END, 239 |     //v end
                                        (0 << 16) );   // v start

		}
        else
           pr_dbg("bt656 input mode is invalid, do nothing \n");

		return;
}

//NTSC or PAL input(interlace mode): CLOCK + D0~D7 + HSYNC + VSYNC + FID
static void reset_bt601in_dec(void)
{
		unsigned temp_data;
		// reset BT656in module.
		temp_data = READ_CBUS_REG(BT_CTRL);
		temp_data |= ( 1 << BT_SOFT_RESET );
		WRITE_CBUS_REG(BT_CTRL, temp_data);

		temp_data = READ_CBUS_REG(BT_CTRL);
		temp_data &= ~( 1 << BT_SOFT_RESET );
		WRITE_CBUS_REG(BT_CTRL, temp_data);

        WRITE_CBUS_REG(BT_PORT_CTRL,    (0 << BT_IDQ_EN )   |     // use external idq pin.
                                                (1 << BT_IDQ_PHASE )   |
                                                ( 1 << BT_FID_HSVS ) |         // FID came from HS VS.
                                                ( 1 << BT_HSYNC_PHASE ) |
                                                (1 << BT_D8B )     |
                                                (4 << BT_FID_DELAY ) |
                                                (5 << BT_VSYNC_DELAY) |
                                                (5 << BT_HSYNC_DELAY));

        WRITE_CBUS_REG(BT_601_CTRL2 , ( 10 << 16));     // FID field check done point.

		WRITE_CBUS_REG(BT_SWAP_CTRL,	( 4 << 0 ) | // suppose the input bitstream format is Cb0 Y0 Cr0 Y1.
							( 5 << 4 ) |
							( 6 << 8 ) |
							( 7 << 13 ) );

        WRITE_CBUS_REG(BT_LINECTRL , ( 1 << 31 ) |   //software line ctrl enable.
                                    (1644 << 16 ) |    //1440 + 204
                                    220 )  ;

        // ANCI is the field blanking data, like close caption. If it connected to digital camara interface, the jpeg bitstream also use this ANCI FIFO.
        WRITE_CBUS_REG(BT_ANCISADR, am656in_dec_info.pbufAddr);
        WRITE_CBUS_REG(BT_ANCIEADR, am656in_dec_info.pbufAddr + BT656IN_ANCI_DATA_SIZE);

		WRITE_CBUS_REG(BT_AFIFO_CTRL,	(1 <<31) |     // load start and end address to afifo.
                        				(1 << 6) |     // fill _en;
                        				(1 << 3)) ;     // urgent

    	WRITE_CBUS_REG(BT_INT_CTRL ,   // (1 << 5) |    //ancififo done int.
//						(1 << 4) |    //SOF interrupt enable.
//						(1 << 3) |      //EOF interrupt enable.
						(1 << 1)); // |      //input overflow interrupt enable.
//						(1 << 0));      //bt656 controller error interrupt enable.
        WRITE_CBUS_REG(BT_ERR_CNT, (626 << 16) | (2000));
                                                                    //otherwise there is always error flag,
                                                                    //because the camera input use HREF ont HSYNC,
                                                                    //there are some lines without HREF sometime
		WRITE_CBUS_REG(BT_FIELDSADR, (1 << 16) | 1);	// field 0/1 start lcnt

		if(am656in_dec_info.input_mode == TVIN_SIG_FMT_HDMI_1440x576I_50Hz) //input is PAL
		{
				WRITE_CBUS_REG(BT_VBIEND, 22 | (22 << 16));		//field 0/1 VBI last line number
				WRITE_CBUS_REG(BT_VIDEOSTART, 23 | (23 << 16));	//Line number of the first video start line in field 0/1.
				WRITE_CBUS_REG(BT_VIDEOEND , 312 |          //  Line number of the last video line in field 1. added video end for avoid overflow.
                     							(312 <<16));					// Line number of the last video line in field 0
				WRITE_CBUS_REG(BT_CTRL ,	(0 << BT_MODE_BIT     ) |    // BT656 standaREAD_CBUS_REG interface.
                                            (1 << BT_AUTO_FMT )     |
                                            (1 << BT_EN_BIT       ) |    // enable BT moduale.
                                            (0 << BT_REF_MODE_BIT ) |    // timing reference is from bit stream.
                                            (0 << BT_FMT_MODE_BIT ) |     //PAL
                                            (1 << BT_SLICE_MODE_BIT )|    // no ancillay flag.
                                            (0 << BT_FID_EN_BIT )   |     // use external fid pin.
                                            (1 << BT_CLK27_SEL_BIT) |  // use external xclk27.
                                            (1 << BT_XCLK27_EN_BIT) );   // xclk27 is input.
                WRITE_CBUS_REG(VDIN_WR_V_START_END, 287 |     //v end
                                        (0 << 16) );   // v start

     }

		else if(am656in_dec_info.input_mode == TVIN_SIG_FMT_HDMI_1440x480I_60Hz) 	//input is NTSC
		{
				WRITE_CBUS_REG(BT_VBIEND, 21 | (21 << 16));		//field 0/1 VBI last line number
				WRITE_CBUS_REG(BT_VIDEOSTART, 18 | (18 << 16));	//Line number of the first video start line in field 0/1.
				WRITE_CBUS_REG(BT_VIDEOEND , 257 |          //  Line number of the last video line in field 1. added video end for avoid overflow.
                     					(257 <<16));		// Line number of the last video line in field 0
				WRITE_CBUS_REG(BT_CTRL ,(0 << BT_MODE_BIT     ) |    // BT656 standaREAD_CBUS_REG interface.
                                        (1 << BT_AUTO_FMT )     |
                                        (1 << BT_EN_BIT       ) |    // enable BT moduale.
                                        (0 << BT_REF_MODE_BIT ) |    // timing reference is from bit stream.
                                        (1 << BT_FMT_MODE_BIT ) |     // NTSC
                                        (1 << BT_SLICE_MODE_BIT )|    // no ancillay flag.
                                        (0 << BT_FID_EN_BIT )   |     // use external fid pin.
                                        (1 << BT_CLK27_SEL_BIT) |  // use external xclk27.
                                        (1 << BT_XCLK27_EN_BIT) );   // xclk27 is input.
                WRITE_CBUS_REG(VDIN_WR_V_START_END, 239 |     //v end
                                        (0 << 16) );   // v start

      }
      else
      {
            pr_dbg("bt601 input mode is invalid, do nothing \n");
      }
		return;
}

//CAMERA input(progressive mode): CLOCK + D0~D7 + HREF + VSYNC
static void reset_camera_dec(void)
{
		unsigned temp_data;
		// reset BT656in module.
		temp_data = READ_CBUS_REG(BT_CTRL);
		temp_data |= ( 1 << BT_SOFT_RESET );
		WRITE_CBUS_REG(BT_CTRL, temp_data);

		temp_data = READ_CBUS_REG(BT_CTRL);
		temp_data &= ~( 1 << BT_SOFT_RESET );
		WRITE_CBUS_REG(BT_CTRL, temp_data);


        WRITE_CBUS_REG(BT_PORT_CTRL,    (0 << BT_IDQ_EN )   |     // use external idq pin.
                                        (0 << BT_IDQ_PHASE )   |
                                        ( 0 << BT_FID_HSVS ) |         // FID came from HS VS.
                                        ( 1 << BT_VSYNC_PHASE ) |
                                        (0 << BT_D8B )     |
                                        (4 << BT_FID_DELAY ) |
                                        (0 << BT_VSYNC_DELAY) |
                                        (0 << BT_HSYNC_DELAY));

        WRITE_CBUS_REG(BT_601_CTRL2 , ( 10 << 16));     // FID field check done point.

		WRITE_CBUS_REG(BT_SWAP_CTRL,	( 7 << 0 ) | // suppose the input bitstream format is Cb0 Y0 Cr0 Y1.
							( 6 << 4 ) |
							( 5 << 8 ) |
							( 4 << 12 ) );

        WRITE_CBUS_REG(BT_LINECTRL , ( 1 << 31 ) |   //software line ctrl enable.
                                    ((am656in_dec_info.active_pixel << 1)<< 16 ) |    //the number of active data per line
                                    0 )  ;

        // ANCI is the field blanking data, like close caption. If it connected to digital camara interface, the jpeg bitstream also use this ANCI FIFO.
        WRITE_CBUS_REG(BT_ANCISADR, am656in_dec_info.pbufAddr);
        WRITE_CBUS_REG(BT_ANCIEADR, am656in_dec_info.pbufAddr + BT656IN_ANCI_DATA_SIZE);

		WRITE_CBUS_REG(BT_AFIFO_CTRL,	(1 <<31) |     // load start and end address to afifo.
                        				(1 << 6) |     // fill _en;
                        				(1 << 3)) ;     // urgent

    	WRITE_CBUS_REG(BT_INT_CTRL ,   // (1 << 5) |    //ancififo done int.
//						(1 << 4) |    //SOF interrupt enable.
//						(1 << 3) |      //EOF interrupt enable.
						(1 << 1)); // |      //input overflow interrupt enable.
//						(1 << 0));      //bt656 controller error interrupt enable.
        WRITE_CBUS_REG(BT_ERR_CNT, ((2000) << 16) | (2000 * 10));   //total lines per frame and total pixel per line
                                                                    //otherwise there is always error flag,
                                                                    //because the camera input use HREF ont HSYNC,
                                                                    //there are some lines without HREF sometime

        WRITE_CBUS_REG(BT_FIELDSADR, (1 << 16) | 1);	// field 0/1 start lcnt
        WRITE_CBUS_REG(BT_VBIEND, 1 | (1 << 16));		//field 0/1 VBI last line number
        WRITE_CBUS_REG(BT_VIDEOSTART, 1 | (1 << 16));	//Line number of the first video start line in field 0/1.
        WRITE_CBUS_REG(BT_VIDEOEND , am656in_dec_info.active_line|          //  Line number of the last video line in field 1. added video end for avoid overflow.
                                    (am656in_dec_info.active_line <<16));					// Line number of the last video line in field 0
        WRITE_CBUS_REG(BT_CTRL ,	(1 << BT_EN_BIT       ) |    // enable BT moduale.
				                (0 << BT_REF_MODE_BIT ) |    // timing reference is from bit stream.
				                (0 << BT_FMT_MODE_BIT ) |     //PAL
				                (1 << BT_SLICE_MODE_BIT )|    // no ancillay flag.
                                (0 << BT_MODE_BIT     ) |    // BT656 standard interface.
                                (1 << BT_CLK27_SEL_BIT) |  // use external xclk27.
                                (0 << BT_FID_EN_BIT )   |     // use external fid pin.
                                (1 << BT_XCLK27_EN_BIT) |   // xclk27 is input.
                                (1 << BT_PROG_MODE  )   |
                                (0 << BT_AUTO_FMT    ) );

		return;
}


void set_next_field_656_601_camera_in_anci_address(unsigned char index)
{
		unsigned pbufAddr;
		pbufAddr = am656in_dec_info.pbufAddr + index * 0x200;
//		//set next field ANCI.
    WRITE_CBUS_REG(BT_ANCISADR, pbufAddr);

    WRITE_CBUS_REG(BT_AFIFO_CTRL,   (1 <<31) |     // load start and end address to afifo.
                                    (1 << 6) |     // fill _en;
                                    (1 << 3)) ;     // urgent



}

//below macro defined is from tvin_global.h, they maybe not exact.
//input_mode is TVIN_SIG_FMT_NULL: disable 656in/601/camera decode;
//input_mode is TVIN_SIG_FMT_COMPONENT_576I_50D000 or TVIN_SIG_FMT_COMPONENT_576I_50D000, NTSC or PAL input(interlace mode): CLOCK + D0~D7(with SAV + EAV )
//              TVIN_SIG_FMT_COMPONENT_576I_50D000:     656--PAL ;
//              TVIN_SIG_FMT_COMPONENT_480I_59D940:     656--NTSC   ccir656 input
//input_mode is TVIN_SIG_FMT_HDMI_1440x576I_50Hz or TVIN_SIG_FMT_HDMI_1440x480I_60Hz,  NTSC or PAL input(interlace mode): CLOCK + D0~D7 + HSYNC + VSYNC + FID
//              TVIN_SIG_FMT_HDMI_1440x576I_50Hz:       601--PAL ;
//              TVIN_SIG_FMT_HDMI_1440x480I_60Hz:6      601--NTSC   ccir656 input
//input_mode is others,    CAMERA input(progressive mode): CLOCK + D0~D7 + HREF + VSYNC
//              TVIN_SIG_FMT_VGA_640X480P_60D000:640x480 camera inout(progressive)
//              TVIN_SIG_FMT_VGA_800X600P_60D317:800x600 camera inout(progressive)
//              TVIN_SIG_FMT_VGA_1024X768P_60D004:1024x768 camera inout(progressive)
//              .....
void start_amvdec_656_601_camera_in(tvin_sig_format_t input_mode)
{
    if((am656in_dec_info.input_mode == TVIN_SIG_FMT_NULL) && (input_mode != TVIN_SIG_FMT_NULL))
    {
        pr_dbg("start ");
        init_656in_dec_parameter(input_mode);

        if((input_mode == TVIN_SIG_FMT_COMPONENT_576I_50D000) || (input_mode == TVIN_SIG_FMT_COMPONENT_480I_59D940))  //NTSC or PAL input(interlace mode): D0~D7(with SAV + EAV )
        {
            pr_dbg("bt656in decode. \n");
            reset_bt656in_dec();
            am656in_dec_info.rd_canvas_index = 0xff - (BT656IN_VF_POOL_SIZE + 2);
            am656in_dec_info.wr_canvas_index =  0;
        }
        else if((input_mode == TVIN_SIG_FMT_HDMI_1440x576I_50Hz) || (input_mode == TVIN_SIG_FMT_HDMI_1440x480I_60Hz))
        {
            pr_dbg("bt601in decode. \n");
            reset_bt601in_dec();
            am656in_dec_info.rd_canvas_index = 0xff - (BT656IN_VF_POOL_SIZE + 2);
            am656in_dec_info.wr_canvas_index =  0;
            if(am656in_dec_info.pin_mux_reg2 != 0)
                SET_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg2, am656in_dec_info.pin_mux_mask2);  //set the related pin mux
            if(am656in_dec_info.pin_mux_reg3 != 0)
               SET_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg3, am656in_dec_info.pin_mux_mask3);  //set the related pin mux

        }
        else
        {
            pr_dbg("camera in decode. \n");
             am656in_dec_info.rd_canvas_index = 0xff - (CAMERA_IN_VF_POOL_SIZE + 2);
            am656in_dec_info.wr_canvas_index =  0;
            reset_camera_dec();
             if(am656in_dec_info.pin_mux_reg2 != 0)
                SET_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg2, am656in_dec_info.pin_mux_mask2);  //set the related pin mux
        }
        if(am656in_dec_info.pin_mux_reg1 != 0)
            SET_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg1, am656in_dec_info.pin_mux_mask1);  //set the related pin mux



        CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, 0x3e07fe);
        CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_7, 0x000fc000);
        CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_8, 0xffc00000);
        CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_10, 0xe0000000);
        CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_12, 0xfff80000);


    }
    return;
}


//below macro defined is from tvin_global.h, they maybe not exact.
//input_mode is TVIN_SIG_FMT_NULL: disable 656in/601/camera decode;
//input_mode is TVIN_SIG_FMT_COMPONENT_576I_50D000 or TVIN_SIG_FMT_COMPONENT_576I_50D000, NTSC or PAL input(interlace mode): CLOCK + D0~D7(with SAV + EAV )
//              TVIN_SIG_FMT_COMPONENT_576I_50D000:     656--PAL ;
//              TVIN_SIG_FMT_COMPONENT_480I_59D940:     656--NTSC   ccir656 input
//input_mode is TVIN_SIG_FMT_HDMI_1440x576I_50Hz or TVIN_SIG_FMT_HDMI_1440x480I_60Hz,  NTSC or PAL input(interlace mode): CLOCK + D0~D7 + HSYNC + VSYNC + FID
//              TVIN_SIG_FMT_HDMI_1440x576I_50Hz:       601--PAL ;
//              TVIN_SIG_FMT_HDMI_1440x480I_60Hz:6      601--NTSC   ccir656 input
//input_mode is others,    CAMERA input(progressive mode): CLOCK + D0~D7 + HREF + VSYNC
//              TVIN_SIG_FMT_VGA_640X480P_60D000:640x480 camera inout(progressive)
//              TVIN_SIG_FMT_VGA_800X600P_60D317:800x600 camera inout(progressive)
//              TVIN_SIG_FMT_VGA_1024X768P_60D004:1024x768 camera inout(progressive)
//              .....
void stop_amvdec_656_601_camera_in(tvin_sig_format_t input_mode)
{
    unsigned temp_data;
		// reset BT656in module.
    if(am656in_dec_info.input_mode != TVIN_SIG_FMT_NULL)
    {
        pr_dbg("stop 656_601_camera_in decode. \n");
        if(am656in_dec_info.pin_mux_reg1 != 0)
            CLEAR_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg1, am656in_dec_info.pin_mux_mask1);

        if(am656in_dec_info.pin_mux_reg2 != 0)
            CLEAR_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg2, am656in_dec_info.pin_mux_mask2);

        if(am656in_dec_info.pin_mux_reg3 != 0)
            CLEAR_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg3, am656in_dec_info.pin_mux_mask3);  //set the related pin mux

        temp_data = READ_CBUS_REG(BT_CTRL);
        temp_data &= ~( 1 << BT_EN_BIT );
        WRITE_CBUS_REG(BT_CTRL, temp_data);	//disable BT656 input

        //reset 656 input module
        temp_data = READ_CBUS_REG(BT_CTRL);
        temp_data |= ( 1 << BT_SOFT_RESET );
        WRITE_CBUS_REG(BT_CTRL, temp_data);
        temp_data = READ_CBUS_REG(BT_CTRL);
        temp_data &= ~( 1 << BT_SOFT_RESET );
        WRITE_CBUS_REG(BT_CTRL, temp_data);

        am656in_dec_info.input_mode = TVIN_SIG_FMT_NULL;
    }

    return;
}



#ifdef HANDLE_BT656IN_IRQ
static irqreturn_t bt656in_dec_irq(int irq, void *dev_id)
{

}
#endif

//bt656 flag error = (pixel counter error) | (line counter error) | (input FIFO over flow)
static void bt656_in_dec_run(vframe_t * info)
{
    unsigned ccir656_status, field_total_line;
    unsigned char last_receiver_buff_index, canvas_id;

    ccir656_status = READ_CBUS_REG(BT_STATUS);

    if(ccir656_status & 0x80)
    {
        pr_dbg("bt656 input FIFO over flow, reset \n");
        reset_bt656in_dec();
        return ;
    }

    field_total_line = READ_CBUS_REG(BT_VLINE_STATUS);
    field_total_line &= 0xfff;
    if((field_total_line < 200) || (field_total_line > 320))  //skip current field
    {
        pr_dbg("656 in total line is less than 200 or more than 320, \n");
        reset_bt656in_dec();
        return;
    }

    else if((field_total_line < 264) && (ccir656_status & 0x4000) && (am656in_dec_info.input_mode != TVIN_SIG_FMT_COMPONENT_480I_59D940)) //current field total line number is 240
    {
        init_656in_dec_parameter(TVIN_SIG_FMT_COMPONENT_480I_59D940);
        reset_bt656in_dec();
        return;
    }

    else if( (field_total_line >= 264) && (!(ccir656_status & 0x4000)) && (am656in_dec_info.input_mode != TVIN_SIG_FMT_COMPONENT_576I_50D000) )  //current field total line number is 288(0x100 + 0x20)
    {
        init_656in_dec_parameter(TVIN_SIG_FMT_COMPONENT_576I_50D000);
        reset_bt656in_dec();
        return;
    }

    else
    {
        if(am656in_dec_info.wr_canvas_index == 0)
              last_receiver_buff_index = BT656IN_VF_POOL_SIZE - 1;
        else
              last_receiver_buff_index = am656in_dec_info.wr_canvas_index - 1;

        if(ccir656_status & 0x1000)  // previous field is field 1.
        {
            am656in_dec_info.buff_flag[am656in_dec_info.wr_canvas_index] = VIDTYPE_INTERLACE_BOTTOM;
            //make sure that the data type received ..../top/bottom/top/buttom/top/buttom/....
            if((am656in_dec_info.buff_flag[last_receiver_buff_index] & 0x0f) == VIDTYPE_INTERLACE_BOTTOM)  //pre field type is the same to cur field
            {
                return;
            }

        }
        else
        {
            am656in_dec_info.buff_flag[am656in_dec_info.wr_canvas_index] = VIDTYPE_INTERLACE_TOP;
            //make sure that the data type received ..../top/bottom/top/buttom/top/buttom/....
            if((am656in_dec_info.buff_flag[last_receiver_buff_index] & 0x0f) == VIDTYPE_INTERLACE_TOP)  //pre field type is the same to cur field
            {
                return;
            }

        }

        if(am656in_dec_info.input_mode == TVIN_SIG_FMT_COMPONENT_576I_50D000)     //PAL data
            am656in_dec_info.buff_flag[am656in_dec_info.wr_canvas_index] |= 0x10;

        if((am656in_dec_info.rd_canvas_index > 0xf0) && (am656in_dec_info.rd_canvas_index < 0xff))
         {
             am656in_dec_info.rd_canvas_index += 1;
             am656in_dec_info.wr_canvas_index += 1;
             if(am656in_dec_info.wr_canvas_index > (BT656IN_VF_POOL_SIZE -1) )
                 am656in_dec_info.wr_canvas_index = 0;
             canvas_id = bt656_index2canvas(am656in_dec_info.wr_canvas_index);
             WRITE_MPEG_REG_BITS(VDIN_WR_CTRL, canvas_id, WR_CANVAS_BIT, WR_CANVAS_WID);
             set_next_field_656_601_camera_in_anci_address(am656in_dec_info.wr_canvas_index);
             return;
           }

         else if(am656in_dec_info.rd_canvas_index == 0xff)
         {
            am656in_dec_info.rd_canvas_index = 0;
         }

        else
        {
            am656in_dec_info.rd_canvas_index++;
            if(am656in_dec_info.rd_canvas_index > (BT656IN_VF_POOL_SIZE -1))
            {
                am656in_dec_info.rd_canvas_index = 0;
            }
        }

        if((am656in_dec_info.buff_flag[am656in_dec_info.rd_canvas_index] & 0xf) == VIDTYPE_INTERLACE_BOTTOM)  // current field is field 1.
        {
            info->type = VIDTYPE_VIU_SINGLE_PLANE | VIDTYPE_VIU_422 | VIDTYPE_VIU_FIELD | VIDTYPE_INTERLACE_BOTTOM;
        }
        else
        {
            info->type = VIDTYPE_VIU_SINGLE_PLANE | VIDTYPE_VIU_422 | VIDTYPE_VIU_FIELD | VIDTYPE_INTERLACE_TOP;
        }
        info->canvas0Addr = info->canvas1Addr = bt656_index2canvas(am656in_dec_info.rd_canvas_index);

        if(am656in_dec_info.buff_flag[am656in_dec_info.rd_canvas_index] & 0x10)     //PAL
        {
            info->width= 720;
            info->height = 576;
            info->duration = 1920;
        }
        else
        {
            info->width= 720;
            info->height = 480;
            info->duration = 1600;
        }


        am656in_dec_info.wr_canvas_index += 1;
        if(am656in_dec_info.wr_canvas_index > (BT656IN_VF_POOL_SIZE -1) )
            am656in_dec_info.wr_canvas_index = 0;
        canvas_id = bt656_index2canvas(am656in_dec_info.wr_canvas_index);
        info->index = canvas_id;
        WRITE_MPEG_REG_BITS(VDIN_WR_CTRL, canvas_id, WR_CANVAS_BIT, WR_CANVAS_WID);
        set_next_field_656_601_camera_in_anci_address(am656in_dec_info.wr_canvas_index);
    }
    return;

}

static void bt601_in_dec_run(vframe_t * info)
{
    unsigned ccir656_status, field_total_line;

    ccir656_status = READ_CBUS_REG(BT_STATUS);

    if(ccir656_status & 0x80)
    {
        pr_dbg("bt601 input FIFO over flow, reset \n");
        reset_bt601in_dec();
        return ;
    }

    field_total_line = READ_CBUS_REG(BT_VLINE_STATUS);
    field_total_line &= 0xfff;
    if((field_total_line < 200) || (field_total_line > 320))  //skip current field
    {
        pr_dbg("601 in total line is less than 200 or more than 320 \n");
        reset_bt601in_dec();
        return;
    }

    else if((field_total_line < 264) && (ccir656_status & 0x4000) && (am656in_dec_info.input_mode != TVIN_SIG_FMT_HDMI_1440x480I_60Hz)) //current field total line number is 240
    {
        init_656in_dec_parameter(TVIN_SIG_FMT_HDMI_1440x480I_60Hz);
        reset_bt601in_dec();
        return;
    }

    else if( (field_total_line >= 264) && (!(ccir656_status & 0x4000)) && (am656in_dec_info.input_mode != TVIN_SIG_FMT_HDMI_1440x576I_50Hz) )  //current field total line number is 288(0x100 + 0x20)
    {
        init_656in_dec_parameter(TVIN_SIG_FMT_HDMI_1440x576I_50Hz);
        reset_bt601in_dec();
        return;
    }

    else
    {
        if(ccir656_status & 0x10)  // current field is field 1.
        {
            info->type = VIDTYPE_VIU_SINGLE_PLANE | VIDTYPE_VIU_422 | VIDTYPE_VIU_FIELD | VIDTYPE_INTERLACE_BOTTOM;
        }
        else
        {
            info->type = VIDTYPE_VIU_SINGLE_PLANE | VIDTYPE_VIU_422 | VIDTYPE_VIU_FIELD | VIDTYPE_INTERLACE_TOP;
        }

        if(am656in_dec_info.input_mode == TVIN_SIG_FMT_HDMI_1440x576I_50Hz)	//the data in the buffer is PAL
        {
            info->width= 720;
            info->height = 576;
            info->duration = 1920;
        }
        else
        {
            info->width= 720;
            info->height = 480;
            info->duration = 1600;
        }
    }

    return;

}

static void camera_in_dec_run(vframe_t * info)
{
    unsigned ccir656_status, field_total_line;
    unsigned char canvas_id;

    ccir656_status = READ_CBUS_REG(BT_STATUS);

    if(ccir656_status & 0x80)
    {
        pr_dbg("camera input FIFO over flow, reset \n");
        reset_camera_dec();
        return ;
    }

    field_total_line = READ_CBUS_REG(BT_VLINE_STATUS);
    field_total_line &= 0xfff;
    if(field_total_line < 200)   //skip current field
    {
        pr_dbg("601 in total line is less than 200 \n");
        reset_camera_dec();
        return;
    }

    else
    {
        if((am656in_dec_info.rd_canvas_index > 0xf0) && (am656in_dec_info.rd_canvas_index < 0xff))
         {
             am656in_dec_info.rd_canvas_index += 1;
             am656in_dec_info.wr_canvas_index += 1;
             if(am656in_dec_info.wr_canvas_index > (CAMERA_IN_VF_POOL_SIZE -1) )
                 am656in_dec_info.wr_canvas_index = 0;
             canvas_id = camera_index2canvas(am656in_dec_info.wr_canvas_index);
             WRITE_MPEG_REG_BITS(VDIN_WR_CTRL, canvas_id, WR_CANVAS_BIT, WR_CANVAS_WID);
             set_next_field_656_601_camera_in_anci_address(am656in_dec_info.wr_canvas_index);
             return;
        }

         else if(am656in_dec_info.rd_canvas_index == 0xff)
         {
            am656in_dec_info.rd_canvas_index = 0;
         }

        else
        {
            am656in_dec_info.rd_canvas_index++;
            if(am656in_dec_info.rd_canvas_index > (CAMERA_IN_VF_POOL_SIZE -1))
            {
                am656in_dec_info.rd_canvas_index = 0;
            }
        }


        info->type = VIDTYPE_VIU_SINGLE_PLANE | VIDTYPE_VIU_422 | VIDTYPE_VIU_FIELD | VIDTYPE_PROGRESSIVE ;
        info->width= am656in_dec_info.active_pixel;
        info->height = am656in_dec_info.active_line;
        info->duration = 9600000/1716;   //17.16 frame per second
        info->canvas0Addr = info->canvas1Addr = camera_index2canvas(am656in_dec_info.rd_canvas_index);
        am656in_dec_info.wr_canvas_index += 1;
        if(am656in_dec_info.wr_canvas_index > (CAMERA_IN_VF_POOL_SIZE -1) )
            am656in_dec_info.wr_canvas_index = 0;
        canvas_id = camera_index2canvas(am656in_dec_info.wr_canvas_index);
        info->index = canvas_id;
        WRITE_MPEG_REG_BITS(VDIN_WR_CTRL, canvas_id, WR_CANVAS_BIT, WR_CANVAS_WID);
        set_next_field_656_601_camera_in_anci_address(am656in_dec_info.wr_canvas_index);
    }

    return;
}

//below macro defined is from tvin_global.h, they maybe not exact.
//input_mode is TVIN_SIG_FMT_NULL: disable 656in/601/camera decode;
//input_mode is TVIN_SIG_FMT_COMPONENT_576I_50D000 or TVIN_SIG_FMT_COMPONENT_576I_50D000, NTSC or PAL input(interlace mode): CLOCK + D0~D7(with SAV + EAV )
//              TVIN_SIG_FMT_COMPONENT_576I_50D000:     656--PAL ;
//              TVIN_SIG_FMT_COMPONENT_480I_59D940:     656--NTSC   ccir656 input
//input_mode is TVIN_SIG_FMT_HDMI_1440x576I_50Hz or TVIN_SIG_FMT_HDMI_1440x480I_60Hz,  NTSC or PAL input(interlace mode): CLOCK + D0~D7 + HSYNC + VSYNC + FID
//              TVIN_SIG_FMT_HDMI_1440x576I_50Hz:       601--PAL ;
//              TVIN_SIG_FMT_HDMI_1440x480I_60Hz:6      601--NTSC   ccir656 input
//input_mode is others,    CAMERA input(progressive mode): CLOCK + D0~D7 + HREF + VSYNC
//              TVIN_SIG_FMT_VGA_640X480P_60D000:640x480 camera inout(progressive)
//              TVIN_SIG_FMT_VGA_800X600P_60D317:800x600 camera inout(progressive)
//              TVIN_SIG_FMT_VGA_1024X768P_60D004:1024x768 camera inout(progressive)
//              .....
int amvdec_656_601_camera_in_run(vframe_t *info)
{
    unsigned ccir656_status;


    if(am656in_dec_info.input_mode == TVIN_SIG_FMT_NULL){
        pr_error("bt656in decoder is not started\n");
        return -1;
    }

    ccir656_status = READ_CBUS_REG(BT_STATUS);
    WRITE_CBUS_REG(BT_STATUS, ccir656_status | (1 << 9));	//WRITE_CBUS_REGite 1 to clean the SOF interrupt bit

    if((am656in_dec_info.input_mode == TVIN_SIG_FMT_COMPONENT_576I_50D000) || (am656in_dec_info.input_mode == TVIN_SIG_FMT_COMPONENT_480I_59D940))  //NTSC or PAL input(interlace mode): D0~D7(with SAV + EAV )
    {
        bt656_in_dec_run(info);
    }
    else if((am656in_dec_info.input_mode == TVIN_SIG_FMT_HDMI_1440x576I_50Hz) || (am656in_dec_info.input_mode == TVIN_SIG_FMT_HDMI_1440x480I_60Hz))
    {
        bt601_in_dec_run(info);
    }
    else
    {
        camera_in_dec_run(info);
    }
    return 0;
}


//static int am656in_open(struct inode *node, struct file *file)
//{
//	 am656in_dev_t *bt656_in_devp;

    /* Get the per-device structure that contains this cdev */
//    bt656_in_devp = container_of(node->i_cdev, am656in_dev_t, cdev);
//    file->private_data = bt656_in_devp;

//	return 0;

//}


//static int am656in_release(struct inode *node, struct file *file)
//{
//    am656in_dev_t *bt656_in_devp = file->private_data;

    /* Reset file pointer */
//    bt656_in_devp->current_pointer = 0;

    /* Release some other fields */
    /* ... */
//    return 0;
//}



//static int am656in_ioctl(struct inode *node, struct file *file, unsigned int cmd,   unsigned long args)
//{
//	int   r = 0;
//	switch (cmd) {
//		case BT656_DECODERIO_START:
//                    start_bt656in_dec();
//			break;

//		case BT656_DECODERIO_STOP:
//                    stop_bt656in_dec();
//			break;

//            default:

//                    break;
//	}
//	return r;
//}

//const static struct file_operations am656in_fops = {
//    .owner    = THIS_MODULE,
//    .open     = am656in_open,
//    .release  = am656in_release,
//    .ioctl    = am656in_ioctl,
//};

static int amvdec_656in_probe(struct platform_device *pdev)
{
    int r = 0;
//    unsigned pbufSize;
    struct resource *s;

    printk("amvdec_656in probe start.\n");



//    r = alloc_chREAD_CBUS_REGev_region(&am656in_id, 0, BT656IN_COUNT, DEVICE_NAME);
//    if (r < 0) {
//        pr_error("Can't register major for am656indec device\n");
//        return r;
//    }

//    am656in_class = class_create(THIS_MODULE, DEVICE_NAME);
//    if (IS_ERR(am656in_class))
//    {
//        unregister_chREAD_CBUS_REGev_region(am656in_id, BT656IN_COUNT);
//        return PTR_ERR(aoe_class);
//    }

//    cdev_init(&am656in_dev_->cdev, &am656in_fops);
//    &am656in_dev_->cdev.owner = THIS_MODULE;
//    cdev_add(&am656in_dev_->cdev, am656in_id, BT656IN_COUNT);

//    am656in_dev = device_create(am656in_class, NULL, am656in_id, "bt656indec%d", 0);
//		if (am656in_dev == NULL) {
//				pr_error("device_create create error\n");
//				class_destroy(am656in_class);
//				r = -EEXIST;
//				return r;
//		}

    s = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if(s != NULL)
    {
	        am656in_dec_info.pbufAddr = (unsigned )(s->start);
	        am656in_dec_info.pbufSize = (unsigned )(s->end) - am656in_dec_info.pbufAddr + 1;
            pr_dbg(" am656in_dec_info memory start addr is %x, mem_size is %x . \n",
                        am656in_dec_info.pbufAddr, am656in_dec_info.pbufSize);

     }
     else
             pr_error("error in getting resource parameters0 \n");

    s = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    if(s != NULL)
    {
	        am656in_dec_info.pin_mux_mask1 = (unsigned )(s->start);
	        am656in_dec_info.pin_mux_reg1 = ((unsigned )(s->end) - am656in_dec_info.pin_mux_mask1 ) & 0xffff;
            pr_dbg(" bt656in pin_mux_reg1 is %x, pin_mux_mask1 is %x . \n",
                am656in_dec_info.pin_mux_reg1,am656in_dec_info.pin_mux_mask1);

//       if(am656in_dec_info.pin_mux_reg1 != 0)
//       	SET_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg1, am656in_dec_info.pin_mux_mask1);
     }
     else
             pr_error("error in getting bt656 resource parameters \n");

    s = platform_get_resource(pdev, IORESOURCE_MEM, 2);
    if(s != NULL)
    {
	        am656in_dec_info.pin_mux_mask2 = (unsigned )(s->start);
	        am656in_dec_info.pin_mux_reg2 = ((unsigned )(s->end) - am656in_dec_info.pin_mux_mask2 ) & 0xffff;
            pr_dbg(" bt656in pin_mux_reg2 is %x, pin_mux_mask2 is %x . \n",
                am656in_dec_info.pin_mux_reg2,am656in_dec_info.pin_mux_mask2);
//       if(am656in_dec_info.pin_mux_reg2 != 0)
//       	SET_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg2, am656in_dec_info.pin_mux_mask2);
     }
     else
             pr_error("error in getting bt601 resource parameters \n");

    s = platform_get_resource(pdev, IORESOURCE_MEM, 3);
    if(s != NULL)
    {
            am656in_dec_info.pin_mux_mask3 = (unsigned )(s->start);
            am656in_dec_info.pin_mux_reg3 = ((unsigned )(s->end) - am656in_dec_info.pin_mux_mask3 ) & 0xffff;
            pr_dbg(" bt656in pin_mux_reg3 is %x, pin_mux_mask3 is %x . \n",
                am656in_dec_info.pin_mux_reg3,am656in_dec_info.pin_mux_mask3);
//          if(am656in_dec_info.pin_mux_reg3 != 0)
//              SET_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg3, am656in_dec_info.pin_mux_mask3);
     }
     else
             pr_error("error in getting camera resource parameters \n");

#ifdef HANDLE_BT656IN_IRQ
        if (request_irq(INT_BT656, bt656in_dec_irq,	IRQF_SHARED, BT656IN_IRQ_NAME, (void *)bt656in_dec_id)
        {
                printk("bt656in irq register error.\n");
                return -ENOENT;
        }
#endif


		printk("amvdec_656in probe end.\n");
		return r;
}

static int amvdec_656in_remove(struct platform_device *pdev)
{
    /* Remove the cdev */
#ifdef HANDLE_BT656IN_IRQ
        free_irq(INT_BT656,(void *)bt656in_dec_id);
#endif

//    cdev_del(&am656in_dev_->cdev);

//    device_destroy(am656in_class, am656in_id);

//    class_destroy(am656in_class);

//    unregister_chREAD_CBUS_REGev_region(am656in_id, BT656IN_COUNT);

	  if(am656in_dec_info.pin_mux_reg1 != 0)
	     CLEAR_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg1, am656in_dec_info.pin_mux_mask1);

	  if(am656in_dec_info.pin_mux_reg2 != 0)
	     CLEAR_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg2, am656in_dec_info.pin_mux_mask2);

	  if(am656in_dec_info.pin_mux_reg3 != 0)
	     CLEAR_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg3, am656in_dec_info.pin_mux_mask3);
    return 0;
}



static struct platform_driver amvdec_656in_driver = {
    .probe      = amvdec_656in_probe,
    .remove     = amvdec_656in_remove,
    .driver     = {
        .name   = DEVICE_NAME,
    }
};

static int __init amvdec_656in_init_module(void)
{
    printk("amvdec_656in module init\n");
    if (platform_driver_register(&amvdec_656in_driver)) {
        printk("failed to register amvdec_656in driver\n");
        return -ENODEV;
    }

    return 0;
}

static void __exit amvdec_656in_exit_module(void)
{
    printk("amvdec_656in module remove.\n");
    platform_driver_unregister(&amvdec_656in_driver);
    return ;
}



module_init(amvdec_656in_init_module);
module_exit(amvdec_656in_exit_module);

MODULE_DESCRIPTION("AMLOGIC BT656_601 input driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");

