/*
 * amlogic_thermal.c - Samsung amlogic thermal (Thermal Management Unit)
 *
 *  Copyright (C) 2011 Samsung Electronics
 *  Donggeun Kim <dg77.kim@samsung.com>
 *  Amit Daniel Kachhap <amit.kachhap@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <linux/module.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/workqueue.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/thermal.h>
#include <linux/cpufreq.h>
#include <linux/cpu_cooling.h>
#include <linux/of.h>
#include <linux/amlogic/saradc.h>
#include <plat/cpu.h>
#include <linux/random.h>
struct freq_trip_table {
	unsigned int freq_state;
};
struct temp_trip{
	unsigned int temperature;
	unsigned int upper_freq;
	unsigned int lower_freq;
	int upper_level;
	int lower_level;
	
};
#define TMP_TRIP_COUNT 4
struct amlogic_thermal_platform_data {
	char *name;
	struct temp_trip tmp_trip[TMP_TRIP_COUNT];
	unsigned int temp_trip_count;
	unsigned int critical_temp;
	unsigned int idle_interval;
	struct thermal_zone_device *therm_dev;
	struct thermal_cooling_device *cool_dev;
	enum thermal_device_mode mode;
	struct mutex lock;
};
struct temp_level{
	unsigned int temperature;
	int high_freq;
	int low_freq;
};

/* CPU Zone information */
#define PANIC_ZONE      4
#define WARN_ZONE       3
#define MONITOR_ZONE    2
#define SAFE_ZONE       1

#define GET_ZONE(trip) (trip + 2)
#define GET_TRIP(zone) (zone - 2)

static void amlogic_unregister_thermal(struct amlogic_thermal_platform_data *pdata);
static int amlogic_register_thermal(struct amlogic_thermal_platform_data *pdata);

/* Get mode callback functions for thermal zone */
static int amlogic_get_mode(struct thermal_zone_device *thermal,
			enum thermal_device_mode *mode)
{
	struct  amlogic_thermal_platform_data *pdata= thermal->devdata;
	
	if (pdata)
		*mode = pdata->mode;
	return 0;
}

/* Set mode callback functions for thermal zone */
static int amlogic_set_mode(struct thermal_zone_device *thermal,
			enum thermal_device_mode mode)
{
	struct  amlogic_thermal_platform_data *pdata= thermal->devdata;
	
	if(!pdata)
		return -EINVAL;
	
	//mutex_lock(&pdata->therm_dev->lock);
	
	if (mode == THERMAL_DEVICE_ENABLED)
		pdata->therm_dev->polling_delay = pdata->idle_interval;
	else
		pdata->therm_dev->polling_delay = 0;

	//mutex_unlock(&pdata->therm_dev->lock);

	pdata->mode = mode;
	thermal_zone_device_update(pdata->therm_dev);
	pr_info("thermal polling set for duration=%d msec\n",
				pdata->therm_dev->polling_delay);
	return 0;
}


/* Get trip type callback functions for thermal zone */
static int amlogic_get_trip_type(struct thermal_zone_device *thermal, int trip,
				 enum thermal_trip_type *type)
{
	switch (GET_ZONE(trip)) {
	case MONITOR_ZONE:
	case WARN_ZONE:
		*type = THERMAL_TRIP_ACTIVE;
		break;
	case PANIC_ZONE:
		*type = THERMAL_TRIP_CRITICAL;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/* Get trip temperature callback functions for thermal zone */
static int amlogic_get_trip_temp(struct thermal_zone_device *thermal, int trip,
				unsigned long *temp)
{
	struct  amlogic_thermal_platform_data *pdata= thermal->devdata;
	
	if(trip > pdata->temp_trip_count ||trip<0)
		return  -EINVAL;
	mutex_lock(&pdata->lock);
	*temp =pdata->tmp_trip[trip].temperature;
	/* convert the temperature into millicelsius */
	mutex_unlock(&pdata->lock);

	return 0;
}

static int amlogic_set_trip_temp(struct thermal_zone_device *thermal, int trip,
				unsigned long temp)
{
	struct  amlogic_thermal_platform_data *pdata= thermal->devdata;
	
	if(trip > pdata->temp_trip_count ||trip<0)
		return  -EINVAL;
	mutex_lock(&pdata->lock);
	pdata->tmp_trip[trip].temperature=temp;
	/* convert the temperature into millicelsius */
	mutex_unlock(&pdata->lock);
	return 0;
}

/* Get critical temperature callback functions for thermal zone */
static int amlogic_get_crit_temp(struct thermal_zone_device *thermal,
				unsigned long *temp)
{
	int ret;
	/* Panic zone */
	ret =amlogic_get_trip_temp(thermal, GET_TRIP(PANIC_ZONE), temp);
	
	return ret;
}


/* Bind callback functions for thermal zone */
static int amlogic_bind(struct thermal_zone_device *thermal,
			struct thermal_cooling_device *cdev)
{
	int ret = 0, i;
	struct  amlogic_thermal_platform_data *pdata= thermal->devdata;
	if(cdev !=pdata->cool_dev){
		ret = -EINVAL;
		goto out;
	}
	/* Bind the thermal zone to the cpufreq cooling device */
	for (i = 0; i < pdata->temp_trip_count; i++) {
		if (thermal_zone_bind_cooling_device(thermal, i, cdev,
							pdata->tmp_trip[i].upper_level,
							pdata->tmp_trip[i].lower_level)) {
			pr_err("error binding cdev inst %d\n", i);
			ret = -EINVAL;
			goto out;
		}
	}
	pr_info("%s bind %s okay !\n",thermal->type,cdev->type);
	return ret;
out:
	return ret;
}

/* Unbind callback functions for thermal zone */
static int amlogic_unbind(struct thermal_zone_device *thermal,
			struct thermal_cooling_device *cdev)
{
	int i;
	if(thermal && cdev){
		struct  amlogic_thermal_platform_data *pdata= thermal->devdata;
		for (i = 0; i < pdata->temp_trip_count; i++) {
			if (thermal_zone_unbind_cooling_device(thermal, i, cdev)) {
				pr_err("error binding cdev inst %d\n", i);
				return -EINVAL;
			}
			return 0;
		}
	}else{
		return -EINVAL;
	}
	return -EINVAL;
}
/* Get temperature callback functions for thermal zone */
int aa=50;
int trend=1;
static int amlogic_get_temp(struct thermal_zone_device *thermal,
			unsigned long *temp)
{
#if 0
	if(aa>=100)
		trend=1;
	else if (aa<=40)
		trend=0;
	
	if(trend)
		aa=aa-5;
	else
		aa=aa+5;
	//get_random_bytes(&aa,4);
	printk("========  temp=%d\n",aa);
	*temp=aa;
#else
	*temp = get_cpu_temp();
	printk("========  temp=%d\n",*temp);
#endif
	return 0;
}

/* Get the temperature trend */
static int amlogic_get_trend(struct thermal_zone_device *thermal,
			int trip, enum thermal_trend *trend)
{
	return 1;
}
/* Operation callback functions for thermal zone */
static struct thermal_zone_device_ops const amlogic_dev_ops = {
	.bind = amlogic_bind,
	.unbind = amlogic_unbind,
	.get_temp = amlogic_get_temp,
	.get_trend = amlogic_get_trend,
	.get_mode = amlogic_get_mode,
	.set_mode = amlogic_set_mode,
	.get_trip_type = amlogic_get_trip_type,
	.get_trip_temp = amlogic_get_trip_temp,
	.set_trip_temp = amlogic_set_trip_temp,
	.get_crit_temp = amlogic_get_crit_temp,
};



/* Register with the in-kernel thermal management */
static int amlogic_register_thermal(struct amlogic_thermal_platform_data *pdata)
{
	int ret=0;
	struct cpumask mask_val;

	cpumask_set_cpu(0, &mask_val);
	pdata->cool_dev= cpufreq_cooling_register(&mask_val);
	if (IS_ERR(pdata->cool_dev)) {
		pr_err("Failed to register cpufreq cooling device\n");
		ret = -EINVAL;
		goto err_unregister;
	}

	pdata->therm_dev = thermal_zone_device_register(pdata->name,
			pdata->temp_trip_count, 7, pdata, &amlogic_dev_ops, NULL, 0,
			pdata->idle_interval);

	if (IS_ERR(pdata->therm_dev)) {
		pr_err("Failed to register thermal zone device\n");
		ret = -EINVAL;
		goto err_unregister;
	}

	pr_info("amlogic: Kernel Thermal management registered\n");

	return 0;

err_unregister:
	amlogic_unregister_thermal(pdata);
	return ret;
}

/* Un-Register with the in-kernel thermal management */
static void amlogic_unregister_thermal(struct amlogic_thermal_platform_data *pdata)
{
	if (pdata->therm_dev)
		thermal_zone_device_unregister(pdata->therm_dev);
	if (pdata->cool_dev)
		cpufreq_cooling_unregister(pdata->cool_dev);

	pr_info("amlogic: Kernel Thermal management unregistered\n");
}

struct amlogic_thermal_platform_data Pdata={
	.name="amlogic, theraml",
	.tmp_trip[0]={
		.temperature=50,
		.upper_freq=1296000,
		.lower_freq=912000,
	},
	.tmp_trip[1]={
		.temperature=100,
		.upper_freq=816000,
		.lower_freq=312000,
	},
	.tmp_trip[2]={
		.temperature=110,
	},
	.temp_trip_count=3,
	.idle_interval=1000,
	.therm_dev=NULL,
	.cool_dev=NULL,
};

static  struct  amlogic_thermal_platform_data *amlogic_get_driver_data(
			struct platform_device *pdev)
{
	struct amlogic_thermal_platform_data *pdata=&Pdata;
	return pdata;
}
int get_desend(void)
{
	int i;
	unsigned int freq = CPUFREQ_ENTRY_INVALID;
	int descend = -1;
	struct cpufreq_frequency_table *table =
					cpufreq_frequency_get_table(0);

	if (!table)
		return -EINVAL;

	for (i = 0; table[i].frequency != CPUFREQ_TABLE_END; i++) {
		/* ignore invalid entries */
		if (table[i].frequency == CPUFREQ_ENTRY_INVALID)
			continue;

		/* ignore duplicate entry */
		if (freq == table[i].frequency)
			continue;

		/* get the frequency order */
		if (freq != CPUFREQ_ENTRY_INVALID && descend == -1){
			descend = !!(freq > table[i].frequency);
			break;
		}

		freq = table[i].frequency;
	}
	return descend;
}
int fix_to_freq(int freqold,int descend)
{
	int i;
	unsigned int freq = CPUFREQ_ENTRY_INVALID;
	struct cpufreq_frequency_table *table =
					cpufreq_frequency_get_table(0);

	if (!table)
		return -EINVAL;

	for (i = 0; table[i].frequency != CPUFREQ_TABLE_END; i++) {
		/* ignore invalid entry */
		if (table[i].frequency == CPUFREQ_ENTRY_INVALID)
			continue;

		/* ignore duplicate entry */
		if (freq == table[i].frequency)
			continue;
		freq = table[i].frequency;
		if(descend){
			if(freqold>=table[i+1].frequency && freqold<=table[i].frequency)
				return table[i+1].frequency;
		}
		else{
			if(freqold>=table[i].frequency && freqold<=table[i+1].frequency)
				return table[i].frequency;
		}
	}
	return -EINVAL;
}
static struct amlogic_thermal_platform_data * amlogic_thermal_init_from_dts(struct platform_device *pdev)
{
	int i=0,ret=-1,val=0,cells,descend;
	struct property *prop;
	struct temp_level *tmp_level=NULL;
	struct amlogic_thermal_platform_data *pdata=NULL;
	if(!of_property_read_u32(pdev->dev.of_node, "trip_point", &val)){
		//INIT FROM DTS
		pdata=kzalloc(sizeof(*pdata),GFP_KERNEL);
		if(!pdata){
			goto err;
		}
		memset((void* )pdata,0,sizeof(*pdata));
		ret=of_property_read_u32(pdev->dev.of_node, "#thermal-cells", &val);
		if(ret){
			dev_err(&pdev->dev, "dt probe #thermal-cells failed: %d\n", ret);
			goto err;
		}
		printk("#thermal-cells=%d\n",val);
		cells=val;
		prop = of_find_property(pdev->dev.of_node, "trip_point", &val);
		if (!prop){
			dev_err(&pdev->dev, "read %s length error\n","trip_point");
			goto err;
		}
		pdata->temp_trip_count=val/cells/sizeof(u32);
		printk("pdata->temp_trip_count=%d\n",pdata->temp_trip_count);
		tmp_level=kzalloc(sizeof(*tmp_level)*pdata->temp_trip_count,GFP_KERNEL);
		if(!tmp_level){
			goto err;
		}
		ret=of_property_read_u32_array(pdev->dev.of_node,"trip_point",(u32 *)tmp_level,val/sizeof(u32));
		if (ret){
			dev_err(&pdev->dev, "read %s data error\n","trip_point");
			goto err;
		}
		descend=get_desend();
		for (i = 0; i < pdata->temp_trip_count; i++) {
			printk("temperature=%d on trip point=%d\n",tmp_level[i].temperature,i);
			pdata->tmp_trip[i].temperature=tmp_level[i].temperature;
			printk("fixing high_freq=%d to ",tmp_level[i].high_freq);
			tmp_level[i].high_freq=fix_to_freq(tmp_level[i].high_freq,descend);
			pdata->tmp_trip[i].lower_level=cpufreq_cooling_get_level(0,tmp_level[i].high_freq);
			printk("%d at trip point %d,level=%d\n",tmp_level[i].high_freq,i,pdata->tmp_trip[i].lower_level);	
			
			printk("fixing low_freq=%d to ",tmp_level[i].low_freq);
			tmp_level[i].low_freq=fix_to_freq(tmp_level[i].low_freq,descend);
			pdata->tmp_trip[i].upper_level=cpufreq_cooling_get_level(0,tmp_level[i].low_freq);
			printk("%d at trip point %d,level=%d\n",tmp_level[i].low_freq,i,pdata->tmp_trip[i].upper_level);	
		}
		
		ret= of_property_read_u32(pdev->dev.of_node, "idle_interval", &val);
		if (ret){
			dev_err(&pdev->dev, "read %s  error\n","idle_interval");
			goto err;
		}
		pdata->idle_interval=val;
		printk("idle interval=%d\n",pdata->idle_interval);
		ret=of_property_read_string(pdev->dev.of_node,"dev_name",&pdata->name);
		if (ret){
			dev_err(&pdev->dev, "read %s  error\n","dev_name");
			goto err;
		}
		printk("pdata->name:%s\n",pdata->name);
		if(tmp_level)
			kfree(tmp_level);
		return pdata;
	}	
err:
	if(tmp_level)
		kfree(tmp_level);
	if(pdata)
		kfree(pdata);
	pdata= NULL;
	return pdata;
}
static struct amlogic_thermal_platform_data * amlogic_thermal_initialize(struct platform_device *pdev)
{
	int i=0;
	struct amlogic_thermal_platform_data *pdata=NULL;
	pdata=amlogic_thermal_init_from_dts(pdev);
	if(!pdata){
		pdata=amlogic_get_driver_data(pdev);
		// Get level
		for (i = 0; i < pdata->temp_trip_count; i++) {
			pdata->tmp_trip[i].upper_level=cpufreq_cooling_get_level(0,pdata->tmp_trip[i].lower_freq);
			pdata->tmp_trip[i].lower_level=cpufreq_cooling_get_level(0,pdata->tmp_trip[i].upper_freq);
			printk("pdata->tmp_trip[%d].upper_level=%d\n",i,pdata->tmp_trip[i].upper_level);
			printk("pdata->tmp_trip[%d].lower_level=%d\n",i,pdata->tmp_trip[i].lower_level);
		}
	}
	
	return pdata;
}

static const struct of_device_id amlogic_thermal_match[] = {
	{
		.compatible = "amlogic-thermal",
	},
};
static int amlogic_thermal_probe(struct platform_device *pdev)
{
	int ret;
	struct amlogic_thermal_platform_data *pdata=NULL;
	//pdata = amlogic_get_driver_data(pdev);
	ret=get_cpu_temp();
	if(NOT_WRITE_EFUSE==ret){
		printk("cpu sensor not ready!!!!!!\n");
		return -1;
	}
	dev_info(&pdev->dev, "amlogic thermal probe start\n");
	pdata = amlogic_thermal_initialize(pdev);
	if (!pdata) {
		dev_err(&pdev->dev, "Failed to initialize thermal\n");
		goto err;
	}
	mutex_init(&pdata->lock);
	pdev->dev.platform_data=pdata;
	platform_set_drvdata(pdev, pdata);
	ret = amlogic_register_thermal(pdata);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register thermal interface\n");
		goto err;
	}
	dev_info(&pdev->dev, "amlogic thermal probe done\n");
	return 0;
err:
	platform_set_drvdata(pdev, NULL);
	return ret;
}

static int amlogic_thermal_remove(struct platform_device *pdev)
{
	struct amlogic_thermal_platform_data *pdata = platform_get_drvdata(pdev);

	amlogic_unregister_thermal(pdata);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int amlogic_thermal_suspend(struct device *dev)
{
	return 0;
}

static int amlogic_thermal_resume(struct device *dev)
{
	return 0;
}

static SIMPLE_DEV_PM_OPS(amlogic_thermal_pm,
			 amlogic_thermal_suspend, amlogic_thermal_resume);
#define amlogic_thermal_PM	(&amlogic_thermal_pm)
#else
#define amlogic_thermal_PM	NULL
#endif

static struct platform_driver amlogic_thermal_driver = {
	.driver = {
		.name   = "amlogic-thermal",
		.owner  = THIS_MODULE,
		.pm     = amlogic_thermal_PM,
		.of_match_table = of_match_ptr(amlogic_thermal_match),
	},
	.probe = amlogic_thermal_probe,
	.remove	= amlogic_thermal_remove,
};
static int __init amlogic_thermal_driver_init(void) 
{ 
	return platform_driver_register(&(amlogic_thermal_driver)); 
} 
late_initcall(amlogic_thermal_driver_init); 
static void __exit amlogic_thermal_driver_exit(void) 
{ 
	platform_driver_unregister(&(amlogic_thermal_driver) ); 
} 
module_exit(amlogic_thermal_driver_exit);

MODULE_DESCRIPTION("amlogic thermal Driver");
MODULE_AUTHOR("Amlogic SH platform team");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:amlogic-thermal");

