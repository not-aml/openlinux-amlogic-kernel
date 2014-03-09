/*****************************************************************
**
**  Copyright (C) 2009 Amlogic,Inc.
**  All rights reserved
**        Filename : atbmfrontend.c
**
**  comment:
**        Driver for ATBM8869 demodulator
**  author :
**	    Shijie.Rong@amlogic
**  version :
**	    v1.0	 12/3/13
*****************************************************************/

/*
    Driver for atbm8869 demodulator
*/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#ifdef ARC_700
#include <asm/arch/am_regs.h>
#else
#include <mach/am_regs.h>
#endif
#include <linux/i2c.h>
#include <linux/gpio.h>
#include "atbm886x.h"
#include "../aml_fe.h"

#include <mach/i2c_aml.h>
#if 1
#define pr_dbg(args...) printk("ATBM: " args)
#else
#define pr_dbg(args...)
#endif
static struct mutex atbm_lock;


#define pr_error(args...) printk("ATBM: " args)
struct i2c_adapter *i2c_adap_atbm;


static int atbm8869_read_status(struct dvb_frontend *fe, fe_status_t * status)
{
	struct aml_fe *afe = fe->demodulator_priv;
	struct aml_fe_dev *dev = afe->dtv_demod;
	unsigned char s=0;
	mutex_lock(&atbm_lock);
	s=ATBMLockedFlag();
	mutex_unlock(&atbm_lock);
//	printk("s is %d\n",s);
	if(s==1)
	{
		*status = FE_HAS_LOCK|FE_HAS_SIGNAL|FE_HAS_CARRIER|FE_HAS_VITERBI|FE_HAS_SYNC;
	}
	else
	{
		*status = FE_TIMEDOUT;
	}

	return  0;
}

static int atbm8869_read_ber(struct dvb_frontend *fe, u32 * ber)
{
	struct aml_fe *afe = fe->demodulator_priv;
	struct aml_fe_dev *dev = afe->dtv_demod;
	mutex_lock(&atbm_lock);
	*ber=ATBMFrameErrorRatio();
	mutex_unlock(&atbm_lock);
	return 0;
}

#if (defined CONFIG_AM_M6_DEMOD)
extern int tuner_get_ch_power(struct aml_fe_dev *adap);
#endif
static int atbm8869_read_signal_strength(struct dvb_frontend *fe, u16 *strength)
{
	struct aml_fe *afe = fe->demodulator_priv;
	struct aml_fe_dev *dev = afe->dtv_demod;
	mutex_lock(&atbm_lock);
#if (defined CONFIG_AM_M6_DEMOD)
	*strength=256-tuner_get_ch_power(dev);
#endif
	mutex_unlock(&atbm_lock);
	return 0;
}

static int atbm8869_read_snr(struct dvb_frontend *fe, u16 * snr)
{
	struct aml_fe *afe = fe->demodulator_priv;
	struct aml_fe_dev *dev = afe->dtv_demod;
	mutex_lock(&atbm_lock);
	*snr=ATBMSignalNoiseRatio();
	mutex_unlock(&atbm_lock);
	return 0;
}

static int atbm8869_read_ucblocks(struct dvb_frontend *fe, u32 * ucblocks)
{
	*ucblocks=0;
	return 0;
}
extern int aml_fe_analog_set_frontend(struct dvb_frontend* fe, struct dvb_frontend_parameters* params);
static int atbm8869_set_frontend(struct dvb_frontend *fe, struct dvb_frontend_parameters *p)
{
	struct aml_fe *afe = fe->demodulator_priv;
	struct aml_fe_dev *dev = afe->dtv_demod;
	int times,error;
	times=2;
	printk("atbm8869_set_frontend\n");
	//The following procedures are to set the tuner's center frequency to switch channel

//	ATBMHoldDSP();
//	Delayms(5);
retry:

	mutex_lock(&atbm_lock);
	aml_fe_analog_set_frontend(fe,p);   //666000Khz  set tuner
	mutex_unlock(&atbm_lock);
//	Delayms(50);
//	ATBMStartDSP();
	times--;
	if(ATBMLockedFlag() && times){
		int lock;

		aml_dmx_start_error_check(afe->ts, fe);
		msleep(20);
		error = aml_dmx_stop_error_check(afe->ts, fe);
		lock  = ATBMLockedFlag();
		if((error > 200) || !lock){
			pr_error("amlfe too many error, error count:%d lock statuc:%d, retry\n", error, lock);
			goto retry;
		}
	}


	aml_dmx_after_retune(afe->ts, fe);
	afe->params = *p;
	msleep(200);
	return  0;
}

static int atbm8869_get_frontend(struct dvb_frontend *fe, struct dvb_frontend_parameters *p)
{//these content will be writed into eeprom .

	struct aml_fe *afe = fe->demodulator_priv;

	*p = afe->params;
	return 0;
}

static int atbm8869_fe_get_ops(struct aml_fe_dev *dev, int mode, void *ops)
{
	struct dvb_frontend_ops *fe_ops = (struct dvb_frontend_ops*)ops;
	int ret;
	pr_dbg("=========================demod init\r\n");
	fe_ops->info.frequency_min = 51000000;
	fe_ops->info.frequency_max = 950000000;
	fe_ops->info.frequency_stepsize = 0;
	fe_ops->info.frequency_tolerance = 0;
	fe_ops->info.caps = FE_CAN_FEC_1_2 | FE_CAN_FEC_2_3 | FE_CAN_FEC_3_4 |
			FE_CAN_FEC_5_6 | FE_CAN_FEC_7_8 | FE_CAN_FEC_AUTO |
			FE_CAN_QPSK | FE_CAN_QAM_16 |
			FE_CAN_QAM_64 | FE_CAN_QAM_AUTO |
			FE_CAN_TRANSMISSION_MODE_AUTO |
			FE_CAN_GUARD_INTERVAL_AUTO |
			FE_CAN_HIERARCHY_AUTO |
			FE_CAN_RECOVER |
			FE_CAN_MUTE_TS;

	fe_ops->set_frontend = atbm8869_set_frontend;
	fe_ops->get_frontend = atbm8869_get_frontend;
	fe_ops->read_status = atbm8869_read_status;
	fe_ops->read_ber = atbm8869_read_ber;
	fe_ops->read_signal_strength = atbm8869_read_signal_strength;
	fe_ops->read_snr = atbm8869_read_snr;
	fe_ops->read_ucblocks = atbm8869_read_ucblocks;
	i2c_adap_atbm=dev->i2c_adap;
	return 0;
/*	ret=ATBMPowerOnInit();
	if(ret==-1){
		pr_dbg("=========================dtmb demod error\r\n");
		return -1;
	}
	ATBMSetDTMBMode();
	return 0;*/
}

static int atbm8869_fe_enter_mode(struct aml_fe *fe, int mode)
{
	struct aml_fe_dev *dev = fe->dtv_demod;
	struct m6tv_dtmb_platform_data *patbm_op = NULL;

	patbm_op = (struct m6tv_dtmb_platform_data*)fe->dtv_demod->frontend_opration;

	if(NULL != patbm_op)
	{
		if(NULL != patbm_op->atbm_device_reset)
		{
			patbm_op->atbm_device_reset();
		}
	}

	pr_dbg("=========================atbm8869_fe_enter_modet\r\n");

	ATBMPowerOnInit();

	ATBMSetDTMBMode();
	msleep(200);

	return 0;
}

static int atbm8869_fe_resume(struct aml_fe_dev *dev)
{
	printk("atbm8869_fe_resume\n");
	struct m6tv_dtmb_platform_data *patbm_op = NULL;
	patbm_op = (struct m6tv_dtmb_platform_data*)dev->frontend_opration;
	if(NULL != patbm_op)
	{
		if(NULL != patbm_op->machine_panel_type)
		{
			int panelindex = patbm_op->machine_panel_type();
			if(2 == panelindex)
				return 0;
		}

		if(NULL != patbm_op->atbm_device_reset)
		{
			patbm_op->atbm_device_reset();
		}
	}
	i2c_adap_atbm=dev->i2c_adap;
	ATBMPowerOnInit();
	ATBMSetDTMBMode();
	return 0;

}

static int atbm8869_fe_suspend(struct aml_fe_dev *dev)
{
	return 0;
}

static struct aml_fe_drv atbm8869_dtv_demod_drv = {
.id         = AM_DTV_DEMOD_ATBM8869,
.name       = "Atbm8869",
.capability = AM_FE_DTMB,
.get_ops    = atbm8869_fe_get_ops,
.enter_mode = atbm8869_fe_enter_mode,
.suspend    = atbm8869_fe_suspend,
.resume     = atbm8869_fe_resume
};

static int __init atbmfrontend_init(void)
{
	pr_dbg("register atbm8869 demod driver\n");
	mutex_init(&atbm_lock);
	return aml_register_fe_drv(AM_DEV_DTV_DEMOD, &atbm8869_dtv_demod_drv);
}


static void __exit atbmfrontend_exit(void)
{
	pr_dbg("unregister atbm8869 demod driver\n");
	mutex_destroy(&atbm_lock);
	aml_unregister_fe_drv(AM_DEV_DTV_DEMOD, &atbm8869_dtv_demod_drv);
}

fs_initcall(atbmfrontend_init);
module_exit(atbmfrontend_exit);


MODULE_DESCRIPTION("atbm8869 DTMB Demodulator driver");
MODULE_AUTHOR("RSJ");
MODULE_LICENSE("GPL");


