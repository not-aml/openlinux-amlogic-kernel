#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-aml.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <mach/am_regs.h>
#include <mach/gpio.h>
#include <linux/amlogic/vout/aml_lcd_extern.h>
#include <linux/amlogic/aml_gpio_consumer.h>

//#define LCD_EXT_DEBUG_INFO
#ifdef LCD_EXT_DEBUG_INFO
#define DBG_PRINT(...)		printk(__VA_ARGS__)
#else
#define DBG_PRINT(...)
#endif

struct aml_lcd_extern_driver_t lcd_ext_driver;

struct aml_lcd_extern_driver_t* aml_lcd_extern_get_driver(void)
{
	return &lcd_ext_driver;
}

static int get_lcd_extern_dt_data(struct device_node* of_node, struct lcd_extern_data_t *pdata)
{
	int err;
	const char *str;
	
	err = of_property_read_string(of_node, "dev_name", (const char **)&pdata->name);
	if (err) {
		pdata->name = "aml_lcd_extern";
		printk("warning: get dev_name failed, set dev_name aml_lcd_extern\n");
	}
	err = of_property_read_u32(of_node, "type", &pdata->type);
	if (err) {
		pdata->type = LCD_EXTERN_MAX;
		printk("warning: get type failed, exit\n");
		return -1;
	}
	err = of_property_read_string(of_node, "status", &str);
	if (err) {
		printk("%s warning: get status failed, default disable\n", pdata->name);
		pdata->status = 0;
	}
	else {
		if ((strncmp(str, "ok", 2) == 0) || (strncmp(str, "enable", 6) == 0))
			pdata->status = 1;
		else
			pdata->status = 0;
	}
	switch (pdata->type) {
		case LCD_EXTERN_I2C:
			err = of_property_read_u32(of_node,"address",&pdata->addr);
			if (err) {
				printk("%s warning: get i2c address failed\n", pdata->name);
				pdata->addr = 0;
			}
			DBG_PRINT("%s: address=0x%02x\n", pdata->name, pdata->addr);
		  
			err = of_property_read_string(of_node, "i2c_bus", &str);
			if (err) {
				printk("%s warning: get i2c_bus failed, use default i2c bus\n", pdata->name);
				pdata->i2c_bus = AML_I2C_MASTER_A;
			}
			else {
				if (strncmp(str, "i2c_bus_a", 9) == 0)
					pdata->i2c_bus = AML_I2C_MASTER_A;
				else if (strncmp(str, "i2c_bus_b", 9) == 0)
					pdata->i2c_bus = AML_I2C_MASTER_B;
				else if (strncmp(str, "i2c_bus_c", 9) == 0)
					pdata->i2c_bus = AML_I2C_MASTER_C;
				else if (strncmp(str, "i2c_bus_d", 9) == 0)
					pdata->i2c_bus = AML_I2C_MASTER_D;
				else if (strncmp(str, "i2c_bus_ao", 10) == 0)
					pdata->i2c_bus = AML_I2C_MASTER_AO;
				else
					pdata->i2c_bus = AML_I2C_MASTER_A; 
			}
			DBG_PRINT("%s: i2c_bus=%s[%d]\n", pdata->name, str, pdata->i2c_bus);
			break;
		case LCD_EXTERN_SPI:
			break;
		default:
			break;
	}
	
	return 0;
}

static int lcd_extern_probe(struct platform_device *pdev)
{
	struct device_node* child;
	struct i2c_board_info i2c_info;
	struct i2c_adapter *adapter;
	struct i2c_client *i2c_client;
	int i = 0;
	struct lcd_extern_data_t *pdata = NULL;
	
	if (!pdata)
		pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
	if (!pdata)	{
		printk("failed to alloc data\n");
		goto lcd_extern_probe_failed;
	}
	
	pdev->dev.platform_data = pdata;
	
	for_each_child_of_node(pdev->dev.of_node, child) {
		if (get_lcd_extern_dt_data(child, pdata) != 0) {
			printk("failed to get dt data\n");
			goto lcd_extern_probe_failed;
		}
		if (pdata->status == 1) {
			switch (pdata->type) {
				case LCD_EXTERN_I2C:
					memset(&i2c_info, 0, sizeof(i2c_info));
					
					adapter = i2c_get_adapter(pdata->i2c_bus);
					if (!adapter) {
						printk("warning£ºfailed to get i2c adapter\n");
						goto lcd_extern_probe_failed;
					}
					
					strncpy(i2c_info.type, pdata->name, I2C_NAME_SIZE);
					i2c_info.addr = pdata->addr;
					i2c_info.platform_data = pdata;
					i2c_info.flags=0;
					if(i2c_info.addr>0x7f)
						i2c_info.flags=0x10;
					i2c_client = i2c_new_device(adapter, &i2c_info);
					if (!i2c_client) {
						printk("%s :failed to new i2c device\n", pdata->name);
						goto lcd_extern_probe_failed;
					}
					else{
						DBG_PRINT("%s: new i2c device succeed\n",((struct lcd_extern_data_t *)(i2c_client->dev.platform_data))->name);
					}
					
					lcd_ext_driver.type = LCD_EXTERN_I2C;
					break;
				case LCD_EXTERN_SPI:
					lcd_ext_driver.type = LCD_EXTERN_SPI;
					break;
				default:
					break;
			}
			goto lcd_extern_probe_successful;	//only 1 driver allowed
		}
		i++;
	}

lcd_extern_probe_successful:
	printk("%s ok\n", __FUNCTION__);
	return 0;
	
lcd_extern_probe_failed:
	if (pdata)
		kfree(pdata);
	return -1;
}

static int lcd_extern_remove(struct platform_device *pdev)
{
	if (pdev->dev.platform_data)
	 	kfree (pdev->dev.platform_data);
    return 0;
}

#ifdef CONFIG_USE_OF
static const struct of_device_id aml_lcd_extern_dt_match[]={
	{	
		.compatible = "amlogic,lcd_extern",
	},
	{},
};
#else
#define aml_lcd_extern_dt_match NULL
#endif

static struct platform_driver aml_lcd_extern_driver = {
	.probe		= lcd_extern_probe,
	.remove		= lcd_extern_remove,
	.driver		= {
		.name	= "aml_lcd_extern",
		.owner	= THIS_MODULE,
#ifdef CONFIG_USE_OF
		.of_match_table = aml_lcd_extern_dt_match,
#endif
	},
};

static int __init lcd_extern_init(void)
{
	int ret;
	printk("%s\n", __FUNCTION__);
	ret = platform_driver_register(&aml_lcd_extern_driver);
	if (ret) {
		printk("failed to register lcd extern driver module\n");
		return -ENODEV;
	}
	return ret;
}

static void __exit lcd_extern_exit(void)
{
	platform_driver_unregister(&aml_lcd_extern_driver);
}

module_init(lcd_extern_init);
module_exit(lcd_extern_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Amlogic LCD External bridge driver");