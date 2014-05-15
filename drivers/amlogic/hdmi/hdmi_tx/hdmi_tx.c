/*
 * Amlogic M1 
 * frame buffer driver-----------HDMI_TX
 * Copyright (C) 2010 Amlogic, Inc.
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
 */


#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/major.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h> 
#include <asm/uaccess.h>
#include <mach/am_regs.h>

#include <linux/osd/osd_dev.h>

#include "hdmi_info_global.h"
#include "hdmi_tx_module.h"

#define DEVICE_NAME "amhdmitx"
#define HDMI_TX_COUNT 32
#define HDMI_TX_POOL_NUM  6
#define HDMI_TX_RESOURCE_NUM 4


#ifdef DEBUG
#define pr_dbg(fmt, args...) printk(KERN_DEBUG "amhdmitx: " fmt, ## args)
#else
#define pr_dbg(fmt, args...)
#endif
#define pr_error(fmt, args...) printk(KERN_ERR "amhdmitx: " fmt, ## args)


static hdmitx_dev_t hdmitx_device;

static dev_t hdmitx_id;
static struct class *hdmitx_class;
static struct device *hdmitx_dev;

//static HDMI_TX_INFO_t hdmi_info;

#undef DISABLE_AUDIO

/*****************************
*    hdmitx attr management :
*    enable
*    mode
*    reg
******************************/
static  int  set_disp_mode(const char *mode)
{
    int ret=-1;
    HDMI_Video_Codes_t vic;
    vic = hdmitx_edid_get_VIC(&hdmitx_device, mode, 1);
    hdmitx_device.cur_VIC = HDMI_Unkown;
    ret = hdmitx_set_display(&hdmitx_device, vic);
    if(ret>=0){
        hdmitx_device.cur_VIC = vic;  
    }
    return ret;
}

static int set_disp_mode_auto(void)
{
    int ret=-1;
    const vinfo_t *info = get_current_vinfo();
    HDMI_Video_Codes_t vic;
    vic = hdmitx_edid_get_VIC(&hdmitx_device, info->name, (hdmitx_device.disp_switch_config==DISP_SWITCH_FORCE)?1:0);
    hdmitx_device.cur_VIC = HDMI_Unkown;
    ret = hdmitx_set_display(&hdmitx_device, vic); //if vic is HDMI_Unkown, hdmitx_set_display will disable HDMI
    if(ret>=0){
        hdmitx_device.cur_VIC = vic;    
    }
    return ret;
}    

/*disp_mode attr*/
static ssize_t show_disp_mode(struct device * dev, struct device_attribute *attr, char * buf)
{
    int pos=0;
    pos+=snprintf(buf+pos, PAGE_SIZE, "VIC:%d\r\n", hdmitx_device.cur_VIC);
    return pos;    
}
    
static ssize_t store_disp_mode(struct device * dev, struct device_attribute *attr, const char * buf, size_t count)
{
    set_disp_mode(buf);
    return 16;    
}

/*aud_mode attr*/
static ssize_t show_aud_mode(struct device * dev, struct device_attribute *attr, char * buf)
{
    return 0;    
}
    
static ssize_t store_aud_mode(struct device * dev, struct device_attribute *attr, const char * buf, size_t count)
{
    //set_disp_mode(buf);
    Hdmi_tx_audio_para_t audio_param;
    if(strncmp(buf, "32k", 3)==0){
        audio_param.sample_rate = FS_32K; 
    }
    else if(strncmp(buf, "44.1k", 5)==0){
        audio_param.sample_rate = FS_44K1; 
    }
    else if(strncmp(buf, "48k", 3)==0){
        audio_param.sample_rate = FS_48K; 
    }
    else{
        return 0;
    }
    audio_param.type = CT_PCM;
    audio_param.channel_num = CC_2CH;
    audio_param.sample_size = SS_16BITS; 
    hdmitx_set_audio(&hdmitx_device, &audio_param);
    
    return 16;    
}

/*edid attr*/
static ssize_t show_edid(struct device *dev, struct device_attribute *attr, char *buf)
{
    return hdmitx_edid_dump(&hdmitx_device, buf, PAGE_SIZE);
}

static ssize_t store_edid(struct device * dev, struct device_attribute *attr, const char * buf, size_t count)
{
    
    if(buf[0]=='d'){
        int ii,jj;
        int block_idx;
        block_idx=simple_strtoul(buf+1,NULL,16);
        if(block_idx<EDID_MAX_BLOCK){
            for(ii=0;ii<8;ii++){
                for(jj=0;jj<16;jj++){
                    printk("%02x ",hdmitx_device.EDID_buf[block_idx*128+ii*16+jj]);
                }
                printk("\n");
            }
            printk("\n");
        }
    }
    return 16;    
}

/*config attr*/
static ssize_t show_config(struct device * dev, struct device_attribute *attr, char * buf)
{   
    int pos=0;
    pos += snprintf(buf+pos, PAGE_SIZE, "disp switch (force or edid): %s\r\n", (hdmitx_device.disp_switch_config==DISP_SWITCH_FORCE)?"force":"edid");
    return pos;    
}
    
static ssize_t store_config(struct device * dev, struct device_attribute *attr, const char * buf, size_t count)
{
    if(strncmp(buf, "force", 5)==0){
        hdmitx_device.disp_switch_config=DISP_SWITCH_FORCE;
    }
    else if(strncmp(buf, "edid", 4)==0){
        hdmitx_device.disp_switch_config=DISP_SWITCH_EDID;
    }
    else if(strncmp(buf, "vdacoff", 7)==0){
        if(hdmitx_device.HWOp.Cntl){
            hdmitx_device.HWOp.Cntl(&hdmitx_device, HDMITX_HWCMD_VDAC_OFF, 0);    
        }
    }
    else if(strncmp(buf, "low_power_on", 12)==0){
        if(hdmitx_device.HWOp.Cntl){
            hdmitx_device.HWOp.Cntl(&hdmitx_device, HDMITX_HWCMD_LOWPOWER_SWITCH, 1); 
        }
    }        
    else if(strncmp(buf, "low_power_off", 13)==0){
        if(hdmitx_device.HWOp.Cntl){
            hdmitx_device.HWOp.Cntl(&hdmitx_device, HDMITX_HWCMD_LOWPOWER_SWITCH, 0);    
        }
    }        
#if 0
    else if(strncmp(buf, "adacoff", 7)==0){
        //CLK_GATE_ON(AIU_AUD_DAC);
        //CLK_GATE_ON(AIU_AUD_DAC_CLK);
        audio_internal_dac_disable();
        //CLK_GATE_OFF(AIU_AUD_DAC_CLK);
        //CLK_GATE_OFF(AIU_AUD_DAC);
    }
#endif
    return 16;    
}
  
    
static ssize_t store_dbg(struct device * dev, struct device_attribute *attr, const char * buf, size_t count)
{
    hdmitx_device.HWOp.DebugFun(buf);
    return 16;    
}

/**/
static ssize_t show_disp_cap(struct device * dev, struct device_attribute *attr, char * buf)
{   
    int i,pos=0;
    char* disp_mode_t[]={"480i","480p","720p","1080i","1080p",NULL};
    char* native_disp_mode = hdmitx_edid_get_native_VIC(&hdmitx_device);
    HDMI_Video_Codes_t vic;
    for(i=0; disp_mode_t[i]; i++){
        vic = hdmitx_edid_get_VIC(&hdmitx_device, disp_mode_t[i], 0);
        if( vic != HDMI_Unkown){
            pos += snprintf(buf+pos, PAGE_SIZE,"%s",disp_mode_t[i]);
            if(native_disp_mode&&(strcmp(native_disp_mode, disp_mode_t[i])==0)){
                pos += snprintf(buf+pos, PAGE_SIZE,"*\n");
            }
            else{
                pos += snprintf(buf+pos, PAGE_SIZE,"\n");
            }                
        }
    }
    return pos;    
}

static DEVICE_ATTR(disp_mode, S_IWUSR | S_IRUGO, show_disp_mode, store_disp_mode);
static DEVICE_ATTR(aud_mode, S_IWUSR | S_IRUGO, show_aud_mode, store_aud_mode);
static DEVICE_ATTR(edid, S_IWUSR | S_IRUGO, show_edid, store_edid);
static DEVICE_ATTR(config, S_IWUSR | S_IRUGO, show_config, store_config);
static DEVICE_ATTR(debug, S_IWUSR | S_IRUGO, NULL, store_dbg);
static DEVICE_ATTR(disp_cap, S_IWUSR | S_IRUGO, show_disp_cap, NULL);

/*****************************
*    hdmitx display client interface 
*    
******************************/
static int hdmitx_notify_callback_v(struct notifier_block *block, unsigned long cmd , void *para)
{
    if (cmd != VOUT_EVENT_MODE_CHANGE)
        return -1;

    set_disp_mode_auto();

    return 0;
}


static struct notifier_block hdmitx_notifier_nb_v = {
    .notifier_call    = hdmitx_notify_callback_v,
};

#ifndef DISABLE_AUDIO

#define AOUT_EVENT_PREPARE  0x1
extern int aout_register_client(struct notifier_block * ) ;
extern int aout_unregister_client(struct notifier_block * ) ;

#include <linux/soundcard.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/control.h>

static int hdmitx_notify_callback_a(struct notifier_block *block, unsigned long cmd , void *para)
{
    if (cmd == AOUT_EVENT_PREPARE){
        struct snd_pcm_substream *substream =(struct snd_pcm_substream*)para;
        Hdmi_tx_audio_para_t audio_param;

        audio_param.type = CT_PCM;
        audio_param.channel_num = CC_2CH;
        audio_param.sample_size = SS_16BITS; 
    
        switch (substream->runtime->rate) {
            case 192000:
                audio_param.sample_rate = FS_192K; 
                break;
            case 176400:
                audio_param.sample_rate = FS_176K4; 
                break;
            case 96000:
                audio_param.sample_rate = FS_96K; 
                break;
            case 88200:
                audio_param.sample_rate = FS_88K2; 
                break;
            case 48000:
                audio_param.sample_rate = FS_48K; 
                break;
            case 44100:
                audio_param.sample_rate = FS_44K1; 
                break;
            case 32000:
                audio_param.sample_rate = FS_32K; 
                break;
            default:
                break;
        }
        hdmitx_set_audio(&hdmitx_device, &audio_param);
        printk("HDMI: aout notify rate %d\n", substream->runtime->rate);
        return 0;
    }
    return -1;
}

static struct notifier_block hdmitx_notifier_nb_a = {
    .notifier_call    = hdmitx_notify_callback_a,
};
#endif
/******************************
*  hdmitx kernel task
*******************************/
static int hdmi_task_handle(void *data) 
{
    hdmitx_dev_t* hdmitx_device = (hdmitx_dev_t*)data;

    hdmitx_init_parameters(&hdmitx_device->hdmi_info);

    HDMITX_M1B_Init(hdmitx_device);

    hdmitx_device->HWOp.SetupIRQ(hdmitx_device);

    while (hdmitx_device->hpd_event != 0xff)
    {
        if (hdmitx_device->hpd_event == 1)
        {
            if(hdmitx_device->HWOp.GetEDIDData(hdmitx_device)){
                hdmitx_edid_clear(hdmitx_device);
                hdmitx_edid_parse(hdmitx_device);
                set_disp_mode_auto();

                hdmitx_device->hpd_event = 0;
            }    
        }
        else if(hdmitx_device->hpd_event == 2)
        {
            hdmitx_edid_clear(hdmitx_device);

            //hdmitx_set_display(hdmitx_device, HDMI_Unkown);
            hdmitx_device->cur_VIC = HDMI_Unkown;

            hdmitx_device->hpd_event = 0;
        }    
        else{
        }            
        msleep(500);
    }

    return 0;

}


/*****************************
*    hdmitx driver file_operations 
*    
******************************/
static int amhdmitx_open(struct inode *node, struct file *file)
{
    hdmitx_dev_t *hdmitx_in_devp;

    /* Get the per-device structure that contains this cdev */
    hdmitx_in_devp = container_of(node->i_cdev, hdmitx_dev_t, cdev);
    file->private_data = hdmitx_in_devp;

    return 0;

}


static int amhdmitx_release(struct inode *node, struct file *file)
{
    //hdmitx_dev_t *hdmitx_in_devp = file->private_data;

    /* Reset file pointer */

    /* Release some other fields */
    /* ... */
    return 0;
}



static int amhdmitx_ioctl(struct inode *node, struct file *file, unsigned int cmd,   unsigned long args)
{
    int   r = 0;
    switch (cmd) {
        default:
            break;
    }
    return r;
}

const static struct file_operations amhdmitx_fops = {
    .owner    = THIS_MODULE,
    .open     = amhdmitx_open,
    .release  = amhdmitx_release,
    .ioctl    = amhdmitx_ioctl,
};


static int amhdmitx_probe(struct platform_device *pdev)
{
    int r;
    pr_dbg("amhdmitx_probe\n");
    r = alloc_chrdev_region(&hdmitx_id, 0, HDMI_TX_COUNT, DEVICE_NAME);
    if (r < 0) {
        pr_error("Can't register major for amhdmitx device\n");
        return r;
    }
    hdmitx_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(hdmitx_class))
    {
        unregister_chrdev_region(hdmitx_id, HDMI_TX_COUNT);
        return -1;
        //return PTR_ERR(aoe_class);
    }

    cdev_init(&(hdmitx_device.cdev), &amhdmitx_fops);
    hdmitx_device.cdev.owner = THIS_MODULE;
    cdev_add(&(hdmitx_device.cdev), hdmitx_id, HDMI_TX_COUNT);

    //hdmitx_dev = device_create(hdmitx_class, NULL, hdmitx_id, "amhdmitx%d", 0);
    hdmitx_dev = device_create(hdmitx_class, NULL, hdmitx_id, NULL, "amhdmitx%d", 0); //kernel>=2.6.27 

    device_create_file(hdmitx_dev, &dev_attr_disp_mode);
    device_create_file(hdmitx_dev, &dev_attr_aud_mode);
    device_create_file(hdmitx_dev, &dev_attr_edid);
    device_create_file(hdmitx_dev, &dev_attr_config);
    device_create_file(hdmitx_dev, &dev_attr_debug);
    device_create_file(hdmitx_dev, &dev_attr_disp_cap);
    
    if (hdmitx_dev == NULL) {
        pr_error("device_create create error\n");
        class_destroy(hdmitx_class);
        r = -EEXIST;
        return r;
    }
    vout_register_client(&hdmitx_notifier_nb_v);
#ifndef DISABLE_AUDIO
    aout_register_client(&hdmitx_notifier_nb_a);
#endif
    hdmitx_device.task = kthread_run(hdmi_task_handle, &hdmitx_device, "kthread_hdmi");

    return r;
}

static int amhdmitx_remove(struct platform_device *pdev)
{
    if(hdmitx_device.HWOp.UnInit){
        hdmitx_device.HWOp.UnInit(&hdmitx_device);
    }
    hdmitx_device.hpd_event = 0xff;
    kthread_stop(hdmitx_device.task);
    
    vout_unregister_client(&hdmitx_notifier_nb_v);    
#ifndef DISABLE_AUDIO
    aout_unregister_client(&hdmitx_notifier_nb_a);
#endif

    /* Remove the cdev */
    device_remove_file(hdmitx_dev, &dev_attr_disp_mode);
    device_remove_file(hdmitx_dev, &dev_attr_aud_mode);
    device_remove_file(hdmitx_dev, &dev_attr_edid);
    device_remove_file(hdmitx_dev, &dev_attr_config);
    device_remove_file(hdmitx_dev, &dev_attr_debug);
    device_remove_file(hdmitx_dev, &dev_attr_disp_cap);

    cdev_del(&hdmitx_device.cdev);

    device_destroy(hdmitx_class, hdmitx_id);

    class_destroy(hdmitx_class);

    unregister_chrdev_region(hdmitx_id, HDMI_TX_COUNT);
    return 0;
}

static struct platform_driver amhdmitx_driver = {
    .probe      = amhdmitx_probe,
    .remove     = amhdmitx_remove,
    .driver     = {
        .name   = DEVICE_NAME,
		    .owner	= THIS_MODULE,
    }
};

static struct platform_device* amhdmi_tx_device = NULL;

static int hdmitx_off = 0;

static int  __init amhdmitx_init(void)
{
    if(hdmitx_off)
        return 0;
        
    pr_dbg("amhdmitx_init\n");
	  amhdmi_tx_device = platform_device_alloc(DEVICE_NAME,0);
    if (!amhdmi_tx_device) {
        pr_error("failed to alloc amhdmi_tx_device\n");
        return -ENOMEM;
    }
    
    if(platform_device_add(amhdmi_tx_device)){
        platform_device_put(amhdmi_tx_device);
        pr_error("failed to add amhdmi_tx_device\n");
        return -ENODEV;
    }
    if (platform_driver_register(&amhdmitx_driver)) {
        pr_error("failed to register amhdmitx module\n");
        
        platform_device_del(amhdmi_tx_device);
        platform_device_put(amhdmi_tx_device);
        return -ENODEV;
    }
    return 0;
}




static void __exit amhdmitx_exit(void)
{
    pr_dbg("amhdmitx_exit\n");
    platform_driver_unregister(&amhdmitx_driver);
    platform_device_unregister(amhdmi_tx_device); 
    amhdmi_tx_device = NULL;
    return ;
}


module_init(amhdmitx_init);
module_exit(amhdmitx_exit);

MODULE_DESCRIPTION("AMLOGIC HDMI TX driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");

static  int __init hdmitx_off_setup(char *s)
{
	if((s[0]=='o')&&(s[1]=='f')&&(s[2]=='f')){
			hdmitx_off = 1;
	}
	return 0;
}

__setup("hdmitx=",hdmitx_off_setup);

