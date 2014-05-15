/*
 * AMLOGIC lcd external driver.
 *
 * Communication protocol:
 * I2C 
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h> 
#include <linux/i2c.h>
#include <linux/i2c-aml.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/sysctl.h>
#include <asm/uaccess.h>
#include <mach/pinmux.h>
#include <mach/gpio.h>
#include <linux/platform_device.h>
#include <linux/amlogic/vout/aml_lcd_extern.h>

static struct i2c_client *aml_tc101_client;
extern struct aml_lcd_extern_driver_t lcd_ext_driver;

#define LCD_EXTERN_NAME			"lcd_tc101"

static unsigned char tc101_init_table[][3] = {
	//{0xff, 0xff, 20},//delay mark(20ms)
	{0xf8, 0x30, 0xb2},
	{0xf8, 0x33, 0xc2},
	{0xf8, 0x31, 0xf0},
	{0xf8, 0x40, 0x80},
	{0xf8, 0x81, 0xec},
	{0xff, 0xff, 0xff},//end mark
};

static int aml_lcd_i2c_write(struct i2c_client *i2client,unsigned char *buff, unsigned len)
{
    int res = 0;
    struct i2c_msg msg[] = {
        {
        .addr = i2client->addr,
        .flags = 0,
        .len = len,
        .buf = buff,
        }
    };
	
    res = i2c_transfer(i2client->adapter, msg, 1);
    if (res < 0) {
        printk("%s: i2c transfer failed [addr 0x%02x]\n", __FUNCTION__, i2client->addr);
    }
    
    return res;
}

static int aml_lcd_i2c_read(struct i2c_client *i2client,unsigned char *buff, unsigned len)
{
    int res = 0;
    struct i2c_msg msgs[] = {
        {
            .addr = i2client->addr,
            .flags = 0,
            .len = 1,
            .buf = buff,
        },
        {
            .addr = i2client->addr,
            .flags = I2C_M_RD,
            .len = len,
            .buf = buff,
        }
    };
    res = i2c_transfer(i2client->adapter, msgs, 2);
    if (res < 0) {
        printk("%s: i2c transfer failed [addr 0x%02x]\n", __FUNCTION__, i2client->addr);
    }

    return res;
}

static int tc101_reg_read(unsigned char reg, unsigned char *buf)
{
	int ret=0;

	return ret;
}

static int tc101_reg_write(unsigned char reg, unsigned char value)
{
	int ret=0;
	
	return ret;
}

static int tc101_init(void)
{
	unsigned char tData[4];
	int i=0, end_mark=0;
	int ret=0;
	
	while (end_mark == 0) {
		if ((tc101_init_table[i][0] == 0xff) && (tc101_init_table[i][1] == 0xff)) {	//special mark
			if (tc101_init_table[i][2] == 0xff) {	//end mark
				end_mark = 1;
			}
			else {	//delay mark
				mdelay(tc101_init_table[i][2]);
			}
		}
		else {
			tData[0]=tc101_init_table[i][0];
			tData[1]=tc101_init_table[i][1];
			tData[2]=tc101_init_table[i][2];
			aml_lcd_i2c_write(aml_tc101_client, tData, 3);
		}
		i++;
	}
	printk("%s\n", __FUNCTION__);
	return ret;
}

static int tc101_remove(void)
{
	int ret=0;
	
	return ret;
}

static int aml_lcd_extern_driver_update(struct aml_lcd_extern_driver_t* lcd_ext)
{
	lcd_ext->name = LCD_EXTERN_NAME;
	lcd_ext->reg_read = tc101_reg_read;
	lcd_ext->reg_write = tc101_reg_write;
	lcd_ext->power_on = tc101_init;
	lcd_ext->power_off = tc101_remove;
	
	return 0;
}

static int aml_tc101_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk("%s: functionality check failed\n", __FUNCTION__);
		return -ENODEV;
	} else {
		aml_tc101_client = client;
		aml_lcd_extern_driver_update(&lcd_ext_driver);
	}

	printk("%s OK\n", __FUNCTION__);

	return 0;
}

static int aml_tc101_remove(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id aml_tc101_id[] = {
	{LCD_EXTERN_NAME, 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, aml_tc101_id);

static struct i2c_driver aml_tc101_driver = {
	.probe    = aml_tc101_probe,
	.remove   = aml_tc101_remove,
	.id_table = aml_tc101_id,
	.driver = {
		.name = LCD_EXTERN_NAME,
		.owner =THIS_MODULE,
	},
};

static int __init aml_tc101_init(void)
{
	int ret = 0;
	//printk("%s\n", __FUNCTION__);

	ret = i2c_add_driver(&aml_tc101_driver);
	if (ret) {
		printk("failed to register aml_tc101_driver\n");
		return -ENODEV;
	}

	return ret;
}

static void __exit aml_tc101_exit(void)
{
	i2c_del_driver(&aml_tc101_driver);
}

//late_initcall(aml_tc101_init);
module_init(aml_tc101_init);
module_exit(aml_tc101_exit);

MODULE_AUTHOR("AMLOGIC");
MODULE_DESCRIPTION("LCD Extern driver for TC101");
MODULE_LICENSE("GPL");
