/*
 * TVIN global definition
 * enum, structure & global parameters used in all TVIN modules.
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


#ifndef __TVIN_GLOBAL_H
#define __TVIN_GLOBAL_H

// ***************************************************************************
// *** enum definitions *********************************************
// ***************************************************************************

#define TVIN_SIG_FMT_NUM  78  //76

typedef enum tvin_sig_format_e {
    TVIN_SIG_FMT_NULL = 0,
	//VGA Formats
    TVIN_SIG_FMT_VGA_512X384P_60D147,
    TVIN_SIG_FMT_VGA_560X384P_60D147,
    TVIN_SIG_FMT_VGA_640X200P_59D924,
    TVIN_SIG_FMT_VGA_640X350P_85D080,
    TVIN_SIG_FMT_VGA_640X400P_59D940,
    TVIN_SIG_FMT_VGA_640X400P_85D080,
    TVIN_SIG_FMT_VGA_640X400P_59D638,
    TVIN_SIG_FMT_VGA_640X400P_56D416,
    TVIN_SIG_FMT_VGA_640X480I_29D970,
    TVIN_SIG_FMT_VGA_640X480P_66D619,
    TVIN_SIG_FMT_VGA_640X480P_66D667,
    TVIN_SIG_FMT_VGA_640X480P_59D940,
    TVIN_SIG_FMT_VGA_640X480P_60D000,
    TVIN_SIG_FMT_VGA_640X480P_72D809,
    TVIN_SIG_FMT_VGA_640X480P_75D000_A,
    TVIN_SIG_FMT_VGA_640X480P_85D008,
    TVIN_SIG_FMT_VGA_640X480P_59D638,
    TVIN_SIG_FMT_VGA_640X480P_75D000_B,
    TVIN_SIG_FMT_VGA_640X870P_75D000,
    TVIN_SIG_FMT_VGA_720X350P_70D086,
    TVIN_SIG_FMT_VGA_720X400P_85D039,
    TVIN_SIG_FMT_VGA_720X400P_70D086,
    TVIN_SIG_FMT_VGA_720X400P_87D849,
    TVIN_SIG_FMT_VGA_720X400P_59D940,
    TVIN_SIG_FMT_VGA_720X480P_59D940,
    TVIN_SIG_FMT_VGA_752X484I_29D970,
    TVIN_SIG_FMT_VGA_768X574I_25D000,
    TVIN_SIG_FMT_VGA_800X600P_56D250,
    TVIN_SIG_FMT_VGA_800X600P_60D317,
    TVIN_SIG_FMT_VGA_800X600P_72D188,
    TVIN_SIG_FMT_VGA_800X600P_75D000,
    TVIN_SIG_FMT_VGA_800X600P_85D061,
    TVIN_SIG_FMT_VGA_832X624P_75D087,
    TVIN_SIG_FMT_VGA_848X480P_84D751,
    TVIN_SIG_FMT_VGA_1024X768P_59D278,
    TVIN_SIG_FMT_VGA_1024X768P_74D927,
    TVIN_SIG_FMT_VGA_1024X768I_43D479,
    TVIN_SIG_FMT_VGA_1024X768P_60D004,
    TVIN_SIG_FMT_VGA_1024X768P_70D069,
    TVIN_SIG_FMT_VGA_1024X768P_75D029,
    TVIN_SIG_FMT_VGA_1024X768P_84D997,
    TVIN_SIG_FMT_VGA_1024X768P_60D000,
    TVIN_SIG_FMT_VGA_1024X768P_74D925,
    TVIN_SIG_FMT_VGA_1024X768P_75D020,
    TVIN_SIG_FMT_VGA_1024X768P_70D008,
    TVIN_SIG_FMT_VGA_1024X768P_75D782,
    TVIN_SIG_FMT_VGA_1024X768P_77D069,
    TVIN_SIG_FMT_VGA_1024X768P_71D799,
    TVIN_SIG_FMT_VGA_1024X1024P_60D000,
    TVIN_SIG_FMT_VGA_1053X754I_43D453,
    TVIN_SIG_FMT_VGA_1056X768I_43D470,
    TVIN_SIG_FMT_VGA_1120X750I_40D021,
    TVIN_SIG_FMT_VGA_1152X864P_70D012,
    TVIN_SIG_FMT_VGA_1152X864P_75D000,
    TVIN_SIG_FMT_VGA_1152X864P_84D999,
    TVIN_SIG_FMT_VGA_1152X870P_75D062,
    TVIN_SIG_FMT_VGA_1152X900P_65D950,
    TVIN_SIG_FMT_VGA_1152X900P_66D004,
    TVIN_SIG_FMT_VGA_1152X900P_76D047,
    TVIN_SIG_FMT_VGA_1152X900P_76D149,
    TVIN_SIG_FMT_VGA_1244X842I_30D000,
    TVIN_SIG_FMT_VGA_1280X768P_59D995,
    TVIN_SIG_FMT_VGA_1280X768P_74D893,
    TVIN_SIG_FMT_VGA_1280X768P_84D837,
    TVIN_SIG_FMT_VGA_1280X960P_60D000,
    TVIN_SIG_FMT_VGA_1280X960P_75D000,
    TVIN_SIG_FMT_VGA_1280X960P_85D002,
    TVIN_SIG_FMT_VGA_1280X1024I_43D436,
    TVIN_SIG_FMT_VGA_1280X1024P_60D020,
    TVIN_SIG_FMT_VGA_1280X1024P_75D025,
    TVIN_SIG_FMT_VGA_1280X1024P_85D024,
    TVIN_SIG_FMT_VGA_1280X1024P_59D979,
    TVIN_SIG_FMT_VGA_1280X1024P_72D005,
    TVIN_SIG_FMT_VGA_1280X1024P_60D002,
    TVIN_SIG_FMT_VGA_1280X1024P_67D003,
    TVIN_SIG_FMT_VGA_1280X1024P_74D112,
    TVIN_SIG_FMT_VGA_1280X1024P_76D179,
    TVIN_SIG_FMT_VGA_1280X1024P_66D718,
    TVIN_SIG_FMT_VGA_1280X1024P_66D677,
    TVIN_SIG_FMT_VGA_1280X1024P_76D107,
    TVIN_SIG_FMT_VGA_1280X1024P_59D996,
    TVIN_SIG_FMT_VGA_1360X768P_59D799,
    TVIN_SIG_FMT_VGA_1360X1024I_51D476,
    TVIN_SIG_FMT_VGA_1440X1080P_60D000,
    TVIN_SIG_FMT_VGA_1600X1200I_48D040,
    TVIN_SIG_FMT_VGA_1600X1200P_60D000,
    TVIN_SIG_FMT_VGA_1600X1200P_65D000,
    TVIN_SIG_FMT_VGA_1600X1200P_70D000,
    TVIN_SIG_FMT_VGA_1600X1200P_75D000,
    TVIN_SIG_FMT_VGA_1600X1200P_80D000,
    TVIN_SIG_FMT_VGA_1600X1200P_85D000,
    TVIN_SIG_FMT_VGA_1600X1280P_66D931,
    TVIN_SIG_FMT_VGA_1680X1080P_60D000,
    TVIN_SIG_FMT_VGA_1792X1344P_60D000,
    TVIN_SIG_FMT_VGA_1792X1344P_74D997,
    TVIN_SIG_FMT_VGA_1856X1392P_59D995,
    TVIN_SIG_FMT_VGA_1856X1392P_75D000,
    TVIN_SIG_FMT_VGA_1868X1200P_75D000,
    TVIN_SIG_FMT_VGA_1920X1080P_60D000,
    TVIN_SIG_FMT_VGA_1920X1080P_75D000,
    TVIN_SIG_FMT_VGA_1920X1080P_85D000,
    TVIN_SIG_FMT_VGA_1920X1200P_84D932,
    TVIN_SIG_FMT_VGA_1920X1200P_75D000,
    TVIN_SIG_FMT_VGA_1920X1200P_85D000,
    TVIN_SIG_FMT_VGA_1920X1234P_75D000,
    TVIN_SIG_FMT_VGA_1920X1234P_85D000,
    TVIN_SIG_FMT_VGA_1920X1440P_60D000,
    TVIN_SIG_FMT_VGA_1920X1440P_75D000,
    TVIN_SIG_FMT_VGA_2048X1536P_60D000_A,
    TVIN_SIG_FMT_VGA_2048X1536P_75D000,
    TVIN_SIG_FMT_VGA_2048X1536P_60D000_B,
    TVIN_SIG_FMT_VGA_2048X2048P_60D008,
    TVIN_SIG_FMT_VGA_MAX,
    //Component Formats
    TVIN_SIG_FMT_COMPONENT_480P_60D000,
    TVIN_SIG_FMT_COMPONENT_480I_59D940,
    TVIN_SIG_FMT_COMPONENT_576P_50D000,
    TVIN_SIG_FMT_COMPONENT_576I_50D000,
    TVIN_SIG_FMT_COMPONENT_720P_59D940,
    TVIN_SIG_FMT_COMPONENT_720P_50D000,
    TVIN_SIG_FMT_COMPONENT_1080P_23D976,
    TVIN_SIG_FMT_COMPONENT_1080P_24D000,
    TVIN_SIG_FMT_COMPONENT_1080P_25D000,
    TVIN_SIG_FMT_COMPONENT_1080P_30D000,
    TVIN_SIG_FMT_COMPONENT_1080P_50D000,
    TVIN_SIG_FMT_COMPONENT_1080P_60D000,
    TVIN_SIG_FMT_COMPONENT_1080I_47D952,
    TVIN_SIG_FMT_COMPONENT_1080I_48D000,
    TVIN_SIG_FMT_COMPONENT_1080I_50D000_A,
    TVIN_SIG_FMT_COMPONENT_1080I_50D000_B,
    TVIN_SIG_FMT_COMPONENT_1080I_50D000_C,
    TVIN_SIG_FMT_COMPONENT_1080I_60D000,
    TVIN_SIG_FMT_COMPONENT_MAX,
    TVIN_SIG_FMT_HDMI_640x480P_60Hz,
    TVIN_SIG_FMT_HDMI_720x480P_60Hz,
    TVIN_SIG_FMT_HDMI_1280x720P_60Hz,
    TVIN_SIG_FMT_HDMI_1920x1080I_60Hz,
    TVIN_SIG_FMT_HDMI_1440x480I_60Hz,
    TVIN_SIG_FMT_HDMI_1440x240P_60Hz,
    TVIN_SIG_FMT_HDMI_2880x480I_60Hz,
    TVIN_SIG_FMT_HDMI_2880x240P_60Hz,
    TVIN_SIG_FMT_HDMI_1440x480P_60Hz,
    TVIN_SIG_FMT_HDMI_1920x1080P_60Hz,
    TVIN_SIG_FMT_HDMI_720x576P_50Hz,
    TVIN_SIG_FMT_HDMI_1280x720P_50Hz,
    TVIN_SIG_FMT_HDMI_1920x1080I_50Hz_A,
    TVIN_SIG_FMT_HDMI_1440x576I_50Hz,
    TVIN_SIG_FMT_HDMI_1440x288P_50Hz,
    TVIN_SIG_FMT_HDMI_2880x576I_50Hz,
    TVIN_SIG_FMT_HDMI_2880x288P_50Hz,
    TVIN_SIG_FMT_HDMI_1440x576P_50Hz,
    TVIN_SIG_FMT_HDMI_1920x1080P_50Hz,
    TVIN_SIG_FMT_HDMI_1920x1080P_24Hz,
    TVIN_SIG_FMT_HDMI_1920x1080P_25Hz,
    TVIN_SIG_FMT_HDMI_1920x1080P_30Hz,
    TVIN_SIG_FMT_HDMI_2880x480P_60Hz,
    TVIN_SIG_FMT_HDMI_2880x576P_60Hz,
    TVIN_SIG_FMT_HDMI_1920x1080I_50Hz_B,
    TVIN_SIG_FMT_HDMI_1920x1080I_100Hz,
    TVIN_SIG_FMT_HDMI_1280x720P_100Hz,
    TVIN_SIG_FMT_HDMI_720x576P_100Hz,
    TVIN_SIG_FMT_HDMI_1440x576I_100Hz,
    TVIN_SIG_FMT_HDMI_1920x1080I_120Hz,
    TVIN_SIG_FMT_HDMI_1280x720P_120Hz,
    TVIN_SIG_FMT_HDMI_720x480P_120Hz,
    TVIN_SIG_FMT_HDMI_1440x480I_120Hz,
    TVIN_SIG_FMT_HDMI_720x576P_200Hz,
    TVIN_SIG_FMT_HDMI_1440x576I_200Hz,
    TVIN_SIG_FMT_HDMI_720x480P_240Hz,
    TVIN_SIG_FMT_HDMI_1440x480I_240Hz,
    TVIN_SIG_FMT_HDMI_1280x720P_24Hz,
    TVIN_SIG_FMT_HDMI_1280x720P_25Hz,
    TVIN_SIG_FMT_HDMI_1280x720P_30Hz,
    TVIN_SIG_FMT_HDMI_1920x1080P_120Hz,
    TVIN_SIG_FMT_HDMI_1920x1080P_100Hz,
    TVIN_SIG_FMT_HDMI_MAX,
    TVIN_SIG_FMT_CVBS_NTSC_M,
    TVIN_SIG_FMT_CVBS_NTSC_443,
    TVIN_SIG_FMT_CVBS_PAL_I,
    TVIN_SIG_FMT_CVBS_PAL_M,
    TVIN_SIG_FMT_CVBS_PAL_60,
    TVIN_SIG_FMT_CVBS_PAL_CN,
    TVIN_SIG_FMT_CVBS_SECAM,
    TVIN_SIG_FMT_MAX,
} tvin_sig_format_t;

typedef enum tvin_sync_pol_e {
	TVIN_SYNC_POL_NULL = 0,
    TVIN_SYNC_POL_NEGATIVE,
    TVIN_SYNC_POL_POSITIVE,
} tvin_sync_pol_t;

typedef enum tvin_scan_mode_e {
	TVIN_SCAN_MODE_NULL = 0,
    TVIN_SCAN_MODE_PROGRESSIVE,
    TVIN_SCAN_MODE_INTERLACED,
} tvin_scan_mode_t;

// ***************************************************************************
// *** structure definitions *********************************************
// ***************************************************************************
//      Hs_cnt        Pixel_Clk(Khz/10)

typedef struct tvin_format_s {
    unsigned short         h_active;        //Th in the unit of pixel
    unsigned short         v_active;        //Tv in the unit of line
    unsigned short         h_cnt;           //Th in the unit of T, while 1/T = 24MHz or 27MHz or even 100MHz
    unsigned short         h_cnt_offset;    //Tolerance of h_cnt
    unsigned short         v_cnt_offset;    //Tolerance of v_cnt
    unsigned short         hs_cnt;          //Ths in the unit of T, while 1/T = 24MHz or 27MHz or even 100MHz
    unsigned short         hs_cnt_offset;   //Tolerance of hs_cnt
    unsigned short         h_total;         //Th in the unit of pixel
    unsigned short         v_total;         //Tv in the unit of line
    unsigned short         hs_front;        //h front proch
    unsigned short         hs_width;        //HS in the unit of pixel
    unsigned short         hs_bp;           //HS in the unit of pixel
    unsigned short         vs_front;        //v front proch
    unsigned short         vs_width;        //VS in the unit of pixel
    unsigned short         vs_bp;           //HS in the unit of pixel
    enum tvin_sync_pol_e   hs_pol;
    enum tvin_sync_pol_e   vs_pol;
    enum tvin_scan_mode_e  scan_mode;
    unsigned short         pixel_clk;       //(Khz/10)
    unsigned short         vbi_line_start;
    unsigned short         vbi_line_end;
} tvin_format_t;

// ***************************************************************************
// *** global parameter definitions *********************************************
// ***************************************************************************

typedef enum tvin_port_select_e {
	TVIN_NONE_CONNECTION = 0,
	TVIN_HDMI1,
	TVIN_HDMI2,
	TVIN_HDMI3,
	TVIN_HDMI4,
	TVIN_HDMI5,
	TVIN_HDMI6,
	TVIN_HDMI7,
	TVIN_HDMI8,
	TVIN_VGA1,
	TVIN_VGA2,
	TVIN_VGA3,
	TVIN_VGA4,
	TVIN_VGA5,
	TVIN_VGA6,
	TVIN_VGA7,
	TVIN_VGA8,
	TVIN_YUV1,		//AVI(analog video input)
	TVIN_YUV2,
	TVIN_YUV3,
	TVIN_YUV4,
	TVIN_YUV5,		//AVI(analog video input)
	TVIN_YUV6,
	TVIN_YUV7,
	TVIN_YUV8,
	TVIN_CVBS1,
	TVIN_CVBS2,
	TVIN_CVBS3,
	TVIN_CVBS4,
	TVIN_CVBS5,
	TVIN_CVBS6,
	TVIN_CVBS7,
	TVIN_CVBS8,
	TVIN_SVIDEO1,
	TVIN_SVIDEO2,
	TVIN_SVIDEO3,
	TVIN_SVIDEO4,
	TVIN_SVIDEO5,
	TVIN_SVIDEO6,
	TVIN_SVIDEO7,
	TVIN_SVIDEO8,
	TVIN_TUNER,
	TVIN_MAX,
} tvin_port_select_t;


typedef enum tvin_color_space_e {
    TVIN_CS_RGB444 = 0,
    TVIN_CS_YUV444,
    TVIN_CS_YUV422_16BITS,
    TVIN_CS_YCbCr422_8BITS,
    TVIN_CS_MAX
} tvin_color_space_t;


typedef enum tvin_video_status_e {
    TVIN_INITIAL = 0,              // formal Unknown
    TVIN_IDLE,                     // no signal
    TVIN_SYNC_SEARCH,              // formal SyncInChange
    TVIN_STANDBY,
    TVIN_NOT_SUPPORTED,            // formal VMNotDetected
    TVIN_FMT_DETECTED,          // formal VMDetected-- wait at least one frame to allow AVI receiving
    TVIN_WAITING_AVI_PLL_LOCK,
    TVIN_WAITING_AVI_OUT_SYNC,     // formal AviDe
    TVIN_VIDEO_ON,			        //
    TVIN_NMB_OF_STATES             // Not a state, but just a limiter. Must be the last at the list
} tvin_video_status_t;

#define VDIN_DEBUG
#ifdef VDIN_DEBUG
#define pr_dbg(fmt, args...) printk(fmt,## args)
#else
#define pr_dbg(fmt, args...)
#endif
#define pr_error(fmt, args...) printk(fmt,## args)


#define VDIN_START_CANVAS               70U
#define BT656IN_VF_POOL_SIZE            8
#define CAMERA_IN_VF_POOL_SIZE          8   //4
#define VDIN_VF_POOL_MAX_SIZE           8

#define BT656IN_ANCI_DATA_SIZE          0x4000 //save anci data from bt656in
#define CAMERA_IN_ANCI_DATA_SIZE        0x4000 //save anci data from bt656in


#endif // __TVIN_GLOBAL_H

