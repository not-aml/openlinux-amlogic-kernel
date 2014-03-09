/*
 *
 * arch/arm/plat-meson/cpu_freq.c
 *
 *  Copyright (C) 2010 AMLOGIC, INC.
 *
 * License terms: GNU General Public License (GPL) version 2
 * CPU frequence management.
 *
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/input.h>

//#include <mach/hardware.h>
#include <mach/clock.h>
#include <plat/cpufreq.h>
#include <asm/system.h>
#include <asm/smp_plat.h>
#include <asm/cpu.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/machine.h>
#include "voltage.h"


struct meson_cpufreq {
    struct device *dev;
    struct clk *armclk;
};

struct regulator_consumer_supply vcck_data;
static struct regulator *vcck;
static struct meson_opp *vcck_opp_table;
static int size_vcck_table;

static struct meson_cpufreq cpufreq;

static DEFINE_MUTEX(meson_cpufreq_mutex);

static void adjust_jiffies(unsigned int freqOld, unsigned int freqNew);

static struct cpufreq_frequency_table meson_freq_table[]=
{
//	0	, CPUFREQ_ENTRY_INVALID    , 
//	1	, CPUFREQ_ENTRY_INVALID    , 
	{0	, 96000    },
	{1	, 192000   },
	{2	, 312000   },
	{3	, 408000   },
	{4	, 504000   },
	{5	, 600000   },
	{6	, 696000   },
	{7	, 816000   },
	{8	, 912000   },
	{9	, 1008000  },
	{10	, 1104000  },
	{11	, 1200000  },
	{12	, 1296000  },
	{13	, 1416000  },
	{14	, 1512000  },
	{15	, CPUFREQ_TABLE_END},
};

//static struct cpufreq_frequency_table *p_meson_freq_table;


static int meson_cpufreq_verify(struct cpufreq_policy *policy)
{
    struct meson_cpufreq_config *pdata = cpufreq.dev->platform_data;

    if (pdata && pdata->freq_table)
        return cpufreq_frequency_table_verify(policy, pdata->freq_table);

    if (policy->cpu)
        return -EINVAL;

    cpufreq_verify_within_limits(policy, policy->cpuinfo.min_freq,
                                 policy->cpuinfo.max_freq);

    policy->min = clk_round_rate(cpufreq.armclk, policy->min * 1000) / 1000;
    policy->max = clk_round_rate(cpufreq.armclk, policy->max * 1000) / 1000;
    cpufreq_verify_within_limits(policy, policy->cpuinfo.min_freq,
                                 policy->cpuinfo.max_freq);
    return 0;
}

static int early_suspend_flag = 0;
#if (defined CONFIG_SMP) && (defined CONFIG_HAS_EARLYSUSPEND)
#include <linux/earlysuspend.h>
static void meson_system_early_suspend(struct early_suspend *h)
{
    early_suspend_flag=1;
}

static void meson_system_late_resume(struct early_suspend *h)
{
    early_suspend_flag=0;

}
static struct early_suspend early_suspend={
        .level = EARLY_SUSPEND_LEVEL_DISABLE_FB,
            .suspend = meson_system_early_suspend,
            .resume = meson_system_late_resume,

};

#endif
static int meson_cpufreq_target_locked(struct cpufreq_policy *policy,
                                       unsigned int target_freq,
                                       unsigned int relation)
{
    struct cpufreq_freqs freqs;
    struct meson_cpufreq_config *pdata = cpufreq.dev->platform_data;
    uint cpu = policy ? policy->cpu : 0;
    int ret = -EINVAL;
    unsigned int freqInt = 0;
	if (cpu > (NR_CPUS - 1)) {
        printk(KERN_ERR"cpu %d set target freq error\n",cpu);
        return ret;
    }
	
#if (defined CONFIG_SMP) && (defined CONFIG_HAS_EARLYSUSPEND)
//    if(early_suspend_flag)
//    {
//        printk("suspend in progress target_freq=%d\n",target_freq);
//        return -EINVAL;
//    }
#endif
    /* Ensure desired rate is within allowed range.  Some govenors
     * (ondemand) will just pass target_freq=0 to get the minimum. */
    if (policy) {
        if (target_freq < policy->min) {
            target_freq = policy->min;
        }
        if (target_freq > policy->max) {
            target_freq = policy->max;
        }
    }

	

    freqs.old = clk_get_rate(cpufreq.armclk) / 1000;
    freqs.new = clk_round_rate(cpufreq.armclk, target_freq * 1000) / 1000;
    freqs.cpu = cpu;

    if (freqs.old == freqs.new) {
        return ret;
    }
	

    cpufreq_notify_transition(policy, &freqs, CPUFREQ_PRECHANGE);

#ifndef CONFIG_CPU_FREQ_DEBUG
    pr_debug("cpufreq-meson: CPU%d transition: %u --> %u\n",
           freqs.cpu, freqs.old, freqs.new);
#endif
	

    /* if moving to higher frequency, move to an intermediate frequency
     * that does not require a voltage change first.
     */
#if 0
    if (pdata && pdata->cur_volt_max_freq && freqs.new > freqs.old) {
        freqInt = pdata->cur_volt_max_freq();
        if (freqInt > freqs.old && freqInt <= freqs.new) {
            adjust_jiffies(freqs.old, freqInt);
            ret = clk_set_rate(cpufreq.armclk, freqInt * 1000);
            if (ret || freqInt == freqs.new)
                goto out;
        } else {
            freqInt = 0;
        }
    }
#endif

    /* if moving to higher frequency, up the voltage beforehand */
    if (pdata && pdata->voltage_scale && freqs.new > freqs.old) {
		
        ret = pdata->voltage_scale(freqs.new);
        if (ret)
            goto out;
    }
	

    if (freqs.new > freqs.old)
        adjust_jiffies(freqInt != 0 ? freqInt : freqs.old, freqs.new);
	
    ret = clk_set_rate(cpufreq.armclk, freqs.new * 1000);
    if (ret)
        goto out;
	

    freqs.new = clk_get_rate(cpufreq.armclk) / 1000;
    if (freqs.new < freqs.old)
        adjust_jiffies(freqs.old, freqs.new);
	

    /* if moving to lower freq, lower the voltage after lowering freq
     * This should be done after CPUFREQ_PRECHANGE, which will adjust lpj and
     * affect our udelays.
     */
    
    if (pdata && pdata->voltage_scale && freqs.new < freqs.old) {
        ret = pdata->voltage_scale(freqs.new);
    }

out:
    freqs.new = clk_get_rate(cpufreq.armclk) / 1000;
    if (ret) {
        adjust_jiffies(freqInt != 0 ? freqInt : freqs.old, freqs.new);
    }
    cpufreq_notify_transition(policy, &freqs, CPUFREQ_POSTCHANGE);

    return ret;
}

static int meson_cpufreq_target(struct cpufreq_policy *policy,
                                unsigned int target_freq,
                                unsigned int relation)
{
    int ret;

    mutex_lock(&meson_cpufreq_mutex);
    ret = meson_cpufreq_target_locked(policy, target_freq, relation);
    mutex_unlock(&meson_cpufreq_mutex);

    return ret;
}

unsigned int meson_cpufreq_get(unsigned int cpu)
{
    unsigned long rate;
    if(cpu > (NR_CPUS-1))
    {
        printk(KERN_ERR "cpu %d on current thread error\n",cpu);
        return 0;
    }
    rate = clk_get_rate(cpufreq.armclk) / 1000;
    return rate;
}

static int meson_cpufreq_init(struct cpufreq_policy *policy)
{
    struct meson_cpufreq_config *pdata = cpufreq.dev->platform_data;
    int result = 0;

    if (policy->cpu != 0)
        return -EINVAL;

    if (policy->cpu > (NR_CPUS - 1)) {
        printk(KERN_ERR "cpu %d on current thread error\n", policy->cpu);
        return -1;
    }

    /* Finish platform specific initialization */
   if (pdata) {
        if (pdata->init) {
            result = pdata->init();
            if (result)
                return result;
        }
        if(!pdata->freq_table)//If not special freq_table in bsp, use default.
            pdata->freq_table = meson_freq_table;


//	result = cpufreq_frequency_table_cpuinfo(policy, meson_freq_table);
//	if (result)
//		goto fail_table;

//	p_meson_freq_table = meson_freq_table;
	cpufreq_frequency_table_get_attr(pdata->freq_table,
                        policy->cpu);
//	cpufreq_frequency_table_get_attr(p_meson_freq_table,
//                        policy->cpu);
    }
    policy->min = policy->cpuinfo.min_freq = clk_round_rate(cpufreq.armclk, 0) / 1000;
    policy->max = policy->cpuinfo.max_freq = clk_round_rate(cpufreq.armclk, 0xffffffff) / 1000;
    policy->cur = clk_get_rate(cpufreq.armclk) / 1000;

    /* FIXME: what's the actual transition time? */
    policy->cpuinfo.transition_latency = 200 * 1000;

	if (is_smp()) {
	 /* Both cores must be set to same frequency.  Set affected_cpus to all. */
//		policy->shared_type = CPUFREQ_SHARED_TYPE_ANY;
		cpumask_setall(policy->cpus);
	}

    return 0;
}

static struct freq_attr *meson_cpufreq_attr[] = {
    &cpufreq_freq_attr_scaling_available_freqs,
    NULL,
};

static unsigned sleep_freq;
static int meson_cpufreq_suspend(struct cpufreq_policy *policy)
{
    struct meson_cpufreq_config *pdata = cpufreq.dev->platform_data;
    /* Ok, this could be made a bit smarter, but let's be robust for now. We
     * always force a speed change to high speed before sleep, to make sure
     * we have appropriate voltage and/or bus speed for the wakeup process,
     */

    mutex_lock(&meson_cpufreq_mutex);

    sleep_freq = clk_get_rate(cpufreq.armclk) / 1000;
    printk("cpufreq suspend sleep_freq=%dMhz max=%dMHz\n", sleep_freq/1000, policy->max/1000);

    if (policy->max > sleep_freq) {
        if (pdata && pdata->voltage_scale) {
            int ret = pdata->voltage_scale(policy->max);
            if (ret) {
                pr_err("failed to set voltage %d\n", ret);
                mutex_unlock(&meson_cpufreq_mutex);
                return 0;
            }
        }
        adjust_jiffies(sleep_freq, policy->max);
    }
    clk_set_rate(cpufreq.armclk, policy->max * 1000);

    mutex_unlock(&meson_cpufreq_mutex);
    return 0;
}

static int meson_cpufreq_resume(struct cpufreq_policy *policy)
{
    unsigned cur;
    struct meson_cpufreq_config *pdata = cpufreq.dev->platform_data;
    printk("cpufreq resume sleep_freq=%dMhz\n", sleep_freq/1000);

    mutex_lock(&meson_cpufreq_mutex);

    clk_set_rate(cpufreq.armclk, sleep_freq * 1000);
    cur = clk_get_rate(cpufreq.armclk) / 1000;
    if (policy->max > cur) {
        adjust_jiffies(policy->max, cur);
        if (pdata && pdata->voltage_scale) {
            int ret = pdata->voltage_scale(cur);
            if (ret) {
                pr_err("failed to set voltage %d\n", ret);
                mutex_unlock(&meson_cpufreq_mutex);
                return 0;
            }
        }
    }
    mutex_unlock(&meson_cpufreq_mutex);
    return 0;
}

static struct cpufreq_driver meson_cpufreq_driver = {
    .flags      = CPUFREQ_STICKY,
    .verify     = meson_cpufreq_verify,
    .target     = meson_cpufreq_target,
    .get        = meson_cpufreq_get,
    .init       = meson_cpufreq_init,
    .name       = "meson_cpufreq",
    .attr       = meson_cpufreq_attr,
    .suspend    = meson_cpufreq_suspend,
    .resume     = meson_cpufreq_resume
};

#ifdef CONFIG_USE_OF
static unsigned int vcck_cur_max_freq(void)
{
    return meson_vcck_cur_max_freq(vcck, vcck_opp_table, size_vcck_table);
}

static int vcck_scale(unsigned int frequency)
{
    return meson_vcck_scale(vcck, vcck_opp_table, size_vcck_table,
                            frequency);
}

static int vcck_regulator_init(void)
{
	vcck = regulator_get(NULL, vcck_data.supply);
	if (WARN(IS_ERR(vcck), "Unable to obtain voltage regulator for vcck;"
					" voltage scaling unsupported\n")) {
		return PTR_ERR(vcck);
	}

	return 0;
}
#endif

static int __init meson_cpufreq_probe(struct platform_device *pdev)
{
	#ifdef CONFIG_USE_OF
			struct meson_cpufreq_config *cpufreq_info;
			int ret,val=0;
			struct device_node *vcck_table_np,*cs_regulator_np,*vcck_init_np;
			phandle phandle;
	
			if (pdev->dev.of_node) {
				ret = of_property_read_u32(pdev->dev.of_node,"cpufreq_info",&val);
				if(ret){
					printk("don't find	match init-data\n");
					goto reg_driver__;
				}
				if(ret==0){
					phandle=val;
					vcck_table_np = of_find_node_by_phandle(phandle);
	
					if(!vcck_table_np){
						printk("%s:%d,can't find device node\n",__func__,__LINE__);
						return -1;
					}
	
					ret = of_property_read_u32(vcck_table_np,"num",&size_vcck_table);
					if(ret){
						printk("don't find	match num\n");
						return -1;
					}
	
					vcck_opp_table = kzalloc(sizeof(struct meson_opp)*size_vcck_table, GFP_KERNEL);
					if(!vcck_opp_table)
					{
						printk("vcck_opp_table can not get mem\n");
						return -1;
					}
	
					ret = of_property_read_u32_array(vcck_table_np,"table",vcck_opp_table, size_vcck_table*sizeof(struct meson_opp)/sizeof(vcck_opp_table));
					if(ret){
						printk("don't find	match table\n");
						goto err;
					}
				}
	
				cs_regulator_np = of_find_node_by_name(NULL,"meson-cs-regulator");
				if(!cs_regulator_np)
				{
					printk("don't find	match meson-cs-regulator node\n");
					goto err;
				}
	
				ret = of_property_read_u32(cs_regulator_np,"init-data",&val);
				if(ret){
					printk("don't find	match init-data \n");
					goto err;
				}
	
				if(ret==0){
					phandle=val;
					vcck_init_np = of_find_node_by_phandle(phandle);
					if(!vcck_init_np){
						printk("%s:%d,can't find device node\n",__func__,__LINE__);
						goto err;
					}
	
					ret = of_property_read_string(vcck_init_np,"vcck_data-supply",&vcck_data.supply);
					if(ret){
						printk("don't find	match table\n");
						goto err;
					}
				}
	
				cpufreq_info = kzalloc(sizeof(struct meson_cpufreq_config), GFP_KERNEL);
				if(!cpufreq_info)
				{
					printk("cpufreq_info can not get mem\n");
					kfree(vcck_opp_table);
					return -1;
				}
	
				cpufreq_info->freq_table = NULL;
				cpufreq_info->init = vcck_regulator_init;
				cpufreq_info->cur_volt_max_freq = vcck_cur_max_freq;
				cpufreq_info->voltage_scale = vcck_scale;
				pdev->dev.platform_data = cpufreq_info;
			}
	#endif

reg_driver__:
    cpufreq.dev = &pdev->dev;
    cpufreq.armclk = clk_get_sys("a9_clk", NULL);
    if (IS_ERR(cpufreq.armclk)) {
        dev_err(cpufreq.dev, "Unable to get ARM clock\n");
        return PTR_ERR(cpufreq.armclk);
    }

   return cpufreq_register_driver(&meson_cpufreq_driver);
   err:
		kfree(vcck_opp_table);
		return -1;
}


static int __exit meson_cpufreq_remove(struct platform_device *pdev)
{
#ifdef CONFIGG_USE_OF
	kfree(pdev->dev->platform_data);
	kfree(vcck_opp_table);
#endif

    return cpufreq_unregister_driver(&meson_cpufreq_driver);
}

#ifdef CONFIG_OF
static const struct of_device_id amlogic_cpufreq_meson_dt_match[]={
	{	.compatible = "amlogic,cpufreq-meson",
	},
	{},
};
#else
#define amlogic_cpufreq_meson_dt_match NULL
#endif


static struct platform_driver meson_cpufreq_parent_driver = {
    .driver = {
        .name   = "cpufreq-meson",
        .owner  = THIS_MODULE,
        .of_match_table = amlogic_cpufreq_meson_dt_match,
    },
    .remove = meson_cpufreq_remove,
};

static int __init meson_cpufreq_parent_init(void)
{
#if (defined CONFIG_SMP) && (defined CONFIG_HAS_EARLYSUSPEND)

//    early_suspend.param = pdev;
    register_early_suspend(&early_suspend);
#endif
//	cpufreq.dev = get_cpu_device(0);
//	cpufreq.dev->platform_data = NULL;


    return platform_driver_probe(&meson_cpufreq_parent_driver,
                                 meson_cpufreq_probe);

//	return meson_cpufreq_probe((struct platform_device *)&cpufreq);
}
late_initcall(meson_cpufreq_parent_init);


/* assumes all CPUs run at same frequency */
static unsigned long global_l_p_j_ref;
static unsigned long global_l_p_j_ref_freq;

static void adjust_jiffies(unsigned int freqOld, unsigned int freqNew)
{
    int i;

    if (!global_l_p_j_ref) {
        global_l_p_j_ref = loops_per_jiffy;
        global_l_p_j_ref_freq = freqOld;
    }

    loops_per_jiffy = cpufreq_scale(global_l_p_j_ref,
                                    global_l_p_j_ref_freq,
                                    freqNew);
#ifdef	CONFIG_SMP
    for_each_present_cpu(i) {
        per_cpu(cpu_data, i).loops_per_jiffy = loops_per_jiffy;
    }
#endif
}

int meson_cpufreq_boost(unsigned int freq)
{
    int ret = 0;
    if (!early_suspend_flag) {
        // only allow freq boost when not in early suspend
        //check last_cpu_rate. inaccurate but no lock
        //printk("%u %u\n", last_cpu_rate, freq);
        //if (last_cpu_rate < freq) {
            if ((clk_get_rate(cpufreq.armclk) / 1000) < freq) {
                mutex_lock(&meson_cpufreq_mutex);
                if ((clk_get_rate(cpufreq.armclk) / 1000) < freq) {
                    ret = meson_cpufreq_target_locked(NULL,
                                                      freq,
                                                      CPUFREQ_RELATION_H);
                }
                mutex_unlock(&meson_cpufreq_mutex);
            }
        //}
    }
    return ret;
}

