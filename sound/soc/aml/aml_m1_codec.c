#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/spi/spi.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <sound/tlv.h>

struct snd_soc_codec_device soc_codec_dev_aml_m1;
static struct snd_soc_codec *aml_m1_codec;

/* codec private data */
struct aml_m1_codec_priv {
	struct snd_soc_codec codec;
	u8 reg_cache[1];
	unsigned int sysclk;
};

static int aml_m1_write(struct snd_soc_codec *codec, unsigned int reg,
							unsigned int value)
{
	u8 *reg_cache = codec->reg_cache;
printk("***Entered %s:%s\n", __FILE__,__func__);
	if (reg >= codec->reg_cache_size)
		return -EINVAL;

	return 0;
}

static unsigned int aml_m1_read_reg_cache(struct snd_soc_codec *codec,
							unsigned int reg)
{
	u8 *reg_cache = codec->reg_cache;
printk("***Entered %s:%s\n", __FILE__,__func__);
	if (reg >= codec->reg_cache_size)
		return -EINVAL;

	return reg_cache[reg];
}

static int aml_m1_codec_hw_params(struct snd_pcm_substream *substream,
			    struct snd_pcm_hw_params *params,
			    struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct aml_m1_codec_priv *aml = codec->private_data;
printk("***Entered %s:%s\n", __FILE__,__func__);
	// TODO
	
	printk("%s sysclk: %d, rate:%d\n", __func__,
		 aml->sysclk, params_rate(params));
	
	return 0;
}


static int aml_m1_codec_pcm_prepare(struct snd_pcm_substream *substream,
			      struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
printk("***Entered %s:%s\n", __FILE__,__func__);
	/* set active */
	
	// TODO

	return 0;
}

static void aml_m1_codec_shutdown(struct snd_pcm_substream *substream,
			    struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
printk("***Entered %s:%s\n", __FILE__,__func__);
	/* deactivate */
	if (!codec->active) {
		udelay(50);
		
		// TODO
	}
}

static int aml_m1_codec_mute(struct snd_soc_dai *dai, int mute)
{
	struct snd_soc_codec *codec = dai->codec;
	
	// TODO
printk("***Entered %s:%s\n", __FILE__,__func__);
	return 0;
}

static int aml_m1_codec_set_dai_sysclk(struct snd_soc_dai *codec_dai,
		int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct aml_m1_codec_priv *aml = codec->private_data;
printk("***Entered %s:%s\n", __FILE__,__func__);
	switch (freq) {
	case 11289600:
	case 12000000:
	case 12288000:
	case 16934400:
	case 18432000:
		aml->sysclk = freq;
		return 0;
	}
	return -EINVAL;
}


static int aml_m1_codec_set_dai_fmt(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	u16 iface = 0;
printk("***Entered %s:%s\n", __FILE__,__func__);
	/* set master/slave audio interface */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		iface |= 0x0040;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		break;
	default:
		return -EINVAL;
	}

	/* interface format */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		iface |= 0x0002;
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		iface |= 0x0001;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		iface |= 0x0003;
		break;
	case SND_SOC_DAIFMT_DSP_B:
		iface |= 0x0013;
		break;
	default:
		return -EINVAL;
	}

	/* clock inversion */
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		break;
	case SND_SOC_DAIFMT_IB_IF:
		iface |= 0x0090;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		iface |= 0x0080;
		break;
	case SND_SOC_DAIFMT_NB_IF:
		iface |= 0x0010;
		break;
	default:
		return -EINVAL;
	}

	/* set iface */
	
	// TODO
	
	return 0;
}

#define AML_RATES SNDRV_PCM_RATE_8000_96000

#define AML_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | \
	SNDRV_PCM_FMTBIT_S24_LE)


static struct snd_soc_dai_ops aml_m1_codec_dai_ops = {
	.prepare	= aml_m1_codec_pcm_prepare,
	.hw_params	= aml_m1_codec_hw_params,
	.shutdown	= aml_m1_codec_shutdown,
	.digital_mute	= aml_m1_codec_mute,
	.set_sysclk	= aml_m1_codec_set_dai_sysclk,
	.set_fmt	= aml_m1_codec_set_dai_fmt,
};

struct snd_soc_dai aml_m1_codec_dai = {
	.name = "AML-M1",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = AML_RATES,
		.formats = AML_FORMATS,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = AML_RATES,
		.formats = AML_FORMATS,},
	.ops = &aml_m1_codec_dai_ops,
	.symmetric_rates = 1,
};
EXPORT_SYMBOL_GPL(aml_m1_codec_dai);

static int aml_m1_codec_probe(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec;
	int ret = 0;
printk("***Entered %s:%s\n", __FILE__,__func__);
	if (!aml_m1_codec) {
		dev_err(&pdev->dev, "AML_M1_CODEC not yet discovered\n");
		return -ENODEV;
	}
	codec = aml_m1_codec;			
	socdev->card->codec = codec;	
	
	/* register pcms */
	ret = snd_soc_new_pcms(socdev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1);
	if (ret < 0) {
		kfree(codec);
		dev_err(codec->dev, "aml m1 codec: failed to create pcms: %d\n", ret);
		return ret;
	}

	return 0;
}


static int aml_m1_codec_remove(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;
printk("***Entered %s:%s\n", __FILE__,__func__);
	snd_soc_free_pcms(socdev);
	kfree(codec);
	return 0;
}

struct snd_soc_codec_device soc_codec_dev_aml_m1 = {
	.probe =	aml_m1_codec_probe,
	.remove =	aml_m1_codec_remove,
};
EXPORT_SYMBOL_GPL(soc_codec_dev_aml_m1);

static int aml_m1_register(struct aml_m1_codec_priv* aml_m1)
{
	struct snd_soc_codec* codec = &aml_m1->codec;
	int ret;
		
	mutex_init(&codec->mutex);
	INIT_LIST_HEAD(&codec->dapm_widgets);
	INIT_LIST_HEAD(&codec->dapm_paths);

	codec->name = "AML_M1_CODEC";
	codec->owner = THIS_MODULE;
	codec->private_data = aml_m1;

	codec->dai = &aml_m1_codec_dai;
	codec->num_dai = 1;

	codec->reg_cache = &aml_m1->reg_cache;
	codec->reg_cache_size = ARRAY_SIZE(aml_m1->reg_cache);
	codec->read = aml_m1_read_reg_cache;
	codec->write = aml_m1_write;

	codec->bias_level = SND_SOC_BIAS_OFF;

	aml_m1_codec_dai.dev = codec->dev;

	aml_m1_codec = codec;

	ret = snd_soc_register_codec(codec);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to register codec: %d\n", ret);
		goto err;
	}

	ret = snd_soc_register_dai(&aml_m1_codec_dai);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to register DAI: %d\n", ret);
		goto err_codec;
	}

	return 0;

err_codec:
	snd_soc_unregister_codec(codec);
err:
	aml_m1_codec = NULL;
	kfree(aml_m1);
	return ret;
		
}

static void aml_m1_unregister(struct aml_m1_codec_priv *aml_m1)
{
	snd_soc_unregister_dai(&aml_m1_codec_dai);
	snd_soc_unregister_codec(&aml_m1->codec);
	aml_m1_codec = NULL;
	kfree(aml_m1);
}

static int aml_m1_codec_platform_probe(struct platform_device *pdev)
{
	struct aml_m1_codec_priv *aml_m1;
	struct snd_soc_codec *codec;

	aml_m1 = kzalloc(sizeof(struct aml_m1_codec_priv), GFP_KERNEL);
	if (aml_m1 == NULL)
		return -ENOMEM;

	codec = &aml_m1->codec;

	codec->control_data = NULL;
	codec->hw_write = NULL;
	codec->pop_time = 0;

	codec->dev = &pdev->dev;
	platform_set_drvdata(pdev, aml_m1);

	return aml_m1_register(aml_m1);
}

static int __exit aml_m1_codec_platform_remove(struct platform_device *pdev)
{
	struct aml_m1_codec_priv *aml_m1 = platform_get_drvdata(pdev);

	aml_m1_unregister(aml_m1);
	return 0;
}

static struct platform_driver aml_m1_codec_platform_driver = {
	.driver = {
		.name = "aml_m1_codec",
		.owner = THIS_MODULE,
		},
	.probe = aml_m1_codec_platform_probe,
	.remove = __exit_p(aml_m1_codec_platform_remove),
};

static int __init aml_m1_codec_modinit(void)
{
		return platform_driver_register(&aml_m1_codec_platform_driver);
}

static void __exit aml_m1_codec_exit(void)
{
		platform_driver_unregister(&aml_m1_codec_platform_driver);
}

module_init(aml_m1_codec_modinit);
module_exit(aml_m1_codec_exit);


MODULE_DESCRIPTION("ASoC AML M1 codec driver");
MODULE_AUTHOR("AMLogic Inc.");
MODULE_LICENSE("GPL");
