/*******************************************************************
 *
 *  Copyright C 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description:
 *
 *  Author: Amlogic Software
 *
 *******************************************************************/
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/pinctrl/consumer.h>
#include <mach/am_regs.h>
#include <linux/delay.h>
#include <mach/gpio.h>
#include <linux/amlogic/aml_audio_codec_probe.h>
#include <linux/amlogic/aml_gpio_consumer.h>



extern struct i2c_client * i2c_new_device(struct i2c_adapter *adap,
            struct i2c_board_info const *info);


static struct platform_device* audio_codec_pdev = NULL;

bool is_rt5631;
bool is_wm8960;
bool is_rt5616;

static int regist_codec_info(struct device_node* p_node, aml_audio_codec_info_t* audio_codec_dev)
{
    int ret = 0;
    ret = of_property_read_string(p_node, "codec_name", &audio_codec_dev->name);
    if (ret) {
        printk("get audio codec name failed!\n");
    }
    ret = of_property_read_string(p_node, "status", &audio_codec_dev->status);
    if(ret){
        printk("%s:this audio codec is disabled!\n",audio_codec_dev->name);
    }
    if(!strcmp(audio_codec_dev->name, "rt5631") && !strcmp(audio_codec_dev->status,"okay")){
        is_rt5631 = true;
    }else if(strcmp(audio_codec_dev->name, "wm8960") && strcmp(audio_codec_dev->status,"okay")){
        is_wm8960 = true;
    }else if(strcmp(audio_codec_dev->name, "rt5616") && strcmp(audio_codec_dev->status,"okay")){
        is_rt5616 = true;
    }

    printk("*********is_rt5631=%d,is_wm8960=%d,is_rt5616=%d*\n",is_rt5631,is_wm8960,is_rt5616);
    return 0;
}


static int get_audio_codec_i2c_info(struct device_node* p_node, aml_audio_codec_info_t* audio_codec_dev)
{
    const char* str;
    int ret = 0;
    unsigned i2c_addr;
    struct i2c_adapter *adapter;

    ret = of_property_read_string(p_node, "codec_name", &audio_codec_dev->name);
    if (ret) {
        printk("get audio codec name failed!\n");
        goto err_out;
    }

    ret = of_property_match_string(p_node,"status","okay");
    if(ret){
        printk("%s:this audio codec is disabled!\n",audio_codec_dev->name);
        goto err_out;
    }
    printk("use audio codec %s\n",audio_codec_dev->name);

    ret = of_property_read_u32(p_node,"capless",&audio_codec_dev->capless);
    if(ret){
        printk("don't find audio codec capless mode!\n");
    }

    ret = of_property_read_string(p_node, "i2c_bus", &str);
    if (ret) {
        printk("%s: faild to get i2c_bus str,use default i2c bus!\n", audio_codec_dev->name);
        audio_codec_dev->i2c_bus_type = AML_I2C_BUS_D;
    } else {
        if (!strncmp(str, "i2c_bus_a", 9))
            audio_codec_dev->i2c_bus_type = AML_I2C_BUS_A;
        else if (!strncmp(str, "i2c_bus_b", 9))
            audio_codec_dev->i2c_bus_type = AML_I2C_BUS_B;
        else if (!strncmp(str, "i2c_bus_c", 9))
            audio_codec_dev->i2c_bus_type = AML_I2C_BUS_C;
        else if (!strncmp(str, "i2c_bus_d", 9))
            audio_codec_dev->i2c_bus_type = AML_I2C_BUS_D;
        else if (!strncmp(str, "i2c_bus_ao", 10))
            audio_codec_dev->i2c_bus_type = AML_I2C_BUS_AO;
        else
            audio_codec_dev->i2c_bus_type = AML_I2C_BUS_D;
    }

    ret = of_property_read_u32(p_node,"i2c_addr",&i2c_addr);
    if(ret){
        printk("don't find i2c adress capless,use default!\n");
        audio_codec_dev->i2c_addr = 0x1B;
    }else{
        audio_codec_dev->i2c_addr = i2c_addr;
    }
    printk("audio codec addr: 0x%x\n", audio_codec_dev->i2c_addr);
    printk("audio codec i2c bus: %d\n", audio_codec_dev->i2c_bus_type);

    /* test if the camera is exist */
    adapter = i2c_get_adapter(audio_codec_dev->i2c_bus_type);
    if (!adapter) {
        printk("can not do probe function\n");
        ret = -1;
        goto err_out;
    }
    ret = 0;

err_out:
    return ret;
}


static int aml_audio_codec_probe(struct platform_device *pdev)
{
    struct device_node* audio_codec_node = pdev->dev.of_node;
    struct device_node* child;
    struct i2c_board_info board_info;
    struct i2c_adapter *adapter;
    aml_audio_codec_info_t temp_audio_codec;
    audio_codec_pdev = pdev;
    is_rt5631 = false;
    is_wm8960 = false;
    is_rt5616 = false;
    for_each_child_of_node(audio_codec_node, child) {

        memset(&temp_audio_codec, 0, sizeof(aml_audio_codec_info_t));
        regist_codec_info(child,&temp_audio_codec);
        if (get_audio_codec_i2c_info(child, &temp_audio_codec)) {
            continue;
        }
        memset(&board_info, 0, sizeof(board_info));
        strncpy(board_info.type, temp_audio_codec.name, I2C_NAME_SIZE);
        adapter = i2c_get_adapter(temp_audio_codec.i2c_bus_type);
        board_info.addr = temp_audio_codec.i2c_addr;
        board_info.platform_data = &temp_audio_codec;
        i2c_new_device(adapter, &board_info);
    }
    return 0;
}


static int aml_audio_codec_remove(struct platform_device *pdev)
{
    is_rt5631 = false;
    is_wm8960 = false;
    is_rt5616 = false;

    return 0;
}

static const struct of_device_id aml_audio_codec_probe_dt_match[]={
    {
        .compatible = "amlogic,audio_codec",
    },
    {},
};

static  struct platform_driver aml_audio_codec_probe_driver = {
    .probe      = aml_audio_codec_probe,
    .remove     = aml_audio_codec_remove,
    .driver     = {
        .name   = "aml_audio_codec_probe",
        .owner  = THIS_MODULE,
        .of_match_table = aml_audio_codec_probe_dt_match,
    },
};

static int __init aml_audio_codec_probe_init(void)
{
    int ret;

    ret = platform_driver_register(&aml_audio_codec_probe_driver);
    if (ret){
        printk(KERN_ERR"aml_audio_codec_probre_driver register failed\n");
        return ret;
    }

    return ret;
}

static void __exit aml_audio_codec_probe_exit(void)
{
    platform_driver_unregister(&aml_audio_codec_probe_driver);
}

module_init(aml_audio_codec_probe_init);
module_exit(aml_audio_codec_probe_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Amlogic Audio Codec prober driver");

