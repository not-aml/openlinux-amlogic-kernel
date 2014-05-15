/*
 * Amlogic OSD
 * frame buffer driver
 *
 * Copyright (C) 2009 Amlogic, Inc.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Author:	Tim Yao <timyao@amlogic.com>
 *
 */

#ifndef OSD_H
#define OSD_H
#include  <linux/fb.h>
//do not change this order

enum osd_type_s {
    OSD_TYPE_32_RGBA    = 0,  // 0
    OSD_TYPE_24_RGB     = 1,   // 0
    OSD_TYPE_02_PAL4    = 2,  // 0
    OSD_TYPE_04_PAL16   = 3, // 0
    OSD_TYPE_08_PAL256  = 4,// 0
    OSD_TYPE_16_655     = 5,  // 0
    OSD_TYPE_16_844     = 6,  // 1
    OSD_TYPE_16_6442    = 7, // 2 
    OSD_TYPE_16_4444_R    = 8, // 3
  
    OSD_TYPE_32_ARGB =9, // 1 
    OSD_TYPE_32_ABGR =10,// 2
    OSD_TYPE_32_BGRA =11,// 3 

    OSD_TYPE_24_888_B=12, //RGB  5
    OSD_TYPE_24_5658=13,  //RGBA  1
    OSD_TYPE_24_8565=14,  // 2
    OSD_TYPE_24_6666_R=15, //RGBA 3 
    OSD_TYPE_24_6666_A=16, //ARGB 4 
    

    OSD_TYPE_16_565 = 17 , //RGB 4
    OSD_TYPE_16_4444_A   = 18, // 5
    OSD_TYPE_16_1555_A  = 19, // 6
    OSD_TYPE_16_4642_R    = 20, // 7

    //YUV  mode
    OSD_TYPE_YUV_422 =21 ,

    

    OSD_TYPE_INVALID=0xff
};
typedef  enum {
	BPP_TYPE_02_PAL4    = 2,  // 0
    	BPP_TYPE_04_PAL16   = 4, // 0
	BPP_TYPE_08_PAL256=8,
	BPP_TYPE_16_655 =9,
	BPP_TYPE_16_844 =10,
	BPP_TYPE_16_6442 =11 ,
	BPP_TYPE_16_4444_R = 12,
	BPP_TYPE_16_4642_R = 13,
	BPP_TYPE_16_1555_A=14,
	BPP_TYPE_16_4444_A = 15,
	BPP_TYPE_16_565 =16,
	
	BPP_TYPE_24_6666_A=19,
	BPP_TYPE_24_6666_R=20,
	BPP_TYPE_24_8565 =21,
	BPP_TYPE_24_5658 = 22,
	BPP_TYPE_24_888_B = 23,
	BPP_TYPE_24_RGB = 24,

	BPP_TYPE_32_BGRA=29,
	BPP_TYPE_32_ABGR = 30,
	BPP_TYPE_32_RGBA=31,
	BPP_TYPE_32_ARGB=32,

	BPP_TYPE_YUV_422=33,
	
}bpp_type_t;
typedef  struct {
	bpp_type_t	type_index;
	u8	red_offset ;
	u8	red_length;
	u8	red_msb_right;
	
	u8	green_offset;
	u8	green_length;
	u8	green_msb_right;

	u8	blue_offset;
	u8	blue_length;
	u8	blue_msb_right;

	u8	transp_offset;
	u8	transp_length;
	u8	transp_msb_right;

	u8	color_type;
	u8	bpp;
	
}bpp_color_bit_define_t;
typedef struct osd_ctl_s {
    enum osd_type_s type;
    u32  xres_virtual;
    u32  yres_virtual;
    u32  xres;
    u32  yres;
    u32  disp_start_x; //coordinate of screen 
    u32  disp_start_y;
    u32  disp_end_x;
    u32  disp_end_y;
    u32  addr;
    u32  index;
} osd_ctl_t;
#define  INVALID_BPP_ITEM    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

static const  bpp_color_bit_define_t   default_color_format_array[]={
	INVALID_BPP_ITEM,
	INVALID_BPP_ITEM,
	{BPP_TYPE_02_PAL4,/*red*/ 0,2,0,/*green*/0,2,0,/*blue*/0,2,0,/*trans*/0,0,0,FB_VISUAL_PSEUDOCOLOR,8},
	INVALID_BPP_ITEM,	
	{BPP_TYPE_04_PAL16,/*red*/ 0,4,0,/*green*/0,4,0,/*blue*/0,4,0,/*trans*/0,0,0,FB_VISUAL_PSEUDOCOLOR,8},
	INVALID_BPP_ITEM,	
	INVALID_BPP_ITEM,	
	INVALID_BPP_ITEM,	
	{BPP_TYPE_08_PAL256,/*red*/ 0,8,0,/*green*/0,8,0,/*blue*/0,8,0,/*trans*/0,0,0,FB_VISUAL_PSEUDOCOLOR,8},
/*16 bit color*/
	{BPP_TYPE_16_655,/*red*/ 10,6,0,/*green*/5,5,0,/*blue*/0,5,0,/*trans*/0,0,0,FB_VISUAL_TRUECOLOR,16},
	{BPP_TYPE_16_844,/*red*/ 8,8,0,/*green*/4,4,0,/*blue*/0,4,0,/*trans*/0,0,0,FB_VISUAL_TRUECOLOR,16},
	{BPP_TYPE_16_6442,/*red*/ 10,6,0,/*green*/6,4,0,/*blue*/2,4,0,/*trans*/0,2,0,FB_VISUAL_TRUECOLOR,16},
	{BPP_TYPE_16_4444_R,/*red*/ 12,4,0,/*green*/8,4,0,/*blue*/4,4,0,/*trans*/0,4,0,FB_VISUAL_TRUECOLOR,16},
	{BPP_TYPE_16_4642_R,/*red*/ 12,4,0,/*green*/6,6,0,/*blue*/2,4,0,/*trans*/0,2,0,FB_VISUAL_TRUECOLOR,16},
	{BPP_TYPE_16_1555_A,/*red*/ 10,5,0,/*green*/5,5,0,/*blue*/0,5,0,/*trans*/15,1,0,FB_VISUAL_TRUECOLOR,16},
	{BPP_TYPE_16_4444_A,/*red*/ 8,4,0,/*green*/4,4,0,/*blue*/0,4,0,/*trans*/12,4,0,FB_VISUAL_TRUECOLOR,16},
	{BPP_TYPE_16_565,/*red*/ 11,5,0,/*green*/5,6,0,/*blue*/0,5,0,/*trans*/12,4,0,FB_VISUAL_TRUECOLOR,16},
/*24 bit color*/
	INVALID_BPP_ITEM,
	INVALID_BPP_ITEM,
	{BPP_TYPE_24_6666_A,/*red*/ 12,6,0,/*green*/6,6,0,/*blue*/0,6,0,/*trans*/18,6,0,FB_VISUAL_TRUECOLOR,24},
	{BPP_TYPE_24_6666_R,/*red*/ 18,6,0,/*green*/12,6,0,/*blue*/6,6,0,/*trans*/0,6,0,FB_VISUAL_TRUECOLOR,24},
	{BPP_TYPE_24_8565,/*red*/ 11,5,0,/*green*/5,6,0,/*blue*/0,5,0,/*trans*/16,8,0,FB_VISUAL_TRUECOLOR,24},
	{BPP_TYPE_24_5658,/*red*/ 19,5,0,/*green*/13,6,0,/*blue*/8,5,0,/*trans*/0,8,0,FB_VISUAL_TRUECOLOR,24},
	{BPP_TYPE_24_888_B,/*red*/ 0,8,0,/*green*/8,8,0,/*blue*/16,8,0,/*trans*/0,0,0,FB_VISUAL_TRUECOLOR,24},
	{BPP_TYPE_24_RGB,/*red*/ 16,8,0,/*green*/8,8,0,/*blue*/0,8,0,/*trans*/0,0,0,FB_VISUAL_TRUECOLOR,24},
/*32 bit color*/
	INVALID_BPP_ITEM,
	INVALID_BPP_ITEM,
	INVALID_BPP_ITEM,
	INVALID_BPP_ITEM,
	{BPP_TYPE_32_BGRA,/*red*/ 8,8,0,/*green*/16,8,0,/*blue*/24,8,0,/*trans*/0,8,0,FB_VISUAL_TRUECOLOR,32},
	{BPP_TYPE_32_ABGR,/*red*/ 0,8,0,/*green*/8,8,0,/*blue*/16,8,0,/*trans*/24,8,0,FB_VISUAL_TRUECOLOR,32},
	{BPP_TYPE_32_RGBA,/*red*/ 24,8,0,/*green*/16,8,0,/*blue*/8,8,0,/*trans*/0,8,0,FB_VISUAL_TRUECOLOR,32},
	{BPP_TYPE_32_ARGB,/*red*/ 16,8,0,/*green*/8,8,0,/*blue*/0,8,0,/*trans*/24,8,0,FB_VISUAL_TRUECOLOR,32},
/*YUV color*/
	{BPP_TYPE_YUV_422,0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,16},
};

typedef struct reg_val_pair{
    uint reg;
    uint val;
} reg_val_pair_t;	
	


#endif /* OSD1_H */
