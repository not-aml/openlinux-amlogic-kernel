/*
 * VDIN driver
 *
 * Author: Lin Xu <lin.xu@amlogic.com>
 *         Bobby Yang <bo.yang@amlogic.com>
 *
 * Copyright (C) 2010 Amlogic Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#ifndef __TVIN_VDIN_H
#define __TVIN_VDIN_H

#include "tvin_global.h"


typedef enum vdin_clk_e {
    VDIN_CLK_AUTO = 0,
    VDIN_CLK_OFF,
    VDIN_CLK_ON,
} vdin_clk_t;

typedef enum vdin_mpeg_field_e {
    VDIN_MPEG_FIELD_EVEN = 0,
    VDIN_MPEG_FIELD_ODD,
} vdin_mpeg_field_t;


typedef enum vdin_src_e {
	VDIN_SRC_NULL = 0,
	VDIN_SRC_MPEG,
	VDIN_SRC_BT656IN,
	VDIN_SRC_TVFE,
	VDIN_SRC_CVD2,
	VDIN_SRC_HDMIRX,
	VDIN_SRC_CAMERA,
} vdin_src_t;

typedef enum vdin_mux_e {
	VDIN_MUX_YCBCR = 0, // input is YCbCr
	VDIN_MUX_YCRCB,     // input is YCrCb
	VDIN_MUX_CBCRY,     // input is CbCrY
	VDIN_MUX_CBYCR,     // input is CbYCr
	VDIN_MUX_CRYCB,     // input is CrYCb
	VDIN_MUX_CRCBY,     // input is CrCbY
} vdin_mux_t;

typedef enum vdin_hscl_flt_e {
    VDIN_HSCL_FLT_BICUBIC = 0,
    VDIN_HSCL_FLT_BILINEAR_PTS2,
    VDIN_HSCL_FLT_BILINEAR_PTS2_BANKLEN2,
} vdin_hscl_flt_t;

typedef enum vdin_hscl_flt_depth_e {
    VDIN_HSCL_FLT_DEPTH_0 = 0,
    VDIN_HSCL_FLT_DEPTH_1,
    VDIN_HSCL_FLT_DEPTH_2,
    VDIN_HSCL_FLT_DEPTH_3,
    VDIN_HSCL_FLT_DEPTH_4,
} vdin_hscl_flt_depth_t;

/*
YUV601:  SDTV BT.601            YCbCr (16~235, 16~240, 16~240)
YUV601F: SDTV BT.601 Full_Range YCbCr ( 0~255,  0~255,  0~255)
YUV709:  HDTV BT.709            YCbCr (16~235, 16~240, 16~240)
YUV709F: HDTV BT.709 Full_Range YCbCr ( 0~255,  0~255,  0~255)
RGBS:                       StudioRGB (16~235, 16~235, 16~235)
RGB:                              RGB ( 0~255,  0~255,  0~255)
*/
typedef enum vdin_matrix_csc_e {
	VDIN_MATRIX_NULL = 0,
    VDIN_MATRIX_RGB_YUV601,
    VDIN_MATRIX_YUV601_RGB,
    VDIN_MATRIX_RGB_YUV601F,
    VDIN_MATRIX_YUV601F_RGB,
    VDIN_MATRIX_RGBS_YUV601,
    VDIN_MATRIX_YUV601_RGBS,
    VDIN_MATRIX_RGBS_YUV601F,
    VDIN_MATRIX_YUV601F_RGBS,
    VDIN_MATRIX_YUV601F_YUV601,
    VDIN_MATRIX_YUV601_YUV601F,
    VDIN_MATRIX_RGB_YUV709,
    VDIN_MATRIX_YUV709_RGB,
    VDIN_MATRIX_RGB_YUV709F,
    VDIN_MATRIX_YUV709F_RGB,
    VDIN_MATRIX_RGBS_YUV709,
    VDIN_MATRIX_YUV709_RGBS,
    VDIN_MATRIX_RGBS_YUV709F,
    VDIN_MATRIX_YUV709F_RGBS,
    VDIN_MATRIX_YUV709F_YUV709,
    VDIN_MATRIX_YUV709_YUV709F,
    VDIN_MATRIX_YUV601_YUV709,
    VDIN_MATRIX_YUV709_YUV601,
    VDIN_MATRIX_YUV601_YUV709F,
    VDIN_MATRIX_YUV709F_YUV601,
    VDIN_MATRIX_YUV601F_YUV709,
    VDIN_MATRIX_YUV709_YUV601F,
    VDIN_MATRIX_YUV601F_YUV709F,
    VDIN_MATRIX_YUV709F_YUV601F,
    VDIN_MATRIX_RGBS_RGB,
    VDIN_MATRIX_RGB_RGBS,
} vdin_matrix_csc_t;

//typedef enum vdin_data_format_e {
//    VDIN_DATA_FORMAT_422 = 0,
//    VDIN_DATA_FORMAT_444,
//} vdin_data_format_t;

typedef enum vdin_stat_src_mux_e {
    VDIN_STAT_SRC_MUX_MAT_OUT = 0,
    VDIN_STAT_SRC_MUX_HSCL_OUT,
    VDIN_STAT_SRC_MUX_PREHSCL_IN,
} vdin_stat_src_mux_t;

typedef enum vdin_bbar_region_wid_pow_e {
    VDIN_BBAR_REGION_WID_POW_0 = 0,
    VDIN_BBAR_REGION_WID_POW_1,
    VDIN_BBAR_REGION_WID_POW_2,
    VDIN_BBAR_REGION_WID_POW_3,
    VDIN_BBAR_REGION_WID_POW_4,
    VDIN_BBAR_REGION_WID_POW_5,
    VDIN_BBAR_REGION_WID_POW_6,
    VDIN_BBAR_REGION_WID_POW_7,
    VDIN_BBAR_REGION_WID_POW_8,
    VDIN_BBAR_REGION_WID_POW_9,
    VDIN_BBAR_REGION_WID_POW_10,
    VDIN_BBAR_REGION_WID_POW_11,
    VDIN_BBAR_REGION_WID_POW_12,
    VDIN_BBAR_REGION_WID_POW_13,
} vdin_bbar_region_wid_pow_t;

typedef enum vdin_bbar_src_e {
    VDIN_BBAR_SRC_Y = 0,
    VDIN_BBAR_SRC_SU, // signed U, SU = U - 512
    VDIN_BBAR_SRC_SV, // signed V, SV = V - 512
    VDIN_BBAR_SRC_U,
    VDIN_BBAR_SRC_V,
} vdin_bbar_src_t;

// ***************************************************************************
// *** structure definitions *********************************************
// ***************************************************************************

typedef struct vdin_clkgate_cfg_s {
    enum vdin_clk_e bbar;
    enum vdin_clk_e hist;
    enum vdin_clk_e lfifo;
    enum vdin_clk_e matrix;
    enum vdin_clk_e hscl;
    enum vdin_clk_e prehscl;
    enum vdin_clk_e top;        // including other parts except meas, which uses seperate lower clk
} vdin_clkgate_cfg_t;

typedef struct vdin_mpeg_cfg_s {
    unsigned int           en;
    enum vdin_mpeg_field_e field;
} vdin_mpeg_cfg_t;

typedef struct vdin_src_mux_cfg_s {
    enum vdin_src_e              src;
    enum vdin_mux_e              mux;
    unsigned int                 go_field_delay;
    unsigned int                 hold_lines;
} vdin_src_mux_cfg_t;

typedef struct vdin_hscl_cfg_s {
    enum vdin_hscl_flt_e            filter;
    unsigned int                    src_w;
    unsigned int                    dst_w;
      signed int                    init_pixi_ptr;
    unsigned int                    prehscl_en;
    unsigned int                    hscl_en;
    unsigned int                    short_lineo_en;
    unsigned int                    hsc_nearest_en;
    unsigned int                    phase0_always_en;
    enum vdin_hscl_flt_depth_e      filter_depth;;
    unsigned int                    hsc_rpt_p0_num;
    unsigned int                    hsc_ini_rcv_num;
    unsigned int                    hsc_ini_phase;
} vdin_hscl_cfg_t;

typedef struct vdin_lfifo_cfg_s {
    unsigned int len;
    unsigned int soft_reset_en;
} vdin_lfifo_cfg_t;

typedef struct vdin_output_cfg_s {
    unsigned int            control;
//    enum vdin_data_format_e data_format;
    enum tvin_color_space_e data_fmt;
    unsigned int            canvas_shadow_en;
    unsigned int            req_urgent;
    unsigned int            req_en;
    unsigned int            canvas_id;
    unsigned int            hstart;
    unsigned int            hend;
    unsigned int            vstart;
    unsigned int            vend;
} vdin_output_cfg_t;

typedef struct vdin_hist_cfg_s {
    unsigned int                pow;
    enum vdin_stat_src_mux_e    mux;
    unsigned int                win_en;
    unsigned int                rd_en;
    unsigned int                hstart;
    unsigned int                hend;
    unsigned int                vstart;
    unsigned int                vend;
} vdin_hist_cfg_t;

typedef struct vdin_bbar_info_s {
    unsigned int valid;
    unsigned int left_hstart;       /* lh_start */
    unsigned int left_hend;         /* lh_end */
    unsigned int right_hstart;      /* rh_start */
    unsigned int right_hend;        /* rh_end */
    unsigned int lleft_blk_pixs;    /* ll_blk_pixs  */
    unsigned int lright_blk_pixs;   /* lr_blk_pixs */
    unsigned int rleft_blk_pixs;    /* rl_blk_pixs */
    unsigned int rright_blk_pixs;   /* rr_blk_pixs */
    unsigned int top;
    unsigned int bottom;
    unsigned int left;
    unsigned int right;
} vdin_bbar_info_t;

typedef struct vdin_bbar_cfg_s {
    unsigned int                        blkpix_thr;
    enum vdin_bbar_region_wid_pow_e     region_wid_pow;
    enum vdin_bbar_src_e                src;
    unsigned int                        stat_en; // For statistics in current regions, otherwise, it runs in continuous mode till the exact edge
    enum vdin_stat_src_mux_e            mux;
    unsigned int                        en;
    unsigned int                        lhstart;
    unsigned int                        rhend;
    unsigned int                        vstart;
    unsigned int                        vend;
    unsigned int                        bbar_thr; // blackpix number thr of blackbar
    unsigned int                        bline_thr_top; // white pix number thr of blackline on top
    unsigned int                        bline_thr_btm; // white pix number thr of blackline on bottom
} vdin_bbar_cfg_t;

typedef struct vdin_stat_s {
    unsigned       min_luma: 8; // VDIN_HIST_MIN[ 7: 0]
    unsigned       max_luma: 8; // VDIN_HIST_MAX[15: 8]
    unsigned       reserved:16; // VDIN_HIST_MAX_MIN[31:16]
    unsigned int   sum_luma   ; // VDIN_HIST_LUMA_SUM_REG
    unsigned int   sum_pixel  ; // VDIN_HIST_PIX_CNT_REG
    unsigned int   sum_chroma ; // VDIN_HIST_CHROMA_SUM_REG
    unsigned short seg[64]    ; // VDIN_HIST00 ~ VDIN_HIST31
} vdin_stat_t;

typedef union vdin_hist_u {
    unsigned int             reg[36];
    struct vdin_stat_s       stat;
} vdin_hist_t;

typedef struct vdin_matrix_lup_s {
    unsigned int pre_offset0_1;
    unsigned int pre_offset2;
    unsigned int coef00_01;
    unsigned int coef02_10;
    unsigned int coef11_12;
    unsigned int coef20_21;
    unsigned int coef22;
    unsigned int post_offset0_1;
    unsigned int post_offset2;
} vdin_matrix_lup_t;


typedef struct vdin_regs_s {
    unsigned int val  : 32;
    unsigned int reg  : 14;
    unsigned int port :  2; // port port_addr            port_data            remark
                        // 0    NA                   NA                   direct access
                        // 1    VPP_CHROMA_ADDR_PORT VPP_CHROMA_DATA_PORT CM port registers
                        // 2    NA                   NA                   reserved
                        // 3    NA                   NA                   reserved
    unsigned int bit  :  5;
    unsigned int wid  :  5;
    unsigned int mode :  1; // 0:read, 1:write
    unsigned int rsv  :  5;
} vdin_regs_t;


// ***************************************************************************
// *** IOCTL command definitions *****************************************
// ***************************************************************************

#define VDIN_IOC_MAGIC 'T'

#define VDIN_IOCS_CLKGATE   _IOW(VDIN_IOC_MAGIC, 0x01, struct vdin_clkgate_cfg_s)
#define VDIN_IOCS_MPEG      _IOW(VDIN_IOC_MAGIC, 0x02, struct vdin_mpeg_cfg_s)
#define VDIN_IOCS_SRC_MUX   _IOW(VDIN_IOC_MAGIC, 0x03, struct vdin_src_mux_cfg_s)
#define VDIN_IOCS_HSCL      _IOW(VDIN_IOC_MAGIC, 0x05, struct vdin_hscl_cfg_s)
#define VDIN_IOCS_MATRIX    _IOW(VDIN_IOC_MAGIC, 0x06, enum   vdin_matrix_csc_e)
#define VDIN_IOCS_LFIFO     _IOW(VDIN_IOC_MAGIC, 0x07, struct vdin_lfifo_cfg_s)
#define VDIN_IOCS_OUTPUT    _IOW(VDIN_IOC_MAGIC, 0x08, struct vdin_output_cfg_s)
#define VDIN_IOCS_HIST      _IOW(VDIN_IOC_MAGIC, 0x09, struct vdin_hist_cfg_s)
#define VDIN_IOCS_BBAR      _IOW(VDIN_IOC_MAGIC, 0x11, struct vdin_bbar_cfg_s)
#define VDIN_IOC_DEBUG      _IOWR(VDIN_IOC_MAGIC, 0x12, unsigned long long)
//#define VDIN_IOC_INIT       _IO(VDIN_IOC_MAGIC, 0x13)
#define VDIN_IOC_START_DEC  _IOW(VDIN_IOC_MAGIC, 0x14, enum tvin_sig_format_e)
#define VDIN_IOC_STOP_DEC   _IO(VDIN_IOC_MAGIC, 0x15)

#endif // __TVIN_VDIN_H

