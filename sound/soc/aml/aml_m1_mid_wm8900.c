#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/gpio.h>

#include "aml_dai.h"
#include "aml_pcm.h"
#include "../codecs/wm8900.h"


static int aml_m1_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
		struct snd_soc_pcm_runtime *rtd = substream->private_data;
		struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
		struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
		int ret;
		// TODO
printk("***Entered %s:%s\n", __FILE__,__func__);				
		
		ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S|SND_SOC_DAIFMT_NB_NF|SND_SOC_DAIFMT_CBS_CFS);
		if(ret<0)
			return ret;
			
		ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S|SND_SOC_DAIFMT_NB_NF|SND_SOC_DAIFMT_CBS_CFS);
		if(ret<0)
			return ret;
		
		
	return 0;
}
	
static struct snd_soc_ops aml_m1_ops = {
	.hw_params = aml_m1_hw_params,
};

static int aml_m1_set_bias_level(struct snd_soc_card *card,
					enum snd_soc_bias_level level)
{
	int ret = 0;
	// TODO
printk("***Entered %s:%s: %d\n", __FILE__,__func__, level);
	switch (level) {
	case SND_SOC_BIAS_ON:
	case SND_SOC_BIAS_PREPARE:
		
		break;
	case SND_SOC_BIAS_OFF:
	case SND_SOC_BIAS_STANDBY:
	
		break;
	};
	
	return ret;
}

static const struct snd_soc_dapm_widget aml_m1_dapm_widgets[] = {
	SND_SOC_DAPM_LINE("Ext Spk", NULL),
};

static const struct snd_soc_dapm_route intercon[] = {

	/* speaker connected to LINEOUT */
	{"Ext Spk", NULL, "LINEOUT1L"},
	{"Ext Spk", NULL, "LINEOUT1R"},
	/* mic is connected to Mic Jack, with WM8731 Mic Bias */
//	{"MICIN", NULL, "Mic Bias"},
//	{"Mic Bias", NULL, "Int Mic"},
};

static int aml_m1_codec_init(struct snd_soc_codec *codec)
{
		int err;
		
printk("***Entered %s:%s\n", __FILE__,__func__);
		snd_soc_dapm_new_controls(codec, aml_m1_dapm_widgets, ARRAY_SIZE(aml_m1_dapm_widgets));
		
		err = snd_soc_dapm_add_routes(codec, intercon,
				      ARRAY_SIZE(intercon));
		
		snd_soc_dapm_nc_pin(codec,"LINPUT1");
		snd_soc_dapm_nc_pin(codec,"RINPUT1");
		
		snd_soc_dapm_enable_pin(codec, "Ext Spk");
		
		snd_soc_dapm_sync(codec);

		return 0;
}


static struct snd_soc_dai_link aml_m1_dai = {
	.name = "AML-M1",
	.stream_name = "AML M1 PCM",
	.cpu_dai = &aml_dai[0],  //////
	.codec_dai = &wm8900_dai,
	.init = aml_m1_codec_init,
	.ops = &aml_m1_ops,
};

static struct snd_soc_card snd_soc_aml_m1 = {
	.name = "AML-M1",
	.platform = &aml_soc_platform,
	.dai_link = &aml_m1_dai,
	.num_links = 1,
	.set_bias_level = aml_m1_set_bias_level,
};

static struct snd_soc_device aml_m1_snd_devdata = {
	.card = &snd_soc_aml_m1,
	.codec_dev = &soc_codec_dev_wm8900,
};

static struct platform_device *aml_m1_snd_device;
static struct platform_device *aml_m1_platform_device;

static int aml_m1_audio_probe(struct platform_device *pdev)
{
		int ret;
		//pdev->dev.platform_data;
		// TODO
printk("***Entered %s:%s\n", __FILE__,__func__);
		aml_m1_snd_device = platform_device_alloc("soc-audio", -1);
		if (!aml_m1_snd_device) {
			printk(KERN_ERR "ASoC: Platform device allocation failed\n");
			ret = -ENOMEM;
		}
	
		platform_set_drvdata(aml_m1_snd_device,&aml_m1_snd_devdata);
		aml_m1_snd_devdata.dev = &aml_m1_snd_device->dev;
	
		ret = platform_device_add(aml_m1_snd_device);
		if (ret) {
			printk(KERN_ERR "ASoC: Platform device allocation failed\n");
			goto error;
		}
		
		aml_m1_platform_device = platform_device_register_simple("aml_m1_codec",
								-1, NULL, 0);
		return 0;							
error:								
		platform_device_put(aml_m1_snd_device);								
		return ret;
}

static int aml_m1_audio_remove(struct platform_device *pdev)
{
printk("***Entered %s:%s\n", __FILE__,__func__);
		platform_device_unregister(aml_m1_snd_device);
		return 0;
}

static struct platform_driver aml_m1_audio_driver = {
	.probe  = aml_m1_audio_probe,
	.remove = aml_m1_audio_remove,
	.driver = {
		.name = "aml_m1_audio_wm8900",
		.owner = THIS_MODULE,
	},
};

static int __init aml_m1_init(void)
{
		return platform_driver_register(&aml_m1_audio_driver);
}

static void __exit aml_m1_exit(void)
{
		platform_driver_unregister(&aml_m1_audio_driver);
}

module_init(aml_m1_init);
module_exit(aml_m1_exit);

/* Module information */
MODULE_AUTHOR("AMLogic, Inc.");
MODULE_DESCRIPTION("ALSA SoC AML M1 AUDIO");
MODULE_LICENSE("GPL");
