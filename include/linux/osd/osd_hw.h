#ifndef  OSD_HW_H
#define OSD_HW_H


typedef struct {
	s32 x_start;
	s32 x_end;
	s32 y_start;
	s32 y_end;
} pandata_t;


#define OSD_GLOBAL_ALPHA_DEF  0xff
#define OSD_DATA_BIG_ENDIAN 	0
#define OSD_DATA_LITTLE_ENDIAN 1
#define OSD_TC_ALPHA_ENABLE_DEF 0  //disable tc_alpha
#define   REG_OFFSET		0x20

extern void  osd_set_colorkey_hw(u32 index,u32 bpp,u32 colorkey ) ;
extern void  osd_srckey_enable_hw(u32  index,u8 enable);
extern void  osd_set_gbl_alpha_hw(u32 index,u32 gbl_alpha);
extern u32  osd_get_gbl_alpha_hw(u32  index);
extern void osd_setup(struct osd_ctl_s *osd_ctl,
                u32 xoffset,
                u32 yoffset,
                u32 xres,
                u32 yres,
                u32 xres_virtual ,
                u32 yres_virtual,
                u32 disp_start_x,
                u32 disp_start_y,
                u32 disp_end_x,
                u32 disp_end_y,
                u32 fbmem,
                enum osd_type_s type,
                int index);
extern void  osddev_update_disp_axis_hw(
			u32 display_h_start,
                  	u32 display_h_end,
                  	u32 display_v_start,
                  	u32 display_v_end,
			u32 xoffset,
                  	u32 yoffset,
                  	u32 mode_change,
                  	u32 index) ;
extern void osd_setpal_hw(unsigned regno, unsigned red, unsigned green, unsigned blue, unsigned transp,int index);
extern void osd_enable_hw(int enable,int index );
extern void osd_pan_display_hw(unsigned int xoffset, unsigned int yoffset,int index );
extern void osd_suspend_hw(void);
extern void osd_resume_hw(void);

#endif 
