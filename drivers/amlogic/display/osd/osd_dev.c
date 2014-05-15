/*
 * Amlogic osd
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
 * Author:  Tim Yao <timyao@amlogic.com>
 *
 */

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <mach/am_regs.h>

#include <linux/osd/osd.h>
#include <linux/osd/osd_dev.h>
#include <linux/osd/osd_hw.h>
/* to-do: TV output mode should be configured by
 * sysfs attribute
 */

void osddev_set(struct myfb_dev *fbdev)
{
    enum osd_type_s type;
	
    const enum osd_type_s typeTab[34] = {
        OSD_TYPE_02_PAL4  , OSD_TYPE_02_PAL4  , OSD_TYPE_02_PAL4  ,
        OSD_TYPE_04_PAL16 , OSD_TYPE_04_PAL16 ,
        OSD_TYPE_08_PAL256, OSD_TYPE_08_PAL256, OSD_TYPE_08_PAL256, OSD_TYPE_08_PAL256,
        OSD_TYPE_16_655  , OSD_TYPE_16_844   , OSD_TYPE_16_6442   , OSD_TYPE_16_4444_R   ,
        OSD_TYPE_16_4642_R  , OSD_TYPE_16_1555_A   , OSD_TYPE_16_4444_A   , OSD_TYPE_16_565/*16*/   ,
        OSD_TYPE_24_RGB     , OSD_TYPE_24_RGB   , OSD_TYPE_24_6666_A   , OSD_TYPE_24_6666_R   ,
        OSD_TYPE_24_8565   , OSD_TYPE_24_5658   , OSD_TYPE_24_888_B   , OSD_TYPE_24_RGB /*24*/  ,
        OSD_TYPE_32_ARGB  , OSD_TYPE_32_ARGB  , OSD_TYPE_32_ARGB  , OSD_TYPE_32_ARGB  ,
        OSD_TYPE_32_BGRA  , OSD_TYPE_32_ABGR  , OSD_TYPE_32_RGBA  , OSD_TYPE_32_ARGB /*32*/ ,
        OSD_TYPE_YUV_422  , //YUV 422
    };


	
   

    fbdev_lock(fbdev);

    //memset((char*) fbdev->fb_mem,0x0,fbdev->fb_len);
    

    type = typeTab[fbdev->bpp_type];
    osd_setup(&fbdev->osd_ctl,
               fbdev->fb_info->var.xoffset,
               fbdev->fb_info->var.yoffset,
               fbdev->fb_info->var.xres,
               fbdev->fb_info->var.yres,
               fbdev->fb_info->var.xres_virtual,
               fbdev->fb_info->var.yres_virtual,
               fbdev->osd_ctl.disp_start_x,
               fbdev->osd_ctl.disp_start_y,
               fbdev->osd_ctl.disp_end_x,
               fbdev->osd_ctl.disp_end_y,
               fbdev->fb_mem_paddr,
               type,
               fbdev->fb_info->node);	



    fbdev_unlock(fbdev);
    	
    return;
}
void osddev_update_disp_axis(struct myfb_dev *fbdev,int  mode_change)
{
	osddev_update_disp_axis_hw(	fbdev->osd_ctl.disp_start_x,
								fbdev->osd_ctl.disp_end_x,
               						fbdev->osd_ctl.disp_start_y,
               						fbdev->osd_ctl.disp_end_y,
               						fbdev->fb_info->var.xoffset,
               						fbdev->fb_info->var.yoffset,
               						mode_change,
               						fbdev->fb_info->node);
}
int osddev_setcolreg(unsigned regno, u16 red, u16 green, u16 blue,
        u16 transp, struct myfb_dev *fbdev)
{
    struct fb_info *info = fbdev->fb_info;

    if ((fbdev->osd_ctl.type == OSD_TYPE_02_PAL4) ||
        (fbdev->osd_ctl.type == OSD_TYPE_04_PAL16) ||
        (fbdev->osd_ctl.type == OSD_TYPE_08_PAL256)) {

        fbdev_lock(fbdev);

        osd_setpal_hw(regno, red, green, blue, transp,fbdev->fb_info->node);

        fbdev_unlock(fbdev);
    }

	if (info->fix.visual == FB_VISUAL_TRUECOLOR) {
		u32 v, r, g, b, a;

		if (regno >= 16)
			return 1;
	 
        r = red    >> (16 - info->var.red.length);
        g = green  >> (16 - info->var.green.length);
        b = blue   >> (16 - info->var.blue.length);
        a = transp >> (16 - info->var.transp.length);

		v = (r << info->var.red.offset)   |
		    (g << info->var.green.offset) |
		    (b << info->var.blue.offset)  |
		    (a << info->var.transp.offset);

  		((u32*)(info->pseudo_palette))[regno] = v;
	}

    return 0;
}

void osddev_enable(int enable,int  index)
{
    osd_enable_hw(enable,index);
}

void osddev_pan_display(struct fb_var_screeninfo *var,struct fb_info *fbi)
{
    osd_pan_display_hw(var->xoffset, var->yoffset,fbi->node);
}
void  osddev_set_colorkey(u32 index,u32 bpp,u32 colorkey )
{
	osd_set_colorkey_hw( index, bpp, colorkey );
}
void  osddev_srckey_enable(u32  index,u8 enable)
{
	osd_srckey_enable_hw(index,enable);
}
void  osddev_set_gbl_alpha(u32 index,u32 gbl_alpha)
{
	osd_set_gbl_alpha_hw(index,gbl_alpha);
}
u32  osddev_get_gbl_alpha(u32  index)
{
	return osd_get_gbl_alpha_hw(index);
}
void  osddev_suspend(void)
{
	osd_suspend_hw();
}
void osddev_resume(void)
{
	osd_resume_hw();
}