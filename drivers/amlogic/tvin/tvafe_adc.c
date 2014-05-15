/*
 * TVAFE adc device driver.
 *
 * Copyright (c) 2010 Frank zhao <frank.zhao@amlogic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the smems of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 */

/******************************Includes************************************/
#include <linux/errno.h>
#include <mach/am_regs.h>

#include "vdin.h"
#include "tvafe.h"
#include "tvafe_regs.h"
//#include "tvafe_general.h"
//#include "tvafe_bitmap.h"
#include "tvafe_adc.h"
/***************************Local defines**********************************/
#define ADC_SIGNAL_OFF_CNT                3     //adc signal off counter 10ms
#define ADC_SIGNAL_ON_CNT                 4     //adc signal on counter 10ms
#define ADC_FORMAT_CHECK_CNT              20    //adc signal on counter 10ms

#define ADC_WINDOW_H_OFFSET              30     //auto phase window h offset
#define ADC_WINDOW_V_OFFSET               2     //auto phase window v offset
#define MAX_PHASE_CNT                    32     // phase total number
#define MAX_AUTO_CLOCK_LEVEL            100     // pixel
//#define AUTO_PHASE_VS_CNT                 3     //phase auto adjust delay
//#define BORDER_READ_DELAY                 5     // 5 Vsync counter
#define VGA_AUTO_VS_DELAY                 5     // 5 Vsync counter
//#define CHECK_WIDTH_CNT                   3     // check three times for better width
#define VGA_AUTO_TRY_COUNTER              9     // vga max adjust counter
#define MIN_POSITION_WIDTH               30     // min auto position width/good width
#define DEFAULT_RGB_THRESHOLD             2     // default rgb threshold value

/***************************Local Structures**********************************/
static struct tvafe_adc_status_s adc_info =
{
    0,
    0,
    0,
    {
    //H_Active V_Active H_cnt Hcnt_offset Vcnt_offset Hs_cnt Hscnt_offset
        0,       0,    0,          0,           0,     0,           0,
    //H_Total V_Total Hs_Front Hs_Width Hs_bp Vs_Front Vs_Width Vs_bp Hs_Polarity
        0,     0,       0,       0,    0,       0,       0,    0, TVIN_SYNC_POL_NULL,
    //Vs_Polarity             Scan_Mode      Pixel_Clk(Khz/10) VBIs vbie
    TVIN_SYNC_POL_NULL,  TVIN_SCAN_MODE_NULL,               0,   0,   0
    }
};

/***************************Local enum**********************************/
static enum tvin_sig_event_e  event_adc = 0;
/***************************Loacal Variables**********************************/
static unsigned int cnt_adc = 0;

// *****************************************************************************
// Function:get ADC DVSS signal status
//
//   Params: none
//
//   Return: none
//
// *****************************************************************************
static unsigned char tvafe_adc_get_pll_status(void)
{
    unsigned char pll_status;

    pll_status = (unsigned char)READ_CBUS_REG_BITS(ADC_REG_35,
                                    PLLLOCKED_BIT, PLLLOCKED_WID);

    return pll_status;
}

// *****************************************************************************
// Function:set adc clock
//
//   Params: format index
//
//   Return: success/error
//
// *****************************************************************************
static void tvafe_adc_set_clock(enum tvin_sig_fmt_e fmt)
{
    unsigned char div_ratio,vco_range_sel,i;
    unsigned short vco_gain;
    unsigned long k_vco,hs_freq,tmp;

    unsigned short charge_pump_range[] = { 20,  30,  40,  50,  60,  70,  80, 90};
    unsigned char  charge_pump_table[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7};

	//select vco range and gain by pixel clock
    if (tvin_fmt_tbl[fmt].pixel_clk < 2500) {      //37Mhz
        vco_range_sel = 0x00;
        vco_gain = 80;   //(Mhz/v)
    } else if (tvin_fmt_tbl[fmt].pixel_clk <  4000) { //72MHz
        vco_range_sel = 0x01;
        vco_gain = 170;  //(Mhz/v)
    } else if (tvin_fmt_tbl[fmt].pixel_clk <  10000) { //72MHz
        vco_range_sel = 0x02;
        vco_gain = 320;  //(Mhz/v)
    } else {
        vco_range_sel = 0x03;
        vco_gain = 670;  //(Mhz/v)
    }
    //set vco sel reg
    WRITE_CBUS_REG_BITS(ADC_REG_68, vco_range_sel, VCORANGESEL_BIT, VCORANGESEL_WID);

    k_vco = (unsigned long)vco_gain;    // Get VCO gain
    // tmp = (((Hfreq * 2 * PI)/14.3)^2 * C * N / Kvco)
    // 2 * PI / 14.3 = .43938, round off to .439
    // tmp = ((Hfreq * 0.439)^2) * C * N / Kvco
    hs_freq = 100*1000/tvin_fmt_tbl[fmt].hs_cnt;    //100Mhz clock
    tmp = hs_freq * 439L / 1000L;   // Loop current formula
    tmp *= tmp;
    tmp /= 1000L;
    tmp *= 39L;
    tmp /= k_vco;
    tmp *= tvin_fmt_tbl[fmt].h_total;
    tmp /= 1000000L;

    for (i = 0; i < 8; i++) {
        if (tmp <= (unsigned long)charge_pump_range[i])
            break;
    }
    //set charge pump current value
    WRITE_CBUS_REG_BITS(ADC_REG_69, charge_pump_table[i], CHARGEPUMPCURR_BIT, CHARGEPUMPCURR_WID);


    // PLL divider programming
    div_ratio = (unsigned char)((tvin_fmt_tbl[fmt].h_total - 1) & 0x0FF0) >> 4;
    WRITE_CBUS_REG_BITS(ADC_REG_01, div_ratio, PLLDIVRATIO_MSB_BIT, PLLDIVRATIO_MSB_WID);

    div_ratio = (unsigned char)((tvin_fmt_tbl[fmt].h_total - 1) & 0x000F) << 4;
    WRITE_CBUS_REG_BITS(ADC_REG_02, div_ratio, PLLDIVRATIO_LSB_BIT, PLLDIVRATIO_LSB_WID);

    return;
}

// *****************************************************************************
// Function:set adc analog buffer bandwidth
//
//   Params: format index, adc channel
//
//   Return: none
//
// *****************************************************************************
static void tvafe_adc_set_bw_lpf(enum tvin_sig_fmt_e fmt)
{
    unsigned char i;
    unsigned int freq[] = {
           5,    7,    9,   13,   16,   20,   25,   28,
          33,   37,   40,   47,   54,   67,   74,   81,
          90,  150,  230,  320,  450,  600,  750
    };
    unsigned char lpf_ctl_tbl[] = {
         0x0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    unsigned char bw_tbl[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
    };

    for (i = 0; i <= 22; i++) {
        if (((tvin_fmt_tbl[fmt].pixel_clk/100)/2) <= (unsigned long)freq[i])  //Mhz
            break;
    }

    if (i > 15)    //if pixel clk > 180Mhz should close lpf
        WRITE_CBUS_REG_BITS(ADC_REG_19, 0, ENLPFA_BIT, ENLPFA_WID);

    WRITE_CBUS_REG_BITS(ADC_REG_19, lpf_ctl_tbl[i], LPFBWCTRA_BIT, LPFBWCTRA_WID);
    WRITE_CBUS_REG_BITS(ADC_REG_19, bw_tbl[i], ANABWCTRLA_BIT, ANABWCTRLA_WID);
    WRITE_CBUS_REG_BITS(ADC_REG_1A, lpf_ctl_tbl[i], LPFBWCTRB_BIT, LPFBWCTRB_WID);
    WRITE_CBUS_REG_BITS(ADC_REG_1A, bw_tbl[i], ANABWCTRLB_BIT, ANABWCTRLB_WID);
    WRITE_CBUS_REG_BITS(ADC_REG_1B, lpf_ctl_tbl[i], LPFBWCTRC_BIT, LPFBWCTRC_WID);
    WRITE_CBUS_REG_BITS(ADC_REG_1B, bw_tbl[i], ANABWCTRLB_BIT, ANABWCTRLC_WID);

    return;
}

// *****************************************************************************
// Function:set adc clamp parameter
//
//   Params: format index
//
//   Return: success/error
//
// *****************************************************************************
void tvafe_adc_set_clamp_para(enum tvin_sig_fmt_e fmt)
{
    unsigned short hs_bp;

    hs_bp = tvin_fmt_tbl[fmt].h_total - tvin_fmt_tbl[fmt].h_active
            - tvin_fmt_tbl[fmt].hs_width - tvin_fmt_tbl[fmt].hs_front - 10;

    WRITE_CBUS_REG_BITS(ADC_REG_03, (tvin_fmt_tbl[fmt].hs_width + 10),
        CLAMPPLACEM_BIT, CLAMPPLACEM_WID);
    WRITE_CBUS_REG_BITS(ADC_REG_04, hs_bp, CLAMPDURATION_BIT, CLAMPDURATION_WID);

    return;
}
// *****************************************************************************
// Function:get ADC signal info(hcnt,vcnt,hpol,vpol)
//
//   Params: none
//
//   Return: none
//
// *****************************************************************************
static void tvafe_adc_get_timing_info(void)
{
    unsigned short tmp;

    adc_info.no_sig   = (unsigned char)READ_CBUS_REG_BITS(TVFE_DVSS_INDICATOR1,
                                        NOSIG_BIT, NOSIG_WID);
    if (adc_info.no_sig == 1)
        return;

    tmp = READ_CBUS_REG_BITS(TVFE_SYNCTOP_INDICATOR3,
                                    SPOL_H_POL_BIT, SPOL_H_POL_WID);
    //sync polarity
    if (tmp == 1)
        adc_info.prop.hs_pol = TVIN_SYNC_POL_POSITIVE;
    else
        adc_info.prop.hs_pol = TVIN_SYNC_POL_NEGATIVE;


    tmp = READ_CBUS_REG_BITS(TVFE_SYNCTOP_INDICATOR3,
                                    SPOL_V_POL_BIT, SPOL_V_POL_WID);
    if (tmp == 1)
        adc_info.prop.vs_pol = TVIN_SYNC_POL_POSITIVE;
    else
        adc_info.prop.vs_pol = TVIN_SYNC_POL_NEGATIVE;


    //sync width counter
    if (adc_info.prop.hs_pol == TVIN_SYNC_POL_POSITIVE)
        adc_info.prop.hs_cnt = (unsigned short)READ_CBUS_REG_BITS(TVFE_SYNCTOP_INDICATOR1_HCNT,
                            SPOL_HCNT_POS_BIT, SPOL_HCNT_POS_WID);
    else
        adc_info.prop.hs_cnt = (unsigned short)READ_CBUS_REG_BITS(TVFE_SYNCTOP_INDICATOR1_HCNT,
                            SPOL_HCNT_NEG_BIT, SPOL_HCNT_NEG_WID);

    if (adc_info.prop.vs_pol == TVIN_SYNC_POL_POSITIVE)
        adc_info.prop.vs_width= (unsigned short)READ_CBUS_REG_BITS(TVFE_SYNCTOP_INDICATOR2_VCNT,
                            SPOL_VCNT_POS_BIT, SPOL_VCNT_POS_WID);
    else
        adc_info.prop.vs_width = (unsigned short)
            READ_CBUS_REG_BITS(TVFE_SYNCTOP_INDICATOR2_VCNT, SPOL_VCNT_NEG_BIT,
                SPOL_VCNT_NEG_WID);

    //htotal cnt and v total cnt
    tmp = (unsigned short)READ_CBUS_REG_BITS(TVFE_SYNCTOP_INDICATOR2_VCNT,
                       SPOL_VCNT_POS_BIT, SPOL_VCNT_POS_WID);
    adc_info.h_offset = ABS(adc_info.prop.h_cnt - tmp);
    adc_info.prop.h_cnt = tmp;

    tmp = (unsigned short)READ_CBUS_REG_BITS(TVFE_SYNCTOP_INDICATOR2_VCNT,
                        SPOL_VCNT_POS_BIT, SPOL_VCNT_POS_WID);
    adc_info.h_offset = ABS(adc_info.prop.v_total - tmp);
    adc_info.prop.v_total = tmp;

    return;
}

// *****************************************************************************
// Function:set ADC sync mux setting
//
//   Params: none
//
//   Return: sucess/error
//
// *****************************************************************************
//static int tvafe_adc_sync_select(enum adc_sync_type_e sync_type)
//{
//    int ret = 0;
//
//    switch (sync_type) {
//        case ADC_SYNC_AUTO:
//            tvafe_reg_set_bits(ADC_REG_39, ADC_REG_SYNCMUXCTRLBYPASS, ADC_REG_SYNCMUXCTRLBYPASS_MASK, 1);
//            tvafe_reg_set_bits(ADC_REG_39, ADC_REG_SYNCMUXCTRL, ADC_REG_SYNCMUXCTRL_MASK, 1);
//            tvafe_reg_set_bits(ADC_REG_2E, ADC_REG_HSYNCACTVOVRD, ADC_REG_HSYNCACTVOVRD_MASK, 1);
//            tvafe_reg_set_bits(ADC_REG_2E, ADC_REG_VSYNCACTVSEL, ADC_REG_VSYNCACTVSEL_MASK, 1);
//            break;
//        case ADC_SYNC_SEPARATE:
//            //...
//            break;
//        case ADC_SYNC_SOG:
//            //...
//            break;
//    }
//
//    return ret;
//}

// *****************************************************************************
// Function: search input format by the info table
//
//   Params: none
//
//   Return: format index
//
// *****************************************************************************
static enum tvin_sig_fmt_e tvafe_adc_search_mode(void)
{
	enum tvin_sig_fmt_e index;

    tvafe_adc_get_timing_info();

    for (index = TVIN_SIG_FMT_VGA_512X384P_60D147;
        index < TVIN_SIG_FMT_COMPONENT_MAX; index++) {

        // Check H,V frequency, Vtotal is within the standard mode limits
		if (index == TVIN_SIG_FMT_VGA_MAX)
            continue;   //ignore VGA mode max number

        //only for VGA and component source
		if (index >= TVIN_SIG_FMT_COMPONENT_MAX) {
            break;
		}

		if ((ABS(tvin_fmt_tbl[index].h_cnt - adc_info.prop.h_cnt) <= tvin_fmt_tbl[index].h_cnt_offset) &&
            (ABS(tvin_fmt_tbl[index].v_total- adc_info.prop.v_total) <= tvin_fmt_tbl[index].v_cnt_offset)) {
            if ((tvin_fmt_tbl[index].hs_pol == adc_info.prop.hs_pol) &&
                (tvin_fmt_tbl[index].vs_pol == adc_info.prop.vs_pol))
                break;
		}
        index++;
	}

    if (index >= TVIN_SIG_FMT_COMPONENT_MAX) {
        index = TVIN_SIG_FMT_NULL;
    }

	return index;
}

// *****************************************************************************
// Function:set auto phase window
//
//   Params: format index
//
//   Return: success/error
//
// *****************************************************************************
static void tvafe_adc_set_bd_window(struct tvafe_vga_sig_win_s *win)
{

    WRITE_CBUS_REG_BITS(TVFE_AP_MUXCTRL1, win->hstart, AP_HSTART_BIT, AP_HSTART_WID);

    WRITE_CBUS_REG_BITS(TVFE_AP_MUXCTRL1, win->hend, AP_HEND_BIT, AP_HEND_WID);

    WRITE_CBUS_REG_BITS(TVFE_AP_MUXCTRL2, win->vstart, AP_VSTART_BIT, AP_VSTART_WID);

    WRITE_CBUS_REG_BITS(TVFE_AP_MUXCTRL2, win->vend, AP_VEND_BIT, AP_VEND_WID);

}

static void tvafe_adc_set_ap_window(struct tvafe_vga_sig_win_s *win)
{

    WRITE_CBUS_REG_BITS(TVFE_BD_MUXCTRL1, win->hstart, BD_HSTART_BIT, BD_HSTART_WID);

    WRITE_CBUS_REG_BITS(TVFE_BD_MUXCTRL1, win->hend, BD_HEND_BIT, BD_HEND_WID);

    WRITE_CBUS_REG_BITS(TVFE_BD_MUXCTRL2, win->vstart, BD_VSTART_BIT, BD_VSTART_WID);

    WRITE_CBUS_REG_BITS(TVFE_BD_MUXCTRL2, win->vend, BD_VEND_BIT, BD_VEND_WID);

    WRITE_CBUS_REG_BITS(TVFE_BD_MUXCTRL1, 1, BD_WIN_EN_BIT, BD_WIN_EN_WID);
}
// *****************************************************************************
// Function:set afe clamp function
//
//   Params: format index
//
//   Return: none
//
// *****************************************************************************
static void tvafe_adc_set_clamp(enum tvin_sig_fmt_e fmt)
{
    //set clamp starting edge and duration
    tvafe_adc_set_clamp_para(fmt);

    //set clamp starting edge and duration
    //if (clamp_type <= CLAMP_BOTTOM_REGULATED)
    //    tvafe_adc_set_clamp_reference(ch, ref_val);

    //enable clamp type
    //tvafe_adc_clamp_select(ch, clamp_type);

}

// *****************************************************************************
// Function:set clock
//
//   Params: clock value
//
//   Return: success/error
//
// *****************************************************************************
static void tvafe_vga_set_clock(unsigned int clock)
{
    unsigned int tmp;

    tmp = (clock >> 4) & 0x000000FF;

 	WRITE_CBUS_REG_BITS(ADC_REG_01, tmp, PLLDIVRATIO_MSB_BIT, PLLDIVRATIO_MSB_WID);

    tmp = clock & 0x0000000F;
	WRITE_CBUS_REG_BITS(ADC_REG_02, tmp, PLLDIVRATIO_LSB_BIT, PLLDIVRATIO_LSB_WID);

    return;
}

// *****************************************************************************
// Function:set phase
//
//   Params: phase value
//
//   Return: none
//
// *****************************************************************************
static void tvafe_vga_set_phase(unsigned int phase)
{
    unsigned char tmp;

    //check phase  range
    if (phase > 31)
	    phase = 31;

    tmp = phase & 0x0000001F;
	WRITE_CBUS_REG_BITS(ADC_REG_56, tmp, PLLDIVRATIO_LSB_BIT, PLLDIVRATIO_LSB_WID);

    return;
}

// *****************************************************************************
// Function:set h  position
//
//   Params: position value
//
//   Return: none
//
// *****************************************************************************
static void tvafe_vga_set_h_pos(enum tvin_sig_fmt_e fmt, unsigned int hpos)
{
    unsigned int tmp;

    //h start
    tmp = tvin_fmt_tbl[fmt].hs_width + tvin_fmt_tbl[fmt].hs_width + hpos;
	WRITE_CBUS_REG_BITS(TVFE_DEG_H, tmp, DEG_HSTART_BIT, DEG_HSTART_WID);

    //h end
    tmp += tvin_fmt_tbl[fmt].h_active;
	WRITE_CBUS_REG_BITS(TVFE_DEG_H, tmp, DEG_HEND_BIT, DEG_HEND_WID);

    return;
}

// *****************************************************************************
// Function:set v position
//
//   Params: v position value
//
//   Return: none
//
// *****************************************************************************
static void tvafe_vga_set_v_pos(enum tvin_sig_fmt_e fmt, unsigned int vpos)
{
    unsigned int tmp;

    //odd position
    tmp = tvin_fmt_tbl[fmt].vs_width + tvin_fmt_tbl[fmt].vs_width + vpos;
    WRITE_CBUS_REG_BITS(TVFE_DEG_VODD, tmp, DEG_VSTART_ODD_BIT, DEG_VSTART_ODD_WID);

    tmp += tvin_fmt_tbl[fmt].v_active;
    WRITE_CBUS_REG_BITS(TVFE_DEG_VODD, tmp, DEG_VEND_ODD_BIT, DEG_VEND_ODD_WID);

    //even position
    tmp = tvin_fmt_tbl[fmt].vs_width + tvin_fmt_tbl[fmt].vs_width + vpos + 1;
    WRITE_CBUS_REG_BITS(TVFE_DEG_VEVEN, tmp, DEG_VSTART_EVEN_BIT, DEG_VSTART_EVEN_WID);

    tmp += tvin_fmt_tbl[fmt].v_active + 1;
    WRITE_CBUS_REG_BITS(TVFE_DEG_VEVEN, tmp, DEG_VEND_EVEN_BIT, DEG_VEND_EVEN_WID);

    return;
}

void tvafe_vga_set_border_cfg(struct tvafe_vga_boarder_cfg_s *bd)
{

    WRITE_CBUS_REG_BITS(TVFE_AP_MUXCTRL3, bd->th_pixel,
        BD_R_TH_BIT, BD_R_TH_WID);
    WRITE_CBUS_REG_BITS(TVFE_AP_MUXCTRL5, bd->th_pixel,
        BD_G_TH_BIT, BD_G_TH_WID);
    WRITE_CBUS_REG_BITS(TVFE_AP_MUXCTRL5, bd->th_pixel,
        BD_B_TH_BIT, BD_B_TH_WID);

    WRITE_CBUS_REG_BITS(TVFE_AP_MUXCTRL1, bd->th_pixel_sel,
        BD_DET_METHOD_BIT, BD_DET_METHOD_WID);

    WRITE_CBUS_REG_BITS(TVFE_BD_MUXCTRL3, bd->th_line,
        BD_VLD_LN_TH_BIT, BD_VLD_LN_TH_WID);
    WRITE_CBUS_REG_BITS(TVFE_BD_MUXCTRL3, bd->th_line_sel,
        BD_VALID_LN_EN_BIT, BD_VALID_LN_EN_WID);

    tvafe_adc_set_bd_window(&bd->bd_win);

    WRITE_CBUS_REG_BITS(TVFE_AP_MUXCTRL1, bd->bd_en,
        BD_DET_EN_BIT, BD_DET_EN_WID);

}

void tvafe_vga_set_ap_cfg(struct tvafe_vga_ap_cfg_s *ap)
{

    WRITE_CBUS_REG_BITS(TVFE_AP_MUXCTRL1, ap->ap_diff_sel,
        AP_DIFF_SEL_BIT, AP_DIFF_SEL_WID);

    WRITE_CBUS_REG_BITS(TVFE_AP_MUXCTRL1, ap->ap_specific_sel,
        AP_SPECIFIC_POINT_OUT_BIT, AP_SPECIFIC_POINT_OUT_WID);

    tvafe_adc_set_ap_window(&ap->ap_win);


    WRITE_CBUS_REG_BITS(TVFE_AP_MUXCTRL3, ap->ap_th_coring,
        AP_CORING_TH_BIT, AP_CORING_TH_WID);
    WRITE_CBUS_REG_BITS(TVFE_AP_MUXCTRL3, ap->ap_th_coring,
        AP_CORING_TH_BIT, AP_CORING_TH_WID);


    WRITE_CBUS_REG_BITS(TVFE_AP_MUXCTRL3, ap->ap_th_coring,
        AP_CORING_TH_BIT, AP_CORING_TH_WID);

    WRITE_CBUS_REG_BITS(TVFE_AP_MUXCTRL1, ap->ap_en,
        AUTOPHASE_EN_BIT, AUTOPHASE_EN_WID);

}

// *****************************************************************************
// Function:get the result of H border detection
//
//   Params: format index
//
//   Return: success/error
//
// *****************************************************************************
void tvafe_vga_get_border_info(struct tvafe_vga_boarder_info_s *bd)
{
    unsigned int tmp;

    tmp = READ_CBUS_REG(TVFE_AP_INDICATOR14);
    bd->r_hstart = (tmp >> BD_R_RIGHT_HCNT_BIT) & BD_R_RIGHT_HCNT_WID;
    bd->r_hend = (tmp >> BD_R_LEFT_HCNT_BIT) & BD_R_LEFT_HCNT_WID;

    tmp = READ_CBUS_REG(TVFE_AP_INDICATOR13);
    bd->r_vstart = (tmp >> BD_R_BOT_VCNT_BIT) & BD_R_BOT_VCNT_WID;
    bd->r_vend = (tmp >> BD_R_TOP_VCNT_BIT) & BD_R_TOP_VCNT_WID;

    tmp = READ_CBUS_REG(TVFE_AP_INDICATOR16);
    bd->g_hstart = (tmp >> BD_G_RIGHT_HCNT_BIT) & BD_G_RIGHT_HCNT_WID;
    bd->g_hend = (tmp >> BD_G_LEFT_HCNT_BIT) & BD_G_LEFT_HCNT_WID;

    tmp = READ_CBUS_REG(TVFE_AP_INDICATOR15);
    bd->g_vstart = (tmp >> BD_G_BOT_VCNT_BIT) & BD_G_BOT_VCNT_WID;
    bd->g_vend = (tmp >> BD_G_TOP_VCNT_BIT) & BD_G_TOP_VCNT_WID;

    tmp = READ_CBUS_REG(TVFE_AP_INDICATOR18);
    bd->b_hstart = (tmp >> BD_B_RIGHT_HCNT_BIT) & BD_B_RIGHT_HCNT_WID;
    bd->b_hend = (tmp >> BD_B_LEFT_HCNT_BIT) & BD_B_LEFT_HCNT_WID;

    tmp = READ_CBUS_REG(TVFE_AP_INDICATOR17);
    bd->b_vstart = (tmp >> BD_B_BOT_VCNT_BIT) & BD_B_BOT_VCNT_WID;
    bd->b_vend = (tmp >> BD_B_TOP_VCNT_BIT) & BD_B_TOP_VCNT_WID;

}

// *****************************************************************************
// Function:read coarse diff sum value of RGB from regs
//
//   Params: phase index
//
//   Return: sucess/error
//
// *****************************************************************************
void tvafe_vga_get_ap_info(struct tvafe_vga_ap_info_s *pixel_info)
{
    pixel_info->diff_sum.r_sum = READ_CBUS_REG(TVFE_AP_INDICATOR1);
    pixel_info->diff_sum.g_sum = READ_CBUS_REG(TVFE_AP_INDICATOR2);
    pixel_info->diff_sum.b_sum = READ_CBUS_REG(TVFE_AP_INDICATOR3);

    pixel_info->val_point.r_max = READ_CBUS_REG_BITS(TVFE_AP_INDICATOR4,
                                        AP_R_MAX_BIT, AP_R_MAX_WID);
    pixel_info->val_point.r_min = READ_CBUS_REG_BITS(TVFE_AP_INDICATOR4,
                                        AP_R_MIN_BIT, AP_R_MIN_WID);
    pixel_info->val_point.r_max_hcnt = READ_CBUS_REG_BITS(TVFE_AP_INDICATOR7,
                                        AP_R_MAX_HCNT_BIT, AP_R_MAX_HCNT_WID);
    pixel_info->val_point.r_max_vcnt = READ_CBUS_REG_BITS(TVFE_AP_INDICATOR7,
                                        AP_R_MAX_VCNT_BIT, AP_R_MAX_VCNT_WID);
    pixel_info->val_point.r_min_hcnt = READ_CBUS_REG_BITS(TVFE_AP_INDICATOR10,
                                        AP_R_MAX_HCNT_BIT, AP_R_MAX_HCNT_WID);
    pixel_info->val_point.r_min_vcnt = READ_CBUS_REG_BITS(TVFE_AP_INDICATOR10,
                                        AP_R_MAX_VCNT_BIT, AP_R_MAX_VCNT_WID);

    pixel_info->val_point.g_max = READ_CBUS_REG_BITS(TVFE_AP_INDICATOR5,
                                        AP_R_MAX_BIT, AP_R_MAX_WID);
    pixel_info->val_point.g_min = READ_CBUS_REG_BITS(TVFE_AP_INDICATOR5,
                                        AP_R_MIN_BIT, AP_R_MIN_WID);
    pixel_info->val_point.g_max_hcnt = READ_CBUS_REG_BITS(TVFE_AP_INDICATOR8,
                                        AP_R_MAX_HCNT_BIT, AP_R_MAX_HCNT_WID);
    pixel_info->val_point.g_max_vcnt = READ_CBUS_REG_BITS(TVFE_AP_INDICATOR8,
                                        AP_R_MAX_VCNT_BIT, AP_R_MAX_VCNT_WID);
    pixel_info->val_point.g_min_hcnt = READ_CBUS_REG_BITS(TVFE_AP_INDICATOR11,
                                        AP_R_MAX_HCNT_BIT, AP_R_MAX_HCNT_WID);
    pixel_info->val_point.g_min_vcnt = READ_CBUS_REG_BITS(TVFE_AP_INDICATOR11,
                                        AP_R_MAX_VCNT_BIT, AP_R_MAX_VCNT_WID);

    pixel_info->val_point.b_max = READ_CBUS_REG_BITS(TVFE_AP_INDICATOR6,
                                        AP_R_MAX_BIT, AP_R_MAX_WID);
    pixel_info->val_point.b_min = READ_CBUS_REG_BITS(TVFE_AP_INDICATOR6,
                                        AP_R_MIN_BIT, AP_R_MIN_WID);
    pixel_info->val_point.b_max_hcnt = READ_CBUS_REG_BITS(TVFE_AP_INDICATOR9,
                                        AP_R_MAX_HCNT_BIT, AP_R_MAX_HCNT_WID);
    pixel_info->val_point.b_max_vcnt = READ_CBUS_REG_BITS(TVFE_AP_INDICATOR9,
                                        AP_R_MAX_VCNT_BIT, AP_R_MAX_VCNT_WID);
    pixel_info->val_point.b_min_hcnt = READ_CBUS_REG_BITS(TVFE_AP_INDICATOR12,
                                        AP_R_MAX_HCNT_BIT, AP_R_MAX_HCNT_WID);
    pixel_info->val_point.b_min_vcnt = READ_CBUS_REG_BITS(TVFE_AP_INDICATOR12,
                                        AP_R_MAX_VCNT_BIT, AP_R_MAX_VCNT_WID);

}

// *****************************************************************************
// Function:configure the format setting
//
//   Params: format index
//
//   Return: none
//
// *****************************************************************************
void tvafe_adc_configure(enum tvin_sig_fmt_e fmt)
{
    //set adc clock by standard
    tvafe_adc_set_clock(fmt);

    //set adc clamp by standard
    tvafe_adc_set_clamp(fmt);

    //set channel bandwidth
    tvafe_adc_set_bw_lpf(fmt);

    //load vga reg hardcode
    //tvafe_adc_load_hardcode();
}

// call by 10ms_timer at frontend side
void tvafe_adc_state_handler(struct tvafe_info_s *info)
{
    enum tvin_sig_fmt_e tmp_fmt = 0;
    //struct tvin_para_s info = {TVIN_SIG_FMT_NULL, TVIN_SIG_STATUS_NULL};

    //new_format = ...;
    tvafe_adc_get_timing_info();
    switch (event_adc)
    {
        case TVIN_EVENT_NOSIG:
            if (adc_info.no_sig == 1)
                cnt_adc |= 0x00000001;
            else
            {
                cnt_adc &= 0xffffff00;
                event_adc = TVIN_EVENT_UNSTABLE;
            }

            if ((cnt_adc & 0x000000ff) > 8)
            {
                cnt_adc &= 0xffffff00;
                if (event_adc != TVIN_EVENT_NOSIG)  // newly confirmed TVIN_SIG_STATUS_NOSIG
                {
                    event_adc = TVIN_EVENT_NOSIG;
                    info->param.fmt = TVIN_SIG_FMT_NULL;
                    info->param.status = TVIN_SIG_STATUS_NOSIG;
                }
            }
            break;
        case TVIN_EVENT_UNSTABLE:
            if (adc_info.no_sig == 1)
                cnt_adc |= 0x00000001;
            else if (adc_info.h_offset > 5 || adc_info.v_offset > 1)
                cnt_adc |= 0x00000100;
            else
                cnt_adc |= 0x00010000;

            if ((cnt_adc & 0x000000ff) > 4)
            {
                cnt_adc &= 0xffffff00;
                if (event_adc != TVIN_EVENT_NOSIG)  // newly confirmed TVIN_SIG_STATUS_UNSTABLE
                {
                    event_adc = TVIN_EVENT_NOSIG;
                    info->param.fmt = TVIN_SIG_FMT_NULL;
                    info->param.status = TVIN_SIG_STATUS_NOSIG;
                }
            }
            if ((cnt_adc & 0x0000ff00) > 8)
            {
                cnt_adc &= 0xffff00ff;
                if (event_adc != TVIN_EVENT_UNSTABLE)  // newly confirmed TVIN_SIG_STATUS_UNSTABLE
                {
                    event_adc = TVIN_EVENT_UNSTABLE;
                    info->param.fmt = TVIN_SIG_FMT_NULL;
                    info->param.status = TVIN_SIG_STATUS_UNSTABLE;
                }
            }

            if ((cnt_adc & 0x00ff0000) > 8)
            {
                cnt_adc &= 0xff00ffff;
                if (tvafe_adc_get_pll_status() == 1)  // newly confirmed TVIN_SIG_STATUS_UNSTABLE
                {
                    event_adc = TVIN_EVENT_STABLE;
                    tmp_fmt = tvafe_adc_search_mode();
                    if (tmp_fmt == TVIN_SIG_FMT_NULL)
                        info->param.status = TVIN_SIG_STATUS_NOTSUP;
                    else
                        info->param.status = TVIN_SIG_STATUS_STABLE;
                    info->param.fmt = tmp_fmt;
                }
            }
            break;
        case TVIN_EVENT_STABLE:
            if (adc_info.no_sig == 1)
                cnt_adc |= 0x00000001;
            else if (adc_info.h_offset > 5 || adc_info.v_offset > 1)
                cnt_adc |= 0x00000100;
            else
                cnt_adc |= 0x00010000;

            if (((cnt_adc & 0x000000ff) > 2) || ((cnt_adc & 0x0000ff00) > 2))
            {
                cnt_adc &= 0xffff0000;
                if (event_adc != TVIN_EVENT_UNSTABLE)  // newly confirmed TVIN_SIG_STATUS_UNSTABLE
                    event_adc = TVIN_EVENT_UNSTABLE;
            }

            if ((cnt_adc & 0x00ff0000) > 16)
            {  //check format again and again
                cnt_adc &= 0xff00ffff;
                if (tvafe_adc_get_pll_status() == 1)
                {
                    tmp_fmt = tvafe_adc_search_mode();
                    if (info->param.fmt != tmp_fmt)
                    {
                        if (tmp_fmt == TVIN_SIG_FMT_NULL)
                            info->param.status = TVIN_SIG_STATUS_NOTSUP;
                        else
                            info->param.status = TVIN_SIG_STATUS_STABLE;
                        info->param.fmt = tmp_fmt;
                    }
                }
            }
            break;
        default:
            break;
    }

}


// call by 10ms_timer at frontend side
void tvafe_adc_state_init(void)
{
    event_adc = TVIN_EVENT_NOSIG;

}

void tvafe_adc_set_param(struct tvafe_info_s *info)
{
    tvafe_vga_set_clock(info->adc_param.clk);
    tvafe_vga_set_phase(info->adc_param.phase);
    tvafe_vga_set_h_pos(info->param.fmt, info->adc_param.hpos);
    tvafe_vga_set_v_pos(info->param.fmt, info->adc_param.vpos);
}

/* ADC */
//TVIN_SIG_FMT_VGA_800X600P_60D317
const static int vga_adc_reg_default[][2] = {
    {ADC_REG_00, 0x00000000,},// ADC_REG_00
    {ADC_REG_01, 0x00000041,},// ADC_REG_01
    {ADC_REG_02, 0x000000f0,},// ADC_REG_02
    {ADC_REG_03, 0x00000008,},// ADC_REG_03
    {ADC_REG_04, 0x00000014,},// ADC_REG_04
    {ADC_REG_05, 0x00000012,},// ADC_REG_05
    {ADC_REG_06, 0x00000000,},// ADC_REG_06
    {ADC_REG_07, 0x00000000,},// ADC_REG_07
    {ADC_REG_08, 0x00000000,},// ADC_REG_08
    {ADC_REG_09, 0x00000000,},// ADC_REG_09
    {ADC_REG_0B, 0x00000010,},// ADC_REG_0B
    {ADC_REG_0C, 0x00000010,},// ADC_REG_0C
    {ADC_REG_0D, 0x00000010,},// ADC_REG_0D
    {ADC_REG_0F, 0x00000040,},// ADC_REG_0F
    {ADC_REG_10, 0x00000040,},// ADC_REG_10
    {ADC_REG_11, 0x00000040,},// ADC_REG_11
    {ADC_REG_13, 0x00000023,},// ADC_REG_13
    {ADC_REG_14, 0x00000023,},// ADC_REG_14
    {ADC_REG_15, 0x000000e3,},// ADC_REG_15
    {ADC_REG_17, 0x00000000,},// ADC_REG_17
    {ADC_REG_18, 0x00000000,},// ADC_REG_18
    {ADC_REG_19, 0x00000030,},// ADC_REG_19
    {ADC_REG_1A, 0x00000030,},// ADC_REG_1A
    {ADC_REG_1B, 0x00000030,},// ADC_REG_1B
    {ADC_REG_1E, 0x00000069,},// ADC_REG_1E
    {ADC_REG_1F, 0x00000088,},// ADC_REG_1F
    {ADC_REG_20, 0x00000010,},// ADC_REG_20
    {ADC_REG_21, 0x00000003,},// ADC_REG_21
    {ADC_REG_24, 0x00000000,},// ADC_REG_24
    {ADC_REG_26, 0x00000078,},// ADC_REG_26
    {ADC_REG_27, 0x00000078,},// ADC_REG_27
    {ADC_REG_28, 0x00000000,},// ADC_REG_28
    {ADC_REG_2A, 0x00000000,},// ADC_REG_2A
    {ADC_REG_2B, 0x00000000,},// ADC_REG_2B
    {ADC_REG_2E, 0x00000046,},// ADC_REG_2E
    {ADC_REG_2F, 0x00000028,},// ADC_REG_2F
    {ADC_REG_30, 0x00000020,},// ADC_REG_30
    {ADC_REG_31, 0x00000020,},// ADC_REG_31
    {ADC_REG_32, 0x0000000c,},// ADC_REG_32
    {ADC_REG_33, 0x0000000f,},// ADC_REG_33
    {ADC_REG_34, 0x00000025,},// ADC_REG_34
    {ADC_REG_35, 0x00000002,},// ADC_REG_35
    {ADC_REG_38, 0x00000020,},// ADC_REG_38
    {ADC_REG_39, 0x000000c0,},// ADC_REG_39
    {ADC_REG_3A, 0x00000003,},// ADC_REG_3A
    {ADC_REG_3B, 0x0000005c,},// ADC_REG_3B
    {ADC_REG_3C, 0x00000054,},// ADC_REG_3C
    {ADC_REG_3D, 0x00000081,},// ADC_REG_3D
    {ADC_REG_3E, 0x00000008,},// ADC_REG_3E
    {ADC_REG_3F, 0x00000008,},// ADC_REG_3F
    {ADC_REG_41, 0x00000007,},// ADC_REG_41
    {ADC_REG_43, 0x000000f0,},// ADC_REG_43
    {ADC_REG_46, 0x00000051,},// ADC_REG_46
    {ADC_REG_47, 0x0000005a,},// ADC_REG_47
    {ADC_REG_56, 0x00000010,},// ADC_REG_56
    {ADC_REG_58, 0x00000000,},// ADC_REG_58
    {ADC_REG_59, 0x00000006,},// ADC_REG_59
    {ADC_REG_5A, 0x00000022,},// ADC_REG_5A
    {ADC_REG_5B, 0x00000022,},// ADC_REG_5B
    {ADC_REG_5C, 0x00000057,},// ADC_REG_5C
    {ADC_REG_5D, 0x0000006f,},// ADC_REG_5D
    {ADC_REG_5E, 0x00000004,},// ADC_REG_5E
    {ADC_REG_5F, 0x00000004,},// ADC_REG_5F
    {ADC_REG_60, 0x000000f4,},// ADC_REG_60
    {ADC_REG_61, 0x00000000,},// ADC_REG_61
    {ADC_REG_62, 0x00000005,},// ADC_REG_62
    {ADC_REG_63, 0x00000029,},// ADC_REG_63
    {ADC_REG_64, 0x00000000,},// ADC_REG_64
    {ADC_REG_65, 0x00000000,},// ADC_REG_65
    {ADC_REG_66, 0x00000010,},// ADC_REG_66
    {ADC_REG_68, 0x0000002d,},// ADC_REG_68
    {ADC_REG_69, 0x00000039,},// ADC_REG_69
    {0xFFFFFFFF, 0x00000000,}
};
/* TOP */ //TVIN_SIG_FMT_VGA_800X600P_60D317
const static  int vga_top_reg_default[][2] = {
    {TVFE_DVSS_MUXCTRL             , 0x07000008,} ,// TVFE_DVSS_MUXCTRL
    {TVFE_DVSS_MUXVS_REF           , 0x00000000,} ,// TVFE_DVSS_MUXVS_REF
    {TVFE_DVSS_MUXCOAST_V          , 0x0200000c,} ,// TVFE_DVSS_MUXCOAST_V
    {TVFE_DVSS_SEP_HVWIDTH         , 0x000a0073,} ,// TVFE_DVSS_SEP_HVWIDTH
    {TVFE_DVSS_SEP_HPARA           , 0x026b0343,} ,// TVFE_DVSS_SEP_HPARA
    {TVFE_DVSS_SEP_VINTEG          , 0x0fff0100,} ,// TVFE_DVSS_SEP_VINTEG
    {TVFE_DVSS_SEP_H_THR           , 0x00005002,} ,// TVFE_DVSS_SEP_H_THR
    {TVFE_DVSS_SEP_CTRL            , 0x40000008,} ,// TVFE_DVSS_SEP_CTRL
    {TVFE_DVSS_GEN_WIDTH           , 0x000a0073,} ,// TVFE_DVSS_GEN_WIDTH
    {TVFE_DVSS_GEN_PRD             , 0x020d0359,} ,// TVFE_DVSS_GEN_PRD
    {TVFE_DVSS_GEN_COAST           , 0x01cc001c,} ,// TVFE_DVSS_GEN_COAST
    {TVFE_DVSS_NOSIG_PARA          , 0x00000009,} ,// TVFE_DVSS_NOSIG_PARA
    {TVFE_DVSS_NOSIG_PLS_TH        , 0x05000010,} ,// TVFE_DVSS_NOSIG_PLS_TH
    {TVFE_DVSS_GATE_H              , 0x00270016,} ,// TVFE_DVSS_GATE_H
    {TVFE_DVSS_GATE_V              , 0x020a0009,} ,// TVFE_DVSS_GATE_V
    {TVFE_DVSS_INDICATOR1          , 0x00000000,} ,// TVFE_DVSS_INDICATOR1
    {TVFE_DVSS_INDICATOR2          , 0x00000000,} ,// TVFE_DVSS_INDICATOR2
    {TVFE_DVSS_MVDET_CTRL1         , 0x00000000,} ,// TVFE_DVSS_MVDET_CTRL1
    {TVFE_DVSS_MVDET_CTRL2         , 0x00000000,} ,// TVFE_DVSS_MVDET_CTRL2
    {TVFE_DVSS_MVDET_CTRL3         , 0x00000000,} ,// TVFE_DVSS_MVDET_CTRL3
    {TVFE_DVSS_MVDET_CTRL4         , 0x00000000,} ,// TVFE_DVSS_MVDET_CTRL4
    {TVFE_DVSS_MVDET_CTRL5         , 0x00000000,} ,// TVFE_DVSS_MVDET_CTRL5
    {TVFE_DVSS_MVDET_CTRL6         , 0x00000000,} ,// TVFE_DVSS_MVDET_CTRL6
    {TVFE_DVSS_MVDET_CTRL7         , 0x00000000,} ,// TVFE_DVSS_MVDET_CTRL7
    {TVFE_SYNCTOP_SPOL_MUXCTRL     , 0x00000009,} ,// TVFE_SYNCTOP_SPOL_MUXCTRL
    {TVFE_SYNCTOP_INDICATOR1_HCNT  , 0x00000000,} ,// TVFE_SYNCTOP_INDICATOR1_HCNT
    {TVFE_SYNCTOP_INDICATOR2_VCNT  , 0x00000000,} ,// TVFE_SYNCTOP_INDICATOR2_VCNT
    {TVFE_SYNCTOP_INDICATOR3       , 0x00000000,} ,// TVFE_SYNCTOP_INDICATOR3
    {TVFE_SYNCTOP_SFG_MUXCTRL1     , 0x89315107,} ,// TVFE_SYNCTOP_SFG_MUXCTRL1
    {TVFE_SYNCTOP_SFG_MUXCTRL2     , 0x00334400,} ,// TVFE_SYNCTOP_SFG_MUXCTRL2
    {TVFE_SYNCTOP_INDICATOR4       , 0x00000000,} ,// TVFE_SYNCTOP_INDICATOR4
    {TVFE_SYNCTOP_SAM_MUXCTRL      , 0x00082001,} ,// TVFE_SYNCTOP_SAM_MUXCTRL
    {TVFE_MISC_WSS1_MUXCTRL1       , 0x00000000,} ,// TVFE_MISC_WSS1_MUXCTRL1
    {TVFE_MISC_WSS1_MUXCTRL2       , 0x00000000,} ,// TVFE_MISC_WSS1_MUXCTRL2
    {TVFE_MISC_WSS2_MUXCTRL1       , 0x00000000,} ,// TVFE_MISC_WSS2_MUXCTRL1
    {TVFE_MISC_WSS2_MUXCTRL2       , 0x00000000,} ,// TVFE_MISC_WSS2_MUXCTRL2
    {TVFE_MISC_WSS1_INDICATOR1     , 0x00000000,} ,// TVFE_MISC_WSS1_INDICATOR1
    {TVFE_MISC_WSS1_INDICATOR2     , 0x00000000,} ,// TVFE_MISC_WSS1_INDICATOR2
    {TVFE_MISC_WSS1_INDICATOR3     , 0x00000000,} ,// TVFE_MISC_WSS1_INDICATOR3
    {TVFE_MISC_WSS1_INDICATOR4     , 0x00000000,} ,// TVFE_MISC_WSS1_INDICATOR4
    {TVFE_MISC_WSS1_INDICATOR5     , 0x00000000,} ,// TVFE_MISC_WSS1_INDICATOR5
    {TVFE_MISC_WSS2_INDICATOR1     , 0x00000000,} ,// TVFE_MISC_WSS2_INDICATOR1
    {TVFE_MISC_WSS2_INDICATOR2     , 0x00000000,} ,// TVFE_MISC_WSS2_INDICATOR2
    {TVFE_MISC_WSS2_INDICATOR3     , 0x00000000,} ,// TVFE_MISC_WSS2_INDICATOR3
    {TVFE_MISC_WSS2_INDICATOR4     , 0x00000000,} ,// TVFE_MISC_WSS2_INDICATOR4
    {TVFE_MISC_WSS2_INDICATOR5     , 0x00000000,} ,// TVFE_MISC_WSS2_INDICATOR5
    {TVFE_AP_MUXCTRL1              , 0x19310010,} ,// TVFE_AP_MUXCTRL1
    {TVFE_AP_MUXCTRL2              , 0x00200010,} ,// TVFE_AP_MUXCTRL2
    {TVFE_AP_MUXCTRL3              , 0x10000030,} ,// TVFE_AP_MUXCTRL3
    {TVFE_AP_MUXCTRL4              , 0x00000000,} ,// TVFE_AP_MUXCTRL4
    {TVFE_AP_MUXCTRL5              , 0x10040000,} ,// TVFE_AP_MUXCTRL5
    {TVFE_AP_INDICATOR1            , 0x00000000,} ,// TVFE_AP_INDICATOR1
    {TVFE_AP_INDICATOR2            , 0x00000000,} ,// TVFE_AP_INDICATOR2
    {TVFE_AP_INDICATOR3            , 0x00000000,} ,// TVFE_AP_INDICATOR3
    {TVFE_AP_INDICATOR4            , 0x00000000,} ,// TVFE_AP_INDICATOR4
    {TVFE_AP_INDICATOR5            , 0x00000000,} ,// TVFE_AP_INDICATOR5
    {TVFE_AP_INDICATOR6            , 0x00000000,} ,// TVFE_AP_INDICATOR6
    {TVFE_AP_INDICATOR7            , 0x00000000,} ,// TVFE_AP_INDICATOR7
    {TVFE_AP_INDICATOR8            , 0x00000000,} ,// TVFE_AP_INDICATOR8
    {TVFE_AP_INDICATOR9            , 0x00000000,} ,// TVFE_AP_INDICATOR9
    {TVFE_AP_INDICATOR10           , 0x00000000,} ,// TVFE_AP_INDICATOR10
    {TVFE_AP_INDICATOR11           , 0x00000000,} ,// TVFE_AP_INDICATOR11
    {TVFE_AP_INDICATOR12           , 0x00000000,} ,// TVFE_AP_INDICATOR12
    {TVFE_AP_INDICATOR13           , 0x00000000,} ,// TVFE_AP_INDICATOR13
    {TVFE_AP_INDICATOR14           , 0x00000000,} ,// TVFE_AP_INDICATOR14
    {TVFE_AP_INDICATOR15           , 0x00000000,} ,// TVFE_AP_INDICATOR15
    {TVFE_AP_INDICATOR16           , 0x00000000,} ,// TVFE_AP_INDICATOR16
    {TVFE_AP_INDICATOR17           , 0x00000000,} ,// TVFE_AP_INDICATOR17
    {TVFE_AP_INDICATOR18           , 0x00000000,} ,// TVFE_AP_INDICATOR18
    {TVFE_AP_INDICATOR19           , 0x00000000,} ,// TVFE_AP_INDICATOR19
    {TVFE_BD_MUXCTRL1              , 0x01320000,} ,// TVFE_BD_MUXCTRL1
    {TVFE_BD_MUXCTRL2              , 0x0020d000,} ,// TVFE_BD_MUXCTRL2
    {TVFE_BD_MUXCTRL3              , 0x00000000,} ,// TVFE_BD_MUXCTRL3
    {TVFE_BD_MUXCTRL4              , 0x00000000,} ,// TVFE_BD_MUXCTRL4
    {TVFE_CLP_MUXCTRL1             , 0x00000000,} ,// TVFE_CLP_MUXCTRL1
    {TVFE_CLP_MUXCTRL2             , 0x00000000,} ,// TVFE_CLP_MUXCTRL2
    {TVFE_CLP_MUXCTRL3             , 0x00000000,} ,// TVFE_CLP_MUXCTRL3
    {TVFE_CLP_MUXCTRL4             , 0x00000000,} ,// TVFE_CLP_MUXCTRL4
    {TVFE_CLP_INDICATOR1           , 0x00000000,} ,// TVFE_CLP_INDICATOR1
    {TVFE_BPG_BACKP_H              , 0x00000000,} ,// TVFE_BPG_BACKP_H
    {TVFE_BPG_BACKP_V              , 0x00000000,} ,// TVFE_BPG_BACKP_V
    {TVFE_DEG_H                    , 0x003f80d8,} ,// TVFE_DEG_H
    {TVFE_DEG_VODD                 , 0x0027301b,} ,// TVFE_DEG_VODD
    {TVFE_DEG_VEVEN                , 0x0027301b,} ,// TVFE_DEG_VEVEN
    {TVFE_OGO_OFFSET1              , 0x00000000,} ,// TVFE_OGO_OFFSET1
    {TVFE_OGO_GAIN1                , 0x00000000,} ,// TVFE_OGO_GAIN1
    {TVFE_OGO_GAIN2                , 0x00000000,} ,// TVFE_OGO_GAIN2
    {TVFE_OGO_OFFSET2              , 0x00000000,} ,// TVFE_OGO_OFFSET2
    {TVFE_OGO_OFFSET3              , 0x00000000,} ,// TVFE_OGO_OFFSET3
    {TVFE_VAFE_CTRL                , 0x00000001,} ,// TVFE_VAFE_CTRL
    {TVFE_VAFE_STATUS              , 0x00000000,} ,// TVFE_VAFE_STATUS
    {TVFE_TOP_CTRL                 , 0x00008750,} ,// TVFE_TOP_CTRL
    {TVFE_CLAMP_INTF               , 0x00000000,} ,// TVFE_CLAMP_INTF
    {TVFE_RST_CTRL                 , 0x00000000,} ,// TVFE_RST_CTRL
    {TVFE_EXT_VIDEO_AFE_CTRL_MUX1  , 0x00000000,} ,// TVFE_EXT_VIDEO_AFE_CTRL_MUX1
    {TVFE_AAFILTER_CTRL1           , 0x00082222,} ,// TVFE_AAFILTER_CTRL1
    {TVFE_AAFILTER_CTRL2           , 0x252b39c6,} ,// TVFE_AAFILTER_CTRL2
    {TVFE_EDID_CONFIG              , 0x00000000,} ,// TVFE_EDID_CONFIG
    {TVFE_EDID_RAM_ADDR            , 0x00000000,} ,// TVFE_EDID_RAM_ADDR
    {TVFE_EDID_RAM_WDATA           , 0x00000000,} ,// TVFE_EDID_RAM_WDATA
    {TVFE_EDID_RAM_RDATA           , 0x00000000,} ,// TVFE_EDID_RAM_RDATA
    {TVFE_APB_ERR_CTRL_MUX1        , 0x00000000,} ,// TVFE_APB_ERR_CTRL_MUX1
    {TVFE_APB_ERR_CTRL_MUX2        , 0x00000000,} ,// TVFE_APB_ERR_CTRL_MUX2
    {TVFE_APB_INDICATOR1           , 0x00000000,} ,// TVFE_APB_INDICATOR1
    {TVFE_APB_INDICATOR2           , 0x00000000,} ,// TVFE_APB_INDICATOR2
    {TVFE_ADC_READBACK_CTRL        , 0xa0142003,} ,// TVFE_ADC_READBACK_CTRL
    {TVFE_ADC_READBACK_INDICATOR   , 0x00000000,} ,// TVFE_ADC_READBACK_INDICATOR
    {TVFE_INT_CLR                  , 0x00000000,} ,// TVFE_INT_CLR
    {TVFE_INT_MSKN                 , 0x00000000,} ,// TVFE_INT_MASKN
    {TVFE_INT_INDICATOR1           , 0x00000000,} ,// TVFE_INT_INDICATOR1
    {TVFE_INT_SET                  , 0x00000000,} ,// TVFE_INT_SET
    {TVFE_CHIP_VERSION             , 0x00000000,} ,// TVFE_CHIP_VERSION
    {0xFFFFFFFF                    , 0x00000000,} // TVFE_CHIP_VERSION
};

//TVIN_SIG_FMT_VGA_800X600P_60D317
void tvafe_set_vga_default(void)
{
    unsigned int i = 0;
    /** write 7740 register **/
    while (vga_adc_reg_default[i][0] != 0xFFFFFFFF) {
        WRITE_CBUS_REG(vga_adc_reg_default[i][0], vga_adc_reg_default[i][1]);
        i++;
    }

    i = 0;
    /** write top register **/
    while (vga_top_reg_default[i][0] != 0xFFFFFFFF) {
        WRITE_CBUS_REG(vga_top_reg_default[i][0], vga_top_reg_default[i][1]);
        i++;
    }

}

/* ADC *//*from 480p, change htotal,vcorangesel0x68[1:0],chargepumpcurr0x69[6:4],pllsddiv0x5f[6:0],pllalfa0x59[4:0]*/
/*pllbeta0x5a[4:0],plllockth0x5e[6:0],pllunlockth0x62[6:0]*/
const static  int comp_adc_reg_default[][2] = {
    {ADC_REG_00, 0x00000000,} ,// ADC_REG_00
    {ADC_REG_01, 0x00000035,} ,// ADC_REG_01
    {ADC_REG_02, 0x000000f0,} ,// ADC_REG_02
    {ADC_REG_03, 0x00000014,} ,// ADC_REG_03
    {ADC_REG_04, 0x00000014,} ,// ADC_REG_04
    {ADC_REG_05, 0x00000012,} ,// ADC_REG_05
    {ADC_REG_06, 0x00000000,} ,// ADC_REG_06
    {ADC_REG_07, 0x00000000,} ,// ADC_REG_07
    {ADC_REG_08, 0x00000000,} ,// ADC_REG_08
    {ADC_REG_09, 0x00000000,} ,// ADC_REG_09
    {ADC_REG_0B, 0x00000010,} ,// ADC_REG_0B
    {ADC_REG_0C, 0x00000010,} ,// ADC_REG_0C
    {ADC_REG_0D, 0x00000010,} ,// ADC_REG_0D
    {ADC_REG_0F, 0x00000040,} ,// ADC_REG_0F
    {ADC_REG_10, 0x00000080,} ,// ADC_REG_10
    {ADC_REG_11, 0x00000080,} ,// ADC_REG_11
    {ADC_REG_13, 0x00000023,} ,// ADC_REG_13
    {ADC_REG_14, 0x00000023,} ,// ADC_REG_14
    {ADC_REG_15, 0x000000a3,} ,// ADC_REG_15
    {ADC_REG_17, 0x00000000,} ,// ADC_REG_17
    {ADC_REG_18, 0x00000000,} ,// ADC_REG_18
    {ADC_REG_19, 0x00000070,} ,// ADC_REG_19
    {ADC_REG_1A, 0x00000070,} ,// ADC_REG_1A
    {ADC_REG_1B, 0x00000070,} ,// ADC_REG_1B
    {ADC_REG_1E, 0x00000069,} ,// ADC_REG_1E
    {ADC_REG_1F, 0x00000088,} ,// ADC_REG_1F
    {ADC_REG_20, 0x00000010,} ,// ADC_REG_20
    {ADC_REG_21, 0x00000003,} ,// ADC_REG_21
    {ADC_REG_24, 0x00000000,} ,// ADC_REG_24
    {ADC_REG_26, 0x00000078,} ,// ADC_REG_26
    {ADC_REG_27, 0x00000078,} ,// ADC_REG_27
    {ADC_REG_28, 0x00000000,} ,// ADC_REG_28
    {ADC_REG_2A, 0x00000000,} ,// ADC_REG_2A
    {ADC_REG_2B, 0x00000000,} ,// ADC_REG_2B
    {ADC_REG_2E, 0x00000064,} ,// ADC_REG_2E
    {ADC_REG_2F, 0x00000028,} ,// ADC_REG_2F
    {ADC_REG_30, 0x00000020,} ,// ADC_REG_30
    {ADC_REG_31, 0x00000020,} ,// ADC_REG_31
    {ADC_REG_32, 0x00000008,} ,// ADC_REG_32
    {ADC_REG_33, 0x0000000f,} ,// ADC_REG_33
    {ADC_REG_34, 0x0000006f,} ,// ADC_REG_34
    {ADC_REG_35, 0x00000002,} ,// ADC_REG_35
    {ADC_REG_38, 0x00000020,} ,// ADC_REG_38
    {ADC_REG_39, 0x000000c0,} ,// ADC_REG_39
    {ADC_REG_3A, 0x00000003,} ,// ADC_REG_3A
    {ADC_REG_3B, 0x0000005c,} ,// ADC_REG_3B
    {ADC_REG_3C, 0x00000054,} ,// ADC_REG_3C
    {ADC_REG_3D, 0x00000081,} ,// ADC_REG_3D
    {ADC_REG_3E, 0x00000004,} ,// ADC_REG_3E
    {ADC_REG_3F, 0x00000002,} ,// ADC_REG_3F
    {ADC_REG_41, 0x00000007,} ,// ADC_REG_41
    {ADC_REG_43, 0x000000f0,} ,// ADC_REG_43
    {ADC_REG_46, 0x00000051,} ,// ADC_REG_46
    {ADC_REG_47, 0x0000005a,} ,// ADC_REG_47
    {ADC_REG_56, 0x00000010,} ,// ADC_REG_56
    {ADC_REG_58, 0x00000000,} ,// ADC_REG_58
    {ADC_REG_59, 0x00000006,} ,// ADC_REG_59
    {ADC_REG_5A, 0x00000023,} ,// ADC_REG_5A
    {ADC_REG_5B, 0x00000022,} ,// ADC_REG_5B
    {ADC_REG_5C, 0x0000008f,} ,// ADC_REG_5C
    {ADC_REG_5D, 0x0000006f,} ,// ADC_REG_5D
    {ADC_REG_5E, 0x0000000e,} ,// ADC_REG_5E
    {ADC_REG_5F, 0x00000002,} ,// ADC_REG_5F
    {ADC_REG_60, 0x000000f4,} ,// ADC_REG_60
    {ADC_REG_61, 0x00000000,} ,// ADC_REG_61
    {ADC_REG_62, 0x0000000e,} ,// ADC_REG_62
    {ADC_REG_63, 0x00000039,} ,// ADC_REG_63
    {ADC_REG_64, 0x00000000,} ,// ADC_REG_64
    {ADC_REG_65, 0x00000000,} ,// ADC_REG_65
    {ADC_REG_66, 0x00000010,} ,// ADC_REG_66
    {ADC_REG_68, 0x0000002c,} ,// ADC_REG_68
    {ADC_REG_69, 0x00000029,} ,// ADC_REG_69
    {0xFFFFFFFF, 0x00000000,}
};
/* TOP */
const static  int comp_top_reg_default[][2] = {
    {TVFE_DVSS_MUXCTRL             , 0x00000000,} ,// TVFE_DVSS_MUXCTRL
    {TVFE_DVSS_MUXVS_REF           , 0x00000000,} ,// TVFE_DVSS_MUXVS_REF
    {TVFE_DVSS_MUXCOAST_V          , 0x00000000,} ,// TVFE_DVSS_MUXCOAST_V
    {TVFE_DVSS_SEP_HVWIDTH         , 0x00000000,} ,// TVFE_DVSS_SEP_HVWIDTH
    {TVFE_DVSS_SEP_HPARA           , 0x00000000,} ,// TVFE_DVSS_SEP_HPARA
    {TVFE_DVSS_SEP_VINTEG          , 0x00000000,} ,// TVFE_DVSS_SEP_VINTEG
    {TVFE_DVSS_SEP_H_THR           , 0x00000000,} ,// TVFE_DVSS_SEP_H_THR
    {TVFE_DVSS_SEP_CTRL            , 0x00000000,} ,// TVFE_DVSS_SEP_CTRL
    {TVFE_DVSS_GEN_WIDTH           , 0x00000000,} ,// TVFE_DVSS_GEN_WIDTH
    {TVFE_DVSS_GEN_PRD             , 0x00000000,} ,// TVFE_DVSS_GEN_PRD
    {TVFE_DVSS_GEN_COAST           , 0x00000000,} ,// TVFE_DVSS_GEN_COAST
    {TVFE_DVSS_NOSIG_PARA          , 0x00000000,} ,// TVFE_DVSS_NOSIG_PARA
    {TVFE_DVSS_NOSIG_PLS_TH        , 0x00000000,} ,// TVFE_DVSS_NOSIG_PLS_TH
    {TVFE_DVSS_GATE_H              , 0x00000000,} ,// TVFE_DVSS_GATE_H
    {TVFE_DVSS_GATE_V              , 0x00000000,} ,// TVFE_DVSS_GATE_V
    {TVFE_DVSS_INDICATOR1          , 0x00000000,} ,// TVFE_DVSS_INDICATOR1
    {TVFE_DVSS_INDICATOR2          , 0x00000000,} ,// TVFE_DVSS_INDICATOR2
    {TVFE_DVSS_MVDET_CTRL1         , 0x00000000,} ,// TVFE_DVSS_MVDET_CTRL1
    {TVFE_DVSS_MVDET_CTRL2         , 0x00000000,} ,// TVFE_DVSS_MVDET_CTRL2
    {TVFE_DVSS_MVDET_CTRL3         , 0x00000000,} ,// TVFE_DVSS_MVDET_CTRL3
    {TVFE_DVSS_MVDET_CTRL4         , 0x00000000,} ,// TVFE_DVSS_MVDET_CTRL4
    {TVFE_DVSS_MVDET_CTRL5         , 0x00000000,} ,// TVFE_DVSS_MVDET_CTRL5
    {TVFE_DVSS_MVDET_CTRL6         , 0x00000000,} ,// TVFE_DVSS_MVDET_CTRL6
    {TVFE_DVSS_MVDET_CTRL7         , 0x00000000,} ,// TVFE_DVSS_MVDET_CTRL7
    {TVFE_SYNCTOP_SPOL_MUXCTRL     , 0x00000009,} ,// TVFE_SYNCTOP_SPOL_MUXCTRL
    {TVFE_SYNCTOP_INDICATOR1_HCNT  , 0x00000000,} ,// TVFE_SYNCTOP_INDICATOR1_HCNT
    {TVFE_SYNCTOP_INDICATOR2_VCNT  , 0x00000000,} ,// TVFE_SYNCTOP_INDICATOR2_VCNT
    {TVFE_SYNCTOP_INDICATOR3       , 0x00000000,} ,// TVFE_SYNCTOP_INDICATOR3
    {TVFE_SYNCTOP_SFG_MUXCTRL1     , 0x892880d8,} ,// TVFE_SYNCTOP_SFG_MUXCTRL1
    {TVFE_SYNCTOP_SFG_MUXCTRL2     , 0x00334400,} ,// TVFE_SYNCTOP_SFG_MUXCTRL2
    {TVFE_SYNCTOP_INDICATOR4       , 0x00000000,} ,// TVFE_SYNCTOP_INDICATOR4
    {TVFE_SYNCTOP_SAM_MUXCTRL      , 0x00082001,} ,// TVFE_SYNCTOP_SAM_MUXCTRL
    {TVFE_MISC_WSS1_MUXCTRL1       , 0x00000000,} ,// TVFE_MISC_WSS1_MUXCTRL1
    {TVFE_MISC_WSS1_MUXCTRL2       , 0x00000000,} ,// TVFE_MISC_WSS1_MUXCTRL2
    {TVFE_MISC_WSS2_MUXCTRL1       , 0x00000000,} ,// TVFE_MISC_WSS2_MUXCTRL1
    {TVFE_MISC_WSS2_MUXCTRL2       , 0x00000000,} ,// TVFE_MISC_WSS2_MUXCTRL2
    {TVFE_MISC_WSS1_INDICATOR1     , 0x00000000,} ,// TVFE_MISC_WSS1_INDICATOR1
    {TVFE_MISC_WSS1_INDICATOR2     , 0x00000000,} ,// TVFE_MISC_WSS1_INDICATOR2
    {TVFE_MISC_WSS1_INDICATOR3     , 0x00000000,} ,// TVFE_MISC_WSS1_INDICATOR3
    {TVFE_MISC_WSS1_INDICATOR4     , 0x00000000,} ,// TVFE_MISC_WSS1_INDICATOR4
    {TVFE_MISC_WSS1_INDICATOR5     , 0x00000000,} ,// TVFE_MISC_WSS1_INDICATOR5
    {TVFE_MISC_WSS2_INDICATOR1     , 0x00000000,} ,// TVFE_MISC_WSS2_INDICATOR1
    {TVFE_MISC_WSS2_INDICATOR2     , 0x00000000,} ,// TVFE_MISC_WSS2_INDICATOR2
    {TVFE_MISC_WSS2_INDICATOR3     , 0x00000000,} ,// TVFE_MISC_WSS2_INDICATOR3
    {TVFE_MISC_WSS2_INDICATOR4     , 0x00000000,} ,// TVFE_MISC_WSS2_INDICATOR4
    {TVFE_MISC_WSS2_INDICATOR5     , 0x00000000,} ,// TVFE_MISC_WSS2_INDICATOR5
    {TVFE_AP_MUXCTRL1              , 0x00000000,} ,// TVFE_AP_MUXCTRL1
    {TVFE_AP_MUXCTRL2              , 0x00000000,} ,// TVFE_AP_MUXCTRL2
    {TVFE_AP_MUXCTRL3              , 0x00000000,} ,// TVFE_AP_MUXCTRL3
    {TVFE_AP_MUXCTRL4              , 0x00000000,} ,// TVFE_AP_MUXCTRL4
    {TVFE_AP_MUXCTRL5              , 0x00000000,} ,// TVFE_AP_MUXCTRL5
    {TVFE_AP_INDICATOR1            , 0x00000000,} ,// TVFE_AP_INDICATOR1
    {TVFE_AP_INDICATOR2            , 0x00000000,} ,// TVFE_AP_INDICATOR2
    {TVFE_AP_INDICATOR3            , 0x00000000,} ,// TVFE_AP_INDICATOR3
    {TVFE_AP_INDICATOR4            , 0x00000000,} ,// TVFE_AP_INDICATOR4
    {TVFE_AP_INDICATOR5            , 0x00000000,} ,// TVFE_AP_INDICATOR5
    {TVFE_AP_INDICATOR6            , 0x00000000,} ,// TVFE_AP_INDICATOR6
    {TVFE_AP_INDICATOR7            , 0x00000000,} ,// TVFE_AP_INDICATOR7
    {TVFE_AP_INDICATOR8            , 0x00000000,} ,// TVFE_AP_INDICATOR8
    {TVFE_AP_INDICATOR9            , 0x00000000,} ,// TVFE_AP_INDICATOR9
    {TVFE_AP_INDICATOR10           , 0x00000000,} ,// TVFE_AP_INDICATOR10
    {TVFE_AP_INDICATOR11           , 0x00000000,} ,// TVFE_AP_INDICATOR11
    {TVFE_AP_INDICATOR12           , 0x00000000,} ,// TVFE_AP_INDICATOR12
    {TVFE_AP_INDICATOR13           , 0x00000000,} ,// TVFE_AP_INDICATOR13
    {TVFE_AP_INDICATOR14           , 0x00000000,} ,// TVFE_AP_INDICATOR14
    {TVFE_AP_INDICATOR15           , 0x00000000,} ,// TVFE_AP_INDICATOR15
    {TVFE_AP_INDICATOR16           , 0x00000000,} ,// TVFE_AP_INDICATOR16
    {TVFE_AP_INDICATOR17           , 0x00000000,} ,// TVFE_AP_INDICATOR17
    {TVFE_AP_INDICATOR18           , 0x00000000,} ,// TVFE_AP_INDICATOR18
    {TVFE_AP_INDICATOR19           , 0x00000000,} ,// TVFE_AP_INDICATOR19
    {TVFE_BD_MUXCTRL1              , 0x00000000,} ,// TVFE_BD_MUXCTRL1
    {TVFE_BD_MUXCTRL2              , 0x00000000,} ,// TVFE_BD_MUXCTRL2
    {TVFE_BD_MUXCTRL3              , 0x00000000,} ,// TVFE_BD_MUXCTRL3
    {TVFE_BD_MUXCTRL4              , 0x00000000,} ,// TVFE_BD_MUXCTRL4
    {TVFE_CLP_MUXCTRL1             , 0x00000000,} ,// TVFE_CLP_MUXCTRL1
    {TVFE_CLP_MUXCTRL2             , 0x00000000,} ,// TVFE_CLP_MUXCTRL2
    {TVFE_CLP_MUXCTRL3             , 0x00000000,} ,// TVFE_CLP_MUXCTRL3
    {TVFE_CLP_MUXCTRL4             , 0x00000000,} ,// TVFE_CLP_MUXCTRL4
    {TVFE_CLP_INDICATOR1           , 0x00000000,} ,// TVFE_CLP_INDICATOR1
    {TVFE_BPG_BACKP_H              , 0x00000000,} ,// TVFE_BPG_BACKP_H
    {TVFE_BPG_BACKP_V              , 0x00000000,} ,// TVFE_BPG_BACKP_V
    {TVFE_DEG_H                    , 0x00354084,} ,// TVFE_DEG_H
    {TVFE_DEG_VODD                 , 0x00236016,} ,// TVFE_DEG_VODD
    {TVFE_DEG_VEVEN                , 0x00237017,} ,// TVFE_DEG_VEVEN
    {TVFE_OGO_OFFSET1              , 0x00000000,} ,// TVFE_OGO_OFFSET1
    {TVFE_OGO_GAIN1                , 0x00000000,} ,// TVFE_OGO_GAIN1
    {TVFE_OGO_GAIN2                , 0x00000000,} ,// TVFE_OGO_GAIN2
    {TVFE_OGO_OFFSET2              , 0x00000000,} ,// TVFE_OGO_OFFSET2
    {TVFE_OGO_OFFSET3              , 0x00000000,} ,// TVFE_OGO_OFFSET3
    {TVFE_VAFE_CTRL                , 0x00000000,} ,// TVFE_VAFE_CTRL
    {TVFE_VAFE_STATUS              , 0x00000000,} ,// TVFE_VAFE_STATUS
    {TVFE_TOP_CTRL                 , 0x00008750,} ,// TVFE_TOP_CTRL
    {TVFE_CLAMP_INTF               , 0x00000000,} ,// TVFE_CLAMP_INTF
    {TVFE_RST_CTRL                 , 0x00000000,} ,// TVFE_RST_CTRL
    {TVFE_EXT_VIDEO_AFE_CTRL_MUX1  , 0x00000000,} ,// TVFE_EXT_VIDEO_AFE_CTRL_MUX1
    {TVFE_AAFILTER_CTRL1           , 0x00082222,} ,// TVFE_AAFILTER_CTRL1
    {TVFE_AAFILTER_CTRL2           , 0x252b39c6,} ,// TVFE_AAFILTER_CTRL2
    {TVFE_EDID_CONFIG              , 0x00000000,} ,// TVFE_EDID_CONFIG
    {TVFE_EDID_RAM_ADDR            , 0x00000000,} ,// TVFE_EDID_RAM_ADDR
    {TVFE_EDID_RAM_WDATA           , 0x00000000,} ,// TVFE_EDID_RAM_WDATA
    {TVFE_EDID_RAM_RDATA           , 0x00000000,} ,// TVFE_EDID_RAM_RDATA
    {TVFE_APB_ERR_CTRL_MUX1        , 0x00000000,} ,// TVFE_APB_ERR_CTRL_MUX1
    {TVFE_APB_ERR_CTRL_MUX2        , 0x00000000,} ,// TVFE_APB_ERR_CTRL_MUX2
    {TVFE_APB_INDICATOR1           , 0x00000000,} ,// TVFE_APB_INDICATOR1
    {TVFE_APB_INDICATOR2           , 0x00000000,} ,// TVFE_APB_INDICATOR2
    {TVFE_ADC_READBACK_CTRL        , 0x00000000,} ,// TVFE_ADC_READBACK_CTRL
    {TVFE_ADC_READBACK_INDICATOR   , 0x00000000,} ,// TVFE_ADC_READBACK_INDICATOR
    {TVFE_INT_CLR                  , 0x00000000,} ,// TVFE_INT_CLR
    {TVFE_INT_MSKN                 , 0x00000000,} ,// TVFE_INT_MASKN
    {TVFE_INT_INDICATOR1           , 0x00000000,} ,// TVFE_INT_INDICATOR1
    {TVFE_INT_SET                  , 0x00000000,} ,// TVFE_INT_SET
    {TVFE_CHIP_VERSION             , 0x00000000,} ,// TVFE_CHIP_VERSION
    {0xFFFFFFFF                    , 0x00000000,}
};

/* TVAFE_SIG_FORMAT_576I_50        */
void tvafe_set_comp_default(void)
{
    unsigned int i = 0;

    /** write 7740 register **/
    while (comp_adc_reg_default[i][0] != 0xFFFFFFFF) {
        WRITE_CBUS_REG(comp_adc_reg_default[i][0], comp_adc_reg_default[i][1]);
        i++;
    }

    i = 0;
    /** write top register **/
    while (comp_top_reg_default[i][0] != 0xFFFFFFFF) {
        WRITE_CBUS_REG(comp_top_reg_default[i][0], comp_top_reg_default[i][1]);
        i++;
    }

}


