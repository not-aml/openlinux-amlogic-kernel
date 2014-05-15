
#ifndef __BT656_601_INPUT_H
#define __BT656_601_INPUT_H
#include <linux/amports/vframe.h>
#include <linux/amports/vframe_provider.h>
//#define DEBUG

//extern void set_next_field_656_601_camera_in_anci_address(unsigned char index);

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
extern void start_amvdec_656_601_camera_in(tvin_sig_format_t input_mode);
extern void stop_amvdec_656_601_camera_in(tvin_sig_format_t input_mode);

extern int amvdec_656_601_camera_in_run(vframe_t *info);

#endif				//__BT656_601_INPUT_H

