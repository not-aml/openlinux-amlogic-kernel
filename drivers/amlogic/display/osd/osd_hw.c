/*
 * Amlogic Apollo
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
#include <linux/spinlock.h>
#include <mach/am_regs.h>
#include <linux/irqreturn.h>
#include <linux/errno.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/osd/osd.h>
#include <linux/osd/osd_hw.h>
#include <linux/amports/canvas.h>
#include "osd_log.h"
#include <linux/amlog.h>
/************************************************************************
**
**	macro  define  part
**
**************************************************************************/
#define  	RESTORE_MEMORY_SIZE    		600
#define  	OSD_RELATIVE_BITS				0x333f0
#define	OSD1_OSD2_SOTRE_OFFSET		(RESTORE_MEMORY_SIZE>>1)

/************************************************************************
**
**	global varible  define  part
**
**************************************************************************/
static  u32  osd_gbl_alpha[2]={OSD_GLOBAL_ALPHA_DEF,OSD_GLOBAL_ALPHA_DEF};

static const u8 blkmode_reg[] = {5, 7, 0, 1, 2, 4, 4, 4, 4,/*32*/5,5,5,/*24*/7,7,7,7,7,/*16*/4,4,4,4,/*yuv*/3};
static const u8 colormat_reg[] = {0, 0, 0, 0, 0, 0, 1, 2, 3,/*32*/1,2,3,/*24*/5,1,2,3,4,/*16*/4,5,6,7,/*yuv*/0};
static const u8 modebits[] = {32,24,2,4,8,16,16,16,16,/*32*/32,32,32,/*24*/24,24,24,24,24,/*16*/16,16,16,16,/*yuv*/16};
static pandata_t pandata[2];
static spinlock_t osd_lock = SPIN_LOCK_UNLOCKED;
static  u32   *reg_status;
static  int reg_index[]={
	VIU_OSD1_TCOLOR_AG0,
	VIU_OSD1_BLK0_CFG_W0,
	VIU_OSD1_BLK0_CFG_W1,
	VIU_OSD1_BLK0_CFG_W2,
	VIU_OSD1_BLK0_CFG_W3,
	VIU_OSD1_BLK0_CFG_W4,
	VIU_OSD1_FIFO_CTRL_STAT,
	VIU_OSD1_CTRL_STAT,
	VPP_MISC,
	VIU_OSD1_COLOR_ADDR,
};

/**********************************************************************/
/**********				 osd vsync irq handler   				***************/
/**********************************************************************/
static irqreturn_t vsync_isr(int irq, void *dev_id)
{
	unsigned  int  fb0_cfg_w0,fb1_cfg_w0;
	unsigned  int  current_field;
	int interlaced = 0;

	WRITE_MPEG_REG(VIU_OSD1_BLK0_CFG_W1,
		(pandata[0].x_start & 0x1fff) | (pandata[0].x_end & 0x1fff) << 16);
	WRITE_MPEG_REG(VIU_OSD1_BLK0_CFG_W2,
		(pandata[0].y_start & 0x1fff) | (pandata[0].y_end & 0x1fff) << 16);

	WRITE_MPEG_REG(VIU_OSD2_BLK0_CFG_W1,
		(pandata[1].x_start & 0x1fff) | (pandata[1].x_end & 0x1fff) << 16);
	WRITE_MPEG_REG(VIU_OSD2_BLK0_CFG_W2,
		(pandata[1].y_start & 0x1fff) | (pandata[1].y_end & 0x1fff) << 16);

	if (READ_MPEG_REG(ENCI_VIDEO_EN) & 1)
		interlaced = 1;
	else if (READ_MPEG_REG(ENCP_VIDEO_MODE) & (1 << 12))
		interlaced = 1;	

	if (interlaced)
	{
		fb0_cfg_w0=READ_MPEG_REG(VIU_OSD1_BLK0_CFG_W0);
		fb1_cfg_w0=READ_MPEG_REG(VIU_OSD1_BLK0_CFG_W0+ REG_OFFSET);
		if (READ_MPEG_REG(ENCP_VIDEO_MODE) & (1 << 12))
        	{
       		 /* 1080I */
			 
        		if (READ_MPEG_REG(VENC_ENCP_LINE) >= 562) {
           		 /* top field */
            			current_field = 0;
        		} else {
           			current_field = 1;
        		}
    		} else {
        		current_field = READ_MPEG_REG(VENC_STATA) & 1;
    		}
		fb0_cfg_w0 &=~1;
		fb1_cfg_w0 &=~1;
		fb0_cfg_w0 |=current_field;
		fb1_cfg_w0 |=current_field;
		WRITE_MPEG_REG(VIU_OSD1_BLK0_CFG_W0, fb0_cfg_w0);
		WRITE_MPEG_REG(VIU_OSD1_BLK0_CFG_W0+ REG_OFFSET, fb1_cfg_w0);
	}
		
       if (READ_MPEG_REG(VENC_ENCI_LINE) >= 12)
             READ_MPEG_REG(VENC_ENCI_LINE);

	return  IRQ_HANDLED ;
}


static void
_init_osd_simple(u32 pix_x_start,
                  u32 pix_x_end,
                  u32 pix_y_start,
                  u32 pix_y_end,
                  u32 display_h_start,
                  u32 display_v_start,
                  u32 display_h_end,
                  u32 display_v_end,
                  u32 canvas_index,
                  enum osd_type_s type,
                  int  index)
{
	u32 data32;
 	u32  vmode ;
	static char   tv_irq_got=0; 	
	
       if(READ_MPEG_REG(ENCI_VIDEO_EN)&0x1)  //interlace
       {
       		vmode=1;
   	}
   	else
   	{
   		if(READ_MPEG_REG(ENCP_VIDEO_MODE) & (1 << 12)) //1080i
   		{
   			vmode=1;
   		}
   		else
   		vmode=0;
   	}
	
   	WRITE_MPEG_REG(VIU_OSD1_TCOLOR_AG0 + REG_OFFSET*index, (unsigned)0xffffff00);
   	data32 = (display_h_start & 0xfff) | (display_h_end & 0xfff) <<16 ;
      	WRITE_MPEG_REG(VIU_OSD1_BLK0_CFG_W3 + REG_OFFSET*index, data32);
   	data32 = (display_v_start & 0xfff) | (display_v_end & 0xfff) <<16 ;
       WRITE_MPEG_REG(index?VIU_OSD2_BLK0_CFG_W4:VIU_OSD1_BLK0_CFG_W4, data32);
	amlog_mask_level(LOG_MASK_HARDWARE,LOG_LEVEL_LOW,"[disp(x-y)] %d:%d-%d:%d\n",display_h_start,\
														display_h_end,display_v_start,display_v_end);

	pandata[index].x_start = pix_x_start;
	pandata[index].x_end   = pix_x_end;
	pandata[index].y_start = pix_y_start;
	pandata[index].y_end   = pix_y_end;

	WRITE_MPEG_REG(VIU_OSD1_BLK0_CFG_W1,
		(pandata[0].x_start & 0x1fff) | (pandata[0].x_end & 0x1fff) << 16);
	WRITE_MPEG_REG(VIU_OSD1_BLK0_CFG_W2,
		(pandata[0].y_start & 0x1fff) | (pandata[0].y_end & 0x1fff) << 16);

	WRITE_MPEG_REG(VIU_OSD2_BLK0_CFG_W1,
		(pandata[1].x_start & 0x1fff) | (pandata[1].x_end & 0x1fff) << 16);
	WRITE_MPEG_REG(VIU_OSD2_BLK0_CFG_W2,
		(pandata[1].y_start & 0x1fff) | (pandata[1].y_end & 0x1fff) << 16);

	data32= (vmode == 1) ? 2 : 0;
	data32 |=READ_MPEG_REG(VIU_OSD1_BLK0_CFG_W0+ REG_OFFSET*index)&0x40;
	if (tv_irq_got == 0)
	{
		if ( request_irq(INT_VIU_VSYNC, &vsync_isr,
                    IRQF_SHARED , "am_osd_tv", osd_setup))
    		{
    			amlog_level(LOG_LEVEL_HIGH,"can't request irq for vsync\r\n");
    		}
		else
		{
			amlog_level(LOG_LEVEL_HIGH,"request irq for vsync \r\n");
			tv_irq_got=1;
		}
	}
    	data32 |= canvas_index << 16 ;
	data32 |= OSD_DATA_LITTLE_ENDIAN	 <<15 ;
    	data32 |= colormat_reg[type]     << 2;	
	data32 |= OSD_TC_ALPHA_ENABLE_DEF   << 6;	
	if(type < OSD_TYPE_YUV_422)
	data32 |= 1                      << 7; /* rgb enable */
	data32 |= blkmode_reg[type]      << 8; /* osd_blk_mode */
	WRITE_MPEG_REG(VIU_OSD1_BLK0_CFG_W0+ REG_OFFSET*index, data32);

    	// Program reg VIU_OSD1_FIFO_CTRL_STAT
    	data32  = 4   << 5;  // hold_fifo_lines
    	data32 |= 3   << 10; // burst_len_sel: 3=64
    	data32 |= 32  << 12; // fifo_depth_val: 32*8=256
    	WRITE_MPEG_REG(VIU_OSD1_FIFO_CTRL_STAT + REG_OFFSET*index, data32);

    	// Program reg VIU_OSD1_CTRL_STAT
    	data32  = 0x1          << 0; // osd_blk_enable
    	data32 |= osd_gbl_alpha[index] << 12;
    	WRITE_MPEG_REG(VIU_OSD1_CTRL_STAT + REG_OFFSET*index, data32);
}
void  osd_set_gbl_alpha_hw(u32 index,u32 gbl_alpha)
{
	u32  reg=VIU_OSD1_CTRL_STAT + REG_OFFSET*index;
	u32  reg_value=0;
	
	if(osd_gbl_alpha[index] != gbl_alpha)
	{
		osd_gbl_alpha[index]=gbl_alpha;
		reg_value=READ_MPEG_REG(reg);
		reg_value&=~(0x1ff<<12);
		reg_value|=(gbl_alpha&0x1ff)<<12;
		WRITE_MPEG_REG(reg,reg_value);
	}
}
u32  osd_get_gbl_alpha_hw(u32  index)
{
	return osd_gbl_alpha[index];
}
void  osd_set_colorkey_hw(u32 index,u32 bpp,u32 colorkey )
{
	u8  r=0,g=0,b=0,a=(colorkey&0xff000000)>>24;
	
	u32  reg=VIU_OSD1_TCOLOR_AG0 + REG_OFFSET*index;

	colorkey&=0x00ffffff;
	 switch(bpp)
	  {
	 		case BPP_TYPE_16_655:
			r=(colorkey>>10&0x3f)<<2;
			g=(colorkey>>5&0x1f)<<3;
			b=(colorkey&0x1f)<<3;
			break;	
			case BPP_TYPE_16_844:
			r=colorkey>>8&0xff;
			g=(colorkey>>4&0xf)<<4;
			b=(colorkey&0xf)<<4;	
			break;	
			case BPP_TYPE_16_565:
			r=(colorkey>>11&0x1f)<<3;
			g=(colorkey>>5&0x3f)<<2;
			b=(colorkey&0x1f)<<3;		
			break;	
			case BPP_TYPE_24_888_B:
			b=colorkey>>16&0xff;
			g=colorkey>>8&0xff;
			r=colorkey&0xff;			
			break;
			case BPP_TYPE_24_RGB:
			case BPP_TYPE_YUV_422:	
			r=colorkey>>16&0xff;
			g=colorkey>>8&0xff;
			b=colorkey&0xff;			
			break;	
	 }	
	amlog_mask_level(LOG_MASK_HARDWARE,LOG_LEVEL_LOW,"bpp:%d--r:0x%x g:0x%x b:0x%x ,a:0x%x\r\n",bpp,r,g,b,a);
	WRITE_MPEG_REG(reg,r<<24|g<<16|b<<8|a);
	return ;
}
void  osd_srckey_enable_hw(u32  index,u8 enable)
{
	u32  reg=VIU_OSD1_BLK0_CFG_W0 + REG_OFFSET*index;
	u32  data=READ_MPEG_REG(reg);

	if(enable)
	{
		data|=(1<<6);
	}
	else
	{
		data&=~(1<<6);
	}
	
	WRITE_MPEG_REG(reg,data );
}

void  osddev_update_disp_axis_hw(
			u32 display_h_start,
                  	u32 display_h_end,
                  	u32 display_v_start,
                  	u32 display_v_end,
			u32 xoffset,
                  	u32 yoffset,
                  	u32 mode_change,
                  	u32 index)
{
	u32  vmode;
	u32	data32;

	if(mode_change)  //modify pandata .
	{
		 if(READ_MPEG_REG(ENCI_VIDEO_EN)&0x1)  //interlace
       		{
       			vmode=1;
   		}
   		else
   		{
   			if(READ_MPEG_REG(ENCP_VIDEO_MODE) & (1 << 12)) //1080i
   			{
   				vmode=1;
   			}
   			else
   			vmode=0;
   		}
		data32=READ_MPEG_REG(VIU_OSD1_BLK0_CFG_W0+ REG_OFFSET*index);
		data32=(data32&~(1<<1)) | (vmode <<1); //interlace enable
		WRITE_MPEG_REG(VIU_OSD1_BLK0_CFG_W0+ REG_OFFSET*index,data32);
		

	}
	data32 = (display_h_start & 0xfff) | (display_h_end & 0xfff) <<16 ;
      	WRITE_MPEG_REG(VIU_OSD1_BLK0_CFG_W3 + REG_OFFSET*index, data32);
   	data32 = (display_v_start & 0xfff) | (display_v_end & 0xfff) <<16 ;
       WRITE_MPEG_REG(index?VIU_OSD2_BLK0_CFG_W4:VIU_OSD1_BLK0_CFG_W4, data32);
	amlog_mask_level(LOG_MASK_HARDWARE,LOG_LEVEL_LOW,"[disp(x-y)] %d:%d-%d:%d\n",display_h_start,\
														display_h_end,display_v_start,display_v_end);   
	//if output mode change then reset pan ofFfset.
	pandata[index].x_start = xoffset;
	pandata[index].x_end   = xoffset + (display_h_end-display_h_start);
	pandata[index].y_start = yoffset;
	pandata[index].y_end   = yoffset + (display_v_end-display_v_start);
	WRITE_MPEG_REG(VIU_OSD1_BLK0_CFG_W1,
	(pandata[0].x_start & 0x1fff) | (pandata[0].x_end & 0x1fff) << 16);
	WRITE_MPEG_REG(VIU_OSD1_BLK0_CFG_W2,
	(pandata[0].y_start & 0x1fff) | (pandata[0].y_end & 0x1fff) << 16);
	WRITE_MPEG_REG(VIU_OSD2_BLK0_CFG_W1,
	(pandata[1].x_start & 0x1fff) | (pandata[1].x_end & 0x1fff) << 16);
	WRITE_MPEG_REG(VIU_OSD2_BLK0_CFG_W2,
	(pandata[1].y_start & 0x1fff) | (pandata[1].y_end & 0x1fff) << 16);													
	
}
void osd_setup(struct osd_ctl_s *osd_ctl,
                u32 xoffset,
                u32 yoffset,
                u32 xres,
                u32 yres,
                u32 xres_virtual,
                u32 yres_virtual,
                u32 disp_start_x,
                u32 disp_start_y,
                u32 disp_end_x,
                u32 disp_end_y,
                u32 fbmem,
                enum osd_type_s type,
                int index 
                )
{
    u32 w = (modebits[type] * xres_virtual + 7) >> 3;

    osd_ctl->type = type;
    osd_ctl->xres = xres;
    osd_ctl->yres = yres;
    osd_ctl->xres_virtual = xres_virtual;
    osd_ctl->yres_virtual = yres_virtual;
    osd_ctl->addr = (u32)fbmem;
    osd_ctl->index = index ?OSD2_CANVAS_INDEX : OSD1_CANVAS_INDEX;

    /* configure canvas */
    canvas_config(osd_ctl->index, osd_ctl->addr,
	              w, osd_ctl->yres_virtual,
	              CANVAS_ADDR_NOWRAP, CANVAS_BLKMODE_LINEAR);

    /* configure OSD */
    _init_osd_simple(xoffset, xoffset + (disp_end_x-disp_start_x),
	                 yoffset, yoffset + (disp_end_y-disp_start_y),
	                 disp_start_x, disp_start_y,
	                 disp_end_x,disp_end_y, osd_ctl->index,
	                 type,index);
	
	SET_MPEG_REG_MASK(VIU_OSD1_CTRL_STAT, 1 << 21);
	SET_MPEG_REG_MASK(VIU_OSD2_CTRL_STAT, 1 << 21);	
	WRITE_MPEG_REG_BITS(VIU_OSD1_FIFO_CTRL_STAT, 0x2, 10, 2); //set burst size 48
	WRITE_MPEG_REG_BITS(VIU_OSD2_FIFO_CTRL_STAT, 0x2, 10, 2);
	SET_MPEG_REG_MASK(VPP_MISC, ((index == 0) ? VPP_OSD1_POSTBLEND :
	                  VPP_OSD2_POSTBLEND) | VPP_POSTBLEND_EN);
    	CLEAR_MPEG_REG_MASK(VPP_MISC, VPP_PREBLEND_EN |
                                  VPP_PRE_FG_OSD2 |
                                  VPP_POST_FG_OSD2);
}

void osd_setpal_hw(unsigned regno,
                 unsigned red,
                 unsigned green,
                 unsigned blue,
                 unsigned transp,
                 int index 
                 )
{

    if (regno < 256) {
        u32 pal;
        pal = ((red   & 0xff) << 24) |
              ((green & 0xff) << 16) |
              ((blue  & 0xff) <<  8) |
              (transp & 0xff);

        WRITE_MPEG_REG(VIU_OSD1_COLOR_ADDR+REG_OFFSET*index, regno);
        WRITE_MPEG_REG(VIU_OSD1_COLOR+REG_OFFSET*index, pal);
    }
}

void osd_enable_hw(int enable ,int index )
{
    unsigned long flags;
    	
    int  enable_bit=index? (VPP_OSD2_POSTBLEND) :(VPP_OSD1_POSTBLEND) ;	//osd1 osd2 select bit.
    
    spin_lock_irqsave(&osd_lock, flags);
    amlog_mask_level(LOG_MASK_HARDWARE,LOG_LEVEL_LOW,"%s bit:0x%x \r\n",enable?"enable":"disable",enable_bit);
    	
		
    if (enable) {
        SET_MPEG_REG_MASK(VPP_MISC, enable_bit);
    }
    else {
        CLEAR_MPEG_REG_MASK(VPP_MISC, enable_bit);
    }

    spin_unlock_irqrestore(&osd_lock, flags);
}

void osd_pan_display_hw(unsigned int xoffset, unsigned int yoffset,int index )
{
	ulong flags;
	long diff_x, diff_y;
	
	if (index >= 2)
		return;
    	
    spin_lock_irqsave(&osd_lock, flags);
	//yoffset=yoffset > 0?0:720;
	diff_x = xoffset - pandata[index].x_start;
	diff_y = yoffset - pandata[index].y_start;

	pandata[index].x_start += diff_x;
	pandata[index].x_end   += diff_x;
	pandata[index].y_start += diff_y;
	pandata[index].y_end   += diff_y;
    amlog_mask_level(LOG_MASK_HARDWARE,LOG_LEVEL_LOW,"offset[%d-%d]x[%d-%d]y[%d-%d]\n", \
				xoffset,yoffset,pandata[index].x_start,pandata[index].x_end, \
				pandata[index].y_start,pandata[index].y_end);
    spin_unlock_irqrestore(&osd_lock, flags);
}

void  osd_suspend_hw(void)
{
	u32 i,j;
	u32 data;
	u32  *preg;
	
	//free irq ,we can not disable it ,maybe video still use it .
	free_irq(INT_VIU_VSYNC,(void *)osd_setup);
	//save all status
	reg_status=(u32*)kmalloc(sizeof(u32)*RESTORE_MEMORY_SIZE,GFP_KERNEL);
	if(IS_ERR (reg_status))
	{
		amlog_level(LOG_LEVEL_HIGH,"can't alloc restore memory\r\n");
		return ;
	}
	preg=reg_status;
	for(i=0;i<ARRAY_SIZE(reg_index);i++)
	{
		switch(reg_index[i])
		{
			case VPP_MISC:
			data=READ_MPEG_REG(VPP_MISC);
			*preg=data&OSD_RELATIVE_BITS; //0x333f0 is osd0&osd1 relative bits
			WRITE_MPEG_REG(VPP_MISC,data&(~OSD_RELATIVE_BITS));
			break;
			case VIU_OSD1_BLK0_CFG_W4:
			data=READ_MPEG_REG(VIU_OSD1_BLK0_CFG_W4);
			*preg=data;
			data=READ_MPEG_REG(VIU_OSD2_BLK0_CFG_W4);
			*(preg+OSD1_OSD2_SOTRE_OFFSET)=data;
			break;
			case VIU_OSD1_COLOR_ADDR: //resotre palette value
			for(j=0;j<256;j++)
			{
				WRITE_MPEG_REG(VIU_OSD1_COLOR_ADDR, 1<<8|j);
				*preg=READ_MPEG_REG(VIU_OSD1_COLOR);
				WRITE_MPEG_REG(VIU_OSD1_COLOR_ADDR+REG_OFFSET, 1<<8|j);
				*(preg+OSD1_OSD2_SOTRE_OFFSET)=READ_MPEG_REG(VIU_OSD1_COLOR+REG_OFFSET);
				preg++;
			}
			break;
			default :
			data=READ_MPEG_REG(reg_index[i]);
			*preg=data;
			break;
		}
		preg++;
	}
	//disable osd relative clock
	return ;
	
}
void osd_resume_hw(void)
{
	u32 i,j;
	u32  *preg;

	// enable osd relative clock	
	//restore status
	if(reg_status)
	{
		preg=reg_status;
		for(i=0;i<ARRAY_SIZE(reg_index);i++)
		{
			switch(reg_index[i])
			{
	       			case VPP_MISC:
	       			SET_MPEG_REG_MASK(VPP_MISC,*preg);
				break;
				case VIU_OSD1_BLK0_CFG_W4:
				WRITE_MPEG_REG(VIU_OSD1_BLK0_CFG_W4,*preg);
				WRITE_MPEG_REG(VIU_OSD2_BLK0_CFG_W4,*(preg+OSD1_OSD2_SOTRE_OFFSET));
				break;
				case VIU_OSD1_COLOR_ADDR: //resotre palette value
				for(j=0;j<256;j++)
				{
					WRITE_MPEG_REG(VIU_OSD1_COLOR_ADDR, j);
					WRITE_MPEG_REG(VIU_OSD1_COLOR,*preg);
					WRITE_MPEG_REG(VIU_OSD1_COLOR_ADDR+REG_OFFSET, j);
					WRITE_MPEG_REG(VIU_OSD1_COLOR+REG_OFFSET,*(preg+OSD1_OSD2_SOTRE_OFFSET));
					preg++;
				}
				break;
				default :
				WRITE_MPEG_REG(reg_index[i],*preg);
				break;
			}
			preg++;
		}
		kfree(reg_status);
		// osd relative clock	
	}
	
	//request irq again
	if ( request_irq(INT_VIU_VSYNC, &vsync_isr,
                    IRQF_SHARED , "am_osd_tv", osd_setup))
    	{
    		amlog_level(LOG_LEVEL_HIGH,"can't request irq when osd resume\r\n");
    	}
	return ;
}

