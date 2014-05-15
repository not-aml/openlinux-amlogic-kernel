/*
 * Sli2177 Device Driver
 *
 * Author: Bobby Yang <hui.fang@amlogic.com>
 *
 *
 * Copyright (C) 2010 Amlogic Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#ifndef __SLI2177_FUN_H
#define __SLI2177_FUN_H

#include <linux/amlogic/tvin/tvin.h>
#include "../aml_fe.h"
#define NO_SI2177_ERROR                     0x00
#define ERROR_SI2177_PARAMETER_OUT_OF_RANGE 0x01
#define ERROR_SI2177_ALLOCATING_CONTEXT     0x02
#define ERROR_SI2177_SENDING_COMMAND        0x03
#define ERROR_SI2177_CTS_TIMEOUT            0x04
#define ERROR_SI2177_ERR                    0x05
#define ERROR_SI2177_POLLING_CTS            0x06
#define ERROR_SI2177_POLLING_RESPONSE       0x07
#define ERROR_SI2177_LOADING_FIRMWARE       0x08
#define ERROR_SI2177_LOADING_BOOTBLOCK      0x09
#define ERROR_SI2177_STARTING_FIRMWARE      0x0a
#define ERROR_SI2177_SW_RESET               0x0b
#define ERROR_SI2177_INCOMPATIBLE_PART      0x0c
#define ERROR_SI2177_TUNINT_TIMEOUT         0x0d
#define ERROR_SI2177_xTVINT_TIMEOUT         0x0e
#define ERROR_SI2177_UNKNOWN_COMMAND        0xf0
#define ERROR_SI2177_UNKNOWN_PROPERTY       0xf1


/* status structure definition */
typedef struct { /* si2177_common_reply_struct */
    unsigned char   tunint;
    unsigned char   atvint;
    unsigned char   dtvint;
    unsigned char   err;
    unsigned char   cts;
} si2177_common_reply_struct;

/* _status_defines_insertion_start */
#define SI2177_COMMAND_PROTOTYPES


/* STATUS fields definition */
/* STATUS, TUNINT field definition (size 1, lsb 0, unsigned)*/
#define  SI2177_STATUS_TUNINT_LSB                            0
#define  SI2177_STATUS_TUNINT_MASK                         0x01
#define SI2177_STATUS_TUNINT_NOT_TRIGGERED     0
#define SI2177_STATUS_TUNINT_TRIGGERED              1
/* STATUS, ATVINT field definition (size 1, lsb 1, unsigned)*/
#define  SI2177_STATUS_ATVINT_LSB                            1
#define  SI2177_STATUS_ATVINT_MASK                         0x01
#define SI2177_STATUS_ATVINT_NOT_TRIGGERED     0
#define SI2177_STATUS_ATVINT_TRIGGERED              1
/* STATUS, DTVINT field definition (size 1, lsb 2, unsigned)*/
#define  SI2177_STATUS_DTVINT_LSB                            2
#define  SI2177_STATUS_DTVINT_MASK                         0x01
#define SI2177_STATUS_DTVINT_NOT_TRIGGERED     0
#define SI2177_STATUS_DTVINT_TRIGGERED              1
/* STATUS, ERR field definition (size 1, lsb 6, unsigned)*/
#define  SI2177_STATUS_ERR_LSB                                  6
#define  SI2177_STATUS_ERR_MASK                               0x01
#define SI2177_STATUS_ERR_ERROR                            1
#define SI2177_STATUS_ERR_NO_ERROR                     0
/* STATUS, CTS field definition (size 1, lsb 7, unsigned)*/
#define  SI2177_STATUS_CTS_LSB                                   7
#define  SI2177_STATUS_CTS_MASK                               0x01
#define SI2177_STATUS_CTS_COMPLETED                   1
#define SI2177_STATUS_CTS_WAIT                                0

/* _status_defines_insertion_point */

/* _commands_defines_insertion_start */
/* SI2177_AGC_OVERRIDE command definition */
#define SI2177_AGC_OVERRIDE_CMD                               0x44

#ifdef    SI2177_AGC_OVERRIDE_CMD

typedef struct { /* SI2177_AGC_OVERRIDE_CMD_struct */
     unsigned char   force_max_gain;
     unsigned char   force_top_gain;
} si2177_agc_override_cmd_struct;

/* AGC_OVERRIDE command, FORCE_MAX_GAIN field definition (size 1, lsb 0, unsigned) */
#define  SI2177_AGC_OVERRIDE_CMD_FORCE_MAX_GAIN_LSB          0
#define  SI2177_AGC_OVERRIDE_CMD_FORCE_MAX_GAIN_MASK       0x01
#define  SI2177_AGC_OVERRIDE_CMD_FORCE_MAX_GAIN_MIN           0
#define  SI2177_AGC_OVERRIDE_CMD_FORCE_MAX_GAIN_MAX          1
#define SI2177_AGC_OVERRIDE_CMD_FORCE_MAX_GAIN_DISABLE  0
#define SI2177_AGC_OVERRIDE_CMD_FORCE_MAX_GAIN_ENABLE   1
/* AGC_OVERRIDE command, FORCE_TOP_GAIN field definition (size 1, lsb 1, unsigned) */
#define  SI2177_AGC_OVERRIDE_CMD_FORCE_TOP_GAIN_LSB           1
#define  SI2177_AGC_OVERRIDE_CMD_FORCE_TOP_GAIN_MASK        0x01
#define  SI2177_AGC_OVERRIDE_CMD_FORCE_TOP_GAIN_MIN           0
#define  SI2177_AGC_OVERRIDE_CMD_FORCE_TOP_GAIN_MAX          1
#define SI2177_AGC_OVERRIDE_CMD_FORCE_TOP_GAIN_DISABLE  0
#define SI2177_AGC_OVERRIDE_CMD_FORCE_TOP_GAIN_ENABLE   1

typedef struct { /* SI2177_AGC_OVERRIDE_CMD_REPLY_struct */
       si2177_common_reply_struct * status;
}  si2177_agc_override_cmd_reply_struct;

#endif /* SI2177_AGC_OVERRIDE_CMD */

/* SI2177_ATV_CW_TEST command definition */
//#define SI2177_ATV_CW_TEST_CMD 0x53

#ifdef    SI2177_ATV_CW_TEST_CMD

typedef struct { /* SI2177_ATV_CW_TEST_CMD_struct */
     unsigned char   pc_lock;
} si2177_atv_cw_test_cmd_struct;

/* ATV_CW_TEST command, PC_LOCK field definition (size 1, lsb 0, unsigned) */
#define  SI2177_ATV_CW_TEST_CMD_PC_LOCK_LSB          0
#define  SI2177_ATV_CW_TEST_CMD_PC_LOCK_MASK       0x01
#define  SI2177_ATV_CW_TEST_CMD_PC_LOCK_MIN          0
#define  SI2177_ATV_CW_TEST_CMD_PC_LOCK_MAX         1
#define SI2177_ATV_CW_TEST_CMD_PC_LOCK_LOCK       1
#define SI2177_ATV_CW_TEST_CMD_PC_LOCK_UNLOCK  0

typedef struct { /* SI2177_ATV_CW_TEST_CMD_REPLY_struct */
       si2177_common_reply_struct * status;
}  si2177_atv_cw_test_cmd_reply_struct;

#endif /* SI2177_ATV_CW_TEST_CMD */

/* SI2177_ATV_RESTART command definition */
#define SI2177_ATV_RESTART_CMD                                         0x51

#ifdef    SI2177_ATV_RESTART_CMD

typedef struct { /* SI2177_ATV_RESTART_CMD_struct */
     unsigned char   mode;
} si2177_atv_restart_cmd_struct;

/* ATV_RESTART command, MODE field definition (size 1, lsb 0, unsigned) */
#define  SI2177_ATV_RESTART_CMD_MODE_LSB                     0
#define  SI2177_ATV_RESTART_CMD_MODE_MASK                  0x01
#define  SI2177_ATV_RESTART_CMD_MODE_MIN                      0
#define  SI2177_ATV_RESTART_CMD_MODE_MAX                     1
#define SI2177_ATV_RESTART_CMD_MODE_AUDIO_ONLY      1
#define SI2177_ATV_RESTART_CMD_MODE_AUDIO_VIDEO    0

typedef struct { /* SI2177_ATV_RESTART_CMD_REPLY_struct */
       si2177_common_reply_struct * status;
}  si2177_atv_restart_cmd_reply_struct;

#endif /* SI2177_ATV_RESTART_CMD */

/* SI2177_ATV_STATUS command definition */
#define SI2177_ATV_STATUS_CMD                                                  0x52

#ifdef    SI2177_ATV_STATUS_CMD

typedef struct { /* SI2177_ATV_STATUS_CMD_struct */
     unsigned char   intack;
} si2177_atv_status_cmd_struct;

/* ATV_STATUS command, INTACK field definition (size 1, lsb 0, unsigned) */
#define  SI2177_ATV_STATUS_CMD_INTACK_LSB                    0
#define  SI2177_ATV_STATUS_CMD_INTACK_MASK                 0x01
#define  SI2177_ATV_STATUS_CMD_INTACK_MIN                    0
#define  SI2177_ATV_STATUS_CMD_INTACK_MAX                   1
#define SI2177_ATV_STATUS_CMD_INTACK_CLEAR               1
#define SI2177_ATV_STATUS_CMD_INTACK_OK                      0

typedef struct { /* SI2177_ATV_STATUS_CMD_REPLY_struct */
       si2177_common_reply_struct * status;
      unsigned char   chlint;
      unsigned char   pclint;
      unsigned char   dlint;
      unsigned char   snrlint;
      unsigned char   snrhint;
      unsigned char   audio_chan_bw;
      unsigned char   chl;
      unsigned char   pcl;
      unsigned char   dl;
      unsigned char   snrl;
      unsigned char   snrh;
      unsigned char   video_snr;
               int             afc_freq;
               int             video_sc_spacing;
      unsigned char   video_sys;
      unsigned char   color;
      unsigned char   lines;
      unsigned char   audio_sys;
      unsigned char   audio_demod_mode;
      unsigned char   sound_level;
      unsigned char   resrved;
}  si2177_atv_status_cmd_reply_struct;
/* ATV_STATUS command, CHLINT field definition (size 1, lsb 0, unsigned)*/
#define  SI2177_ATV_STATUS_RESPONSE_CHLINT_LSB                  0
#define  SI2177_ATV_STATUS_RESPONSE_CHLINT_MASK               0x01
#define SI2177_ATV_STATUS_RESPONSE_CHLINT_CHANGED       1
#define SI2177_ATV_STATUS_RESPONSE_CHLINT_NO_CHANGE   0
/* ATV_STATUS command, PCLINT field definition (size 1, lsb 1, unsigned)*/
#define  SI2177_ATV_STATUS_RESPONSE_PCLINT_LSB                   1
#define  SI2177_ATV_STATUS_RESPONSE_PCLINT_MASK                0x01
#define SI2177_ATV_STATUS_RESPONSE_PCLINT_CHANGED        1
#define SI2177_ATV_STATUS_RESPONSE_PCLINT_NO_CHANGE    0
/* ATV_STATUS command, DLINT field definition (size 1, lsb 2, unsigned)*/
#define  SI2177_ATV_STATUS_RESPONSE_DLINT_LSB                      2
#define  SI2177_ATV_STATUS_RESPONSE_DLINT_MASK                   0x01
#define SI2177_ATV_STATUS_RESPONSE_DLINT_CHANGED           1
#define SI2177_ATV_STATUS_RESPONSE_DLINT_NO_CHANGE       0
/* ATV_STATUS command, SNRLINT field definition (size 1, lsb 3, unsigned)*/
#define  SI2177_ATV_STATUS_RESPONSE_SNRLINT_LSB                 3
#define  SI2177_ATV_STATUS_RESPONSE_SNRLINT_MASK              0x01
#define SI2177_ATV_STATUS_RESPONSE_SNRLINT_CHANGED      1
#define SI2177_ATV_STATUS_RESPONSE_SNRLINT_NO_CHANGE  0
/* ATV_STATUS command, SNRHINT field definition (size 1, lsb 4, unsigned)*/
#define  SI2177_ATV_STATUS_RESPONSE_SNRHINT_LSB                 4
#define  SI2177_ATV_STATUS_RESPONSE_SNRHINT_MASK              0x01
#define SI2177_ATV_STATUS_RESPONSE_SNRHINT_CHANGED      1
#define SI2177_ATV_STATUS_RESPONSE_SNRHINT_NO_CHANGE  0
/* ATV_STATUS command, AUDIO_CHAN_BW field definition (size 4, lsb 0, unsigned)*/
#define  SI2177_ATV_STATUS_RESPONSE_AUDIO_CHAN_BW_LSB                         0
#define  SI2177_ATV_STATUS_RESPONSE_AUDIO_CHAN_BW_MASK                      0x0f
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_CHAN_BW_12X_OVERMOD      3
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_CHAN_BW_150_KHZ_OFFSET  8
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_CHAN_BW_15_KHZ_OFFSET    5
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_CHAN_BW_30_KHZ_OFFSET    6
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_CHAN_BW_4X_OVERMOD         1
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_CHAN_BW_75_KHZ_OFFSET     7
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_CHAN_BW_7P5_KHZ_OFFSET  4
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_CHAN_BW_8X_OVERMOD         2
    #define SI2177_ATV_STATUS_RESPONSE_AUDIO_CHAN_BW_CUSTOM          9
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_CHAN_BW_RSVD                       0
   #define  SI2177_ATV_STATUS_RESPONSE_SOUND_LEVEL_LSB         0
   #define  SI2177_ATV_STATUS_RESPONSE_SOUND_LEVEL_MASK        0xff
   #define  SI2177_ATV_STATUS_RESPONSE_SOUND_LEVEL_SHIFT       24
/* ATV_STATUS command, CHL field definition (size 1, lsb 0, unsigned)*/
#define  SI2177_ATV_STATUS_RESPONSE_CHL_LSB                                            0
#define  SI2177_ATV_STATUS_RESPONSE_CHL_MASK                                         0x01
#define SI2177_ATV_STATUS_RESPONSE_CHL_CHANNEL                                 1
#define SI2177_ATV_STATUS_RESPONSE_CHL_NO_CHANNEL                          0
/* ATV_STATUS command, PCL field definition (size 1, lsb 1, unsigned)*/
#define  SI2177_ATV_STATUS_RESPONSE_PCL_LSB                                            1
#define  SI2177_ATV_STATUS_RESPONSE_PCL_MASK                                         0x01
#define SI2177_ATV_STATUS_RESPONSE_PCL_LOCKED                                    1
#define SI2177_ATV_STATUS_RESPONSE_PCL_NO_LOCK                                  0
/* ATV_STATUS command, DL field definition (size 1, lsb 2, unsigned)*/
#define  SI2177_ATV_STATUS_RESPONSE_DL_LSB                                              2
#define  SI2177_ATV_STATUS_RESPONSE_DL_MASK                                           0x01
#define SI2177_ATV_STATUS_RESPONSE_DL_LOCKED                                       1
#define SI2177_ATV_STATUS_RESPONSE_DL_NO_LOCK                                     0
/* ATV_STATUS command, SNRL field definition (size 1, lsb 3, unsigned)*/
#define  SI2177_ATV_STATUS_RESPONSE_SNRL_LSB                                         3
#define  SI2177_ATV_STATUS_RESPONSE_SNRL_MASK                                      0x01
#define SI2177_ATV_STATUS_RESPONSE_SNRL_SNR_LOW                               1
#define SI2177_ATV_STATUS_RESPONSE_SNRL_SNR_NOT_LOW                      0
/* ATV_STATUS command, SNRH field definition (size 1, lsb 4, unsigned)*/
#define  SI2177_ATV_STATUS_RESPONSE_SNRH_LSB                                         4
#define  SI2177_ATV_STATUS_RESPONSE_SNRH_MASK                                      0x01
#define SI2177_ATV_STATUS_RESPONSE_SNRH_SNR_HIGH                             1
#define SI2177_ATV_STATUS_RESPONSE_SNRH_SNR_NOT_HIGH                    0
/* ATV_STATUS command, VIDEO_SNR field definition (size 8, lsb 0, unsigned)*/
#define  SI2177_ATV_STATUS_RESPONSE_VIDEO_SNR_LSB                               0
#define  SI2177_ATV_STATUS_RESPONSE_VIDEO_SNR_MASK                            0xff
/* ATV_STATUS command, AFC_FREQ field definition (size 16, lsb 0, signed)*/
#define  SI2177_ATV_STATUS_RESPONSE_AFC_FREQ_LSB                                 0
#define  SI2177_ATV_STATUS_RESPONSE_AFC_FREQ_MASK                              0xffff
#define  SI2177_ATV_STATUS_RESPONSE_AFC_FREQ_SHIFT                             16
/* ATV_STATUS command, VIDEO_SC_SPACING field definition (size 16, lsb 0, signed)*/
#define  SI2177_ATV_STATUS_RESPONSE_VIDEO_SC_SPACING_LSB                0
#define  SI2177_ATV_STATUS_RESPONSE_VIDEO_SC_SPACING_MASK             0xffff
#define  SI2177_ATV_STATUS_RESPONSE_VIDEO_SC_SPACING_SHIFT            16
/* ATV_STATUS command, VIDEO_SYS field definition (size 3, lsb 0, unsigned)*/
#define  SI2177_ATV_STATUS_RESPONSE_VIDEO_SYS_LSB                                0
#define  SI2177_ATV_STATUS_RESPONSE_VIDEO_SYS_MASK                             0x07
#define SI2177_ATV_STATUS_RESPONSE_VIDEO_SYS_B                                     0
#define SI2177_ATV_STATUS_RESPONSE_VIDEO_SYS_DK                                  5
#define SI2177_ATV_STATUS_RESPONSE_VIDEO_SYS_GH                                  1
#define SI2177_ATV_STATUS_RESPONSE_VIDEO_SYS_I                                      4
#define SI2177_ATV_STATUS_RESPONSE_VIDEO_SYS_L                                     6
#define SI2177_ATV_STATUS_RESPONSE_VIDEO_SYS_LP                                   7
#define SI2177_ATV_STATUS_RESPONSE_VIDEO_SYS_M                                     2
#define SI2177_ATV_STATUS_RESPONSE_VIDEO_SYS_N                                     3
/* ATV_STATUS command, COLOR field definition (size 1, lsb 4, unsigned)*/
#define  SI2177_ATV_STATUS_RESPONSE_COLOR_LSB                                        4
#define  SI2177_ATV_STATUS_RESPONSE_COLOR_MASK                                     0x01
#define SI2177_ATV_STATUS_RESPONSE_COLOR_PAL_NTSC                             0
#define SI2177_ATV_STATUS_RESPONSE_COLOR_SECAM                                   1
/* ATV_STATUS command, TRANS field definition (size 1, lsb 6, unsigned)*/
   #define  SI2177_ATV_STATUS_RESPONSE_LINES_LSB         7
   #define  SI2177_ATV_STATUS_RESPONSE_LINES_MASK        0x01
    #define SI2177_ATV_STATUS_RESPONSE_LINES_525  0
    #define SI2177_ATV_STATUS_RESPONSE_LINES_625  1
/* ATV_STATUS command, AUDIO_SYS field definition (size 4, lsb 0, unsigned)*/
#define  SI2177_ATV_STATUS_RESPONSE_AUDIO_SYS_LSB                                  0
#define  SI2177_ATV_STATUS_RESPONSE_AUDIO_SYS_MASK                               0x0f
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_SYS_A2                                     3
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_SYS_A2_DK2                            4
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_SYS_A2_DK3                            5
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_SYS_A2_DK4                            9
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_SYS_BTSC                                6
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_SYS_EIAJ                                  7
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_SYS_MONO                               1
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_SYS_MONO_NICAM                  2
    #define SI2177_ATV_STATUS_RESPONSE_AUDIO_SYS_MONO_NICAM_10DB  12
    #define SI2177_ATV_STATUS_RESPONSE_AUDIO_SYS_MONO_NICAM_6DB   11
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_SYS_RSVD                                0
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_SYS_SCAN                                8
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_SYS_WIDE_SCAN                     10
/* ATV_STATUS command, AUDIO_DEMOD_MODE field definition (size 2, lsb 4, unsigned)*/
#define  SI2177_ATV_STATUS_RESPONSE_AUDIO_DEMOD_MODE_LSB                 4
#define  SI2177_ATV_STATUS_RESPONSE_AUDIO_DEMOD_MODE_MASK              0x03
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_DEMOD_MODE_AM                   1
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_DEMOD_MODE_FM1                 2
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_DEMOD_MODE_FM2                 3
#define SI2177_ATV_STATUS_RESPONSE_AUDIO_DEMOD_MODE_SIF                  0

#endif /* SI2177_ATV_STATUS_CMD */

/* SI2177_CONFIG_CLOCKS command definition */
#define   SI2177_CONFIG_CLOCKS_CMD 0xc0

#ifdef    SI2177_CONFIG_CLOCKS_CMD
  #define SI2177_CONFIG_CLOCKS_CMD_CODE 0x0100c0

    typedef struct { /* SI2177_CONFIG_CLOCKS_CMD_struct */
     unsigned char   subcode;
     unsigned char   clock_mode;
     unsigned char   en_xout;
   } si2177_config_clocks_cmd_struct;


    typedef struct { /* SI2177_CONFIG_CLOCKS_CMD_REPLY_struct */
       si2177_common_reply_struct * STATUS;
   }  SI2177_CONFIG_CLOCKS_CMD_REPLY_struct;

   /* CONFIG_CLOCKS command, SUBCODE field definition (address 1,size 8, lsb 0, unsigned) */
   #define  SI2177_CONFIG_CLOCKS_CMD_SUBCODE_LSB         0
   #define  SI2177_CONFIG_CLOCKS_CMD_SUBCODE_MASK        0xff
   #define  SI2177_CONFIG_CLOCKS_CMD_SUBCODE_MIN         0
   #define  SI2177_CONFIG_CLOCKS_CMD_SUBCODE_MAX         0
    #define SI2177_CONFIG_CLOCKS_CMD_SUBCODE_CODE  0
   /* CONFIG_CLOCKS command, CLOCK_MODE field definition (address 2,size 2, lsb 0, unsigned) */
   #define  SI2177_CONFIG_CLOCKS_CMD_CLOCK_MODE_LSB         0
   #define  SI2177_CONFIG_CLOCKS_CMD_CLOCK_MODE_MASK        0x03
   #define  SI2177_CONFIG_CLOCKS_CMD_CLOCK_MODE_MIN         0
   #define  SI2177_CONFIG_CLOCKS_CMD_CLOCK_MODE_MAX         2
    #define SI2177_CONFIG_CLOCKS_CMD_CLOCK_MODE_EXTCLK  2
    #define SI2177_CONFIG_CLOCKS_CMD_CLOCK_MODE_XTAL    0
   /* CONFIG_CLOCKS command, EN_XOUT field definition (address 2,size 3, lsb 2, unsigned) */
   #define  SI2177_CONFIG_CLOCKS_CMD_EN_XOUT_LSB         2
   #define  SI2177_CONFIG_CLOCKS_CMD_EN_XOUT_MASK        0x07
   #define  SI2177_CONFIG_CLOCKS_CMD_EN_XOUT_MIN         0
   #define  SI2177_CONFIG_CLOCKS_CMD_EN_XOUT_MAX         3
    #define SI2177_CONFIG_CLOCKS_CMD_EN_XOUT_DIS_XOUT  0
    #define SI2177_CONFIG_CLOCKS_CMD_EN_XOUT_EN_XOUT   3
#endif /* SI2177_CONFIG_CLOCKS_CMD */

/* SI2177_CONFIG_PINS command definition */
#define   SI2177_CONFIG_PINS_CMD 0x12

#ifdef    SI2177_CONFIG_PINS_CMD

typedef struct { /* SI2177_CONFIG_PINS_CMD_struct */
     unsigned char   gpio1_mode;
     unsigned char   gpio1_read;
     unsigned char   gpio2_mode;
     unsigned char   gpio2_read;
     unsigned char   reserved1;
     unsigned char   reserved2;
     unsigned char   reserved3;
   } si2177_config_pins_cmd_struct;


    typedef struct { /* SI2177_CONFIG_PINS_CMD_REPLY_struct */
      si2177_common_reply_struct * STATUS;
      unsigned char   gpio1_mode;
      unsigned char   gpio1_state;
      unsigned char   gpio2_mode;
      unsigned char   gpio2_state;
      unsigned char   reserved1;
      unsigned char   reserved2;
      unsigned char   reserved3;
   }  si2177_config_pins_cmd_reply_struct;

   /* CONFIG_PINS command, GPIO1_MODE field definition (address 1,size 7, lsb 0, unsigned) */
   #define  SI2177_CONFIG_PINS_CMD_GPIO1_MODE_LSB         0
   #define  SI2177_CONFIG_PINS_CMD_GPIO1_MODE_MASK        0x7f
   #define  SI2177_CONFIG_PINS_CMD_GPIO1_MODE_MIN         0
   #define  SI2177_CONFIG_PINS_CMD_GPIO1_MODE_MAX         3
    #define SI2177_CONFIG_PINS_CMD_GPIO1_MODE_DISABLE    1
    #define SI2177_CONFIG_PINS_CMD_GPIO1_MODE_DRIVE_0    2
    #define SI2177_CONFIG_PINS_CMD_GPIO1_MODE_DRIVE_1    3
    #define SI2177_CONFIG_PINS_CMD_GPIO1_MODE_NO_CHANGE  0
   /* CONFIG_PINS command, GPIO1_READ field definition (address 1,size 1, lsb 7, unsigned) */
   #define  SI2177_CONFIG_PINS_CMD_GPIO1_READ_LSB         7
   #define  SI2177_CONFIG_PINS_CMD_GPIO1_READ_MASK        0x01
   #define  SI2177_CONFIG_PINS_CMD_GPIO1_READ_MIN         0
   #define  SI2177_CONFIG_PINS_CMD_GPIO1_READ_MAX         1
    #define SI2177_CONFIG_PINS_CMD_GPIO1_READ_DO_NOT_READ  0
    #define SI2177_CONFIG_PINS_CMD_GPIO1_READ_READ         1
   /* CONFIG_PINS command, GPIO2_MODE field definition (address 2,size 7, lsb 0, unsigned) */
   #define  SI2177_CONFIG_PINS_CMD_GPIO2_MODE_LSB         0
   #define  SI2177_CONFIG_PINS_CMD_GPIO2_MODE_MASK        0x7f
   #define  SI2177_CONFIG_PINS_CMD_GPIO2_MODE_MIN         0
   #define  SI2177_CONFIG_PINS_CMD_GPIO2_MODE_MAX         3
    #define SI2177_CONFIG_PINS_CMD_GPIO2_MODE_DISABLE    1
    #define SI2177_CONFIG_PINS_CMD_GPIO2_MODE_DRIVE_0    2
    #define SI2177_CONFIG_PINS_CMD_GPIO2_MODE_DRIVE_1    3
    #define SI2177_CONFIG_PINS_CMD_GPIO2_MODE_NO_CHANGE  0
   /* CONFIG_PINS command, GPIO2_READ field definition (address 2,size 1, lsb 7, unsigned) */
   #define  SI2177_CONFIG_PINS_CMD_GPIO2_READ_LSB         7
   #define  SI2177_CONFIG_PINS_CMD_GPIO2_READ_MASK        0x01
   #define  SI2177_CONFIG_PINS_CMD_GPIO2_READ_MIN         0
   #define  SI2177_CONFIG_PINS_CMD_GPIO2_READ_MAX         1
    #define SI2177_CONFIG_PINS_CMD_GPIO2_READ_DO_NOT_READ  0
    #define SI2177_CONFIG_PINS_CMD_GPIO2_READ_READ         1
   /* CONFIG_PINS command, RESERVED1 field definition (address 3,size 8, lsb 0, unsigned) */
   #define  SI2177_CONFIG_PINS_CMD_RESERVED1_LSB         0
   #define  SI2177_CONFIG_PINS_CMD_RESERVED1_MASK        0xff
   #define  SI2177_CONFIG_PINS_CMD_RESERVED1_MIN         1
   #define  SI2177_CONFIG_PINS_CMD_RESERVED1_MAX         1
    #define SI2177_CONFIG_PINS_CMD_RESERVED1_RESERVED  1
   /* CONFIG_PINS command, RESERVED2 field definition (address 4,size 8, lsb 0, unsigned) */
   #define  SI2177_CONFIG_PINS_CMD_RESERVED2_LSB         0
   #define  SI2177_CONFIG_PINS_CMD_RESERVED2_MASK        0xff
   #define  SI2177_CONFIG_PINS_CMD_RESERVED2_MIN         1
   #define  SI2177_CONFIG_PINS_CMD_RESERVED2_MAX         1
    #define SI2177_CONFIG_PINS_CMD_RESERVED2_RESERVED  1
   /* CONFIG_PINS command, RESERVED3 field definition (address 5,size 8, lsb 0, unsigned) */
   #define  SI2177_CONFIG_PINS_CMD_RESERVED3_LSB         0
   #define  SI2177_CONFIG_PINS_CMD_RESERVED3_MASK        0xff
   #define  SI2177_CONFIG_PINS_CMD_RESERVED3_MIN         1
   #define  SI2177_CONFIG_PINS_CMD_RESERVED3_MAX         1
    #define SI2177_CONFIG_PINS_CMD_RESERVED3_RESERVED  1
   /* CONFIG_PINS command, GPIO1_MODE field definition (address 1, size 7, lsb 0, unsigned)*/
   #define  SI2177_CONFIG_PINS_RESPONSE_GPIO1_MODE_LSB         0
   #define  SI2177_CONFIG_PINS_RESPONSE_GPIO1_MODE_MASK        0x7f
    #define SI2177_CONFIG_PINS_RESPONSE_GPIO1_MODE_DISABLE  1
    #define SI2177_CONFIG_PINS_RESPONSE_GPIO1_MODE_DRIVE_0  2
    #define SI2177_CONFIG_PINS_RESPONSE_GPIO1_MODE_DRIVE_1  3
   /* CONFIG_PINS command, GPIO1_STATE field definition (address 1, size 1, lsb 7, unsigned)*/
   #define  SI2177_CONFIG_PINS_RESPONSE_GPIO1_STATE_LSB         7
   #define  SI2177_CONFIG_PINS_RESPONSE_GPIO1_STATE_MASK        0x01
    #define SI2177_CONFIG_PINS_RESPONSE_GPIO1_STATE_READ_0  0
    #define SI2177_CONFIG_PINS_RESPONSE_GPIO1_STATE_READ_1  1
   /* CONFIG_PINS command, GPIO2_MODE field definition (address 2, size 7, lsb 0, unsigned)*/
   #define  SI2177_CONFIG_PINS_RESPONSE_GPIO2_MODE_LSB         0
   #define  SI2177_CONFIG_PINS_RESPONSE_GPIO2_MODE_MASK        0x7f
    #define SI2177_CONFIG_PINS_RESPONSE_GPIO2_MODE_DISABLE  1
    #define SI2177_CONFIG_PINS_RESPONSE_GPIO2_MODE_DRIVE_0  2
    #define SI2177_CONFIG_PINS_RESPONSE_GPIO2_MODE_DRIVE_1  3
   /* CONFIG_PINS command, GPIO2_STATE field definition (address 2, size 1, lsb 7, unsigned)*/
   #define  SI2177_CONFIG_PINS_RESPONSE_GPIO2_STATE_LSB         7
   #define  SI2177_CONFIG_PINS_RESPONSE_GPIO2_STATE_MASK        0x01
    #define SI2177_CONFIG_PINS_RESPONSE_GPIO2_STATE_READ_0  0
    #define SI2177_CONFIG_PINS_RESPONSE_GPIO2_STATE_READ_1  1
   /* CONFIG_PINS command, RESERVED1 field definition (address 3, size 8, lsb 0, unsigned)*/
   #define  SI2177_CONFIG_PINS_RESPONSE_RESERVED1_LSB         0
   #define  SI2177_CONFIG_PINS_RESPONSE_RESERVED1_MASK        0xff
    #define SI2177_CONFIG_PINS_RESPONSE_RESERVED1_RESERVED1_MIN  1
    #define SI2177_CONFIG_PINS_RESPONSE_RESERVED1_RESERVED1_MAX  1
   /* CONFIG_PINS command, RESERVED2 field definition (address 4, size 8, lsb 0, unsigned)*/
   #define  SI2177_CONFIG_PINS_RESPONSE_RESERVED2_LSB         0
   #define  SI2177_CONFIG_PINS_RESPONSE_RESERVED2_MASK        0xff
    #define SI2177_CONFIG_PINS_RESPONSE_RESERVED2_RESERVED2_MIN  1
    #define SI2177_CONFIG_PINS_RESPONSE_RESERVED2_RESERVED2_MAX  1
   /* CONFIG_PINS command, RESERVED3 field definition (address 5, size 8, lsb 0, unsigned)*/
   #define  SI2177_CONFIG_PINS_RESPONSE_RESERVED3_LSB         0
   #define  SI2177_CONFIG_PINS_RESPONSE_RESERVED3_MASK        0xff
    #define SI2177_CONFIG_PINS_RESPONSE_RESERVED3_RESERVED3_MIN  1
    #define SI2177_CONFIG_PINS_RESPONSE_RESERVED3_RESERVED3_MAX  1


#endif /* SI2177_CONFIG_PINS_CMD */

/* SI2177_DOWNLOAD_DATASET_CONTINUE command definition */
//#define SI2177_DOWNLOAD_DATASET_CONTINUE_CMD 0xb9

#ifdef    SI2177_DOWNLOAD_DATASET_CONTINUE_CMD

typedef struct { /* SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_struct */
     unsigned char   data0;
     unsigned char   data1;
     unsigned char   data2;
     unsigned char   data3;
     unsigned char   data4;
     unsigned char   data5;
     unsigned char   data6;
} si2177_download_dataset_continue_cmd_struct;

/* DOWNLOAD_DATASET_CONTINUE command, DATA0 field definition (size 8, lsb 0, unsigned) */
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA0_LSB               0
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA0_MASK            0xff
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA0_MIN               0
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA0_MAX              255
#define SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA0_DATA0_MIN  0
#define SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA0_DATA0_MAX 255
/* DOWNLOAD_DATASET_CONTINUE command, DATA1 field definition (size 8, lsb 0, unsigned) */
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA1_LSB               0
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA1_MASK            0xff
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA1_MIN               0
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA1_MAX              255
#define SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA1_DATA1_MIN  0
#define SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA1_DATA1_MAX 255
/* DOWNLOAD_DATASET_CONTINUE command, DATA2 field definition (size 8, lsb 0, unsigned) */
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA2_LSB               0
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA2_MASK            0xff
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA2_MIN               0
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA2_MAX              255
#define SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA2_DATA2_MIN  0
#define SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA2_DATA2_MAX 255
/* DOWNLOAD_DATASET_CONTINUE command, DATA3 field definition (size 8, lsb 0, unsigned) */
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA3_LSB               0
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA3_MASK            0xff
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA3_MIN                0
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA3_MAX              255
#define SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA3_DATA3_MIN   0
#define SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA3_DATA3_MAX  255
/* DOWNLOAD_DATASET_CONTINUE command, DATA4 field definition (size 8, lsb 0, unsigned) */
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA4_LSB                0
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA4_MASK             0xff
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA4_MIN                0
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA4_MAX               255
#define SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA4_DATA4_MIN   0
#define SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA4_DATA4_MAX  255
/* DOWNLOAD_DATASET_CONTINUE command, DATA5 field definition (size 8, lsb 0, unsigned) */
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA5_LSB                0
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA5_MASK             0xff
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA5_MIN                 0
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA5_MAX                255
#define SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA5_DATA5_MIN    0
#define SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA5_DATA5_MAX   255
/* DOWNLOAD_DATASET_CONTINUE command, DATA6 field definition (size 8, lsb 0, unsigned) */
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA6_LSB                 0
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA6_MASK              0xff
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA6_MIN                  0
#define  SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA6_MAX                 255
#define SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA6_DATA6_MIN     0
#define SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_DATA6_DATA6_MAX    255

typedef struct { /* SI2177_DOWNLOAD_DATASET_CONTINUE_CMD_REPLY_struct */
       si2177_common_reply_struct * status;
}  si2177_download_dataset_continue_cmd_reply_struct;

#endif /* SI2177_DOWNLOAD_DATASET_CONTINUE_CMD */

/* SI2177_DOWNLOAD_DATASET_START command definition */
//#define SI2177_DOWNLOAD_DATASET_START_CMD 0xb8

#ifdef    SI2177_DOWNLOAD_DATASET_START_CMD

typedef struct { /* SI2177_DOWNLOAD_DATASET_START_CMD_struct */
     unsigned char   dataset_id;
     unsigned char   dataset_checksum;
     unsigned char   data0;
     unsigned char   data1;
     unsigned char   data2;
     unsigned char   data3;
     unsigned char   data4;
} si2177_download_dataset_start_cmd_struct;
/* DOWNLOAD_DATASET_START command, DATASET_ID field definition (size 8, lsb 0, unsigned) */
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_LSB                        0
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_MASK                     0xff
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_MIN                         0
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_MAX                        29
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_APFBBYPASS_B     6
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_APFBBYPASS_DK   7
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_APFBBYPASS_G     8
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_APFBBYPASS_I       9
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_APFBBYPASS_L      10
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_APFBBYPASS_M     11
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_APFBNOTCH_B       12
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_APFBNOTCH_DK     13
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_APFBNOTCH_G       14
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_APFBNOTCH_I        15
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_APFBNOTCH_L       16
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_APFBNOTCH_M      17
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_APFB_LPF_B           21
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_APFB_LPF_DK        22
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_APFB_LPF_G           23
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_APFB_LPF_I            24
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_APFB_LPF_L           25
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_APFB_LPF_M          26
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_VIDEOFILT_ALIF_6  27
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_VIDEOFILT_ALIF_7  28
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_VIDEOFILT_ALIF_8  29
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_VIDEOFILT_B           0
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_VIDEOFILT_DK        1
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_VIDEOFILT_DTV_6  18
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_VIDEOFILT_DTV_7  19
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_VIDEOFILT_DTV_8  20
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_VIDEOFILT_G           2
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_VIDEOFILT_I            3
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_VIDEOFILT_L           4
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_ID_VIDEOFILT_M          5
/* DOWNLOAD_DATASET_START command, DATASET_CHECKSUM field definition (size 8, lsb 0, unsigned) */
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_CHECKSUM_LSB         0
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_CHECKSUM_MASK      0xff
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_CHECKSUM_MIN         0
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_CHECKSUM_MAX        255
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_CHECKSUM_DATASET_CHECKSUM_MIN  0
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATASET_CHECKSUM_DATASET_CHECKSUM_MAX  255
/* DOWNLOAD_DATASET_START command, DATA0 field definition (size 8, lsb 0, unsigned) */
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA0_LSB              0
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA0_MASK           0xff
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA0_MIN               0
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA0_MAX              255
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATA0_DATA0_MIN   0
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATA0_DATA0_MAX  255
/* DOWNLOAD_DATASET_START command, DATA1 field definition (size 8, lsb 0, unsigned) */
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA1_LSB               0
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA1_MASK            0xff
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA1_MIN               0
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA1_MAX              255
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATA1_DATA1_MIN   0
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATA1_DATA1_MAX  255
/* DOWNLOAD_DATASET_START command, DATA2 field definition (size 8, lsb 0, unsigned) */
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA2_LSB                0
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA2_MASK             0xff
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA2_MIN                 0
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA2_MAX                255
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATA2_DATA2_MIN    0
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATA2_DATA2_MAX  255
/* DOWNLOAD_DATASET_START command, DATA3 field definition (size 8, lsb 0, unsigned) */
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA3_LSB                 0
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA3_MASK              0xff
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA3_MIN                 0
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA3_MAX                255
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATA3_DATA3_MIN    0
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATA3_DATA3_MAX   255
/* DOWNLOAD_DATASET_START command, DATA4 field definition (size 8, lsb 0, unsigned) */
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA4_LSB                 0
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA4_MASK              0xff
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA4_MIN                 0
#define  SI2177_DOWNLOAD_DATASET_START_CMD_DATA4_MAX                255
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATA4_DATA4_MIN    0
#define SI2177_DOWNLOAD_DATASET_START_CMD_DATA4_DATA4_MAX   255

typedef struct { /* SI2177_DOWNLOAD_DATASET_START_CMD_REPLY_struct */
       si2177_common_reply_struct * status;
}  si2177_download_dataset_start_cmd_reply_struct;

#endif /* SI2177_DOWNLOAD_DATASET_START_CMD */

/* SI2177_DTV_RESTART command definition */
#define SI2177_DTV_RESTART_CMD 0x61

#ifdef    SI2177_DTV_RESTART_CMD

typedef struct { /* SI2177_DTV_RESTART_CMD_struct */
       unsigned char   nothing;
} si2177_dtv_restart_cmd_struct;


typedef struct { /* SI2177_DTV_RESTART_CMD_REPLY_struct */
       si2177_common_reply_struct * status;
}  si2177_dtv_restart_cmd_reply_struct;

#endif /* SI2177_DTV_RESTART_CMD */

/* SI2177_DTV_STATUS command definition */
#define SI2177_DTV_STATUS_CMD 0x62

#ifdef    SI2177_DTV_STATUS_CMD

typedef struct { /* SI2177_DTV_STATUS_CMD_struct */
     unsigned char   intack;
} si2177_dtv_status_cmd_struct;

/* DTV_STATUS command, INTACK field definition (size 1, lsb 0, unsigned) */
#define  SI2177_DTV_STATUS_CMD_INTACK_LSB          0
#define  SI2177_DTV_STATUS_CMD_INTACK_MASK       0x01
#define  SI2177_DTV_STATUS_CMD_INTACK_MIN          0
#define  SI2177_DTV_STATUS_CMD_INTACK_MAX         1
#define SI2177_DTV_STATUS_CMD_INTACK_CLEAR      1
#define SI2177_DTV_STATUS_CMD_INTACK_OK             0

typedef struct { /* SI2177_DTV_STATUS_CMD_REPLY_struct */
       si2177_common_reply_struct * status;
      unsigned char   chlint;
      unsigned char   chl;
      unsigned char   bw;
      unsigned char   modulation;
}  si2177_dtv_status_cmd_reply_struct;

/* DTV_STATUS command, CHLINT field definition (size 1, lsb 0, unsigned)*/
#define  SI2177_DTV_STATUS_RESPONSE_CHLINT_LSB                 0
#define  SI2177_DTV_STATUS_RESPONSE_CHLINT_MASK              0x01
#define SI2177_DTV_STATUS_RESPONSE_CHLINT_CHANGED       1
#define SI2177_DTV_STATUS_RESPONSE_CHLINT_NO_CHANGE   0
/* DTV_STATUS command, CHL field definition (size 1, lsb 0, unsigned)*/
#define  SI2177_DTV_STATUS_RESPONSE_CHL_LSB                       0
#define  SI2177_DTV_STATUS_RESPONSE_CHL_MASK                    0x01
#define SI2177_DTV_STATUS_RESPONSE_CHL_CHANNEL              1
#define SI2177_DTV_STATUS_RESPONSE_CHL_NO_CHANNEL       0
/* DTV_STATUS command, BW field definition (size 4, lsb 0, unsigned)*/
#define  SI2177_DTV_STATUS_RESPONSE_BW_LSB                         0
#define  SI2177_DTV_STATUS_RESPONSE_BW_MASK                      0x0f
#define SI2177_DTV_STATUS_RESPONSE_BW_BW_6MHZ                6
#define SI2177_DTV_STATUS_RESPONSE_BW_BW_7MHZ                7
#define SI2177_DTV_STATUS_RESPONSE_BW_BW_8MHZ                8
/* DTV_STATUS command, MODULATION field definition (size 4, lsb 4, unsigned)*/
#define  SI2177_DTV_STATUS_RESPONSE_MODULATION_LSB       4
#define  SI2177_DTV_STATUS_RESPONSE_MODULATION_MASK    0x0f
#define SI2177_DTV_STATUS_RESPONSE_MODULATION_ATSC      0
#define SI2177_DTV_STATUS_RESPONSE_MODULATION_DTMB      6
#define SI2177_DTV_STATUS_RESPONSE_MODULATION_DVBC      3
#define SI2177_DTV_STATUS_RESPONSE_MODULATION_DVBT       2
#define SI2177_DTV_STATUS_RESPONSE_MODULATION_ISDBC     5
#define SI2177_DTV_STATUS_RESPONSE_MODULATION_ISDBT     4
#define SI2177_DTV_STATUS_RESPONSE_MODULATION_QAM_US  1

#endif /* SI2177_DTV_STATUS_CMD */

/* SI2177_EXIT_BOOTLOADER command definition */
#define SI2177_EXIT_BOOTLOADER_CMD                                             0x01

#ifdef    SI2177_EXIT_BOOTLOADER_CMD

typedef struct { /* SI2177_EXIT_BOOTLOADER_CMD_struct */
     unsigned char   func;
     unsigned char   ctsien;
} si2177_exit_bootloader_cmd_struct;

/* EXIT_BOOTLOADER command, FUNC field definition (size 4, lsb 0, unsigned) */
#define  SI2177_EXIT_BOOTLOADER_CMD_FUNC_LSB                        0
   #define  SI2177_EXIT_BOOTLOADER_CMD_FUNC_MASK        0x07
#define  SI2177_EXIT_BOOTLOADER_CMD_FUNC_MIN                        0
#define  SI2177_EXIT_BOOTLOADER_CMD_FUNC_MAX                       1
#define SI2177_EXIT_BOOTLOADER_CMD_FUNC_BOOTLOADER       0
#define SI2177_EXIT_BOOTLOADER_CMD_FUNC_TUNER                   1
/* EXIT_BOOTLOADER command, CTSIEN field definition (size 1, lsb 7, unsigned) */
#define  SI2177_EXIT_BOOTLOADER_CMD_CTSIEN_LSB                     7
#define  SI2177_EXIT_BOOTLOADER_CMD_CTSIEN_MASK                 0x01
#define  SI2177_EXIT_BOOTLOADER_CMD_CTSIEN_MIN                     0
#define  SI2177_EXIT_BOOTLOADER_CMD_CTSIEN_MAX                    1
#define SI2177_EXIT_BOOTLOADER_CMD_CTSIEN_OFF                     0
#define SI2177_EXIT_BOOTLOADER_CMD_CTSIEN_ON                       1

typedef struct { /* SI2177_EXIT_BOOTLOADER_CMD_REPLY_struct */
       si2177_common_reply_struct * status;
}  si2177_exit_bootloader_cmd_reply_struct;

#endif /* SI2177_EXIT_BOOTLOADER_CMD */

/* SI2177_FINE_TUNE command definition */
#define SI2177_FINE_TUNE_CMD                                                            0x45

#ifdef    SI2177_FINE_TUNE_CMD

typedef struct { /* SI2177_FINE_TUNE_CMD_struct */
     unsigned char   persistence;
     unsigned char   apply_to_lif;
                       int    offset_500hz;
} si2177_fine_tune_cmd_struct;

/* FINE_TUNE command, RESERVED field definition (size 8, lsb 0, unsigned) */
   #define  SI2177_FINE_TUNE_CMD_PERSISTENCE_LSB         0
   #define  SI2177_FINE_TUNE_CMD_PERSISTENCE_MASK        0x01
   #define  SI2177_FINE_TUNE_CMD_PERSISTENCE_MIN         0
   #define  SI2177_FINE_TUNE_CMD_PERSISTENCE_MAX         1
    #define SI2177_FINE_TUNE_CMD_PERSISTENCE_NON_PERSISTENT  0
    #define SI2177_FINE_TUNE_CMD_PERSISTENCE_PERSISTENT      1
   #define  SI2177_FINE_TUNE_CMD_APPLY_TO_LIF_LSB         1
   #define  SI2177_FINE_TUNE_CMD_APPLY_TO_LIF_MASK        0x01
   #define  SI2177_FINE_TUNE_CMD_APPLY_TO_LIF_MIN         0
   #define  SI2177_FINE_TUNE_CMD_APPLY_TO_LIF_MAX         1
    #define SI2177_FINE_TUNE_CMD_APPLY_TO_LIF_APPLY_TO_LIF         1
    #define SI2177_FINE_TUNE_CMD_APPLY_TO_LIF_DO_NOT_APPLY_TO_LIF  0
/* FINE_TUNE command, OFFSET_500HZ field definition (size 16, lsb 0, signed) */
#define  SI2177_FINE_TUNE_CMD_OFFSET_500HZ_LSB                      0
#define  SI2177_FINE_TUNE_CMD_OFFSET_500HZ_MASK                   0xffff
#define  SI2177_FINE_TUNE_CMD_OFFSET_500HZ_SHIFT                  16
#define  SI2177_FINE_TUNE_CMD_OFFSET_500HZ_MIN                      -4000
#define  SI2177_FINE_TUNE_CMD_OFFSET_500HZ_MAX                     4000
#define SI2177_FINE_TUNE_CMD_OFFSET_500HZ_OFFSET_500HZ_MIN  -4000
#define SI2177_FINE_TUNE_CMD_OFFSET_500HZ_OFFSET_500HZ_MAX  4000

typedef struct { /* SI2177_FINE_TUNE_CMD_REPLY_struct */
       si2177_common_reply_struct * status;
}  si2177_fine_tune_cmd_reply_struct;

#endif /* SI2177_FINE_TUNE_CMD */

/* SI2177_GET_PROPERTY command definition */
#define SI2177_GET_PROPERTY_CMD                                                    0x15

#ifdef    SI2177_GET_PROPERTY_CMD

typedef struct { /* SI2177_GET_PROPERTY_CMD_struct */
     unsigned char   reserved;
     unsigned   int    prop;
} si2177_get_property_cmd_struct;

/* GET_PROPERTY command, RESERVED field definition (size 8, lsb 0, unsigned) */
#define  SI2177_GET_PROPERTY_CMD_RESERVED_LSB                     0
#define  SI2177_GET_PROPERTY_CMD_RESERVED_MASK                  0xff
#define  SI2177_GET_PROPERTY_CMD_RESERVED_MIN                      0
#define  SI2177_GET_PROPERTY_CMD_RESERVED_MAX                     0
#define SI2177_GET_PROPERTY_CMD_RESERVED_RESERVED_MIN  0
#define SI2177_GET_PROPERTY_CMD_RESERVED_RESERVED_MAX 0
/* GET_PROPERTY command, PROP field definition (size 16, lsb 0, unsigned) */
#define  SI2177_GET_PROPERTY_CMD_PROP_LSB                                0
#define  SI2177_GET_PROPERTY_CMD_PROP_MASK                             0xffff
#define  SI2177_GET_PROPERTY_CMD_PROP_MIN                                0
#define  SI2177_GET_PROPERTY_CMD_PROP_MAX                               65535
#define SI2177_GET_PROPERTY_CMD_PROP_PROP_MIN                      0
#define SI2177_GET_PROPERTY_CMD_PROP_PROP_MAX                     65535

typedef struct { /* SI2177_GET_PROPERTY_CMD_REPLY_struct */
      si2177_common_reply_struct * status;
      unsigned char   reserved;
      unsigned int    data;
}  si2177_get_property_cmd_reply_struct;

/* GET_PROPERTY command, RESERVED field definition (size 8, lsb 0, unsigned)*/
#define  SI2177_GET_PROPERTY_RESPONSE_RESERVED_LSB         0
#define  SI2177_GET_PROPERTY_RESPONSE_RESERVED_MASK      0xff
/* GET_PROPERTY command, DATA field definition (size 16, lsb 0, unsigned)*/
#define  SI2177_GET_PROPERTY_RESPONSE_DATA_LSB                   0
#define  SI2177_GET_PROPERTY_RESPONSE_DATA_MASK                0xffff

#endif /* SI2177_GET_PROPERTY_CMD */

/* SI2177_GET_REV command definition */
#define SI2177_GET_REV_CMD                                                                  0x11

#ifdef    SI2177_GET_REV_CMD

typedef struct { /* SI2177_GET_REV_CMD_struct */
    unsigned char   nothing;
} si2177_get_rev_cmd_struct;


typedef struct { /* SI2177_GET_REV_CMD_REPLY_struct */
      si2177_common_reply_struct * status;
      unsigned  char                          pn;
      unsigned  char                          fwmajor;
      unsigned  char                          fwminor;
      unsigned  int                             patch;
      unsigned  char                          cmpmajor;
      unsigned  char                          cmpminor;
      unsigned  char                          cmpbuild;
      unsigned  char                          chiprev;
}  si2177_get_rev_cmd_reply_struct;

/* GET_REV command, PN field definition (size 8, lsb 0, unsigned)*/
#define  SI2177_GET_REV_RESPONSE_PN_LSB                                   0
#define  SI2177_GET_REV_RESPONSE_PN_MASK                                0xff
/* GET_REV command, FWMAJOR field definition (size 8, lsb 0, unsigned)*/
#define  SI2177_GET_REV_RESPONSE_FWMAJOR_LSB                       0
#define  SI2177_GET_REV_RESPONSE_FWMAJOR_MASK                    0xff
/* GET_REV command, FWMINOR field definition (size 8, lsb 0, unsigned)*/
#define  SI2177_GET_REV_RESPONSE_FWMINOR_LSB                       0
#define  SI2177_GET_REV_RESPONSE_FWMINOR_MASK                    0xff
/* GET_REV command, PATCH field definition (size 16, lsb 0, unsigned)*/
#define  SI2177_GET_REV_RESPONSE_PATCH_LSB                            0
#define  SI2177_GET_REV_RESPONSE_PATCH_MASK                         0xffff
/* GET_REV command, CMPMAJOR field definition (size 8, lsb 0, unsigned)*/
#define  SI2177_GET_REV_RESPONSE_CMPMAJOR_LSB                     0
#define  SI2177_GET_REV_RESPONSE_CMPMAJOR_MASK                  0xff
/* GET_REV command, CMPMINOR field definition (size 8, lsb 0, unsigned)*/
#define  SI2177_GET_REV_RESPONSE_CMPMINOR_LSB                     0
#define  SI2177_GET_REV_RESPONSE_CMPMINOR_MASK                  0xff
/* GET_REV command, CMPBUILD field definition (size 8, lsb 0, unsigned)*/
#define  SI2177_GET_REV_RESPONSE_CMPBUILD_LSB                      0
#define  SI2177_GET_REV_RESPONSE_CMPBUILD_MASK                   0xff
#define SI2177_GET_REV_RESPONSE_CMPBUILD_CMPBUILD_MIN   0
#define SI2177_GET_REV_RESPONSE_CMPBUILD_CMPBUILD_MAX  255
/* GET_REV command, CHIPREV field definition (size 4, lsb 0, unsigned)*/
#define  SI2177_GET_REV_RESPONSE_CHIPREV_LSB                        0
#define  SI2177_GET_REV_RESPONSE_CHIPREV_MASK                     0x0f
#define SI2177_GET_REV_RESPONSE_CHIPREV_A                              1
#define SI2177_GET_REV_RESPONSE_CHIPREV_B                              2

#endif /* SI2177_GET_REV_CMD */

/* SI2177_PART_INFO command definition */
#define SI2177_PART_INFO_CMD                                                            0x02

#ifdef    SI2177_PART_INFO_CMD

typedef struct { /* SI2177_PART_INFO_CMD_struct */
     unsigned char   nothing;
} si2177_part_info_cmd_struct;


typedef struct { /* SI2177_PART_INFO_CMD_REPLY_struct */
      si2177_common_reply_struct * status;
      unsigned char   chiprev;
      unsigned char   romid;
      unsigned char   part;
      unsigned char   pmajor;
      unsigned char   pminor;
      unsigned char   pbuild;
      unsigned int    reserved;
      unsigned long   serial;
}  si2177_part_info_cmd_reply_struct;

/* PART_INFO command, CHIPREV field definition (size 4, lsb 0, unsigned)*/
#define  SI2177_PART_INFO_RESPONSE_CHIPREV_LSB           0
#define  SI2177_PART_INFO_RESPONSE_CHIPREV_MASK        0x0f
#define SI2177_PART_INFO_RESPONSE_CHIPREV_A                 1
#define SI2177_PART_INFO_RESPONSE_CHIPREV_B                 2
/* PART_INFO command, ROMID field definition (size 8, lsb 0, unsigned)*/
#define  SI2177_PART_INFO_RESPONSE_ROMID_LSB                0
#define  SI2177_PART_INFO_RESPONSE_ROMID_MASK             0xff
/* PART_INFO command, PART field definition (size 8, lsb 0, unsigned)*/
#define  SI2177_PART_INFO_RESPONSE_PART_LSB                   0
#define  SI2177_PART_INFO_RESPONSE_PART_MASK                0xff
/* PART_INFO command, PMAJOR field definition (size 8, lsb 0, unsigned)*/
#define  SI2177_PART_INFO_RESPONSE_PMAJOR_LSB               0
#define  SI2177_PART_INFO_RESPONSE_PMAJOR_MASK            0xff
/* PART_INFO command, PMINOR field definition (size 8, lsb 0, unsigned)*/
#define  SI2177_PART_INFO_RESPONSE_PMINOR_LSB               0
#define  SI2177_PART_INFO_RESPONSE_PMINOR_MASK            0xff
/* PART_INFO command, PBUILD field definition (size 8, lsb 0, unsigned)*/
#define  SI2177_PART_INFO_RESPONSE_PBUILD_LSB                0
#define  SI2177_PART_INFO_RESPONSE_PBUILD_MASK             0xff
/* PART_INFO command, RESERVED field definition (size 16, lsb 0, unsigned)*/
#define  SI2177_PART_INFO_RESPONSE_RESERVED_LSB         0
#define  SI2177_PART_INFO_RESPONSE_RESERVED_MASK      0xffff
/* PART_INFO command, SERIAL field definition (size 32, lsb 0, unsigned)*/
#define  SI2177_PART_INFO_RESPONSE_SERIAL_LSB                0
#define  SI2177_PART_INFO_RESPONSE_SERIAL_MASK             0xffffffff

#endif /* SI2177_PART_INFO_CMD */

/* SI2177_POWER_DOWN command definition */
#define SI2177_POWER_DOWN_CMD                                               0x13

#ifdef    SI2177_POWER_DOWN_CMD

typedef struct { /* SI2177_POWER_DOWN_CMD_struct */
       unsigned char   nothing;
} si2177_power_down_cmd_struct;


typedef struct { /* SI2177_POWER_DOWN_CMD_REPLY_struct */
       si2177_common_reply_struct * status;
}  si2177_power_down_cmd_reply_struct;

#endif /* SI2177_POWER_DOWN_CMD */

/* SI2177_POWER_DOWN_HW command definition */
#define   SI2177_POWER_DOWN_HW_CMD 0xc0

#ifdef    SI2177_POWER_DOWN_HW_CMD
  #define SI2177_POWER_DOWN_HW_CMD_CODE 0x0200c0

    typedef struct { /* SI2177_POWER_DOWN_HW_CMD_struct */
     unsigned char   subcode;
     unsigned char   pd_xo_osc;
     unsigned char   reserved1;
     unsigned char   en_xout;
     unsigned char   reserved2;
     unsigned char   pd_ldo;
     unsigned char   reserved3;
     unsigned char   reserved4;
     unsigned char   reserved5;
     unsigned char   reserved6;
     unsigned char   reserved7;
     unsigned char   reserved8;
   } SI2177_POWER_DOWN_HW_CMD_struct;


    typedef struct { /* SI2177_POWER_DOWN_HW_CMD_REPLY_struct */
       si2177_common_reply_struct * STATUS;
   }  SI2177_POWER_DOWN_HW_CMD_REPLY_struct;

   /* POWER_DOWN_HW command, SUBCODE field definition (address 1,size 8, lsb 0, unsigned) */
   #define  SI2177_POWER_DOWN_HW_CMD_SUBCODE_LSB         0
   #define  SI2177_POWER_DOWN_HW_CMD_SUBCODE_MASK        0xff
   #define  SI2177_POWER_DOWN_HW_CMD_SUBCODE_MIN         0
   #define  SI2177_POWER_DOWN_HW_CMD_SUBCODE_MAX         0
    #define SI2177_POWER_DOWN_HW_CMD_SUBCODE_CODE  0
   /* POWER_DOWN_HW command, PD_XO_OSC field definition (address 2,size 1, lsb 0, unsigned) */
   #define  SI2177_POWER_DOWN_HW_CMD_PD_XO_OSC_LSB         0
   #define  SI2177_POWER_DOWN_HW_CMD_PD_XO_OSC_MASK        0x01
   #define  SI2177_POWER_DOWN_HW_CMD_PD_XO_OSC_MIN         0
   #define  SI2177_POWER_DOWN_HW_CMD_PD_XO_OSC_MAX         1
    #define SI2177_POWER_DOWN_HW_CMD_PD_XO_OSC_XO_OSC_POWER_DOWN  1
    #define SI2177_POWER_DOWN_HW_CMD_PD_XO_OSC_XO_OSC_POWER_UP    0
   /* POWER_DOWN_HW command, RESERVED1 field definition (address 2,size 1, lsb 1, unsigned) */
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED1_LSB         1
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED1_MASK        0x01
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED1_MIN         0
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED1_MAX         0
    #define SI2177_POWER_DOWN_HW_CMD_RESERVED1_RESERVED  0
   /* POWER_DOWN_HW command, EN_XOUT field definition (address 2,size 3, lsb 2, unsigned) */
   #define  SI2177_POWER_DOWN_HW_CMD_EN_XOUT_LSB         2
   #define  SI2177_POWER_DOWN_HW_CMD_EN_XOUT_MASK        0x07
   #define  SI2177_POWER_DOWN_HW_CMD_EN_XOUT_MIN         0
   #define  SI2177_POWER_DOWN_HW_CMD_EN_XOUT_MAX         3
    #define SI2177_POWER_DOWN_HW_CMD_EN_XOUT_DIS_XOUT  0
    #define SI2177_POWER_DOWN_HW_CMD_EN_XOUT_EN_XOUT   3
   /* POWER_DOWN_HW command, RESERVED2 field definition (address 2,size 3, lsb 5, unsigned) */
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED2_LSB         5
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED2_MASK        0x07
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED2_MIN         1
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED2_MAX         1
    #define SI2177_POWER_DOWN_HW_CMD_RESERVED2_RESERVED  1
   /* POWER_DOWN_HW command, PD_LDO field definition (address 3,size 1, lsb 0, unsigned) */
   #define  SI2177_POWER_DOWN_HW_CMD_PD_LDO_LSB         0
   #define  SI2177_POWER_DOWN_HW_CMD_PD_LDO_MASK        0x01
   #define  SI2177_POWER_DOWN_HW_CMD_PD_LDO_MIN         0
   #define  SI2177_POWER_DOWN_HW_CMD_PD_LDO_MAX         1
    #define SI2177_POWER_DOWN_HW_CMD_PD_LDO_LDO_POWER_DOWN  1
    #define SI2177_POWER_DOWN_HW_CMD_PD_LDO_LDO_POWER_UP    0
   /* POWER_DOWN_HW command, RESERVED3 field definition (address 4,size 8, lsb 0, unsigned) */
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED3_LSB         0
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED3_MASK        0xff
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED3_MIN         0
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED3_MAX         0
    #define SI2177_POWER_DOWN_HW_CMD_RESERVED3_RESERVED  0
   /* POWER_DOWN_HW command, RESERVED4 field definition (address 5,size 8, lsb 0, unsigned) */
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED4_LSB         0
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED4_MASK        0xff
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED4_MIN         1
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED4_MAX         1
    #define SI2177_POWER_DOWN_HW_CMD_RESERVED4_RESERVED  1
   /* POWER_DOWN_HW command, RESERVED5 field definition (address 6,size 8, lsb 0, unsigned) */
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED5_LSB         0
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED5_MASK        0xff
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED5_MIN         0
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED5_MAX         0
    #define SI2177_POWER_DOWN_HW_CMD_RESERVED5_RESERVED  0
   /* POWER_DOWN_HW command, RESERVED6 field definition (address 7,size 8, lsb 0, unsigned) */
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED6_LSB         0
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED6_MASK        0xff
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED6_MIN         0
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED6_MAX         0
    #define SI2177_POWER_DOWN_HW_CMD_RESERVED6_RESERVED  0
   /* POWER_DOWN_HW command, RESERVED7 field definition (address 8,size 8, lsb 0, unsigned) */
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED7_LSB         0
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED7_MASK        0xff
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED7_MIN         0
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED7_MAX         0
    #define SI2177_POWER_DOWN_HW_CMD_RESERVED7_RESERVED  0
   /* POWER_DOWN_HW command, RESERVED8 field definition (address 9,size 8, lsb 0, unsigned) */
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED8_LSB         0
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED8_MASK        0xff
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED8_MIN         0
   #define  SI2177_POWER_DOWN_HW_CMD_RESERVED8_MAX         0
    #define SI2177_POWER_DOWN_HW_CMD_RESERVED8_RESERVED  0
#endif /* SI2177_POWER_DOWN_HW_CMD */

/* SI2177_POWER_UP command definition */
#define SI2177_POWER_UP_CMD                                                    0xc0

#ifdef    SI2177_POWER_UP_CMD

typedef struct { /* SI2177_POWER_UP_CMD_struct */
	unsigned char	subcode;
	unsigned char	clock_mode;
	unsigned char	en_xout;
	unsigned char	pd_ldo;
	unsigned char	reserved2;
	unsigned char	reserved3;
	unsigned char	reserved4;
	unsigned char	reserved5;
	unsigned char	reserved6;
	unsigned char	reserved7;
	unsigned char	reset;
	unsigned char	clock_freq;
	unsigned char	reserved8;
	unsigned char	func;
	unsigned char	reserved9;
	unsigned char	ctsien;
	unsigned char	wake_up;

} si2177_power_up_cmd_struct;

 /* POWER_UP command, SUBCODE field definition (address 1,size 8, lsb 0, unsigned) */
   #define  SI2177_POWER_UP_CMD_SUBCODE_LSB         0
   #define  SI2177_POWER_UP_CMD_SUBCODE_MASK        0xff
   #define  SI2177_POWER_UP_CMD_SUBCODE_MIN         0
   #define  SI2177_POWER_UP_CMD_SUBCODE_MAX         0
    #define SI2177_POWER_UP_CMD_SUBCODE_CODE  0
   /* POWER_UP command, CLOCK_MODE field definition (address 2,size 2, lsb 0, unsigned) */
   #define  SI2177_POWER_UP_CMD_CLOCK_MODE_LSB         0
   #define  SI2177_POWER_UP_CMD_CLOCK_MODE_MASK        0x03
   #define  SI2177_POWER_UP_CMD_CLOCK_MODE_MIN         0
   #define  SI2177_POWER_UP_CMD_CLOCK_MODE_MAX         2
    #define SI2177_POWER_UP_CMD_CLOCK_MODE_EXTCLK  2
    #define SI2177_POWER_UP_CMD_CLOCK_MODE_XTAL    0
   /* POWER_UP command, EN_XOUT field definition (address 2,size 3, lsb 2, unsigned) */
   #define  SI2177_POWER_UP_CMD_EN_XOUT_LSB         2
   #define  SI2177_POWER_UP_CMD_EN_XOUT_MASK        0x07
   #define  SI2177_POWER_UP_CMD_EN_XOUT_MIN         0
   #define  SI2177_POWER_UP_CMD_EN_XOUT_MAX         3
    #define SI2177_POWER_UP_CMD_EN_XOUT_DIS_XOUT  0
    #define SI2177_POWER_UP_CMD_EN_XOUT_EN_XOUT   3
   /* POWER_UP command, PD_LDO field definition (address 3,size 1, lsb 0, unsigned) */
   #define  SI2177_POWER_UP_CMD_PD_LDO_LSB         0
   #define  SI2177_POWER_UP_CMD_PD_LDO_MASK        0x01
   #define  SI2177_POWER_UP_CMD_PD_LDO_MIN         0
   #define  SI2177_POWER_UP_CMD_PD_LDO_MAX         1
    #define SI2177_POWER_UP_CMD_PD_LDO_LDO_POWER_DOWN  1
    #define SI2177_POWER_UP_CMD_PD_LDO_LDO_POWER_UP    0
   /* POWER_UP command, RESERVED2 field definition (address 4,size 8, lsb 0, unsigned) */
   #define  SI2177_POWER_UP_CMD_RESERVED2_LSB         0
   #define  SI2177_POWER_UP_CMD_RESERVED2_MASK        0xff
   #define  SI2177_POWER_UP_CMD_RESERVED2_MIN         0
   #define  SI2177_POWER_UP_CMD_RESERVED2_MAX         0
    #define SI2177_POWER_UP_CMD_RESERVED2_RESERVED  0
   /* POWER_UP command, RESERVED3 field definition (address 5,size 8, lsb 0, unsigned) */
   #define  SI2177_POWER_UP_CMD_RESERVED3_LSB         0
   #define  SI2177_POWER_UP_CMD_RESERVED3_MASK        0xff
   #define  SI2177_POWER_UP_CMD_RESERVED3_MIN         1
   #define  SI2177_POWER_UP_CMD_RESERVED3_MAX         1
    #define SI2177_POWER_UP_CMD_RESERVED3_RESERVED  1
   /* POWER_UP command, RESERVED4 field definition (address 6,size 8, lsb 0, unsigned) */
   #define  SI2177_POWER_UP_CMD_RESERVED4_LSB         0
   #define  SI2177_POWER_UP_CMD_RESERVED4_MASK        0xff
   #define  SI2177_POWER_UP_CMD_RESERVED4_MIN         1
   #define  SI2177_POWER_UP_CMD_RESERVED4_MAX         1
    #define SI2177_POWER_UP_CMD_RESERVED4_RESERVED  1
   /* POWER_UP command, RESERVED5 field definition (address 7,size 8, lsb 0, unsigned) */
   #define  SI2177_POWER_UP_CMD_RESERVED5_LSB         0
   #define  SI2177_POWER_UP_CMD_RESERVED5_MASK        0xff
   #define  SI2177_POWER_UP_CMD_RESERVED5_MIN         1
   #define  SI2177_POWER_UP_CMD_RESERVED5_MAX         1
    #define SI2177_POWER_UP_CMD_RESERVED5_RESERVED  1
   /* POWER_UP command, RESERVED6 field definition (address 8,size 8, lsb 0, unsigned) */
   #define  SI2177_POWER_UP_CMD_RESERVED6_LSB         0
   #define  SI2177_POWER_UP_CMD_RESERVED6_MASK        0xff
   #define  SI2177_POWER_UP_CMD_RESERVED6_MIN         1
   #define  SI2177_POWER_UP_CMD_RESERVED6_MAX         1
    #define SI2177_POWER_UP_CMD_RESERVED6_RESERVED  1
   /* POWER_UP command, RESERVED7 field definition (address 9,size 8, lsb 0, unsigned) */
   #define  SI2177_POWER_UP_CMD_RESERVED7_LSB         0
   #define  SI2177_POWER_UP_CMD_RESERVED7_MASK        0xff
   #define  SI2177_POWER_UP_CMD_RESERVED7_MIN         1
   #define  SI2177_POWER_UP_CMD_RESERVED7_MAX         1
    #define SI2177_POWER_UP_CMD_RESERVED7_RESERVED  1
   /* POWER_UP command, RESET field definition (address 10,size 8, lsb 0, unsigned) */
   #define  SI2177_POWER_UP_CMD_RESET_LSB         0
   #define  SI2177_POWER_UP_CMD_RESET_MASK        0xff
   #define  SI2177_POWER_UP_CMD_RESET_MIN         1
   #define  SI2177_POWER_UP_CMD_RESET_MAX         1
    #define SI2177_POWER_UP_CMD_RESET_RESET  1
   /* POWER_UP command, CLOCK_FREQ field definition (address 11,size 2, lsb 0, unsigned) */
   #define  SI2177_POWER_UP_CMD_CLOCK_FREQ_LSB         0
   #define  SI2177_POWER_UP_CMD_CLOCK_FREQ_MASK        0x03
   #define  SI2177_POWER_UP_CMD_CLOCK_FREQ_MIN         0
   #define  SI2177_POWER_UP_CMD_CLOCK_FREQ_MAX         3
    #define SI2177_POWER_UP_CMD_CLOCK_FREQ_CLK_24MHZ  2
   /* POWER_UP command, RESERVED8 field definition (address 12,size 8, lsb 0, unsigned) */
   #define  SI2177_POWER_UP_CMD_RESERVED8_LSB         0
   #define  SI2177_POWER_UP_CMD_RESERVED8_MASK        0xff
   #define  SI2177_POWER_UP_CMD_RESERVED8_MIN         0
   #define  SI2177_POWER_UP_CMD_RESERVED8_MAX         0
    #define SI2177_POWER_UP_CMD_RESERVED8_RESERVED  0
   /* POWER_UP command, FUNC field definition (address 13,size 3, lsb 0, unsigned) */
   #define  SI2177_POWER_UP_CMD_FUNC_LSB         0
   #define  SI2177_POWER_UP_CMD_FUNC_MASK        0x07
   #define  SI2177_POWER_UP_CMD_FUNC_MIN         0
   #define  SI2177_POWER_UP_CMD_FUNC_MAX         1
    #define SI2177_POWER_UP_CMD_FUNC_BOOTLOADER  0
    #define SI2177_POWER_UP_CMD_FUNC_NORMAL      1
   /* POWER_UP command, RESERVED9 field definition (address 13,size 4, lsb 3, unsigned) */
   #define  SI2177_POWER_UP_CMD_RESERVED9_LSB         3
   #define  SI2177_POWER_UP_CMD_RESERVED9_MASK        0x0f
   #define  SI2177_POWER_UP_CMD_RESERVED9_MIN         0
   #define  SI2177_POWER_UP_CMD_RESERVED9_MAX         0
    #define SI2177_POWER_UP_CMD_RESERVED9_RESERVED  0
   /* POWER_UP command, CTSIEN field definition (address 13,size 1, lsb 7, unsigned) */
   #define  SI2177_POWER_UP_CMD_CTSIEN_LSB         7
   #define  SI2177_POWER_UP_CMD_CTSIEN_MASK        0x01
   #define  SI2177_POWER_UP_CMD_CTSIEN_MIN         0
   #define  SI2177_POWER_UP_CMD_CTSIEN_MAX         1
    #define SI2177_POWER_UP_CMD_CTSIEN_DISABLE  0
    #define SI2177_POWER_UP_CMD_CTSIEN_ENABLE   1
   /* POWER_UP command, WAKE_UP field definition (address 14,size 1, lsb 0, unsigned) */
   #define  SI2177_POWER_UP_CMD_WAKE_UP_LSB         0
   #define  SI2177_POWER_UP_CMD_WAKE_UP_MASK        0x01
   #define  SI2177_POWER_UP_CMD_WAKE_UP_MIN         1
   #define  SI2177_POWER_UP_CMD_WAKE_UP_MAX         1
    #define SI2177_POWER_UP_CMD_WAKE_UP_WAKE_UP  1

typedef struct { /* SI2177_POWER_UP_CMD_REPLY_struct */
       si2177_common_reply_struct * status;
}  si2177_power_up_cmd_reply_struct;

#endif /* SI2177_POWER_UP_CMD */

/* SI2177_SET_PROPERTY command definition */
#define SI2177_SET_PROPERTY_CMD                                         0x14

#ifdef    SI2177_SET_PROPERTY_CMD

typedef struct { /* SI2177_SET_PROPERTY_CMD_struct */
     unsigned char   reserved;
     unsigned int      prop;
     unsigned int      data;
} si2177_set_property_cmd_struct;

/* SET_PROPERTY command, RESERVED field definition (size 8, lsb 0, unsigned) */
#define  SI2177_SET_PROPERTY_CMD_RESERVED_LSB         0
#define  SI2177_SET_PROPERTY_CMD_RESERVED_MASK      0xff
#define  SI2177_SET_PROPERTY_CMD_RESERVED_MIN         0
#define  SI2177_SET_PROPERTY_CMD_RESERVED_MAX        255.0
/* SET_PROPERTY command, PROP field definition (size 16, lsb 0, unsigned) */
#define  SI2177_SET_PROPERTY_CMD_PROP_LSB                   0
#define  SI2177_SET_PROPERTY_CMD_PROP_MASK                0xffff
#define  SI2177_SET_PROPERTY_CMD_PROP_MIN                   0
#define  SI2177_SET_PROPERTY_CMD_PROP_MAX                  65535
#define SI2177_SET_PROPERTY_CMD_PROP_PROP_MIN        0
#define SI2177_SET_PROPERTY_CMD_PROP_PROP_MAX       65535
/* SET_PROPERTY command, DATA field definition (size 16, lsb 0, unsigned) */
#define  SI2177_SET_PROPERTY_CMD_DATA_LSB                   0
#define  SI2177_SET_PROPERTY_CMD_DATA_MASK                0xffff
#define  SI2177_SET_PROPERTY_CMD_DATA_MIN                   0
#define  SI2177_SET_PROPERTY_CMD_DATA_MAX                  65535
#define SI2177_SET_PROPERTY_CMD_DATA_DATA_MIN         0
#define SI2177_SET_PROPERTY_CMD_DATA_DATA_MAX        65535

typedef struct { /* SI2177_SET_PROPERTY_CMD_REPLY_struct */
       si2177_common_reply_struct * status;
      unsigned  char                           reserved;
      unsigned  int                              data;
}  si2177_set_property_cmd_reply_struct;

/* SET_PROPERTY command, RESERVED field definition (size 8, lsb 0, unsigned)*/
#define  SI2177_SET_PROPERTY_RESPONSE_RESERVED_LSB                      0
#define  SI2177_SET_PROPERTY_RESPONSE_RESERVED_MASK                   0xff
#define SI2177_SET_PROPERTY_RESPONSE_RESERVED_RESERVED_MIN  0
#define SI2177_SET_PROPERTY_RESPONSE_RESERVED_RESERVED_MAX 0
/* SET_PROPERTY command, DATA field definition (size 16, lsb 0, unsigned)*/
#define  SI2177_SET_PROPERTY_RESPONSE_DATA_LSB                                 0
#define  SI2177_SET_PROPERTY_RESPONSE_DATA_MASK                              0xffff

#endif /* SI2177_SET_PROPERTY_CMD */

/* SI2177_STANDBY command definition */
#define SI2177_STANDBY_CMD                                                                             0x16

#ifdef    SI2177_STANDBY_CMD

typedef struct { /* SI2177_STANDBY_CMD_struct */
     unsigned char   type;
} si2177_standby_cmd_struct;


typedef struct { /* SI2177_STANDBY_CMD_REPLY_struct */
       si2177_common_reply_struct * status;
}  si2177_standby_cmd_reply_struct;

   /* STANDBY command, TYPE field definition (address 1,size 1, lsb 0, unsigned) */
   #define  SI2177_STANDBY_CMD_TYPE_LSB         0
   #define  SI2177_STANDBY_CMD_TYPE_MASK        0x01
   #define  SI2177_STANDBY_CMD_TYPE_MIN         0
   #define  SI2177_STANDBY_CMD_TYPE_MAX         1
    #define SI2177_STANDBY_CMD_TYPE_LNA_OFF  1
    #define SI2177_STANDBY_CMD_TYPE_LNA_ON   0
#endif /* SI2177_STANDBY_CMD */

/* SI2177_TUNER_STATUS command definition */
#define SI2177_TUNER_STATUS_CMD                                                                 0x42

#ifdef    SI2177_TUNER_STATUS_CMD

typedef struct { /* SI2177_TUNER_STATUS_CMD_struct */
     unsigned char   intack;
} si2177_tuner_status_cmd_struct;

/* TUNER_STATUS command, INTACK field definition (size 1, lsb 0, unsigned) */
#define  SI2177_TUNER_STATUS_CMD_INTACK_LSB                             0
#define  SI2177_TUNER_STATUS_CMD_INTACK_MASK                          0x01
#define  SI2177_TUNER_STATUS_CMD_INTACK_MIN                             0
#define  SI2177_TUNER_STATUS_CMD_INTACK_MAX                            1
#define SI2177_TUNER_STATUS_CMD_INTACK_CLEAR                         1
#define SI2177_TUNER_STATUS_CMD_INTACK_OK                                0

typedef struct { /* SI2177_TUNER_STATUS_CMD_REPLY_struct */
      si2177_common_reply_struct * status;
      unsigned  char                          tcint;
      unsigned  char                          rssilint;
      unsigned  char                          rssihint;
                         int                           vco_code;
      unsigned  char                          tc;
      unsigned  char                          rssil;
      unsigned  char                          rssih;
                      char                           rssi;
      unsigned    int                            freq;
      unsigned  char                          mode;
      unsigned  char                          resrved;
}  si2177_tuner_status_cmd_reply_struct;

/* TUNER_STATUS command, TCINT field definition (size 1, lsb 0, unsigned)*/
#define  SI2177_TUNER_STATUS_RESPONSE_TCINT_LSB                      0
#define  SI2177_TUNER_STATUS_RESPONSE_TCINT_MASK                   0x01
#define SI2177_TUNER_STATUS_RESPONSE_TCINT_CHANGED            1
#define SI2177_TUNER_STATUS_RESPONSE_TCINT_NO_CHANGE        0
/* TUNER_STATUS command, RSSILINT field definition (size 1, lsb 1, unsigned)*/
#define  SI2177_TUNER_STATUS_RESPONSE_RSSILINT_LSB                1
#define  SI2177_TUNER_STATUS_RESPONSE_RSSILINT_MASK             0x01
#define SI2177_TUNER_STATUS_RESPONSE_RSSILINT_CHANGED      1
#define SI2177_TUNER_STATUS_RESPONSE_RSSILINT_NO_CHANGE  0
/* TUNER_STATUS command, RSSIHINT field definition (size 1, lsb 2, unsigned)*/
#define  SI2177_TUNER_STATUS_RESPONSE_RSSIHINT_LSB                2
#define  SI2177_TUNER_STATUS_RESPONSE_RSSIHINT_MASK             0x01
#define SI2177_TUNER_STATUS_RESPONSE_RSSIHINT_CHANGED      1
#define SI2177_TUNER_STATUS_RESPONSE_RSSIHINT_NO_CHANGE  0
/* TUNER_STATUS command, VCO_CODE field definition (size 16, lsb 0, signed)*/
#define  SI2177_TUNER_STATUS_RESPONSE_VCO_CODE_LSB              0
#define  SI2177_TUNER_STATUS_RESPONSE_VCO_CODE_MASK           0xffff
#define  SI2177_TUNER_STATUS_RESPONSE_VCO_CODE_SHIFT          16
/* TUNER_STATUS command, TC field definition (size 1, lsb 0, unsigned)*/
#define  SI2177_TUNER_STATUS_RESPONSE_TC_LSB                             0
#define  SI2177_TUNER_STATUS_RESPONSE_TC_MASK                          0x01
#define SI2177_TUNER_STATUS_RESPONSE_TC_BUSY                           0
#define SI2177_TUNER_STATUS_RESPONSE_TC_DONE                           1
/* TUNER_STATUS command, RSSIL field definition (size 1, lsb 1, unsigned)*/
#define  SI2177_TUNER_STATUS_RESPONSE_RSSIL_LSB                        1
#define  SI2177_TUNER_STATUS_RESPONSE_RSSIL_MASK                     0x01
#define SI2177_TUNER_STATUS_RESPONSE_RSSIL_LOW                        1
#define SI2177_TUNER_STATUS_RESPONSE_RSSIL_OK                           0
/* TUNER_STATUS command, RSSIH field definition (size 1, lsb 2, unsigned)*/
#define  SI2177_TUNER_STATUS_RESPONSE_RSSIH_LSB                        2
#define  SI2177_TUNER_STATUS_RESPONSE_RSSIH_MASK                     0x01
#define SI2177_TUNER_STATUS_RESPONSE_RSSIH_HIGH                       1
#define SI2177_TUNER_STATUS_RESPONSE_RSSIH_OK                           0
/* TUNER_STATUS command, RSSI field definition (size 8, lsb 0, signed)*/
#define  SI2177_TUNER_STATUS_RESPONSE_RSSI_LSB                           0
#define  SI2177_TUNER_STATUS_RESPONSE_RSSI_MASK                        0xff
#define  SI2177_TUNER_STATUS_RESPONSE_RSSI_SHIFT                       24
/* TUNER_STATUS command, FREQ field definition (size 32, lsb 0, unsigned)*/
#define  SI2177_TUNER_STATUS_RESPONSE_FREQ_LSB                          0
#define  SI2177_TUNER_STATUS_RESPONSE_FREQ_MASK                       0xffffffff
/* TUNER_STATUS command, MODE field definition (size 1, lsb 0, unsigned)*/
#define  SI2177_TUNER_STATUS_RESPONSE_MODE_LSB                         0
#define  SI2177_TUNER_STATUS_RESPONSE_MODE_MASK                      0x01
#define SI2177_TUNER_STATUS_RESPONSE_MODE_ATV                          1
#define SI2177_TUNER_STATUS_RESPONSE_MODE_DTV                          0

#endif /* SI2177_TUNER_STATUS_CMD */

/* SI2177_TUNER_TUNE_FREQ command definition */
#define SI2177_TUNER_TUNE_FREQ_CMD                                                  0x41

#ifdef    SI2177_TUNER_TUNE_FREQ_CMD

typedef struct { /* SI2177_TUNER_TUNE_FREQ_CMD_struct */
     unsigned char   mode;
     unsigned long   freq;
} si2177_tuner_tune_freq_cmd_struct;

/* TUNER_TUNE_FREQ command, MODE field definition (size 1, lsb 0, unsigned) */
#define  SI2177_TUNER_TUNE_FREQ_CMD_MODE_LSB                            0
#define  SI2177_TUNER_TUNE_FREQ_CMD_MODE_MASK                         0x01
#define  SI2177_TUNER_TUNE_FREQ_CMD_MODE_MIN                            0
#define  SI2177_TUNER_TUNE_FREQ_CMD_MODE_MAX                           1
#define SI2177_TUNER_TUNE_FREQ_CMD_MODE_ATV                             1
#define SI2177_TUNER_TUNE_FREQ_CMD_MODE_DTV                             0
/* TUNER_TUNE_FREQ command, FREQ field definition (size 32, lsb 0, unsigned) */
#define  SI2177_TUNER_TUNE_FREQ_CMD_FREQ_LSB                             0
#define  SI2177_TUNER_TUNE_FREQ_CMD_FREQ_MASK                          0xffffffff
   #define  SI2177_TUNER_TUNE_FREQ_CMD_FREQ_MIN         40000000
#define  SI2177_TUNER_TUNE_FREQ_CMD_FREQ_MAX                            1002000000
    #define SI2177_TUNER_TUNE_FREQ_CMD_FREQ_FREQ_MIN  40000000
#define SI2177_TUNER_TUNE_FREQ_CMD_FREQ_FREQ_MAX                 1002000000

typedef struct { /* SI2177_TUNER_TUNE_FREQ_CMD_REPLY_struct */
       si2177_common_reply_struct * status;
}  si2177_tuner_tune_freq_cmd_reply_struct;

#endif /* SI2177_TUNER_TUNE_FREQ_CMD */

/* _commands_defines_insertion_point */

/* _commands_struct_insertion_start */

/* --------------------------------------------*/
/* COMMANDS STRUCT                             */
/* This is used to store all command fields    */
/* --------------------------------------------*/
typedef union { /* SI2177_CmdObj union */
    #ifdef    SI2177_AGC_OVERRIDE_CMD
              si2177_agc_override_cmd_struct               agc_override;
    #endif /* SI2177_AGC_OVERRIDE_CMD */
    #ifdef    SI2177_ATV_CW_TEST_CMD
              si2177_atv_cw_test_cmd_struct                atv_cw_test;
    #endif /* SI2177_ATV_CW_TEST_CMD */
    #ifdef    SI2177_ATV_RESTART_CMD
              si2177_atv_restart_cmd_struct                  atv_restart;
    #endif /* SI2177_ATV_RESTART_CMD */
    #ifdef    SI2177_ATV_STATUS_CMD
              si2177_atv_status_cmd_struct                   atv_status;
    #endif /* SI2177_ATV_STATUS_CMD */
    #ifdef    SI2177_CONFIG_CLOCKS_CMD
              si2177_config_clocks_cmd_struct    config_clocks;
    #endif /* SI2177_CONFIG_CLOCKS_CMD */
    #ifdef    SI2177_CONFIG_PINS_CMD
              si2177_config_pins_cmd_struct                 config_pins;
    #endif /* SI2177_CONFIG_PINS_CMD */
    #ifdef    SI2177_DOWNLOAD_DATASET_CONTINUE_CMD
              si2177_download_dataset_continue_cmd_struct  download_dataset_continue;
    #endif /* SI2177_DOWNLOAD_DATASET_CONTINUE_CMD */
    #ifdef    SI2177_DOWNLOAD_DATASET_START_CMD
              si2177_download_dataset_start_cmd_struct     download_dataset_start;
    #endif /* SI2177_DOWNLOAD_DATASET_START_CMD */
    #ifdef    SI2177_DTV_RESTART_CMD
              si2177_dtv_restart_cmd_struct                dtv_restart;
    #endif /* SI2177_DTV_RESTART_CMD */
    #ifdef    SI2177_DTV_STATUS_CMD
              si2177_dtv_status_cmd_struct                 dtv_status;
    #endif /* SI2177_DTV_STATUS_CMD */
    #ifdef    SI2177_EXIT_BOOTLOADER_CMD
              si2177_exit_bootloader_cmd_struct         exit_bootloader;
    #endif /* SI2177_EXIT_BOOTLOADER_CMD */
    #ifdef    SI2177_FINE_TUNE_CMD
              si2177_fine_tune_cmd_struct                   fine_tune;
    #endif /* SI2177_FINE_TUNE_CMD */
    #ifdef    SI2177_GET_PROPERTY_CMD
              si2177_get_property_cmd_struct             get_property;
    #endif /* SI2177_GET_PROPERTY_CMD */
    #ifdef    SI2177_GET_REV_CMD
              si2177_get_rev_cmd_struct                      get_rev;
    #endif /* SI2177_GET_REV_CMD */
    #ifdef    SI2177_PART_INFO_CMD
              si2177_part_info_cmd_struct                    part_info;
    #endif /* SI2177_PART_INFO_CMD */
    #ifdef    SI2177_POWER_DOWN_CMD
              si2177_power_down_cmd_struct              power_down;
    #endif /* SI2177_POWER_DOWN_CMD */
    #ifdef    SI2177_POWER_DOWN_HW_CMD
              SI2177_POWER_DOWN_HW_CMD_struct    power_down_hw;
    #endif /* SI2177_POWER_DOWN_HW_CMD */
    #ifdef    SI2177_POWER_UP_CMD
              si2177_power_up_cmd_struct                   power_up;
    #endif /* SI2177_POWER_UP_CMD */
    #ifdef    SI2177_SET_PROPERTY_CMD
              si2177_set_property_cmd_struct               set_property;
    #endif /* SI2177_SET_PROPERTY_CMD */
    #ifdef    SI2177_STANDBY_CMD
              si2177_standby_cmd_struct                      standby;
    #endif /* SI2177_STANDBY_CMD */
    #ifdef    SI2177_TUNER_STATUS_CMD
              si2177_tuner_status_cmd_struct               tuner_status;
    #endif /* SI2177_TUNER_STATUS_CMD */
    #ifdef    SI2177_TUNER_TUNE_FREQ_CMD
              si2177_tuner_tune_freq_cmd_struct          tuner_tune_freq;
    #endif /* SI2177_TUNER_TUNE_FREQ_CMD */
} si2177_cmdobj_t;
/* _commands_struct_insertion_point */

/* _commands_reply_struct_insertion_start */

  /* --------------------------------------------*/
  /* COMMANDS REPLY STRUCT                       */
  /* This stores all command reply fields        */
  /* --------------------------------------------*/
typedef struct { /* SI2177_CmdReplyObj struct */
    #ifdef    SI2177_AGC_OVERRIDE_CMD
              si2177_agc_override_cmd_reply_struct              agc_override;
    #endif /* SI2177_AGC_OVERRIDE_CMD */
    #ifdef    SI2177_ATV_CW_TEST_CMD
              si2177_atv_cw_test_cmd_reply_struct                 atv_cw_test;
    #endif /* SI2177_ATV_CW_TEST_CMD */
    #ifdef    SI2177_ATV_RESTART_CMD
              si2177_atv_restart_cmd_reply_struct                  atv_restart;
    #endif /* SI2177_ATV_RESTART_CMD */
    #ifdef    SI2177_ATV_STATUS_CMD
              si2177_atv_status_cmd_reply_struct                   atv_status;
    #endif /* SI2177_ATV_STATUS_CMD */
    #ifdef    SI2177_CONFIG_CLOCKS_CMD
              SI2177_CONFIG_CLOCKS_CMD_REPLY_struct    config_clocks;
    #endif /* SI2177_CONFIG_CLOCKS_CMD */
    #ifdef    SI2177_CONFIG_PINS_CMD
              si2177_config_pins_cmd_reply_struct                 config_pins;
    #endif /* SI2177_CONFIG_PINS_CMD */
    #ifdef    SI2177_DOWNLOAD_DATASET_CONTINUE_CMD
              si2177_download_dataset_continue_cmd_reply_struct  download_dataset_continue;
    #endif /* SI2177_DOWNLOAD_DATASET_CONTINUE_CMD */
    #ifdef    SI2177_DOWNLOAD_DATASET_START_CMD
              si2177_download_dataset_start_cmd_reply_struct        download_dataset_start;
    #endif /* SI2177_DOWNLOAD_DATASET_START_CMD */
    #ifdef    SI2177_EXIT_BOOTLOADER_CMD
              si2177_exit_bootloader_cmd_reply_struct                     exit_bootloader;
    #endif /* SI2177_EXIT_BOOTLOADER_CMD */
    #ifdef    SI2177_FINE_TUNE_CMD
              si2177_fine_tune_cmd_reply_struct                               fine_tune;
    #endif /* SI2177_FINE_TUNE_CMD */
    #ifdef    SI2177_GET_PROPERTY_CMD
              si2177_get_property_cmd_reply_struct                         get_property;
    #endif /* SI2177_GET_PROPERTY_CMD */
    #ifdef    SI2177_GET_REV_CMD
              si2177_get_rev_cmd_reply_struct                    get_rev;
    #endif /* SI2177_GET_REV_CMD */
    #ifdef    SI2177_PART_INFO_CMD
              si2177_part_info_cmd_reply_struct                  part_info;
    #endif /* SI2177_PART_INFO_CMD */
    #ifdef    SI2177_POWER_DOWN_CMD
              si2177_power_down_cmd_reply_struct           power_down;
    #endif /* SI2177_POWER_DOWN_CMD */
    #ifdef    SI2177_POWER_DOWN_HW_CMD
              SI2177_POWER_DOWN_HW_CMD_REPLY_struct    power_down_hw;
    #endif /* SI2177_POWER_DOWN_HW_CMD */
    #ifdef    SI2177_POWER_UP_CMD
              si2177_power_up_cmd_reply_struct                power_up;
    #endif /* SI2177_POWER_UP_CMD */
    #ifdef    SI2177_SET_PROPERTY_CMD
              si2177_set_property_cmd_reply_struct           set_property;
    #endif /* SI2177_SET_PROPERTY_CMD */
    #ifdef    SI2177_STANDBY_CMD
              si2177_standby_cmd_reply_struct                  standby;
    #endif /* SI2177_STANDBY_CMD */
    #ifdef    SI2177_TUNER_STATUS_CMD
              si2177_tuner_status_cmd_reply_struct           tuner_status;
    #endif /* SI2177_TUNER_STATUS_CMD */
    #ifdef    SI2177_TUNER_TUNE_FREQ_CMD
              si2177_tuner_tune_freq_cmd_reply_struct      tuner_tune_freq;
    #endif /* SI2177_TUNER_TUNE_FREQ_CMD */
} si2177_cmdreplyobj_t;
/* _commands_reply_struct_insertion_point */

/* _properties_defines_insertion_start */
/* SI2177 ATV_AFC_RANGE property definition */
#define   SI2177_ATV_AFC_RANGE_PROP                                     0x0610

#ifdef    SI2177_ATV_AFC_RANGE_PROP

typedef struct { /* SI2177_ATV_AFC_RANGE_PROP_struct */
      unsigned int    range_khz;
} si2177_atv_afc_range_prop_struct;

   /* ATV_AFC_RANGE property, RANGE_KHZ field definition (NO TITLE)*/
   #define  SI2177_ATV_AFC_RANGE_PROP_RANGE_KHZ_LSB         0
   #define  SI2177_ATV_AFC_RANGE_PROP_RANGE_KHZ_MASK        0xffff
   #define  SI2177_ATV_AFC_RANGE_PROP_RANGE_KHZ_DEFAULT    1000
    #define SI2177_ATV_AFC_RANGE_PROP_RANGE_KHZ_RANGE_KHZ_MIN  0
    #define SI2177_ATV_AFC_RANGE_PROP_RANGE_KHZ_RANGE_KHZ_MAX  65535

#endif /* SI2177_ATV_AFC_RANGE_PROP */

/* SI2177 ATV_AF_OUT property definition */
#define   SI2177_ATV_AF_OUT_PROP                                           0x060b

#ifdef    SI2177_ATV_AF_OUT_PROP

typedef struct { /* SI2177_ATV_AF_OUT_PROP_struct */
      unsigned char   mute;
      unsigned char   volume;
} si2177_atv_af_out_prop_struct;

   #define  SI2177_ATV_AF_OUT_PROP_MUTE_LSB         6
   #define  SI2177_ATV_AF_OUT_PROP_MUTE_MASK        0x01
   #define  SI2177_ATV_AF_OUT_PROP_MUTE_DEFAULT    0
    #define SI2177_ATV_AF_OUT_PROP_MUTE_NORMAL  0
    #define SI2177_ATV_AF_OUT_PROP_MUTE_MUTE    1
/* ATV_AF_OUT property, VOLUME field definition (NO TITLE)*/
#define  SI2177_ATV_AF_OUT_PROP_VOLUME_LSB                  0
#define  SI2177_ATV_AF_OUT_PROP_VOLUME_MASK               0x3f
#define  SI2177_ATV_AF_OUT_PROP_VOLUME_DEFAULT         0
#define SI2177_ATV_AF_OUT_PROP_VOLUME_VOLUME_MIN   0
#define SI2177_ATV_AF_OUT_PROP_VOLUME_VOLUME_MAX  63

#endif /* SI2177_ATV_AF_OUT_PROP */

/* SI2177 ATV_AGC_SPEED property definition */
#define   SI2177_ATV_AGC_SPEED_PROP                                   0x0611

#ifdef    SI2177_ATV_AGC_SPEED_PROP

typedef struct { /* SI2177_ATV_AGC_SPEED_PROP_struct */
      unsigned char   if_agc_speed;
} si2177_atv_agc_speed_prop_struct;

/* ATV_AGC_SPEED property, IF_AGC_SPEED field definition (NO TITLE)*/
#define  SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_LSB           0
#define  SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_MASK        0xff
#define  SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_DEFAULT  0
#define SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_AUTO          0
#define SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_89               89
#define SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_105             105
#define SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_121             121
#define SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_137             137
#define SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_158             158
#define SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_172             172
    #define SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_178     178
#define SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_185             185
#define SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_196             196
#define SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_206             206
#define SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_216             216
#define SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_219             219
#define SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_222             222
    #define SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_223     223
#define SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_248             248
#define SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_250             250
#define SI2177_ATV_AGC_SPEED_PROP_IF_AGC_SPEED_251             251

#endif /* SI2177_ATV_AGC_SPEED_PROP */

/* SI2177 ATV_AGC_SPEED_LOW_RSSI property definition */
#define   SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP 0x0623

#ifdef    SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP
  #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_CODE 0x000623


    typedef struct { /* SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_struct */
      unsigned char   if_agc_speed;
               char   thld;
   } SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_struct;

   /* ATV_AGC_SPEED_LOW_RSSI property, IF_AGC_SPEED field definition (NO TITLE)*/
   #define  SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_LSB         0
   #define  SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_MASK        0xff
   #define  SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_DEFAULT    158
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_89      89
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_105     105
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_121     121
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_137     137
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_158     158
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_172     172
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_178     178
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_185     185
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_196     196
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_206     206
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_216     216
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_219     219
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_222     222
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_223     223
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_248     248
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_250     250
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_IF_AGC_SPEED_251     251

   /* ATV_AGC_SPEED_LOW_RSSI property, THLD field definition (NO TITLE)*/
   #define  SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_THLD_LSB         8
   #define  SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_THLD_MASK        0xff
   #define  SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_THLD_DEFAULT    -128
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_THLD_THLD_MIN  -128
    #define SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_THLD_THLD_MAX  127

#endif /* SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP */

/* SI2177 ATV_ARTIFICIAL_SNOW property definition */
#define   SI2177_ATV_ARTIFICIAL_SNOW_PROP 0x0624

#ifdef    SI2177_ATV_ARTIFICIAL_SNOW_PROP
  #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_CODE 0x000624


    typedef struct { /* SI2177_ATV_ARTIFICIAL_SNOW_PROP_struct */
      unsigned char   gain;
               char   offset;
      unsigned char   period;
      unsigned char   sound;
   } SI2177_ATV_ARTIFICIAL_SNOW_PROP_struct;

   /* ATV_ARTIFICIAL_SNOW property, GAIN field definition (NO TITLE)*/
   #define  SI2177_ATV_ARTIFICIAL_SNOW_PROP_GAIN_LSB         0
   #define  SI2177_ATV_ARTIFICIAL_SNOW_PROP_GAIN_MASK        0x0f
   #define  SI2177_ATV_ARTIFICIAL_SNOW_PROP_GAIN_DEFAULT    0
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_GAIN_AUTO  0
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_GAIN_0DB   1
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_GAIN_6DB   2
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_GAIN_12DB  3
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_GAIN_18DB  4
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_GAIN_24DB  5
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_GAIN_30DB  6
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_GAIN_36DB  7
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_GAIN_42DB  8
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_GAIN_OFF   9

   /* ATV_ARTIFICIAL_SNOW property, OFFSET field definition (NO TITLE)*/
   #define  SI2177_ATV_ARTIFICIAL_SNOW_PROP_OFFSET_LSB         8
   #define  SI2177_ATV_ARTIFICIAL_SNOW_PROP_OFFSET_MASK        0xff
   #define  SI2177_ATV_ARTIFICIAL_SNOW_PROP_OFFSET_DEFAULT    0
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_OFFSET_OFFSET_MIN  -128
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_OFFSET_OFFSET_MAX  127

   /* ATV_ARTIFICIAL_SNOW property, PERIOD field definition (NO TITLE)*/
   #define  SI2177_ATV_ARTIFICIAL_SNOW_PROP_PERIOD_LSB         7
   #define  SI2177_ATV_ARTIFICIAL_SNOW_PROP_PERIOD_MASK        0x01
   #define  SI2177_ATV_ARTIFICIAL_SNOW_PROP_PERIOD_DEFAULT    0
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_PERIOD_LONG   0
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_PERIOD_SHORT  1

   /* ATV_ARTIFICIAL_SNOW property, SOUND field definition (NO TITLE)*/
   #define  SI2177_ATV_ARTIFICIAL_SNOW_PROP_SOUND_LSB         4
   #define  SI2177_ATV_ARTIFICIAL_SNOW_PROP_SOUND_MASK        0x07
   #define  SI2177_ATV_ARTIFICIAL_SNOW_PROP_SOUND_DEFAULT    0
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_SOUND_MUTE          0
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_SOUND_6DB           1
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_SOUND_12DB          2
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_SOUND_18DB          3
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_SOUND_24DB          4
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_SOUND_30DB          5
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_SOUND_36DB          6
    #define SI2177_ATV_ARTIFICIAL_SNOW_PROP_SOUND_PASS_THROUGH  7

#endif /* SI2177_ATV_AGC_SPEED_PROP */

/* SI2177 ATV_AUDIO_MODE property definition */
#define   SI2177_ATV_AUDIO_MODE_PROP                                            0x0602

#ifdef    SI2177_ATV_AUDIO_MODE_PROP

typedef struct { /* SI2177_ATV_AUDIO_MODE_PROP_struct */
      unsigned char   audio_sys;
      unsigned char   chan_bw;
      unsigned char   demod_mode;
} si2177_atv_audio_mode_prop_struct;

/* ATV_AUDIO_MODE property, AUDIO_SYS field definition (NO TITLE)*/
#define  SI2177_ATV_AUDIO_MODE_PROP_AUDIO_SYS_LSB           0
#define  SI2177_ATV_AUDIO_MODE_PROP_AUDIO_SYS_MASK        0x0f
#define  SI2177_ATV_AUDIO_MODE_PROP_AUDIO_SYS_DEFAULT  0
#define SI2177_ATV_AUDIO_MODE_PROP_AUDIO_SYS_DEFAULT   0
#define SI2177_ATV_AUDIO_MODE_PROP_AUDIO_SYS_MONO        1
#define SI2177_ATV_AUDIO_MODE_PROP_AUDIO_SYS_MONO_NICAM  2
#define SI2177_ATV_AUDIO_MODE_PROP_AUDIO_SYS_A2              3
#define SI2177_ATV_AUDIO_MODE_PROP_AUDIO_SYS_A2_DK2     4
#define SI2177_ATV_AUDIO_MODE_PROP_AUDIO_SYS_A2_DK3     5
#define SI2177_ATV_AUDIO_MODE_PROP_AUDIO_SYS_BTSC         6
#define SI2177_ATV_AUDIO_MODE_PROP_AUDIO_SYS_EIAJ           7
#define SI2177_ATV_AUDIO_MODE_PROP_AUDIO_SYS_SCAN        8
#define SI2177_ATV_AUDIO_MODE_PROP_AUDIO_SYS_A2_DK4     9
#define SI2177_ATV_AUDIO_MODE_PROP_AUDIO_SYS_WIDE_SCAN   10
    #define SI2177_ATV_AUDIO_MODE_PROP_AUDIO_SYS_MONO_NICAM_6DB   11
    #define SI2177_ATV_AUDIO_MODE_PROP_AUDIO_SYS_MONO_NICAM_10DB  12

/* ATV_AUDIO_MODE property, CHAN_BW field definition (NO TITLE)*/
#define  SI2177_ATV_AUDIO_MODE_PROP_CHAN_BW_LSB             8
#define  SI2177_ATV_AUDIO_MODE_PROP_CHAN_BW_MASK          0x0f
#define  SI2177_ATV_AUDIO_MODE_PROP_CHAN_BW_DEFAULT    0
#define SI2177_ATV_AUDIO_MODE_PROP_CHAN_BW_DEFAULT     0
#define SI2177_ATV_AUDIO_MODE_PROP_CHAN_BW_4X_OVERMOD         1
#define SI2177_ATV_AUDIO_MODE_PROP_CHAN_BW_8X_OVERMOD         2
#define SI2177_ATV_AUDIO_MODE_PROP_CHAN_BW_12X_OVERMOD       3
#define SI2177_ATV_AUDIO_MODE_PROP_CHAN_BW_7P5_KHZ_OFFSET  4
#define SI2177_ATV_AUDIO_MODE_PROP_CHAN_BW_15_KHZ_OFFSET    5
#define SI2177_ATV_AUDIO_MODE_PROP_CHAN_BW_30_KHZ_OFFSET    6
#define SI2177_ATV_AUDIO_MODE_PROP_CHAN_BW_75_KHZ_OFFSET    7
#define SI2177_ATV_AUDIO_MODE_PROP_CHAN_BW_150_KHZ_OFFSET  8
    #define SI2177_ATV_AUDIO_MODE_PROP_CHAN_BW_CUSTOM          9

/* ATV_AUDIO_MODE property, DEMOD_MODE field definition (NO TITLE)*/
#define  SI2177_ATV_AUDIO_MODE_PROP_DEMOD_MODE_LSB                  4
#define  SI2177_ATV_AUDIO_MODE_PROP_DEMOD_MODE_MASK               0x03
#define  SI2177_ATV_AUDIO_MODE_PROP_DEMOD_MODE_DEFAULT         0
#define SI2177_ATV_AUDIO_MODE_PROP_DEMOD_MODE_SIF                     0
#define SI2177_ATV_AUDIO_MODE_PROP_DEMOD_MODE_AM                      1
#define SI2177_ATV_AUDIO_MODE_PROP_DEMOD_MODE_FM1                    2
#define SI2177_ATV_AUDIO_MODE_PROP_DEMOD_MODE_FM2                    3

#endif /* SI2177_ATV_AUDIO_MODE_PROP */

/* SI2177 ATV_CONFIG_IF_PORT property definition */
#define   SI2177_ATV_CONFIG_IF_PORT_PROP 0x0603

#ifdef    SI2177_ATV_CONFIG_IF_PORT_PROP
  #define SI2177_ATV_CONFIG_IF_PORT_PROP_CODE 0x000603


    typedef struct { /* SI2177_ATV_CONFIG_IF_PORT_PROP_struct */
      unsigned char   atv_out_type;
   } SI2177_ATV_CONFIG_IF_PORT_PROP_struct;

   /* ATV_CONFIG_IF_PORT property, ATV_OUT_TYPE field definition (NO TITLE)*/
   #define  SI2177_ATV_CONFIG_IF_PORT_PROP_ATV_OUT_TYPE_LSB         0
   #define  SI2177_ATV_CONFIG_IF_PORT_PROP_ATV_OUT_TYPE_MASK        0x0f
   #define  SI2177_ATV_CONFIG_IF_PORT_PROP_ATV_OUT_TYPE_DEFAULT    0
    #define SI2177_ATV_CONFIG_IF_PORT_PROP_ATV_OUT_TYPE_CVBS_IF2B_SOUND_IF2A  0

#endif /* SI2177_ATV_AUDIO_MODE_PROP */

/* SI2177 ATV_CVBS_OUT property definition */
#define   SI2177_ATV_CVBS_OUT_PROP                                      0x0609

#ifdef    SI2177_ATV_CVBS_OUT_PROP

typedef struct { /* SI2177_ATV_CVBS_OUT_PROP_struct */
      unsigned char   amp;
      unsigned char   offset;
} si2177_atv_cvbs_out_prop_struct;

/* ATV_CVBS_OUT property, AMP field definition (NO TITLE)*/
#define  SI2177_ATV_CVBS_OUT_PROP_AMP_LSB                   8
#define  SI2177_ATV_CVBS_OUT_PROP_AMP_MASK                0xff
#define  SI2177_ATV_CVBS_OUT_PROP_AMP_DEFAULT          200
#define SI2177_ATV_CVBS_OUT_PROP_AMP_AMP_MIN           0
#define SI2177_ATV_CVBS_OUT_PROP_AMP_AMP_MAX          255

/* ATV_CVBS_OUT property, OFFSET field definition (NO TITLE)*/
#define  SI2177_ATV_CVBS_OUT_PROP_OFFSET_LSB                0
#define  SI2177_ATV_CVBS_OUT_PROP_OFFSET_MASK             0xff
#define  SI2177_ATV_CVBS_OUT_PROP_OFFSET_DEFAULT       25
#define SI2177_ATV_CVBS_OUT_PROP_OFFSET_OFFSET_MIN  0
#define SI2177_ATV_CVBS_OUT_PROP_OFFSET_OFFSET_MAX  255

#endif /* SI2177_ATV_CVBS_OUT_PROP */

/* SI2177 ATV_CVBS_OUT_FINE property definition */
#define   SI2177_ATV_CVBS_OUT_FINE_PROP                     0x0614

#ifdef    SI2177_ATV_CVBS_OUT_FINE_PROP

typedef struct { /* SI2177_ATV_CVBS_OUT_FINE_PROP_struct */
      unsigned char   amp;
                      char   offset;
} si2177_atv_cvbs_out_fine_prop_struct;

/* ATV_CVBS_OUT_FINE property, AMP field definition (NO TITLE)*/
#define  SI2177_ATV_CVBS_OUT_FINE_PROP_AMP_LSB            8
#define  SI2177_ATV_CVBS_OUT_FINE_PROP_AMP_MASK         0xff
#define  SI2177_ATV_CVBS_OUT_FINE_PROP_AMP_DEFAULT   100
#define SI2177_ATV_CVBS_OUT_FINE_PROP_AMP_AMP_MIN    25
#define SI2177_ATV_CVBS_OUT_FINE_PROP_AMP_AMP_MAX   100

/* ATV_CVBS_OUT_FINE property, OFFSET field definition (NO TITLE)*/
#define  SI2177_ATV_CVBS_OUT_FINE_PROP_OFFSET_LSB                 0
#define  SI2177_ATV_CVBS_OUT_FINE_PROP_OFFSET_MASK              0xff
#define  SI2177_ATV_CVBS_OUT_FINE_PROP_OFFSET_DEFAULT        0
#define SI2177_ATV_CVBS_OUT_FINE_PROP_OFFSET_OFFSET_MIN   -128
#define SI2177_ATV_CVBS_OUT_FINE_PROP_OFFSET_OFFSET_MAX  127

#endif /* SI2177_ATV_CVBS_OUT_FINE_PROP */

/* SI2177 ATV_HSYNC_OUT property definition */
#define   SI2177_ATV_HSYNC_OUT_PROP 0x0627

#ifdef    SI2177_ATV_HSYNC_OUT_PROP



    typedef struct { /* SI2177_ATV_HSYNC_OUT_PROP_struct */
      unsigned char   gpio_sel;
               char   offset;
      unsigned char   width;
   } SI2177_ATV_HSYNC_OUT_PROP_struct;

   /* ATV_HSYNC_OUT property, GPIO_SEL field definition (NO TITLE)*/
   #define  SI2177_ATV_HSYNC_OUT_PROP_GPIO_SEL_LSB         0
   #define  SI2177_ATV_HSYNC_OUT_PROP_GPIO_SEL_MASK        0x03
   #define  SI2177_ATV_HSYNC_OUT_PROP_GPIO_SEL_DEFAULT    0
    #define SI2177_ATV_HSYNC_OUT_PROP_GPIO_SEL_NONE   0
    #define SI2177_ATV_HSYNC_OUT_PROP_GPIO_SEL_GPIO1  1
    #define SI2177_ATV_HSYNC_OUT_PROP_GPIO_SEL_GPIO2  2

   /* ATV_HSYNC_OUT property, OFFSET field definition (NO TITLE)*/
   #define  SI2177_ATV_HSYNC_OUT_PROP_OFFSET_LSB         8
   #define  SI2177_ATV_HSYNC_OUT_PROP_OFFSET_MASK        0xff
   #define  SI2177_ATV_HSYNC_OUT_PROP_OFFSET_DEFAULT    0
    #define SI2177_ATV_HSYNC_OUT_PROP_OFFSET_OFFSET_MIN  -128
    #define SI2177_ATV_HSYNC_OUT_PROP_OFFSET_OFFSET_MAX  127

   /* ATV_HSYNC_OUT property, WIDTH field definition (NO TITLE)*/
   #define  SI2177_ATV_HSYNC_OUT_PROP_WIDTH_LSB         2
   #define  SI2177_ATV_HSYNC_OUT_PROP_WIDTH_MASK        0x3f
   #define  SI2177_ATV_HSYNC_OUT_PROP_WIDTH_DEFAULT    42
    #define SI2177_ATV_HSYNC_OUT_PROP_WIDTH_WIDTH_MIN  0
    #define SI2177_ATV_HSYNC_OUT_PROP_WIDTH_WIDTH_MAX  51

#endif /* SI2177_ATV_HSYNC_OUT_PROP */

/* SI2177 ATV_IEN property definition */
#define   SI2177_ATV_IEN_PROP                                           0x0601

#ifdef    SI2177_ATV_IEN_PROP

typedef struct { /* SI2177_ATV_IEN_PROP_struct */
      unsigned char   chlien;
      unsigned char   dlien;
      unsigned char   pclien;
      unsigned char   snrhien;
      unsigned char   snrlien;
} si2177_atv_ien_prop_struct;

/* ATV_IEN property, CHLIEN field definition (NO TITLE)*/
#define  SI2177_ATV_IEN_PROP_CHLIEN_LSB                 0
#define  SI2177_ATV_IEN_PROP_CHLIEN_MASK              0x01
   #define  SI2177_ATV_IEN_PROP_CHLIEN_DEFAULT    1
#define SI2177_ATV_IEN_PROP_CHLIEN_DISABLE          0
#define SI2177_ATV_IEN_PROP_CHLIEN_ENABLE           1

/* ATV_IEN property, DLIEN field definition (NO TITLE)*/
#define  SI2177_ATV_IEN_PROP_DLIEN_LSB                    2
#define  SI2177_ATV_IEN_PROP_DLIEN_MASK                 0x01
#define  SI2177_ATV_IEN_PROP_DLIEN_DEFAULT           0
#define SI2177_ATV_IEN_PROP_DLIEN_DISABLE             0
#define SI2177_ATV_IEN_PROP_DLIEN_ENABLE              1

/* ATV_IEN property, PCLIEN field definition (NO TITLE)*/
#define  SI2177_ATV_IEN_PROP_PCLIEN_LSB                  1
#define  SI2177_ATV_IEN_PROP_PCLIEN_MASK               0x01
#define  SI2177_ATV_IEN_PROP_PCLIEN_DEFAULT         0
#define SI2177_ATV_IEN_PROP_PCLIEN_DISABLE           0
#define SI2177_ATV_IEN_PROP_PCLIEN_ENABLE            1

/* ATV_IEN property, SNRHIEN field definition (NO TITLE)*/
#define  SI2177_ATV_IEN_PROP_SNRHIEN_LSB               4
#define  SI2177_ATV_IEN_PROP_SNRHIEN_MASK            0x01
#define  SI2177_ATV_IEN_PROP_SNRHIEN_DEFAULT      0
#define SI2177_ATV_IEN_PROP_SNRHIEN_DISABLE        0
#define SI2177_ATV_IEN_PROP_SNRHIEN_ENABLE         1

/* ATV_IEN property, SNRLIEN field definition (NO TITLE)*/
#define  SI2177_ATV_IEN_PROP_SNRLIEN_LSB                3
#define  SI2177_ATV_IEN_PROP_SNRLIEN_MASK             0x01
#define  SI2177_ATV_IEN_PROP_SNRLIEN_DEFAULT       0
#define SI2177_ATV_IEN_PROP_SNRLIEN_DISABLE         0
#define SI2177_ATV_IEN_PROP_SNRLIEN_ENABLE          1

#endif /* SI2177_ATV_IEN_PROP */

/* SI2177 ATV_INT_SENSE property definition */
#define   SI2177_ATV_INT_SENSE_PROP                            0x0613

#ifdef    SI2177_ATV_INT_SENSE_PROP

typedef struct { /* SI2177_ATV_INT_SENSE_PROP_struct */
      unsigned char   chlnegen;
      unsigned char   chlposen;
      unsigned char   dlnegen;
      unsigned char   dlposen;
      unsigned char   pclnegen;
      unsigned char   pclposen;
      unsigned char   snrhnegen;
      unsigned char   snrhposen;
      unsigned char   snrlnegen;
      unsigned char   snrlposen;
} si2177_atv_int_sense_prop_struct;

/* ATV_INT_SENSE property, CHLNEGEN field definition (NO TITLE)*/
#define  SI2177_ATV_INT_SENSE_PROP_CHLNEGEN_LSB             0
#define  SI2177_ATV_INT_SENSE_PROP_CHLNEGEN_MASK          0x01
#define  SI2177_ATV_INT_SENSE_PROP_CHLNEGEN_DEFAULT    0
#define SI2177_ATV_INT_SENSE_PROP_CHLNEGEN_DISABLE      0
#define SI2177_ATV_INT_SENSE_PROP_CHLNEGEN_ENABLE       1

/* ATV_INT_SENSE property, CHLPOSEN field definition (NO TITLE)*/
#define  SI2177_ATV_INT_SENSE_PROP_CHLPOSEN_LSB             8
#define  SI2177_ATV_INT_SENSE_PROP_CHLPOSEN_MASK          0x01
#define  SI2177_ATV_INT_SENSE_PROP_CHLPOSEN_DEFAULT   1
#define SI2177_ATV_INT_SENSE_PROP_CHLPOSEN_DISABLE     0
#define SI2177_ATV_INT_SENSE_PROP_CHLPOSEN_ENABLE      1

/* ATV_INT_SENSE property, DLNEGEN field definition (NO TITLE)*/
#define  SI2177_ATV_INT_SENSE_PROP_DLNEGEN_LSB               2
#define  SI2177_ATV_INT_SENSE_PROP_DLNEGEN_MASK            0x01
#define  SI2177_ATV_INT_SENSE_PROP_DLNEGEN_DEFAULT     0
#define SI2177_ATV_INT_SENSE_PROP_DLNEGEN_DISABLE       0
#define SI2177_ATV_INT_SENSE_PROP_DLNEGEN_ENABLE        1

/* ATV_INT_SENSE property, DLPOSEN field definition (NO TITLE)*/
#define  SI2177_ATV_INT_SENSE_PROP_DLPOSEN_LSB              10
#define  SI2177_ATV_INT_SENSE_PROP_DLPOSEN_MASK           0x01
#define  SI2177_ATV_INT_SENSE_PROP_DLPOSEN_DEFAULT    1
#define SI2177_ATV_INT_SENSE_PROP_DLPOSEN_DISABLE      0
#define SI2177_ATV_INT_SENSE_PROP_DLPOSEN_ENABLE       1

/* ATV_INT_SENSE property, PCLNEGEN field definition (NO TITLE)*/
#define  SI2177_ATV_INT_SENSE_PROP_PCLNEGEN_LSB           1
#define  SI2177_ATV_INT_SENSE_PROP_PCLNEGEN_MASK        0x01
#define  SI2177_ATV_INT_SENSE_PROP_PCLNEGEN_DEFAULT  0
#define SI2177_ATV_INT_SENSE_PROP_PCLNEGEN_DISABLE    0
#define SI2177_ATV_INT_SENSE_PROP_PCLNEGEN_ENABLE     1

/* ATV_INT_SENSE property, PCLPOSEN field definition (NO TITLE)*/
#define  SI2177_ATV_INT_SENSE_PROP_PCLPOSEN_LSB             9
#define  SI2177_ATV_INT_SENSE_PROP_PCLPOSEN_MASK          0x01
#define  SI2177_ATV_INT_SENSE_PROP_PCLPOSEN_DEFAULT    1
#define SI2177_ATV_INT_SENSE_PROP_PCLPOSEN_DISABLE      0
#define SI2177_ATV_INT_SENSE_PROP_PCLPOSEN_ENABLE       1

/* ATV_INT_SENSE property, SNRHNEGEN field definition (NO TITLE)*/
#define  SI2177_ATV_INT_SENSE_PROP_SNRHNEGEN_LSB           4
#define  SI2177_ATV_INT_SENSE_PROP_SNRHNEGEN_MASK        0x01
#define  SI2177_ATV_INT_SENSE_PROP_SNRHNEGEN_DEFAULT  0
#define SI2177_ATV_INT_SENSE_PROP_SNRHNEGEN_DISABLE    0
#define SI2177_ATV_INT_SENSE_PROP_SNRHNEGEN_ENABLE     1

/* ATV_INT_SENSE property, SNRHPOSEN field definition (NO TITLE)*/
#define  SI2177_ATV_INT_SENSE_PROP_SNRHPOSEN_LSB            12
#define  SI2177_ATV_INT_SENSE_PROP_SNRHPOSEN_MASK         0x01
#define  SI2177_ATV_INT_SENSE_PROP_SNRHPOSEN_DEFAULT   1
#define SI2177_ATV_INT_SENSE_PROP_SNRHPOSEN_DISABLE     0
#define SI2177_ATV_INT_SENSE_PROP_SNRHPOSEN_ENABLE      1

/* ATV_INT_SENSE property, SNRLNEGEN field definition (NO TITLE)*/
#define  SI2177_ATV_INT_SENSE_PROP_SNRLNEGEN_LSB             3
#define  SI2177_ATV_INT_SENSE_PROP_SNRLNEGEN_MASK          0x01
#define  SI2177_ATV_INT_SENSE_PROP_SNRLNEGEN_DEFAULT   0
#define SI2177_ATV_INT_SENSE_PROP_SNRLNEGEN_DISABLE     0
#define SI2177_ATV_INT_SENSE_PROP_SNRLNEGEN_ENABLE      1

/* ATV_INT_SENSE property, SNRLPOSEN field definition (NO TITLE)*/
#define  SI2177_ATV_INT_SENSE_PROP_SNRLPOSEN_LSB            11
#define  SI2177_ATV_INT_SENSE_PROP_SNRLPOSEN_MASK         0x01
#define  SI2177_ATV_INT_SENSE_PROP_SNRLPOSEN_DEFAULT   1
#define SI2177_ATV_INT_SENSE_PROP_SNRLPOSEN_DISABLE     0
#define SI2177_ATV_INT_SENSE_PROP_SNRLPOSEN_ENABLE      1

#endif /* SI2177_ATV_INT_SENSE_PROP */

/* SI2177 ATV_MIN_LVL_LOCK property definition */
#define   SI2177_ATV_MIN_LVL_LOCK_PROP                                  0x060f

#ifdef    SI2177_ATV_MIN_LVL_LOCK_PROP

typedef struct { /* SI2177_ATV_MIN_LVL_LOCK_PROP_struct */
      unsigned char   thrs;
} si2177_atv_min_lvl_lock_prop_struct;

/* ATV_MIN_LVL_LOCK property, THRS field definition (NO TITLE)*/
#define  SI2177_ATV_MIN_LVL_LOCK_PROP_THRS_LSB              0
#define  SI2177_ATV_MIN_LVL_LOCK_PROP_THRS_MASK           0xff
#define  SI2177_ATV_MIN_LVL_LOCK_PROP_THRS_DEFAULT     34
#endif /* SI2177_ATV_MIN_LVL_LOCK_PROP */
/* SI2177 ATV_PGA_TARGET property definition */
#define   SI2177_ATV_PGA_TARGET_PROP 0x0617

#ifdef    SI2177_ATV_PGA_TARGET_PROP
  #define SI2177_ATV_PGA_TARGET_PROP_CODE 0x000617


    typedef struct { /* SI2177_ATV_PGA_TARGET_PROP_struct */
      unsigned char   override_enable;
               char   pga_target;
   } SI2177_ATV_PGA_TARGET_PROP_struct;

   /* ATV_PGA_TARGET property, OVERRIDE_ENABLE field definition (NO TITLE)*/
   #define  SI2177_ATV_PGA_TARGET_PROP_OVERRIDE_ENABLE_LSB         8
   #define  SI2177_ATV_PGA_TARGET_PROP_OVERRIDE_ENABLE_MASK        0x01
   #define  SI2177_ATV_PGA_TARGET_PROP_OVERRIDE_ENABLE_DEFAULT    0
    #define SI2177_ATV_PGA_TARGET_PROP_OVERRIDE_ENABLE_DISABLE  0
    #define SI2177_ATV_PGA_TARGET_PROP_OVERRIDE_ENABLE_ENABLE   1

   /* ATV_PGA_TARGET property, PGA_TARGET field definition (NO TITLE)*/
   #define  SI2177_ATV_PGA_TARGET_PROP_PGA_TARGET_LSB         0
   #define  SI2177_ATV_PGA_TARGET_PROP_PGA_TARGET_MASK        0xff
   #define  SI2177_ATV_PGA_TARGET_PROP_PGA_TARGET_DEFAULT    0
    #define SI2177_ATV_PGA_TARGET_PROP_PGA_TARGET_PGA_TARGET_MIN  -13
    #define SI2177_ATV_PGA_TARGET_PROP_PGA_TARGET_PGA_TARGET_MAX  7

#endif /* SI2177_ATV_PGA_TARGET_PROP */

/* SI2177 ATV_RF_TOP property definition */
#define   SI2177_ATV_RF_TOP_PROP                                              0x0612

#ifdef    SI2177_ATV_RF_TOP_PROP

typedef struct { /* SI2177_ATV_RF_TOP_PROP_struct */
      unsigned char   atv_rf_top;
} si2177_atv_rf_top_prop_struct;

/* ATV_RF_TOP property, ATV_RF_TOP field definition (NO TITLE)*/
#define SI2177_ATV_RF_TOP_PROP_ATV_RF_TOP_LSB              0
#define SI2177_ATV_RF_TOP_PROP_ATV_RF_TOP_MASK           0xff
#define SI2177_ATV_RF_TOP_PROP_ATV_RF_TOP_DEFAULT     0
#define SI2177_ATV_RF_TOP_PROP_ATV_RF_TOP_AUTO            0
    #define SI2177_ATV_RF_TOP_PROP_ATV_RF_TOP_P2DB   4
    #define SI2177_ATV_RF_TOP_PROP_ATV_RF_TOP_P1DB   5
#define SI2177_ATV_RF_TOP_PROP_ATV_RF_TOP_0DB               6
#define SI2177_ATV_RF_TOP_PROP_ATV_RF_TOP_M1DB            7
#define SI2177_ATV_RF_TOP_PROP_ATV_RF_TOP_M2DB            8
    #define SI2177_ATV_RF_TOP_PROP_ATV_RF_TOP_M3DB   9
#define SI2177_ATV_RF_TOP_PROP_ATV_RF_TOP_M4DB            10
#define SI2177_ATV_RF_TOP_PROP_ATV_RF_TOP_M5DB            11
#define SI2177_ATV_RF_TOP_PROP_ATV_RF_TOP_M6DB            12
#define SI2177_ATV_RF_TOP_PROP_ATV_RF_TOP_M7DB            13
#define SI2177_ATV_RF_TOP_PROP_ATV_RF_TOP_M8DB            14
#define SI2177_ATV_RF_TOP_PROP_ATV_RF_TOP_M9DB            15
    #define SI2177_ATV_RF_TOP_PROP_ATV_RF_TOP_M10DB  16
    #define SI2177_ATV_RF_TOP_PROP_ATV_RF_TOP_M11DB  17

#endif /* SI2177_ATV_RF_TOP_PROP */

/* SI2177 ATV_RSQ_RSSI_THRESHOLD property definition */
#define   SI2177_ATV_RSQ_RSSI_THRESHOLD_PROP                0x0605

#ifdef    SI2177_ATV_RSQ_RSSI_THRESHOLD_PROP

typedef struct { /* SI2177_ATV_RSQ_RSSI_THRESHOLD_PROP_struct */
               char   hi;
               char   lo;
} si2177_atv_rsq_rssi_threshold_prop_struct;

/* ATV_RSQ_RSSI_THRESHOLD property, HI field definition (NO TITLE)*/
#define  SI2177_ATV_RSQ_RSSI_THRESHOLD_PROP_HI_LSB              8
#define  SI2177_ATV_RSQ_RSSI_THRESHOLD_PROP_HI_MASK           0xff
#define  SI2177_ATV_RSQ_RSSI_THRESHOLD_PROP_HI_DEFAULT    0
#define SI2177_ATV_RSQ_RSSI_THRESHOLD_PROP_HI_HI_MIN         -128
#define SI2177_ATV_RSQ_RSSI_THRESHOLD_PROP_HI_HI_MAX        127

/* ATV_RSQ_RSSI_THRESHOLD property, LO field definition (NO TITLE)*/
#define  SI2177_ATV_RSQ_RSSI_THRESHOLD_PROP_LO_LSB             0
#define  SI2177_ATV_RSQ_RSSI_THRESHOLD_PROP_LO_MASK          0xff
#define  SI2177_ATV_RSQ_RSSI_THRESHOLD_PROP_LO_DEFAULT    -70
#define SI2177_ATV_RSQ_RSSI_THRESHOLD_PROP_LO_LO_MIN        -128
#define SI2177_ATV_RSQ_RSSI_THRESHOLD_PROP_LO_LO_MAX       127

#endif /* SI2177_ATV_RSQ_RSSI_THRESHOLD_PROP */

/* SI2177 ATV_RSQ_SNR_THRESHOLD property definition */
#define   SI2177_ATV_RSQ_SNR_THRESHOLD_PROP                           0x0606

#ifdef    SI2177_ATV_RSQ_SNR_THRESHOLD_PROP

typedef struct { /* SI2177_ATV_RSQ_SNR_THRESHOLD_PROP_struct */
      unsigned char   hi;
      unsigned char   lo;
} si2177_atv_rsq_snr_threshold_prop_struct;

/* ATV_RSQ_SNR_THRESHOLD property, HI field definition (NO TITLE)*/
#define  SI2177_ATV_RSQ_SNR_THRESHOLD_PROP_HI_LSB            8
#define  SI2177_ATV_RSQ_SNR_THRESHOLD_PROP_HI_MASK         0xff
#define  SI2177_ATV_RSQ_SNR_THRESHOLD_PROP_HI_DEFAULT   45
#define SI2177_ATV_RSQ_SNR_THRESHOLD_PROP_HI_HI_MIN         0
#define SI2177_ATV_RSQ_SNR_THRESHOLD_PROP_HI_HI_MAX       255

/* ATV_RSQ_SNR_THRESHOLD property, LO field definition (NO TITLE)*/
#define  SI2177_ATV_RSQ_SNR_THRESHOLD_PROP_LO_LSB           0
#define  SI2177_ATV_RSQ_SNR_THRESHOLD_PROP_LO_MASK        0xff
#define  SI2177_ATV_RSQ_SNR_THRESHOLD_PROP_LO_DEFAULT  25
#define SI2177_ATV_RSQ_SNR_THRESHOLD_PROP_LO_LO_MIN       0
#define SI2177_ATV_RSQ_SNR_THRESHOLD_PROP_LO_LO_MAX     255

#endif /* SI2177_ATV_RSQ_SNR_THRESHOLD_PROP */

/* SI2177 ATV_SIF_OUT property definition */
#define   SI2177_ATV_SIF_OUT_PROP                                          0x060a

#ifdef    SI2177_ATV_SIF_OUT_PROP

typedef struct { /* SI2177_ATV_SIF_OUT_PROP_struct */
      unsigned char   amp;
      unsigned char   offset;
} si2177_atv_sif_out_prop_struct;

/* ATV_SIF_OUT property, AMP field definition (NO TITLE)*/
#define  SI2177_ATV_SIF_OUT_PROP_AMP_LSB                          8
#define  SI2177_ATV_SIF_OUT_PROP_AMP_MASK                       0xff
#define  SI2177_ATV_SIF_OUT_PROP_AMP_DEFAULT                 60
#define SI2177_ATV_SIF_OUT_PROP_AMP_AMP_MIN                  0
#define SI2177_ATV_SIF_OUT_PROP_AMP_AMP_MAX                 255

/* ATV_SIF_OUT property, OFFSET field definition (NO TITLE)*/
#define  SI2177_ATV_SIF_OUT_PROP_OFFSET_LSB                    0
#define  SI2177_ATV_SIF_OUT_PROP_OFFSET_MASK                 0xff
#define  SI2177_ATV_SIF_OUT_PROP_OFFSET_DEFAULT          135
#define SI2177_ATV_SIF_OUT_PROP_OFFSET_OFFSET_MIN     0
#define SI2177_ATV_SIF_OUT_PROP_OFFSET_OFFSET_MAX    255

#endif /* SI2177_ATV_SIF_OUT_PROP */

/* SI2177 ATV_SOUND_AGC_LIMIT property definition */
#define   SI2177_ATV_SOUND_AGC_LIMIT_PROP                       0x0618

#ifdef    SI2177_ATV_SOUND_AGC_LIMIT_PROP

typedef struct { /* SI2177_ATV_SOUND_AGC_LIMIT_PROP_struct */
               char   max_gain;
               char   min_gain;
} si2177_atv_sound_agc_limit_prop_struct;

/* ATV_SOUND_AGC_LIMIT property, MAX_GAIN field definition (NO TITLE)*/
#define  SI2177_ATV_SOUND_AGC_LIMIT_PROP_MAX_GAIN_LSB                      0
#define  SI2177_ATV_SOUND_AGC_LIMIT_PROP_MAX_GAIN_MASK                  0xff
#define  SI2177_ATV_SOUND_AGC_LIMIT_PROP_MAX_GAIN_DEFAULT            84
#define SI2177_ATV_SOUND_AGC_LIMIT_PROP_MAX_GAIN_MAX_GAIN_MIN  -84
#define SI2177_ATV_SOUND_AGC_LIMIT_PROP_MAX_GAIN_MAX_GAIN_MAX  84

/* ATV_SOUND_AGC_LIMIT property, MIN_GAIN field definition (NO TITLE)*/
#define  SI2177_ATV_SOUND_AGC_LIMIT_PROP_MIN_GAIN_LSB                       8
#define  SI2177_ATV_SOUND_AGC_LIMIT_PROP_MIN_GAIN_MASK                    0xff
#define  SI2177_ATV_SOUND_AGC_LIMIT_PROP_MIN_GAIN_DEFAULT              -84
#define SI2177_ATV_SOUND_AGC_LIMIT_PROP_MIN_GAIN_MIN_GAIN_MIN      -84
#define SI2177_ATV_SOUND_AGC_LIMIT_PROP_MIN_GAIN_MIN_GAIN_MAX     84

#endif /* SI2177_ATV_SOUND_AGC_LIMIT_PROP */
/* SI2177 ATV_SOUND_AGC_SPEED property definition */
#define  SI2177_ATV_SOUND_AGC_SPEED_PROP 0x0619

#ifdef   SI2177_ATV_SOUND_AGC_SPEED_PROP

    typedef struct { /* SI2177_ATV_SOUND_AGC_SPEED_PROP_struct */
      unsigned char   other_systems;
      unsigned char   system_l;
   } si2177_atv_sound_agc_speed_prop_struct;

   /* ATV_SOUND_AGC_SPEED property, OTHER_SYSTEMS field definition (NO TITLE)*/
   #define  SI2177_ATV_SOUND_AGC_SPEED_PROP_OTHER_SYSTEMS_LSB         8
   #define  SI2177_ATV_SOUND_AGC_SPEED_PROP_OTHER_SYSTEMS_MASK        0xff
   #define  SI2177_ATV_SOUND_AGC_SPEED_PROP_OTHER_SYSTEMS_DEFAULT    4
    #define SI2177_ATV_SOUND_AGC_SPEED_PROP_OTHER_SYSTEMS_OTHER_SYSTEMS_MIN  0
    #define SI2177_ATV_SOUND_AGC_SPEED_PROP_OTHER_SYSTEMS_OTHER_SYSTEMS_MAX  15

   /* ATV_SOUND_AGC_SPEED property, SYSTEM_L field definition (NO TITLE)*/
   #define  SI2177_ATV_SOUND_AGC_SPEED_PROP_SYSTEM_L_LSB         0
   #define  SI2177_ATV_SOUND_AGC_SPEED_PROP_SYSTEM_L_MASK        0xff
   #define  SI2177_ATV_SOUND_AGC_SPEED_PROP_SYSTEM_L_DEFAULT    5
    #define SI2177_ATV_SOUND_AGC_SPEED_PROP_SYSTEM_L_SYSTEM_L_MIN  0
    #define SI2177_ATV_SOUND_AGC_SPEED_PROP_SYSTEM_L_SYSTEM_L_MAX  15

#endif /* Si2177_ATV_SOUND_AGC_SPEED_PROP */

/* SI2177 ATV_VIDEO_EQUALIZER property definition */
#define   SI2177_ATV_VIDEO_EQUALIZER_PROP                                       0x0608

#ifdef    SI2177_ATV_VIDEO_EQUALIZER_PROP

typedef struct { /* SI2177_ATV_VIDEO_EQUALIZER_PROP_struct */
               char   slope;
} si2177_atv_video_equalizer_prop_struct;

/* ATV_VIDEO_EQUALIZER property, SLOPE field definition (NO TITLE)*/
#define  SI2177_ATV_VIDEO_EQUALIZER_PROP_SLOPE_LSB                 0
#define  SI2177_ATV_VIDEO_EQUALIZER_PROP_SLOPE_MASK              0xff
#define  SI2177_ATV_VIDEO_EQUALIZER_PROP_SLOPE_DEFAULT       0
#define SI2177_ATV_VIDEO_EQUALIZER_PROP_SLOPE_SLOPE_MIN    -8
#define SI2177_ATV_VIDEO_EQUALIZER_PROP_SLOPE_SLOPE_MAX   7

#endif /* SI2177_ATV_VIDEO_EQUALIZER_PROP */

/* SI2177 ATV_VIDEO_MODE property definition */
#define   SI2177_ATV_VIDEO_MODE_PROP                                                0x0604

#ifdef    SI2177_ATV_VIDEO_MODE_PROP

typedef struct { /* SI2177_ATV_VIDEO_MODE_PROP_struct */
      unsigned char   color;
      unsigned char   invert_signal;
      unsigned char   trans;
      unsigned char   video_sys;
} si2177_atv_video_mode_prop_struct;

/* ATV_VIDEO_MODE property, COLOR field definition (NO TITLE)*/
#define  SI2177_ATV_VIDEO_MODE_PROP_COLOR_LSB                         4
#define  SI2177_ATV_VIDEO_MODE_PROP_COLOR_MASK                      0x01
#define  SI2177_ATV_VIDEO_MODE_PROP_COLOR_DEFAULT                0
#define SI2177_ATV_VIDEO_MODE_PROP_COLOR_PAL_NTSC               0
#define SI2177_ATV_VIDEO_MODE_PROP_COLOR_SECAM                     1

/* ATV_VIDEO_MODE property, INVERT_SIGNAL field definition (NO TITLE)*/
#define  SI2177_ATV_VIDEO_MODE_PROP_INVERT_SIGNAL_LSB          10
#define  SI2177_ATV_VIDEO_MODE_PROP_INVERT_SIGNAL_MASK       0x01
#define  SI2177_ATV_VIDEO_MODE_PROP_INVERT_SIGNAL_DEFAULT  0
#define SI2177_ATV_VIDEO_MODE_PROP_INVERT_SIGNAL_NORMAL     0
#define SI2177_ATV_VIDEO_MODE_PROP_INVERT_SIGNAL_INVERTED  1

/* ATV_VIDEO_MODE property, TRANS field definition (NO TITLE)*/
#define  SI2177_ATV_VIDEO_MODE_PROP_TRANS_LSB                           8
#define  SI2177_ATV_VIDEO_MODE_PROP_TRANS_MASK                        0x01
#define  SI2177_ATV_VIDEO_MODE_PROP_TRANS_DEFAULT                 0
#define SI2177_ATV_VIDEO_MODE_PROP_TRANS_TERRESTRIAL          0
#define SI2177_ATV_VIDEO_MODE_PROP_TRANS_CABLE                       1

/* ATV_VIDEO_MODE property, VIDEO_SYS field definition (NO TITLE)*/
#define  SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_LSB                   0
#define  SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_MASK                0x07
#define  SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_DEFAULT         0
#define SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_B                        0
#define SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_GH                     1
#define SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_M                        2
#define SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_N                        3
#define SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_I                         4
#define SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_DK                     5
#define SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_L                        6
#define SI2177_ATV_VIDEO_MODE_PROP_VIDEO_SYS_LP                      7

#endif /* SI2177_ATV_VIDEO_MODE_PROP */

/* SI2177 ATV_VSNR_CAP property definition */
#define   SI2177_ATV_VSNR_CAP_PROP                                                  0x0616

#ifdef    SI2177_ATV_VSNR_CAP_PROP

typedef struct { /* SI2177_ATV_VSNR_CAP_PROP_struct */
      unsigned char   atv_vsnr_cap;
} si2177_atv_vsnr_cap_prop_struct;

/* ATV_VSNR_CAP property, ATV_VSNR_CAP field definition (NO TITLE)*/
#define  SI2177_ATV_VSNR_CAP_PROP_ATV_VSNR_CAP_LSB                               0
#define  SI2177_ATV_VSNR_CAP_PROP_ATV_VSNR_CAP_MASK                            0xff
#define  SI2177_ATV_VSNR_CAP_PROP_ATV_VSNR_CAP_DEFAULT                      0
#define SI2177_ATV_VSNR_CAP_PROP_ATV_VSNR_CAP_ATV_VSNR_CAP_MIN   0
#define SI2177_ATV_VSNR_CAP_PROP_ATV_VSNR_CAP_ATV_VSNR_CAP_MAX  127

#endif /* SI2177_ATV_VSNR_CAP_PROP */

/* SI2177 ATV_VSYNC_TRACKING property definition */
//#define   SI2177_ATV_VSYNC_TRACKING_PROP                                                        0x0615

#ifdef    SI2177_ATV_VSYNC_TRACKING_PROP

typedef struct { /* SI2177_ATV_VSYNC_TRACKING_PROP_struct */
      unsigned char   min_fields_to_unlock;
      unsigned char   min_pulses_to_lock;
} si2177_atv_vsync_tracking_prop_struct;

/* ATV_VSYNC_TRACKING property, MIN_FIELDS_TO_UNLOCK field definition (NO TITLE)*/
#define  SI2177_ATV_VSYNC_TRACKING_PROP_MIN_FIELDS_TO_UNLOCK_LSB            8
#define  SI2177_ATV_VSYNC_TRACKING_PROP_MIN_FIELDS_TO_UNLOCK_MASK         0xff
#define  SI2177_ATV_VSYNC_TRACKING_PROP_MIN_FIELDS_TO_UNLOCK_DEFAULT   16
#define SI2177_ATV_VSYNC_TRACKING_PROP_MIN_FIELDS_TO_UNLOCK_MIN_FIELDS_TO_UNLOCK_MIN  0
#define SI2177_ATV_VSYNC_TRACKING_PROP_MIN_FIELDS_TO_UNLOCK_MIN_FIELDS_TO_UNLOCK_MAX  255

/* ATV_VSYNC_TRACKING property, MIN_PULSES_TO_LOCK field definition (NO TITLE)*/
#define  SI2177_ATV_VSYNC_TRACKING_PROP_MIN_PULSES_TO_LOCK_LSB                0
#define  SI2177_ATV_VSYNC_TRACKING_PROP_MIN_PULSES_TO_LOCK_MASK             0xff
#define  SI2177_ATV_VSYNC_TRACKING_PROP_MIN_PULSES_TO_LOCK_DEFAULT      4
#define SI2177_ATV_VSYNC_TRACKING_PROP_MIN_PULSES_TO_LOCK_MIN_PULSES_TO_LOCK_MIN   0
#define SI2177_ATV_VSYNC_TRACKING_PROP_MIN_PULSES_TO_LOCK_MIN_PULSES_TO_LOCK_MAX  9

#endif /* SI2177_ATV_VSYNC_TRACKING_PROP */

/* SI2177 CRYSTAL_TRIM property definition */
#define   SI2177_CRYSTAL_TRIM_PROP                                        0x0402

#ifdef    SI2177_CRYSTAL_TRIM_PROP

typedef struct { /* SI2177_CRYSTAL_TRIM_PROP_struct */
      unsigned char   xo_cap;
} si2177_crystal_trim_prop_struct;

/* CRYSTAL_TRIM property, XO_CAP field definition (NO TITLE)*/
#define  SI2177_CRYSTAL_TRIM_PROP_XO_CAP_LSB		 0
#define  SI2177_CRYSTAL_TRIM_PROP_XO_CAP_MASK		 0x0f
#define  SI2177_CRYSTAL_TRIM_PROP_XO_CAP_DEFAULT	8
 #define SI2177_CRYSTAL_TRIM_PROP_XO_CAP_4P7PF	 0
 #define SI2177_CRYSTAL_TRIM_PROP_XO_CAP_5P2PF	 2
 #define SI2177_CRYSTAL_TRIM_PROP_XO_CAP_5P7PF	 4
 #define SI2177_CRYSTAL_TRIM_PROP_XO_CAP_6P2PF	 6
 #define SI2177_CRYSTAL_TRIM_PROP_XO_CAP_6P7PF	 8
 #define SI2177_CRYSTAL_TRIM_PROP_XO_CAP_7P2PF	 10
 #define SI2177_CRYSTAL_TRIM_PROP_XO_CAP_7P7PF	 12
 #define SI2177_CRYSTAL_TRIM_PROP_XO_CAP_8P2PF	 14


/* SI2177 DTV_AGC_FREEZE_INPUT property definition */

#endif /* SI2177_CRYSTAL_TRIM_PROP */

/* SI2177 DTV_AGC_SPEED property definition */
#define   SI2177_DTV_AGC_SPEED_PROP 0x0708

#ifdef    SI2177_DTV_AGC_SPEED_PROP

typedef struct { /* SI2177_DTV_AGC_SPEED_PROP_struct */
      unsigned char   agc_decim;
      unsigned char   if_agc_speed;
} si2177_dtv_agc_speed_prop_struct;

/* DTV_AGC_SPEED property, AGC_DECIM field definition (NO TITLE)*/
#define  SI2177_DTV_AGC_SPEED_PROP_AGC_DECIM_LSB            8
#define  SI2177_DTV_AGC_SPEED_PROP_AGC_DECIM_MASK         0x03
#define  SI2177_DTV_AGC_SPEED_PROP_AGC_DECIM_DEFAULT   0
#define SI2177_DTV_AGC_SPEED_PROP_AGC_DECIM_OFF             0
#define SI2177_DTV_AGC_SPEED_PROP_AGC_DECIM_2                  1
#define SI2177_DTV_AGC_SPEED_PROP_AGC_DECIM_4                  2
#define SI2177_DTV_AGC_SPEED_PROP_AGC_DECIM_8                  3

/* DTV_AGC_SPEED property, IF_AGC_SPEED field definition (NO TITLE)*/
#define  SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_LSB             0
#define  SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_MASK          0xff
#define  SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_DEFAULT    0
#define SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_AUTO            0
#define SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_39                  39
#define SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_54                  54
#define SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_63                  63
#define SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_89                  89
#define SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_105                105
#define SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_121                121
#define SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_137                137
#define SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_158                158
#define SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_172                172
#define SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_185                185
#define SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_196                196
#define SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_206                206
#define SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_216                216
#define SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_219                219
#define SI2177_DTV_AGC_SPEED_PROP_IF_AGC_SPEED_222                222

#endif /* SI2177_DTV_AGC_SPEED_PROP */

/* SI2177 DTV_CONFIG_IF_PORT property definition */
#define   SI2177_DTV_CONFIG_IF_PORT_PROP                                    0x0702

#ifdef    SI2177_DTV_CONFIG_IF_PORT_PROP

typedef struct { /* SI2177_DTV_CONFIG_IF_PORT_PROP_struct */
      unsigned char   dtv_agc_source;
      unsigned char   dtv_out_type;
} si2177_dtv_config_if_port_prop_struct;

/* DTV_CONFIG_IF_PORT property, DTV_AGC_SOURCE field definition (NO TITLE)*/
#define  SI2177_DTV_CONFIG_IF_PORT_PROP_DTV_AGC_SOURCE_LSB                      8
#define  SI2177_DTV_CONFIG_IF_PORT_PROP_DTV_AGC_SOURCE_MASK                   0x07
#define  SI2177_DTV_CONFIG_IF_PORT_PROP_DTV_AGC_SOURCE_DEFAULT             0
#define SI2177_DTV_CONFIG_IF_PORT_PROP_DTV_AGC_SOURCE_INTERNAL            0
    #define SI2177_DTV_CONFIG_IF_PORT_PROP_DTV_AGC_SOURCE_AGC1_3DB   1
    #define SI2177_DTV_CONFIG_IF_PORT_PROP_DTV_AGC_SOURCE_AGC2_3DB   2

/* DTV_CONFIG_IF_PORT property, DTV_OUT_TYPE field definition (NO TITLE)*/
#define  SI2177_DTV_CONFIG_IF_PORT_PROP_DTV_OUT_TYPE_LSB                            0
#define  SI2177_DTV_CONFIG_IF_PORT_PROP_DTV_OUT_TYPE_MASK                         0x0f
#define  SI2177_DTV_CONFIG_IF_PORT_PROP_DTV_OUT_TYPE_DEFAULT                   0
#define SI2177_DTV_CONFIG_IF_PORT_PROP_DTV_OUT_TYPE_LIF_IF1                        0
#define SI2177_DTV_CONFIG_IF_PORT_PROP_DTV_OUT_TYPE_LIF_IF2                        1
#define SI2177_DTV_CONFIG_IF_PORT_PROP_DTV_OUT_TYPE_LIF_SE_IF1A               4
#define SI2177_DTV_CONFIG_IF_PORT_PROP_DTV_OUT_TYPE_LIF_SE_IF2A               5

#endif /* SI2177_DTV_CONFIG_IF_PORT_PROP */

/* SI2177 DTV_EXT_AGC property definition */
#define   SI2177_DTV_EXT_AGC_PROP 0x0705

#ifdef    SI2177_DTV_EXT_AGC_PROP

typedef struct { /* SI2177_DTV_EXT_AGC_PROP_struct */
      unsigned char   max_10mv;
      unsigned char   min_10mv;
} si2177_dtv_ext_agc_prop_struct;

/* DTV_EXT_AGC property, MAX_10MV field definition (NO TITLE)*/
#define  SI2177_DTV_EXT_AGC_PROP_MAX_10MV_LSB                         8
#define  SI2177_DTV_EXT_AGC_PROP_MAX_10MV_MASK                      0xff
#define  SI2177_DTV_EXT_AGC_PROP_MAX_10MV_DEFAULT                250
#define SI2177_DTV_EXT_AGC_PROP_MAX_10MV_MAX_10MV_MIN      0
#define SI2177_DTV_EXT_AGC_PROP_MAX_10MV_MAX_10MV_MAX     255

/* DTV_EXT_AGC property, MIN_10MV field definition (NO TITLE)*/
#define  SI2177_DTV_EXT_AGC_PROP_MIN_10MV_LSB                          0
#define  SI2177_DTV_EXT_AGC_PROP_MIN_10MV_MASK                       0xff
#define  SI2177_DTV_EXT_AGC_PROP_MIN_10MV_DEFAULT                 50
#define SI2177_DTV_EXT_AGC_PROP_MIN_10MV_MIN_10MV_MIN        0
#define SI2177_DTV_EXT_AGC_PROP_MIN_10MV_MIN_10MV_MAX       255

#endif /* SI2177_DTV_EXT_AGC_PROP */

/* SI2177 DTV_IEN property definition */
#define   SI2177_DTV_IEN_PROP 0x0701

#ifdef    SI2177_DTV_IEN_PROP

typedef struct { /* SI2177_DTV_IEN_PROP_struct */
      unsigned char   chlien;
} si2177_dtv_ien_prop_struct;

/* DTV_IEN property, CHLIEN field definition (NO TITLE)*/
#define  SI2177_DTV_IEN_PROP_CHLIEN_LSB                 0
#define  SI2177_DTV_IEN_PROP_CHLIEN_MASK              0x01
   #define  SI2177_DTV_IEN_PROP_CHLIEN_DEFAULT    1
#define SI2177_DTV_IEN_PROP_CHLIEN_DISABLE          0
#define SI2177_DTV_IEN_PROP_CHLIEN_ENABLE           1

#endif /* SI2177_DTV_IEN_PROP */

/* SI2177 DTV_INT_SENSE property definition */
#define   SI2177_DTV_INT_SENSE_PROP 0x070a

#ifdef    SI2177_DTV_INT_SENSE_PROP

typedef struct { /* SI2177_DTV_INT_SENSE_PROP_struct */
      unsigned char   chlnegen;
      unsigned char   chlposen;
} si2177_dtv_int_sense_prop_struct;

/* DTV_INT_SENSE property, CHLNEGEN field definition (NO TITLE)*/
#define  SI2177_DTV_INT_SENSE_PROP_CHLNEGEN_LSB             0
#define  SI2177_DTV_INT_SENSE_PROP_CHLNEGEN_MASK          0x01
#define  SI2177_DTV_INT_SENSE_PROP_CHLNEGEN_DEFAULT    0
#define SI2177_DTV_INT_SENSE_PROP_CHLNEGEN_DISABLE      0
#define SI2177_DTV_INT_SENSE_PROP_CHLNEGEN_ENABLE       1

/* DTV_INT_SENSE property, CHLPOSEN field definition (NO TITLE)*/
#define  SI2177_DTV_INT_SENSE_PROP_CHLPOSEN_LSB             8
#define  SI2177_DTV_INT_SENSE_PROP_CHLPOSEN_MASK          0x01
#define  SI2177_DTV_INT_SENSE_PROP_CHLPOSEN_DEFAULT    1
#define SI2177_DTV_INT_SENSE_PROP_CHLPOSEN_DISABLE      0
#define SI2177_DTV_INT_SENSE_PROP_CHLPOSEN_ENABLE       1

#endif /* SI2177_DTV_INT_SENSE_PROP */

/* SI2177 DTV_LIF_FREQ property definition */
#define   SI2177_DTV_LIF_FREQ_PROP 0x0706

#ifdef    SI2177_DTV_LIF_FREQ_PROP

typedef struct { /* SI2177_DTV_LIF_FREQ_PROP_struct */
      unsigned int    offset;
} si2177_dtv_lif_freq_prop_struct;

/* DTV_LIF_FREQ property, OFFSET field definition (NO TITLE)*/
#define  SI2177_DTV_LIF_FREQ_PROP_OFFSET_LSB                    0
#define  SI2177_DTV_LIF_FREQ_PROP_OFFSET_MASK                 0xffff
#define  SI2177_DTV_LIF_FREQ_PROP_OFFSET_DEFAULT           5000
#define SI2177_DTV_LIF_FREQ_PROP_OFFSET_OFFSET_MIN      0
#define SI2177_DTV_LIF_FREQ_PROP_OFFSET_OFFSET_MAX     7000

#endif /* SI2177_DTV_LIF_FREQ_PROP */

/* SI2177 DTV_LIF_OUT property definition */
#define   SI2177_DTV_LIF_OUT_PROP 0x0707

#ifdef    SI2177_DTV_LIF_OUT_PROP

typedef struct { /* SI2177_DTV_LIF_OUT_PROP_struct */
      unsigned char   amp;
      unsigned char   offset;
} si2177_dtv_lif_out_prop_struct;

/* DTV_LIF_OUT property, AMP field definition (NO TITLE)*/
#define  SI2177_DTV_LIF_OUT_PROP_AMP_LSB                8
#define  SI2177_DTV_LIF_OUT_PROP_AMP_MASK             0xff
#define  SI2177_DTV_LIF_OUT_PROP_AMP_DEFAULT       27
#define SI2177_DTV_LIF_OUT_PROP_AMP_AMP_MIN        0
#define SI2177_DTV_LIF_OUT_PROP_AMP_AMP_MAX       255

/* DTV_LIF_OUT property, OFFSET field definition (NO TITLE)*/
#define  SI2177_DTV_LIF_OUT_PROP_OFFSET_LSB                 0
#define  SI2177_DTV_LIF_OUT_PROP_OFFSET_MASK              0xff
#define  SI2177_DTV_LIF_OUT_PROP_OFFSET_DEFAULT        148
#define SI2177_DTV_LIF_OUT_PROP_OFFSET_OFFSET_MIN   0
#define SI2177_DTV_LIF_OUT_PROP_OFFSET_OFFSET_MAX  255

#endif /* SI2177_DTV_LIF_OUT_PROP */

/* SI2177 DTV_MODE property definition */
#define   SI2177_DTV_MODE_PROP 0x0703

#ifdef    SI2177_DTV_MODE_PROP

typedef struct { /* SI2177_DTV_MODE_PROP_struct */
      unsigned char   bw;
      unsigned char   invert_spectrum;
      unsigned char   modulation;
} si2177_dtv_mode_prop_struct;

/* DTV_MODE property, BW field definition (NO TITLE)*/
#define  SI2177_DTV_MODE_PROP_BW_LSB                      0
#define  SI2177_DTV_MODE_PROP_BW_MASK                   0x0f
#define  SI2177_DTV_MODE_PROP_BW_DEFAULT             8
#define SI2177_DTV_MODE_PROP_BW_BW_6MHZ             6
#define SI2177_DTV_MODE_PROP_BW_BW_7MHZ             7
#define SI2177_DTV_MODE_PROP_BW_BW_8MHZ             8
    #define SI2177_DTV_MODE_PROP_BW_BW_1P7MHZ  9
    #define SI2177_DTV_MODE_PROP_BW_BW_6P1MHZ  10

/* DTV_MODE property, INVERT_SPECTRUM field definition (NO TITLE)*/
#define  SI2177_DTV_MODE_PROP_INVERT_SPECTRUM_LSB            8
#define  SI2177_DTV_MODE_PROP_INVERT_SPECTRUM_MASK         0x01
#define  SI2177_DTV_MODE_PROP_INVERT_SPECTRUM_DEFAULT   0
#define SI2177_DTV_MODE_PROP_INVERT_SPECTRUM_NORMAL     0
#define SI2177_DTV_MODE_PROP_INVERT_SPECTRUM_INVERTED  1

/* DTV_MODE property, MODULATION field definition (NO TITLE)*/
#define  SI2177_DTV_MODE_PROP_MODULATION_LSB            4
#define  SI2177_DTV_MODE_PROP_MODULATION_MASK         0x0f
#define  SI2177_DTV_MODE_PROP_MODULATION_DEFAULT   2
#define SI2177_DTV_MODE_PROP_MODULATION_ATSC           0
#define SI2177_DTV_MODE_PROP_MODULATION_QAM_US      1
#define SI2177_DTV_MODE_PROP_MODULATION_DVBT           2
#define SI2177_DTV_MODE_PROP_MODULATION_DVBC           3
#define SI2177_DTV_MODE_PROP_MODULATION_ISDBT          4
#define SI2177_DTV_MODE_PROP_MODULATION_ISDBC          5
#define SI2177_DTV_MODE_PROP_MODULATION_DTMB           6

#endif /* SI2177_DTV_MODE_PROP */

/* SI2177 DTV_RF_TOP property definition */
#define   SI2177_DTV_RF_TOP_PROP 0x0709

#ifdef    SI2177_DTV_RF_TOP_PROP

typedef struct { /* SI2177_DTV_RF_TOP_PROP_struct */
      unsigned char   dtv_rf_top;
} si2177_dtv_rf_top_prop_struct;

/* DTV_RF_TOP property, DTV_RF_TOP field definition (NO TITLE)*/
#define  SI2177_DTV_RF_TOP_PROP_DTV_RF_TOP_LSB            0
#define  SI2177_DTV_RF_TOP_PROP_DTV_RF_TOP_MASK         0xff
#define  SI2177_DTV_RF_TOP_PROP_DTV_RF_TOP_DEFAULT   0
#define SI2177_DTV_RF_TOP_PROP_DTV_RF_TOP_AUTO           0
#define SI2177_DTV_RF_TOP_PROP_DTV_RF_TOP_0DB              6
#define SI2177_DTV_RF_TOP_PROP_DTV_RF_TOP_M1DB           7
#define SI2177_DTV_RF_TOP_PROP_DTV_RF_TOP_M2DB           8
#define SI2177_DTV_RF_TOP_PROP_DTV_RF_TOP_M4DB          10
#define SI2177_DTV_RF_TOP_PROP_DTV_RF_TOP_M5DB          11
#define SI2177_DTV_RF_TOP_PROP_DTV_RF_TOP_M6DB          12
#define SI2177_DTV_RF_TOP_PROP_DTV_RF_TOP_M7DB          13
#define SI2177_DTV_RF_TOP_PROP_DTV_RF_TOP_M8DB          14
#define SI2177_DTV_RF_TOP_PROP_DTV_RF_TOP_M9DB          15

#endif /* SI2177_DTV_RF_TOP_PROP */

/* SI2177 DTV_RSQ_RSSI_THRESHOLD property definition */
#define   SI2177_DTV_RSQ_RSSI_THRESHOLD_PROP 0x0704

#ifdef    SI2177_DTV_RSQ_RSSI_THRESHOLD_PROP

typedef struct { /* SI2177_DTV_RSQ_RSSI_THRESHOLD_PROP_struct */
               char   hi;
               char   lo;
} si2177_dtv_rsq_rssi_threshold_prop_struct;

/* DTV_RSQ_RSSI_THRESHOLD property, HI field definition (NO TITLE)*/
#define  SI2177_DTV_RSQ_RSSI_THRESHOLD_PROP_HI_LSB              8
#define  SI2177_DTV_RSQ_RSSI_THRESHOLD_PROP_HI_MASK           0xff
#define  SI2177_DTV_RSQ_RSSI_THRESHOLD_PROP_HI_DEFAULT     0
#define SI2177_DTV_RSQ_RSSI_THRESHOLD_PROP_HI_HI_MIN          -128
#define SI2177_DTV_RSQ_RSSI_THRESHOLD_PROP_HI_HI_MAX        127

/* DTV_RSQ_RSSI_THRESHOLD property, LO field definition (NO TITLE)*/
#define  SI2177_DTV_RSQ_RSSI_THRESHOLD_PROP_LO_LSB            0
#define  SI2177_DTV_RSQ_RSSI_THRESHOLD_PROP_LO_MASK         0xff
#define  SI2177_DTV_RSQ_RSSI_THRESHOLD_PROP_LO_DEFAULT   -80
#define SI2177_DTV_RSQ_RSSI_THRESHOLD_PROP_LO_LO_MIN       -128
#define SI2177_DTV_RSQ_RSSI_THRESHOLD_PROP_LO_LO_MAX      127

#endif /* SI2177_DTV_RSQ_RSSI_THRESHOLD_PROP */

/* SI2177 MASTER_IEN property definition */
#define   SI2177_MASTER_IEN_PROP                                    0x0401

#ifdef    SI2177_MASTER_IEN_PROP

typedef struct { /* SI2177_MASTER_IEN_PROP_struct */
      unsigned char   atvien;
      unsigned char   ctsien;
      unsigned char   dtvien;
      unsigned char   errien;
      unsigned char   tunien;
} si2177_master_ien_prop_struct;

/* MASTER_IEN property, ATVIEN field definition (NO TITLE)*/
#define  SI2177_MASTER_IEN_PROP_ATVIEN_LSB            1
#define  SI2177_MASTER_IEN_PROP_ATVIEN_MASK         0x01
#define  SI2177_MASTER_IEN_PROP_ATVIEN_DEFAULT   0
#define SI2177_MASTER_IEN_PROP_ATVIEN_OFF             0
#define SI2177_MASTER_IEN_PROP_ATVIEN_ON               1

/* MASTER_IEN property, CTSIEN field definition (NO TITLE)*/
#define  SI2177_MASTER_IEN_PROP_CTSIEN_LSB            7
#define  SI2177_MASTER_IEN_PROP_CTSIEN_MASK         0x01
#define  SI2177_MASTER_IEN_PROP_CTSIEN_DEFAULT   0
#define SI2177_MASTER_IEN_PROP_CTSIEN_OFF             0
#define SI2177_MASTER_IEN_PROP_CTSIEN_ON               1

/* MASTER_IEN property, DTVIEN field definition (NO TITLE)*/
#define  SI2177_MASTER_IEN_PROP_DTVIEN_LSB            2
#define  SI2177_MASTER_IEN_PROP_DTVIEN_MASK         0x01
#define  SI2177_MASTER_IEN_PROP_DTVIEN_DEFAULT   0
#define SI2177_MASTER_IEN_PROP_DTVIEN_OFF             0
#define SI2177_MASTER_IEN_PROP_DTVIEN_ON               1

/* MASTER_IEN property, ERRIEN field definition (NO TITLE)*/
#define  SI2177_MASTER_IEN_PROP_ERRIEN_LSB             6
#define  SI2177_MASTER_IEN_PROP_ERRIEN_MASK          0x01
#define  SI2177_MASTER_IEN_PROP_ERRIEN_DEFAULT    0
#define SI2177_MASTER_IEN_PROP_ERRIEN_OFF              0
#define SI2177_MASTER_IEN_PROP_ERRIEN_ON                1

/* MASTER_IEN property, TUNIEN field definition (NO TITLE)*/
#define  SI2177_MASTER_IEN_PROP_TUNIEN_LSB             0
#define  SI2177_MASTER_IEN_PROP_TUNIEN_MASK          0x01
#define  SI2177_MASTER_IEN_PROP_TUNIEN_DEFAULT    0
#define SI2177_MASTER_IEN_PROP_TUNIEN_OFF               0
#define SI2177_MASTER_IEN_PROP_TUNIEN_ON                1

#endif /* SI2177_MASTER_IEN_PROP */

/* SI2177 TUNER_BLOCKED_VCO property definition */
#define   SI2177_TUNER_BLOCKED_VCO_PROP                         0x0504

#ifdef    SI2177_TUNER_BLOCKED_VCO_PROP

typedef struct { /* SI2177_TUNER_BLOCKED_VCO_PROP_struct */
               int    vco_code;
} si2177_tuner_blocked_vco_prop_struct;

/* TUNER_BLOCKED_VCO property, VCO_CODE field definition (NO TITLE)*/
#define  SI2177_TUNER_BLOCKED_VCO_PROP_VCO_CODE_LSB             0
#define  SI2177_TUNER_BLOCKED_VCO_PROP_VCO_CODE_MASK          0xffff
#define  SI2177_TUNER_BLOCKED_VCO_PROP_VCO_CODE_DEFAULT    0x8000
    #define SI2177_TUNER_BLOCKED_VCO_PROP_VCO_CODE_VCO_CODE_MIN  -32768
    #define SI2177_TUNER_BLOCKED_VCO_PROP_VCO_CODE_VCO_CODE_MAX  32767
#endif /* SI2177_TUNER_BLOCKED_VCO_PROP */

/* SI2177 TUNER_IEN property definition */
#define   SI2177_TUNER_IEN_PROP                                     0x0501

#ifdef    SI2177_TUNER_IEN_PROP

typedef struct { /* SI2177_TUNER_IEN_PROP_struct */
      unsigned char   rssihien;
      unsigned char   rssilien;
      unsigned char   tcien;
} si2177_tuner_ien_prop_struct;

/* TUNER_IEN property, RSSIHIEN field definition (NO TITLE)*/
#define  SI2177_TUNER_IEN_PROP_RSSIHIEN_LSB             2
#define  SI2177_TUNER_IEN_PROP_RSSIHIEN_MASK          0x01
#define  SI2177_TUNER_IEN_PROP_RSSIHIEN_DEFAULT    0
#define SI2177_TUNER_IEN_PROP_RSSIHIEN_DISABLE      0
#define SI2177_TUNER_IEN_PROP_RSSIHIEN_ENABLE       1

/* TUNER_IEN property, RSSILIEN field definition (NO TITLE)*/
#define  SI2177_TUNER_IEN_PROP_RSSILIEN_LSB             1
#define  SI2177_TUNER_IEN_PROP_RSSILIEN_MASK          0x01
#define  SI2177_TUNER_IEN_PROP_RSSILIEN_DEFAULT    0
#define SI2177_TUNER_IEN_PROP_RSSILIEN_DISABLE      0
#define SI2177_TUNER_IEN_PROP_RSSILIEN_ENABLE       1

/* TUNER_IEN property, TCIEN field definition (NO TITLE)*/
#define  SI2177_TUNER_IEN_PROP_TCIEN_LSB                    0
#define  SI2177_TUNER_IEN_PROP_TCIEN_MASK                 0x01
   #define  SI2177_TUNER_IEN_PROP_TCIEN_DEFAULT    1
#define SI2177_TUNER_IEN_PROP_TCIEN_DISABLE             0
#define SI2177_TUNER_IEN_PROP_TCIEN_ENABLE              1

#endif /* SI2177_TUNER_IEN_PROP */

/* SI2177 TUNER_INT_SENSE property definition */
#define   SI2177_TUNER_INT_SENSE_PROP                          0x0505

#ifdef    SI2177_TUNER_INT_SENSE_PROP

typedef struct { /* SI2177_TUNER_INT_SENSE_PROP_struct */
      unsigned char   rssihnegen;
      unsigned char   rssihposen;
      unsigned char   rssilnegen;
      unsigned char   rssilposen;
      unsigned char   tcnegen;
      unsigned char   tcposen;
} si2177_tuner_int_sense_prop_struct;

/* TUNER_INT_SENSE property, RSSIHNEGEN field definition (NO TITLE)*/
#define  SI2177_TUNER_INT_SENSE_PROP_RSSIHNEGEN_LSB             2
#define  SI2177_TUNER_INT_SENSE_PROP_RSSIHNEGEN_MASK          0x01
#define  SI2177_TUNER_INT_SENSE_PROP_RSSIHNEGEN_DEFAULT    0
#define SI2177_TUNER_INT_SENSE_PROP_RSSIHNEGEN_DISABLE      0
#define SI2177_TUNER_INT_SENSE_PROP_RSSIHNEGEN_ENABLE       1

/* TUNER_INT_SENSE property, RSSIHPOSEN field definition (NO TITLE)*/
#define  SI2177_TUNER_INT_SENSE_PROP_RSSIHPOSEN_LSB             10
#define  SI2177_TUNER_INT_SENSE_PROP_RSSIHPOSEN_MASK          0x01
#define  SI2177_TUNER_INT_SENSE_PROP_RSSIHPOSEN_DEFAULT    1
#define SI2177_TUNER_INT_SENSE_PROP_RSSIHPOSEN_DISABLE      0
#define SI2177_TUNER_INT_SENSE_PROP_RSSIHPOSEN_ENABLE       1

/* TUNER_INT_SENSE property, RSSILNEGEN field definition (NO TITLE)*/
#define  SI2177_TUNER_INT_SENSE_PROP_RSSILNEGEN_LSB             1
#define  SI2177_TUNER_INT_SENSE_PROP_RSSILNEGEN_MASK          0x01
#define  SI2177_TUNER_INT_SENSE_PROP_RSSILNEGEN_DEFAULT    0
#define SI2177_TUNER_INT_SENSE_PROP_RSSILNEGEN_DISABLE      0
#define SI2177_TUNER_INT_SENSE_PROP_RSSILNEGEN_ENABLE       1

/* TUNER_INT_SENSE property, RSSILPOSEN field definition (NO TITLE)*/
#define  SI2177_TUNER_INT_SENSE_PROP_RSSILPOSEN_LSB             9
#define  SI2177_TUNER_INT_SENSE_PROP_RSSILPOSEN_MASK          0x01
#define  SI2177_TUNER_INT_SENSE_PROP_RSSILPOSEN_DEFAULT    1
#define SI2177_TUNER_INT_SENSE_PROP_RSSILPOSEN_DISABLE      0
#define SI2177_TUNER_INT_SENSE_PROP_RSSILPOSEN_ENABLE       1

/* TUNER_INT_SENSE property, TCNEGEN field definition (NO TITLE)*/
#define  SI2177_TUNER_INT_SENSE_PROP_TCNEGEN_LSB                   0
#define  SI2177_TUNER_INT_SENSE_PROP_TCNEGEN_MASK               0x01
#define  SI2177_TUNER_INT_SENSE_PROP_TCNEGEN_DEFAULT         0
#define SI2177_TUNER_INT_SENSE_PROP_TCNEGEN_DISABLE           0
#define SI2177_TUNER_INT_SENSE_PROP_TCNEGEN_ENABLE            1

/* TUNER_INT_SENSE property, TCPOSEN field definition (NO TITLE)*/
#define  SI2177_TUNER_INT_SENSE_PROP_TCPOSEN_LSB                  8
#define  SI2177_TUNER_INT_SENSE_PROP_TCPOSEN_MASK               0x01
#define  SI2177_TUNER_INT_SENSE_PROP_TCPOSEN_DEFAULT         1
#define SI2177_TUNER_INT_SENSE_PROP_TCPOSEN_DISABLE           0
#define SI2177_TUNER_INT_SENSE_PROP_TCPOSEN_ENABLE            1

#endif /* SI2177_TUNER_INT_SENSE_PROP */

/* SI2177 TUNER_LO_INJECTION property definition */
#define   SI2177_TUNER_LO_INJECTION_PROP                                    0x0506

#ifdef    SI2177_TUNER_LO_INJECTION_PROP

typedef struct { /* SI2177_TUNER_LO_INJECTION_PROP_struct */
      unsigned char   band_1;
      unsigned char   band_2;
      unsigned char   band_3;
} si2177_tuner_lo_injection_prop_struct;

/* TUNER_LO_INJECTION property, BAND_1 field definition (NO TITLE)*/
#define  SI2177_TUNER_LO_INJECTION_PROP_BAND_1_LSB              0
#define  SI2177_TUNER_LO_INJECTION_PROP_BAND_1_MASK           0x01
#define  SI2177_TUNER_LO_INJECTION_PROP_BAND_1_DEFAULT    1
#define SI2177_TUNER_LO_INJECTION_PROP_BAND_1_LOW_SIDE   0
#define SI2177_TUNER_LO_INJECTION_PROP_BAND_1_HIGH_SIDE  1

/* TUNER_LO_INJECTION property, BAND_2 field definition (NO TITLE)*/
#define  SI2177_TUNER_LO_INJECTION_PROP_BAND_2_LSB              1
#define  SI2177_TUNER_LO_INJECTION_PROP_BAND_2_MASK           0x01
#define  SI2177_TUNER_LO_INJECTION_PROP_BAND_2_DEFAULT    0
#define SI2177_TUNER_LO_INJECTION_PROP_BAND_2_LOW_SIDE   0
#define SI2177_TUNER_LO_INJECTION_PROP_BAND_2_HIGH_SIDE  1

/* TUNER_LO_INJECTION property, BAND_3 field definition (NO TITLE)*/
#define  SI2177_TUNER_LO_INJECTION_PROP_BAND_3_LSB              2
#define  SI2177_TUNER_LO_INJECTION_PROP_BAND_3_MASK           0x01
#define  SI2177_TUNER_LO_INJECTION_PROP_BAND_3_DEFAULT    0
#define SI2177_TUNER_LO_INJECTION_PROP_BAND_3_LOW_SIDE   0
#define SI2177_TUNER_LO_INJECTION_PROP_BAND_3_HIGH_SIDE  1


#endif /* SI2177_TUNER_LO_INJECTION_PROP */

/* SI2177 TUNER_RETURN_LOSS property definition */
#define   SI2177_TUNER_RETURN_LOSS_PROP 0x0507

#ifdef    SI2177_TUNER_RETURN_LOSS_PROP
  #define SI2177_TUNER_RETURN_LOSS_PROP_CODE 0x000507


    typedef struct { /* SI2177_TUNER_RETURN_LOSS_PROP_struct */
      unsigned char   config;
      unsigned char   mode;
   } SI2177_TUNER_RETURN_LOSS_PROP_struct;

   /* TUNER_RETURN_LOSS property, CONFIG field definition (NO TITLE)*/
   #define  SI2177_TUNER_RETURN_LOSS_PROP_CONFIG_LSB         0
   #define  SI2177_TUNER_RETURN_LOSS_PROP_CONFIG_MASK        0xff
   #define  SI2177_TUNER_RETURN_LOSS_PROP_CONFIG_DEFAULT    127
    #define SI2177_TUNER_RETURN_LOSS_PROP_CONFIG_75   75
    #define SI2177_TUNER_RETURN_LOSS_PROP_CONFIG_83   83
    #define SI2177_TUNER_RETURN_LOSS_PROP_CONFIG_91   91
    #define SI2177_TUNER_RETURN_LOSS_PROP_CONFIG_103  103
    #define SI2177_TUNER_RETURN_LOSS_PROP_CONFIG_115  115
    #define SI2177_TUNER_RETURN_LOSS_PROP_CONFIG_127  127

   /* TUNER_RETURN_LOSS property, MODE field definition (NO TITLE)*/
   #define  SI2177_TUNER_RETURN_LOSS_PROP_MODE_LSB         8
   #define  SI2177_TUNER_RETURN_LOSS_PROP_MODE_MASK        0xff
   #define  SI2177_TUNER_RETURN_LOSS_PROP_MODE_DEFAULT    0
    #define SI2177_TUNER_RETURN_LOSS_PROP_MODE_TERRESTRIAL  0
    #define SI2177_TUNER_RETURN_LOSS_PROP_MODE_CABLE        1

#endif /* SI2177_TUNER_RETURN_LOSS_PROP */

/* SI2177 XOUT property definition */
#define   SI2177_XOUT_PROP 0x0404

#ifdef    SI2177_XOUT_PROP
  #define SI2177_XOUT_PROP_CODE 0x000404


    typedef struct { /* SI2177_XOUT_PROP_struct */
      unsigned char   amp;
   } SI2177_XOUT_PROP_struct;

   /* XOUT property, AMP field definition (NO TITLE)*/
   #define  SI2177_XOUT_PROP_AMP_LSB         0
   #define  SI2177_XOUT_PROP_AMP_MASK        0x01
   #define  SI2177_XOUT_PROP_AMP_DEFAULT    0
    #define SI2177_XOUT_PROP_AMP_HIGH  0
    #define SI2177_XOUT_PROP_AMP_LOW   1

#endif /* SI2177_XOUT_PROP */



  /* --------------------------------------------*/
  /* PROPERTIES STRUCT                           */
  /* This stores all property fields             */
  /* --------------------------------------------*/
  typedef struct {
    #ifdef    SI2177_ATV_AFC_RANGE_PROP
              si2177_atv_afc_range_prop_struct               atv_afc_range;
    #endif /* SI2177_ATV_AFC_RANGE_PROP */
    #ifdef    SI2177_ATV_AF_OUT_PROP
              si2177_atv_af_out_prop_struct                     atv_af_out;
    #endif /* SI2177_ATV_AF_OUT_PROP */
    #ifdef    SI2177_ATV_AGC_SPEED_PROP
              si2177_atv_agc_speed_prop_struct             atv_agc_speed;
    #endif /* SI2177_ATV_AGC_SPEED_PROP */
    #ifdef    SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP
              SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP_struct        atv_agc_speed_low_rssi;
    #endif /* SI2177_ATV_AGC_SPEED_LOW_RSSI_PROP */
    #ifdef    SI2177_ATV_ARTIFICIAL_SNOW_PROP
              SI2177_ATV_ARTIFICIAL_SNOW_PROP_struct           atv_artificial_snow;
    #endif /* SI2177_ATV_ARTIFICIAL_SNOW_PROP */
    #ifdef    SI2177_ATV_AUDIO_MODE_PROP
              si2177_atv_audio_mode_prop_struct           atv_audio_mode;
    #endif /* SI2177_ATV_AUDIO_MODE_PROP */
    #ifdef    SI2177_ATV_CONFIG_IF_PORT_PROP
              SI2177_ATV_CONFIG_IF_PORT_PROP_struct            atv_config_if_port;
    #endif /* SI2177_ATV_CONFIG_IF_PORT_PROP */
    #ifdef    SI2177_ATV_CVBS_OUT_PROP
              si2177_atv_cvbs_out_prop_struct                atv_cvbs_out;
    #endif /* SI2177_ATV_CVBS_OUT_PROP */
    #ifdef    SI2177_ATV_CVBS_OUT_FINE_PROP
              si2177_atv_cvbs_out_fine_prop_struct        atv_cvbs_out_fine;
    #endif /* SI2177_ATV_CVBS_OUT_FINE_PROP */
    #ifdef    SI2177_ATV_HSYNC_OUT_PROP
              SI2177_ATV_HSYNC_OUT_PROP_struct                 atv_hsync_out;
    #endif /* SI2177_ATV_HSYNC_OUT_PROP */
    #ifdef    SI2177_ATV_IEN_PROP
              si2177_atv_ien_prop_struct                         atv_ien;
    #endif /* SI2177_ATV_IEN_PROP */
    #ifdef    SI2177_ATV_INT_SENSE_PROP
              si2177_atv_int_sense_prop_struct              atv_int_sense;
    #endif /* SI2177_ATV_INT_SENSE_PROP */
    #ifdef    SI2177_ATV_PGA_TARGET_PROP
              SI2177_ATV_PGA_TARGET_PROP_struct                atv_pga_target;
    #endif /* SI2177_ATV_PGA_TARGET_PROP */
    #ifdef    SI2177_ATV_RF_TOP_PROP
              si2177_atv_rf_top_prop_struct                    atv_rf_top;
    #endif /* SI2177_ATV_RF_TOP_PROP */
    #ifdef    SI2177_ATV_RSQ_RSSI_THRESHOLD_PROP
              si2177_atv_rsq_rssi_threshold_prop_struct  atv_rsq_rssi_threshold;
    #endif /* SI2177_ATV_RSQ_RSSI_THRESHOLD_PROP */
    #ifdef    SI2177_ATV_RSQ_SNR_THRESHOLD_PROP
              si2177_atv_rsq_snr_threshold_prop_struct   atv_rsq_snr_threshold;
    #endif /* SI2177_ATV_RSQ_SNR_THRESHOLD_PROP */
    #ifdef    SI2177_ATV_SIF_OUT_PROP
              si2177_atv_sif_out_prop_struct                    atv_sif_out;
    #endif /* SI2177_ATV_SIF_OUT_PROP */
    #ifdef    SI2177_ATV_SOUND_AGC_LIMIT_PROP
              si2177_atv_sound_agc_limit_prop_struct           atv_sound_agc_limit;
    #endif /* SI2177_ATV_SOUND_AGC_LIMIT_PROP */
    #ifdef    SI2177_ATV_SOUND_AGC_SPEED_PROP
              si2177_atv_sound_agc_speed_prop_struct           atv_sound_agc_speed;
    #endif /* SI2177_ATV_SOUND_AGC_SPEED_PROP */
    #ifdef    SI2177_ATV_VIDEO_EQUALIZER_PROP
              si2177_atv_video_equalizer_prop_struct           atv_video_equalizer;
    #endif /* SI2177_ATV_VIDEO_EQUALIZER_PROP */

    #ifdef    SI2177_ATV_VIDEO_MODE_PROP
              si2177_atv_video_mode_prop_struct           atv_video_mode;
    #endif /* SI2177_ATV_VIDEO_MODE_PROP */
    #ifdef    SI2177_ATV_VSNR_CAP_PROP
              si2177_atv_vsnr_cap_prop_struct                atv_vsnr_cap;
    #endif /* SI2177_ATV_VSNR_CAP_PROP */
    #ifdef    SI2177_CRYSTAL_TRIM_PROP
              si2177_crystal_trim_prop_struct                  crystal_trim;
    #endif /* SI2177_CRYSTAL_TRIM_PROP */
    #ifdef    SI2177_DTV_AGC_SPEED_PROP
              si2177_dtv_agc_speed_prop_struct            dtv_agc_speed;
    #endif /* SI2177_DTV_AGC_SPEED_PROP */
    #ifdef    SI2177_DTV_CONFIG_IF_PORT_PROP
              si2177_dtv_config_if_port_prop_struct        dtv_config_if_port;
    #endif /* SI2177_DTV_CONFIG_IF_PORT_PROP */
    #ifdef    SI2177_DTV_EXT_AGC_PROP
              si2177_dtv_ext_agc_prop_struct                 dtv_ext_agc;
    #endif /* SI2177_DTV_EXT_AGC_PROP */
    #ifdef    SI2177_DTV_IEN_PROP
              si2177_dtv_ien_prop_struct                        dtv_ien;
    #endif /* SI2177_DTV_IEN_PROP */
    #ifdef    SI2177_DTV_INT_SENSE_PROP
              si2177_dtv_int_sense_prop_struct              dtv_int_sense;
    #endif /* SI2177_DTV_INT_SENSE_PROP */
    #ifdef    SI2177_DTV_LIF_FREQ_PROP
              si2177_dtv_lif_freq_prop_struct                   dtv_lif_freq;
    #endif /* SI2177_DTV_LIF_FREQ_PROP */
    #ifdef    SI2177_DTV_LIF_OUT_PROP
              si2177_dtv_lif_out_prop_struct                    dtv_lif_out;
    #endif /* SI2177_DTV_LIF_OUT_PROP */
    #ifdef    SI2177_DTV_MODE_PROP
              si2177_dtv_mode_prop_struct                     dtv_mode;
    #endif /* SI2177_DTV_MODE_PROP */
    #ifdef    SI2177_DTV_RF_TOP_PROP
              si2177_dtv_rf_top_prop_struct                     dtv_rf_top;
    #endif /* SI2177_DTV_RF_TOP_PROP */
    #ifdef    SI2177_DTV_RSQ_RSSI_THRESHOLD_PROP
              si2177_dtv_rsq_rssi_threshold_prop_struct  dtv_rsq_rssi_threshold;
    #endif /* SI2177_DTV_RSQ_RSSI_THRESHOLD_PROP */
    #ifdef    SI2177_MASTER_IEN_PROP
              si2177_master_ien_prop_struct                    master_ien;
    #endif /* SI2177_MASTER_IEN_PROP */
    #ifdef    SI2177_TUNER_BLOCKED_VCO_PROP
              si2177_tuner_blocked_vco_prop_struct       tuner_blocked_vco;
    #endif /* SI2177_TUNER_BLOCKED_VCO_PROP */
    #ifdef    SI2177_TUNER_IEN_PROP
              si2177_tuner_ien_prop_struct                      tuner_ien;
    #endif /* SI2177_TUNER_IEN_PROP */
    #ifdef    SI2177_TUNER_INT_SENSE_PROP
              si2177_tuner_int_sense_prop_struct            tuner_int_sense;
    #endif /* SI2177_TUNER_INT_SENSE_PROP */
    #ifdef    SI2177_TUNER_LO_INJECTION_PROP
              si2177_tuner_lo_injection_prop_struct         tuner_lo_injection;
    #endif /* SI2177_TUNER_LO_INJECTION_PROP */
    #ifdef    SI2177_TUNER_RETURN_LOSS_PROP
              SI2177_TUNER_RETURN_LOSS_PROP_struct             tuner_return_loss;
    #endif /* SI2177_TUNER_RETURN_LOSS_PROP */
    #ifdef    SI2177_XOUT_PROP
              SI2177_XOUT_PROP_struct                          xout;
    #endif /* SI2177_XOUT_PROP */
} si2177_propobj_t;
/* _properties_struct_insertion_point */

typedef struct si2177_device_s{
    struct class *clsp;
    struct i2c_client tuner_client;
    si2177_common_reply_struct si_common_reply;
    si2177_propobj_t     si_prop;
    si2177_cmdreplyobj_t si_cmd_reply;
    struct analog_parameters parm;
    int fre_offset;
}si2177_device_t;

int si2177_init(struct i2c_client *si2177, si2177_cmdreplyobj_t *rsp, si2177_common_reply_struct *common_reply);
//int si2177_powerupwithpatch(struct i2c_client *si2177, si2177_cmdreplyobj_t *rsp, si2177_common_reply_struct *common_reply);
//int si2177_loadfirmware(struct i2c_client *si2177, unsigned char* firmwaretable, int lines, si2177_common_reply_struct *common_reply);
//int si2177_startfirmware(struct i2c_client *si2177, si2177_cmdreplyobj_t *rsp, si2177_common_reply_struct *common_reply);
int si2177_configure(struct i2c_client *si2177, si2177_propobj_t *prop, si2177_cmdreplyobj_t *rsp, si2177_common_reply_struct *common_reply);
//int si2177_loadvideofilter(struct i2c_client *si2177, unsigned char* vidfilttable, int lines, si2177_common_reply_struct *common_reply);
int si2177_tune(struct i2c_client *si2177, unsigned char mode, unsigned long freq, si2177_cmdreplyobj_t *rsp, si2177_common_reply_struct *common_reply);
int si2177_atvconfig(struct i2c_client *si2177, si2177_propobj_t *prop, si2177_cmdreplyobj_t *rsp);
int si2177_dtvconfig(struct i2c_client *si2177, si2177_propobj_t *prop, si2177_cmdreplyobj_t *rsp);
//int si2177_tunerconfig(struct i2c_client *si2177, si2177_propobj_t *prop, si2177_cmdreplyobj_t *rsp);
//int si2177_setuptunerdefaults(si2177_propobj_t *prop);
//int si2177_setupcommondefaults(si2177_propobj_t *prop);
//int si2177_setupatvdefaults(si2177_propobj_t *prop);
unsigned char si2177_atv_status(struct i2c_client *si2177, unsigned char intack, si2177_cmdreplyobj_t *rsp);
unsigned char si2177_tuner_status(struct i2c_client *si2177, unsigned char intack, si2177_cmdreplyobj_t *rsp);
unsigned char si2177_power_down(struct i2c_client *si2177, si2177_cmdreplyobj_t *rsp);
unsigned char si2177_fine_tune(struct i2c_client *si2177, unsigned char  persistence,unsigned char   apply_to_lif,int offset_500hz, si2177_cmdreplyobj_t *rsp);
unsigned char si2177_atv_restart(struct i2c_client *si2177, unsigned char   mode,  si2177_cmdreplyobj_t *rsp);
unsigned char si2177_dtv_restart(struct i2c_client *si2177, si2177_cmdreplyobj_t *rsp);
unsigned char si2177_set_property(struct i2c_client *si2177,unsigned char   reserved, unsigned int prop, unsigned int data,  si2177_cmdreplyobj_t *rsp);
unsigned char si2177_get_property(struct i2c_client *si2177, unsigned char   reserved,unsigned int  prop, si2177_cmdreplyobj_t *rsp);
unsigned char si2177_config_clocks  (struct i2c_client *si2177,unsigned char  subcode,unsigned char clock_mode,unsigned char en_xout,si2177_cmdreplyobj_t *rsp);


#endif /* __SLI2177_FUN_H */
