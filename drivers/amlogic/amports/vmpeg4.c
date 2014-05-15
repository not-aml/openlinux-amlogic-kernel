/*
 * AMLOGIC Audio/Video streaming port driver.
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
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/amports/amstream.h>
#include <linux/amports/ptsserv.h>
#include <linux/amports/canvas.h>
#include <linux/amports/vframe.h>
#include <linux/amports/vframe_provider.h>
#include <mach/am_regs.h>

//#define CONFIG_AM_VDEC_MPEG4_LOG
#ifdef CONFIG_AM_VDEC_MPEG4_LOG
#define AMLOG
#define LOG_LEVEL_VAR		amlog_level_vmpeg4
#define LOG_MASK_VAR		amlog_mask_vmpeg4
#define LOG_LEVEL_ERROR		0
#define LOG_LEVEL_INFO		1
#define LOG_LEVEL_DESC	"0:ERROR, 1:INFO"
#define LOG_MASK_PTS	0x01
#define LOG_MASK_DESC	"0x01:DEBUG_PTS"
#endif
#include <linux/amlog.h>
MODULE_AMLOG(LOG_LEVEL_ERROR, 0, LOG_LEVEL_DESC, LOG_DEFAULT_MASK_DESC);

#include "amvdec.h"
#include "vmpeg4_mc.h"

#define DRIVER_NAME "amvdec_mpeg4"
#define MODULE_NAME "amvdec_mpeg4"

#define HANDLE_MPEG4_IRQ
#define DEBUG_PTS

#define I_PICTURE   0
#define P_PICTURE   1
#define B_PICTURE   2

#define ORI_BUFFER_START_ADDR   0x81000000

#define INTERLACE_FLAG          0x80
#define BOTTOM_FIELD_FIRST_FLAG 0x40

/* protocol registers */
#define MP4_PIC_RATIO       AV_SCRATCH_0
#define MREG_PIC_WIDTH      AV_SCRATCH_6
#define MREG_PIC_HEIGHT     AV_SCRATCH_7
#define MREG_BUFFERIN       AV_SCRATCH_8
#define MREG_BUFFEROUT      AV_SCRATCH_9
#define MP4_REPEAT_COUNT    AV_SCRATCH_A
#define MP4_TIME_STAMP      AV_SCRATCH_B
#define MP4_OFFSET_REG      AV_SCRATCH_C
#define MEM_OFFSET_REG      AV_SCRATCH_F

#define PARC_FORBIDDEN              0
#define PARC_SQUARE                 1
#define PARC_CIF                    2
#define PARC_10_11                  3
#define PARC_16_11                  4
#define PARC_40_33                  5
#define PARC_RESERVED               6
/* values between 6 and 14 are reserved */
#define PARC_EXTENDED              15

#define VF_POOL_SIZE        12
#define PUT_INTERVAL        HZ/100

#define INCPTR(p) ptr_atomic_wrap_inc(&p)

#define STAT_TIMER_INIT     0x01
#define STAT_MC_LOAD        0x02
#define STAT_ISR_REG        0x04
#define STAT_VF_HOOK        0x08
#define STAT_TIMER_ARM      0x10
#define STAT_VDEC_RUN       0x20

static vframe_t *vmpeg_vf_peek(void);
static vframe_t *vmpeg_vf_get(void);
static void vmpeg_vf_put(vframe_t *);

static const char vmpeg4_dec_id[] = "vmpeg4-dev";

static const struct vframe_provider_s vmpeg_vf_provider = {
        .peek = vmpeg_vf_peek,
        .get = vmpeg_vf_get,
        .put = vmpeg_vf_put,
};

static struct vframe_s vfpool[VF_POOL_SIZE];
static u32 vfpool_idx[VF_POOL_SIZE];
static s32 vfbuf_use[4];
static s32 fill_ptr, get_ptr, putting_ptr, put_ptr;
static u32 frame_width, frame_height, frame_dur, frame_prog;
static struct timer_list recycle_timer;
static u32 stat;
static u32 buf_start, buf_size, buf_offset;
static u32 avi_flag = 0;
static u32 vmpeg4_ratio;
u32 vmpeg4_format;

u32 pts_by_offset = 1;
u32 total_frame;
u32 next_pts;
#ifdef CONFIG_AM_VDEC_MPEG4_LOG
u32 pts_hit, pts_missed, pts_i_hit, pts_i_missed;
#endif
u32 last_picture_type;
u32 last_pts;

static struct dec_sysinfo vmpeg4_amstream_dec_info;

static unsigned char aspect_ratio_table[16] = {
        PARC_FORBIDDEN,
        PARC_SQUARE,
        PARC_CIF,
        PARC_10_11,
        PARC_16_11,
        PARC_40_33,
        PARC_RESERVED, PARC_RESERVED, PARC_RESERVED, PARC_RESERVED,
        PARC_RESERVED, PARC_RESERVED, PARC_RESERVED, PARC_RESERVED,
        PARC_RESERVED, PARC_EXTENDED
};

static inline u32 index2canvas(u32 index)
{
        const u32 canvas_tab[4] = {
                0x020100, 0x050403, 0x080706, 0x0b0a09
        };

        return canvas_tab[index];
}

static inline void ptr_atomic_wrap_inc(u32 *ptr)
{
        u32 i = *ptr;

        i++;

        if (i >= VF_POOL_SIZE)
                i = 0;

        *ptr = i;
}

static void set_aspect_ratio(vframe_t *vf, unsigned pixel_ratio)
{
        int ar = 0;

        if (vmpeg4_ratio == 0) 
        {
                vf->ratio_control |=(0x90<<DISP_RATIO_ASPECT_RATIO_BIT); // always stretch to 16:9
        }
        else if (pixel_ratio > 0x0f) 
        {
                ar = (pixel_ratio&0xff) * vmpeg4_amstream_dec_info.height * vmpeg4_ratio/((pixel_ratio>>8) * vmpeg4_amstream_dec_info.width);
        }
        else 
        {
                switch (aspect_ratio_table[pixel_ratio])
                {
                case 0:
                    ar = vmpeg4_amstream_dec_info.height*vmpeg4_ratio/vmpeg4_amstream_dec_info.width;
                    break;
                case 1:
                    ar = vf->height*vmpeg4_ratio/vf->width;
                    break;
                case 2:
                    ar = vf->height*vmpeg4_ratio*11/(vf->width*12);
                    break;
                case 3:
                    ar = vf->height*vmpeg4_ratio*11/(vf->width*10);
                    break;
                case 4:
                    ar = vf->height*vmpeg4_ratio*11/(vf->width*16);
                    break;
                case 5:
                    ar = vf->height*vmpeg4_ratio*33/(vf->width*40);
                    break;
                default:
                    ar = vf->height*vmpeg4_ratio/vf->width;
                    break;
                }
        }

        ar = min(ar, DISP_RATIO_ASPECT_RATIO_MAX);

        vf->ratio_control = (ar<<DISP_RATIO_ASPECT_RATIO_BIT);
        //vf->ratio_control |= DISP_RATIO_FORCECONFIG | DISP_RATIO_KEEPRATIO;
}

#ifdef HANDLE_MPEG4_IRQ
static irqreturn_t vmpeg4_isr(int irq, void *dev_id)
#else
static void vmpeg4_isr(void)
#endif
{
        u32 reg;
        vframe_t *vf;
        u32 repeat_count;
        u32 picture_type;
        u32 buffer_index;
        unsigned int pts, pts_valid=0, offset=0;

        reg = READ_MPEG_REG(MREG_BUFFEROUT);

        if (reg) 
        {
                if (pts_by_offset)
                {
                        offset = READ_MPEG_REG(MP4_OFFSET_REG);
                        if (pts_lookup_offset(PTS_TYPE_VIDEO, offset, &pts, 0) == 0) 
                        {
                                pts_valid = 1;
                        #ifdef CONFIG_AM_VDEC_MPEG4_LOG
                                pts_hit++;
                        #endif
                        }
                        else
                        {
                        #ifdef CONFIG_AM_VDEC_MPEG4_LOG
                                pts_missed++;
                        #endif
                        }
                }                

                repeat_count = READ_MPEG_REG(MP4_REPEAT_COUNT) + 1;
                buffer_index = ((reg & 0x7) - 1) & 3;
                picture_type = (reg >> 3) & 7;
            #ifdef CONFIG_AM_VDEC_MPEG4_LOG
                if (picture_type == I_PICTURE)
                {
                    amlog_mask(LOG_MASK_PTS, "I offset 0x%x, pts_valid %d reg=0x%x\n", offset, pts_valid,reg);
                    if (!pts_valid)
                        pts_i_missed++;
                    else
                        pts_i_hit++;
                }
            #endif

                if (reg & INTERLACE_FLAG) // interlace
                {
                        vfpool_idx[fill_ptr] = buffer_index;
                        vf = &vfpool[fill_ptr];
                        vf->width = vmpeg4_amstream_dec_info.width;
                        vf->height = vmpeg4_amstream_dec_info.height;
                        vf->bufWidth = 1920;

                        if (((I_PICTURE == picture_type) || (P_PICTURE == picture_type)) && pts_valid)
                        {
                                if ((I_PICTURE == picture_type) && (B_PICTURE == last_picture_type))
                                {
                                    vf->pts = last_pts;
                                }
                                else if ((P_PICTURE == picture_type) && (B_PICTURE == last_picture_type))
                                {
                                    vf->pts = 0;
                                }
                                else
                                {
                                    vf->pts = pts;
                                }
                                if ((repeat_count > 1) && avi_flag)
                                {
                                        vf->duration = vmpeg4_amstream_dec_info.rate * repeat_count >> 1;
                                        next_pts = pts + (vmpeg4_amstream_dec_info.rate * repeat_count >> 1)*15/16;
                                }
                                else
                                {
                                        vf->duration = vmpeg4_amstream_dec_info.rate >> 1;
                                        next_pts = 0;
                                }
								amlog_mask(LOG_MASK_PTS, "[%s:%d]vf->duration=0x%x rate=%d\n", __FUNCTION__,__LINE__, vf->duration,vmpeg4_amstream_dec_info.rate);
                        }
                        else
                        {
                                vf->pts = next_pts;
                                if ((repeat_count > 1) && avi_flag)
                                {
                                        vf->duration = vmpeg4_amstream_dec_info.rate * repeat_count >> 1;
                                        if (next_pts != 0)
                                        {
                                                next_pts += ((vf->duration) - ((vf->duration)>>4));
                                        }
                                }
                                else
                                {
                                        vf->duration = vmpeg4_amstream_dec_info.rate >> 1;
                                        next_pts = 0;
                                }
								amlog_mask(LOG_MASK_PTS, "[%s:%d]vf->duration=0x%x rate=%d picture_type=%d pts_valid=%d\n", 
														__FUNCTION__,__LINE__, vf->duration,vmpeg4_amstream_dec_info.rate,picture_type,pts_valid);
                       }

                        vf->duration_pulldown = 0;
                        vf->type = (reg & BOTTOM_FIELD_FIRST_FLAG) ? VIDTYPE_INTERLACE_BOTTOM : VIDTYPE_INTERLACE_TOP;
                        vf->canvas0Addr = vf->canvas1Addr = index2canvas(buffer_index);

                        set_aspect_ratio(vf, READ_MPEG_REG(MP4_PIC_RATIO));

                        vfbuf_use[buffer_index]++;

                        INCPTR(fill_ptr);

                        vfpool_idx[fill_ptr] = buffer_index;
                        vf = &vfpool[fill_ptr];
                        vf->width = vmpeg4_amstream_dec_info.width;
                        vf->height = vmpeg4_amstream_dec_info.height;
                        vf->bufWidth = 1920;

                        vf->pts = next_pts;
                        if ((repeat_count > 1) && avi_flag)
                        {
                                vf->duration = vmpeg4_amstream_dec_info.rate * repeat_count >> 1;
                                if (next_pts != 0)
                                {
                                        next_pts += ((vf->duration) - ((vf->duration)>>4));
                                }
                        }
                        else
                        {
                                vf->duration = vmpeg4_amstream_dec_info.rate >> 1;
                                next_pts = 0;
                        }

                        vf->duration_pulldown = 0;
                        vf->type = (reg & BOTTOM_FIELD_FIRST_FLAG) ? VIDTYPE_INTERLACE_BOTTOM : VIDTYPE_INTERLACE_TOP;
                        vf->canvas0Addr = vf->canvas1Addr = index2canvas(buffer_index);

                        set_aspect_ratio(vf, READ_MPEG_REG(MP4_PIC_RATIO));

                        vfbuf_use[buffer_index]++;

                        INCPTR(fill_ptr);
                }
                else  // progressive
                {
                        vfpool_idx[fill_ptr] = buffer_index;
                        vf = &vfpool[fill_ptr];
                        vf->width = vmpeg4_amstream_dec_info.width;
                        vf->height = vmpeg4_amstream_dec_info.height;
                        vf->bufWidth = 1920;

                        if (((I_PICTURE == picture_type) || (P_PICTURE == picture_type))&& pts_valid)
                        {
                                if ((I_PICTURE == picture_type) && (B_PICTURE == last_picture_type))
                                {
                                    vf->pts = last_pts;
                                }
                                else if ((P_PICTURE == picture_type) && (B_PICTURE == last_picture_type))
                                {
                                    vf->pts = 0;
                                }
                                else
                                {
                                    vf->pts = pts;
                                }
                                
                                if ((repeat_count > 1) && avi_flag)
                                {
                                        vf->duration = vmpeg4_amstream_dec_info.rate * repeat_count;
                                        next_pts = pts + (vmpeg4_amstream_dec_info.rate * repeat_count)*15/16;
                                }
                                else
                                {
                                        vf->duration = vmpeg4_amstream_dec_info.rate;
                                        next_pts = 0;
                                }
								amlog_mask(LOG_MASK_PTS, "[%s:%d]vf->duration=0x%x rate=%d picture_type=%d pts_valid=%d\n", 
														__FUNCTION__,__LINE__, vf->duration,vmpeg4_amstream_dec_info.rate,picture_type,pts_valid);
                        }
                        else
                        {
                                vf->pts = next_pts;
                                if ((repeat_count > 1) && avi_flag)
                                {
                                        vf->duration = vmpeg4_amstream_dec_info.rate * repeat_count;
                                        if (next_pts != 0)
                                        {
                                                next_pts += ((vf->duration) - ((vf->duration)>>4));
                                        }
                                }
                                else
                                {
                                        vf->duration = vmpeg4_amstream_dec_info.rate;
                                        next_pts = 0;
                                }
								amlog_mask(LOG_MASK_PTS, "[%s:%d]vf->duration=0x%x rate=%d picture_type=%d pts_valid=%d\n", 
														__FUNCTION__,__LINE__, vf->duration,vmpeg4_amstream_dec_info.rate,picture_type,pts_valid);
                        }

                        vf->duration_pulldown = 0;
                        vf->type = VIDTYPE_PROGRESSIVE | VIDTYPE_VIU_FIELD;
                        vf->canvas0Addr = vf->canvas1Addr = index2canvas(buffer_index);

                        set_aspect_ratio(vf, READ_MPEG_REG(MP4_PIC_RATIO));

                        vfbuf_use[buffer_index]++;

                        INCPTR(fill_ptr);
                }

                total_frame++;

                WRITE_MPEG_REG(MREG_BUFFEROUT, 0);

                last_picture_type = picture_type;
                last_pts = pts_valid ? pts : 0;
        }

        WRITE_MPEG_REG(ASSIST_MBOX1_CLR_REG, 1);

#ifdef HANDLE_MPEG4_IRQ
        return IRQ_HANDLED;
#else
        return;
#endif
}

static vframe_t *vmpeg_vf_peek(void)
{
        if (get_ptr == fill_ptr)
                return NULL;

        return &vfpool[get_ptr];
}

static vframe_t *vmpeg_vf_get(void)
{
        vframe_t *vf;

        if (get_ptr == fill_ptr)
                return NULL;

        vf = &vfpool[get_ptr];

        INCPTR(get_ptr);

        return vf;
}

static void vmpeg_vf_put(vframe_t *vf)
{
        INCPTR(putting_ptr);
}

static void vmpeg_put_timer_func(unsigned long arg)
{
        struct timer_list *timer = (struct timer_list *)arg;

#ifndef HANDLE_MPEG4_IRQ
        vmpeg4_isr();
#endif
        if ((putting_ptr != put_ptr)&& (READ_MPEG_REG(MREG_BUFFERIN) == 0))
        {
                u32 index = vfpool_idx[put_ptr];

                if (--vfbuf_use[index] == 0) 
                {
                        WRITE_MPEG_REG(MREG_BUFFERIN, ~(1<<index));
                }

                INCPTR(put_ptr);
        }

        timer->expires = jiffies + PUT_INTERVAL;

        add_timer(timer);
}

int vmpeg4_dec_status(struct vdec_status *vstatus)
{
    vstatus->width = vmpeg4_amstream_dec_info.width;
    vstatus->height = vmpeg4_amstream_dec_info.height;
    if(0!=vmpeg4_amstream_dec_info.rate)
        vstatus->fps = 96000/vmpeg4_amstream_dec_info.rate;
    else 
        vstatus->fps = 96000;
    vstatus->error_count = READ_MPEG_REG(AV_SCRATCH_4);
    vstatus->status = stat;

    return 0;
}

/****************************************/
static void vmpeg4_canvas_init(void)
{
        int i;
        u32 canvas_width, canvas_height;
        u32 decbuf_size, decbuf_y_size, decbuf_uv_size;
        u32 disp_addr = 0xffffffff;

        if (buf_size <= 0x00400000) 
        {
                /* SD only */
                canvas_width = 768;
                canvas_height = 576;
                decbuf_y_size = 0x80000;
                decbuf_uv_size = 0x20000;
                decbuf_size = 0x100000;
        }
        else 
        {
                /* HD & SD */
                canvas_width = 1920;
                canvas_height = 1088;
                decbuf_y_size = 0x200000;
                decbuf_uv_size = 0x80000;
                decbuf_size = 0x300000;
        }

        if (READ_MPEG_REG(VPP_MISC) & VPP_VD1_POSTBLEND) 
        {
	        canvas_t cur_canvas;
	    
	        canvas_read((READ_MPEG_REG(VD1_IF0_CANVAS0) & 0xff), &cur_canvas);
            disp_addr = (cur_canvas.addr + 7) >> 3;
        }

        for (i = 0; i < 4; i++) 
        {
            if (((buf_start + i * decbuf_size + 7) >> 3) == disp_addr) 
            {
                canvas_config(3 * i + 0,
                        buf_start + 4 * decbuf_size,
                        canvas_width, canvas_height,
                        CANVAS_ADDR_NOWRAP, CANVAS_BLKMODE_32X32);
                canvas_config(3 * i + 1,
                        buf_start + 4 * decbuf_size + decbuf_y_size,
                        canvas_width / 2, canvas_height / 2,
                        CANVAS_ADDR_NOWRAP, CANVAS_BLKMODE_32X32);
                canvas_config(3 * i + 2,
                        buf_start + 4 * decbuf_size + decbuf_y_size + decbuf_uv_size,
                        canvas_width/2, canvas_height/2,
                        CANVAS_ADDR_NOWRAP, CANVAS_BLKMODE_32X32);
            } 
            else 
            {
                canvas_config(3 * i + 0,
                              buf_start + i * decbuf_size,
                              canvas_width, canvas_height,
                              CANVAS_ADDR_NOWRAP, CANVAS_BLKMODE_32X32);
                canvas_config(3 * i + 1,
                              buf_start + i * decbuf_size + decbuf_y_size,
                              canvas_width / 2, canvas_height / 2,
                              CANVAS_ADDR_NOWRAP, CANVAS_BLKMODE_32X32);
                canvas_config(3 * i + 2,
                              buf_start + i * decbuf_size + decbuf_y_size + decbuf_uv_size, 
                              canvas_width / 2, canvas_height / 2, 
                              CANVAS_ADDR_NOWRAP, CANVAS_BLKMODE_32X32);
            }
        }
}

static void vmpeg4_prot_init(void)
{
        WRITE_MPEG_REG(RESET0_REGISTER, RESET_IQIDCT | RESET_MC);

        vmpeg4_canvas_init();

        /* index v << 16 | u << 8 | y */
        WRITE_MPEG_REG(AV_SCRATCH_0, 0x020100);
        WRITE_MPEG_REG(AV_SCRATCH_1, 0x050403);
        WRITE_MPEG_REG(AV_SCRATCH_2, 0x080706);
        WRITE_MPEG_REG(AV_SCRATCH_3, 0x0b0a09);

        /* notify ucode the buffer offset */
        WRITE_MPEG_REG(AV_SCRATCH_F, buf_offset >> 12);

        /* disable PSCALE for hardware sharing */
        WRITE_MPEG_REG(PSCALE_CTRL, 0);

        WRITE_MPEG_REG(MREG_BUFFERIN, 0);
        WRITE_MPEG_REG(MREG_BUFFEROUT, 0);

        /* clear mailbox interrupt */
        WRITE_MPEG_REG(ASSIST_MBOX1_CLR_REG, 1);

        /* enable mailbox interrupt */
        WRITE_MPEG_REG(ASSIST_MBOX1_MASK, 1);

}

static void vmpeg4_local_init(void)
{
        int i;

        vmpeg4_ratio = vmpeg4_amstream_dec_info.ratio;

        avi_flag = (u32)vmpeg4_amstream_dec_info.param;
    
        fill_ptr = get_ptr = put_ptr = putting_ptr = 0;

        frame_width = frame_height = frame_dur = frame_prog = 0;

        total_frame = 0;

        next_pts = 0;

#ifdef CONFIG_AM_VDEC_MPEG4_LOG
        pts_hit = pts_missed = pts_i_hit = pts_i_missed = 0;
#endif

        last_picture_type = -1;

        last_pts = 0;

        for (i = 0; i < 4; i++)
                vfbuf_use[i] = 0;
}

static s32 vmpeg4_init(void)
{
        amlog_level(LOG_LEVEL_INFO, "vmpeg4_init\n");
        init_timer(&recycle_timer);

        stat |= STAT_TIMER_INIT;

        vmpeg4_local_init();

        if (vmpeg4_amstream_dec_info.format == VIDEO_DEC_FORMAT_MPEG4_3)
        {
                amlog_level(LOG_LEVEL_INFO, "load VIDEO_DEC_FORMAT_MPEG4_3\n");
                if (amvdec_loadmc(vmpeg4_mc_311) < 0) 
                {
                        amlog_level(LOG_LEVEL_ERROR, "VIDEO_DEC_FORMAT_MPEG4_3 ucode loading failed\n");
                        return -EBUSY;
                }
        }
        else if (vmpeg4_amstream_dec_info.format == VIDEO_DEC_FORMAT_MPEG4_4)
        {
                amlog_level(LOG_LEVEL_INFO, "load VIDEO_DEC_FORMAT_MPEG4_4\n");
                if (amvdec_loadmc(vmpeg4_mc_4) < 0) 
                {
                        amlog_level(LOG_LEVEL_ERROR, "VIDEO_DEC_FORMAT_MPEG4_4 ucode loading failed\n");
                        return -EBUSY;
                }
        }
        else if (vmpeg4_amstream_dec_info.format == VIDEO_DEC_FORMAT_MPEG4_5)
        {
                amlog_level(LOG_LEVEL_INFO, "load VIDEO_DEC_FORMAT_MPEG4_5\n");
                if (amvdec_loadmc(vmpeg4_mc_5) < 0) 
                {
                        amlog_level(LOG_LEVEL_ERROR, "VIDEO_DEC_FORMAT_MPEG4_5 ucode loading failed\n");
                        return -EBUSY;
                }
        } 
        else if (vmpeg4_amstream_dec_info.format == VIDEO_DEC_FORMAT_H263)
        {
                amlog_level(LOG_LEVEL_INFO, "load VIDEO_DEC_FORMAT_H263\n");
                if (amvdec_loadmc(h263_mc) < 0) 
                {
                        amlog_level(LOG_LEVEL_ERROR, "VIDEO_DEC_FORMAT_H263 ucode loading failed\n");
                        return -EBUSY;
                }
        } 
        else
        {
                amlog_level(LOG_LEVEL_ERROR, "not supported MPEG4 format\n");
        }    

        stat |= STAT_MC_LOAD;

        /* enable AMRISC side protocol */
        vmpeg4_prot_init();

#ifdef HANDLE_MPEG4_IRQ
        if (request_irq(INT_MAILBOX_1A, vmpeg4_isr,
                        IRQF_SHARED, "vmpeg4-irq", (void *)vmpeg4_dec_id))
        {
                amlog_level(LOG_LEVEL_ERROR, "vmpeg4 irq register error.\n");
                return -ENOENT;
        }
#endif

        stat |= STAT_ISR_REG;

        vf_reg_provider(&vmpeg_vf_provider);

        stat |= STAT_VF_HOOK;

        recycle_timer.data = (ulong) & recycle_timer;
        recycle_timer.function = vmpeg_put_timer_func;
        recycle_timer.expires = jiffies + PUT_INTERVAL;

        add_timer(&recycle_timer);

        stat |= STAT_TIMER_ARM;

        amvdec_start();

        stat |= STAT_VDEC_RUN;

        set_vdec_func(&vmpeg4_dec_status);
        
        return 0;
}

static int amvdec_mpeg4_probe(struct platform_device *pdev)
{
        struct resource *mem;

        if (!(mem = platform_get_resource(pdev, IORESOURCE_MEM, 0)))
        {
                amlog_level(LOG_LEVEL_ERROR, "amvdec_mpeg4 memory resource undefined.\n");
                return -EFAULT;
        }

        buf_start = mem->start;
        buf_size = mem->end - mem->start + 1;
        buf_offset = buf_start - ORI_BUFFER_START_ADDR;

        memcpy(&vmpeg4_amstream_dec_info, (void *)mem[1].start, sizeof(vmpeg4_amstream_dec_info));

        if (vmpeg4_init() < 0) 
        {
                amlog_level(LOG_LEVEL_ERROR, "amvdec_mpeg4 init failed.\n");

                return -ENODEV;
        }

        return 0;
}

static int amvdec_mpeg4_remove(struct platform_device *pdev)
{
        if (stat & STAT_VDEC_RUN) 
        {
                amvdec_stop();
                stat &= ~STAT_VDEC_RUN;
        }

        if (stat & STAT_ISR_REG) 
        {
                free_irq(INT_MAILBOX_1A, (void *)vmpeg4_dec_id);
                stat &= ~STAT_ISR_REG;
        }

        if (stat & STAT_TIMER_ARM) 
        {
                del_timer_sync(&recycle_timer);
                stat &= ~STAT_TIMER_ARM;
        }

        if (stat & STAT_VF_HOOK) 
        {
                vf_unreg_provider();
                stat &= ~STAT_VF_HOOK;
        }

        amlog_mask(LOG_MASK_PTS, "pts hit %d, pts missed %d, i hit %d, missed %d\n",
            pts_hit, pts_missed, pts_i_hit, pts_i_missed);
        amlog_mask(LOG_MASK_PTS, "total frame %d, avi_flag %d, rate %d\n",
            total_frame, avi_flag, vmpeg4_amstream_dec_info.rate);

        return 0;
}

/****************************************/

static struct platform_driver amvdec_mpeg4_driver = {
        .probe  = amvdec_mpeg4_probe,
        .remove = amvdec_mpeg4_remove,
        .driver = {
                .name = DRIVER_NAME,
        }
};

static int __init amvdec_mpeg4_driver_init_module(void)
{
        amlog_level(LOG_LEVEL_INFO, "amvdec_mpeg4 module init\n");

        if (platform_driver_register(&amvdec_mpeg4_driver)) 
        {
                amlog_level(LOG_LEVEL_ERROR, "failed to register amvdec_mpeg4 driver\n");
                return -ENODEV;
        }

        return 0;
}

static void __exit amvdec_mpeg4_driver_remove_module(void)
{
        amlog_level(LOG_LEVEL_INFO, "amvdec_mpeg4 module remove.\n");

        platform_driver_unregister(&amvdec_mpeg4_driver);
}

/****************************************/

module_param(stat, uint, 0664);
MODULE_PARM_DESC(stat, "\n amvdec_mpeg4 stat \n");

module_init(amvdec_mpeg4_driver_init_module);
module_exit(amvdec_mpeg4_driver_remove_module);

MODULE_DESCRIPTION("AMLOGIC MPEG4 Video Decoder Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tim Yao <timyao@amlogic.com>");

