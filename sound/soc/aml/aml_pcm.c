#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/soundcard.h>
#include <linux/timer.h>
#include <linux/debugfs.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/control.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>

#include <mach/am_regs.h>
#include <mach/pinmux.h>


#include "aml_pcm.h"
#include "aml_audio_hw.h"


#define AOUT_EVENT_PREPARE  0x1
extern int aout_notifier_call_chain(unsigned long val, void *v);

static struct timer_list timer;

/*--------------------------------------------------------------------------*\
 * Hardware definition
\*--------------------------------------------------------------------------*/
/* TODO: These values were taken from the AML platform driver, check
 *	 them against real values for AML
 */
static const struct snd_pcm_hardware aml_pcm_hardware = {
	.info			= SNDRV_PCM_INFO_INTERLEAVED|
							SNDRV_PCM_INFO_BLOCK_TRANSFER|
							SNDRV_PCM_INFO_MMAP |
				  		SNDRV_PCM_INFO_MMAP_VALID |
				  		SNDRV_PCM_INFO_PAUSE,
				  		
	.formats		= SNDRV_PCM_FMTBIT_S16_LE,
	.period_bytes_min	= 64,
	.period_bytes_max	= 8*1024,
	.periods_min		= 2,
	.periods_max		= 1024,
	.buffer_bytes_max	= 32 * 1024,
	
	.rate_min = 32000,
  .rate_max = 48000,
  .channels_min = 1,
  .channels_max = 2,
  .fifo_size = 0,  
};


/*--------------------------------------------------------------------------*\
 * Data types
\*--------------------------------------------------------------------------*/
struct aml_runtime_data {
	struct aml_pcm_dma_params *params;
	dma_addr_t dma_buffer;		/* physical address of dma buffer */
	dma_addr_t dma_buffer_end;	/* first address beyond DMA buffer */

	struct snd_pcm *pcm;
	audio_stream_t s[2];	
};


/*--------------------------------------------------------------------------*\
 * Helper functions
\*--------------------------------------------------------------------------*/
static int aml_pcm_preallocate_dma_buffer(struct snd_pcm *pcm,
	int stream)
{
	struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	size_t size = aml_pcm_hardware.buffer_bytes_max;
	buf->dev.type = SNDRV_DMA_TYPE_DEV;
	buf->dev.dev = pcm->card->dev;
	buf->private_data = NULL;
	buf->area = dma_alloc_coherent(pcm->card->dev, size,
					  &buf->addr, GFP_KERNEL);
	printk("aml-pcm:"
		"preallocate_dma_buffer: area=%p, addr=%p, size=%d\n",
		(void *) buf->area,
		(void *) buf->addr,
		size);

	if (!buf->area)
		return -ENOMEM;

	buf->bytes = size;
	return 0;
}
/*--------------------------------------------------------------------------*\
 * ISR
\*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*\
 * PCM operations
\*--------------------------------------------------------------------------*/
static int aml_pcm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct aml_runtime_data *prtd = runtime->private_data;
	struct snd_soc_pcm_runtime *rtd = snd_pcm_substream_chip(substream);
	audio_stream_t *s = &prtd->s[substream->stream];
	
	/* this may get called several times by oss emulation
	 * with different params */

	snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);
	runtime->dma_bytes = params_buffer_bytes(params);

	s->I2S_addr = runtime->dma_addr;
	
	return 0;
}

static int aml_pcm_hw_free(struct snd_pcm_substream *substream)
{
	struct aml_runtime_data *prtd = substream->runtime->private_data;
	struct aml_pcm_dma_params *params = prtd->params;
	if (params != NULL) {
					
	}

	return 0;
}

static int aml_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct aml_runtime_data *prtd = runtime->private_data;
	audio_stream_t *s = &prtd->s[substream->stream];
	
	if(prtd == 0)
		return 0;
	
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
			
	}
	else{
			
	}
	
	switch(runtime->rate){
		case 192000:
			s->sample_rate	=	AUDIO_CLK_FREQ_192;
			break;
		case 176400:
			s->sample_rate	=	AUDIO_CLK_FREQ_1764;
			break;
		case 96000:	
			s->sample_rate	=	AUDIO_CLK_FREQ_96;
			break;
		case 88200:	
			s->sample_rate	=	AUDIO_CLK_FREQ_882;
			break;
		case 48000:	
			s->sample_rate	=	AUDIO_CLK_FREQ_48;
			break;
		case 44100:	
			s->sample_rate	=	AUDIO_CLK_FREQ_441;
			break;
		case 32000:	
			s->sample_rate	=	AUDIO_CLK_FREQ_32;
			break;
		default:
			s->sample_rate	=	AUDIO_CLK_FREQ_441;
			break;	
	};
	audio_set_clk(s->sample_rate, AUDIO_CLK_256FS);
	audio_dac_set(s->sample_rate);
	audio_set_aiubuf(runtime->dma_addr,
                     runtime->dma_bytes);
	
	memset((void*)runtime->dma_area,0,runtime->dma_bytes);

    aout_notifier_call_chain(AOUT_EVENT_PREPARE, substream);

	return 0;
}

static int aml_pcm_trigger(struct snd_pcm_substream *substream,
	int cmd)
{
	struct snd_pcm_runtime *rtd = substream->runtime;
	struct aml_runtime_data *prtd = rtd->private_data;
	audio_stream_t *s = &prtd->s[substream->stream];
	int ret = 0;
	
	spin_lock(&s->lock);
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		
		del_timer_sync(&timer);
		
		timer.expires = jiffies + 1;
    del_timer(&timer);
    add_timer(&timer);
        
		// TODO
		printk("aml_pcm_trigger: SNDRV_PCM_TRIGGER_START\n");
		s->active = 1;
		audio_enable_ouput(1);
#if 0		
		printk("AIU_MEM_I2S_CONTROL = %x\n",READ_MPEG_REG(AIU_MEM_I2S_CONTROL));
		printk("AIU_I2S_MUTE_SWAP   = %x\n",READ_MPEG_REG(AIU_I2S_MUTE_SWAP));
		printk("AIU_MEM_I2S_START_PTR = %x\n",READ_MPEG_REG(AIU_MEM_I2S_START_PTR));
		printk("AIU_MEM_I2S_END_PTR = %x\n",READ_MPEG_REG(AIU_MEM_I2S_END_PTR));
		printk("AIU_I2S_MISC = %x\n",READ_MPEG_REG(AIU_I2S_MISC));
		printk("AIU_I2S_MUTE_SWAP = %x\n",READ_MPEG_REG(AIU_I2S_MUTE_SWAP));
		printk("AIU_I2S_DAC_CFG = %x\n",READ_MPEG_REG(AIU_I2S_DAC_CFG));
		printk("AIU_I2S_SOURCE_DESC = %x\n",READ_MPEG_REG(AIU_I2S_SOURCE_DESC));
		printk("AIU_MEM_I2S_MASKS = %x\n",READ_MPEG_REG(AIU_MEM_I2S_MASKS));
		printk("AIU_CLK_CTRL = %x\n",READ_MPEG_REG(AIU_CLK_CTRL));
		printk("ADAC_RESET = %x\n",READ_APB_REG(ADAC_RESET));
		printk("ADAC_CLOCK = %x\n",READ_APB_REG(ADAC_CLOCK));
		printk("ADAC_I2S_CONFIG_REG1 = %x\n",READ_APB_REG(ADAC_I2S_CONFIG_REG1));
		printk("ADAC_I2S_CONFIG_REG2 = %x\n",READ_APB_REG(ADAC_I2S_CONFIG_REG2));
		printk("ADAC_POWER_CTRL_REG1 = %x\n",READ_APB_REG(ADAC_POWER_CTRL_REG1));
		printk("ADAC_POWER_CTRL_REG2 = %x\n",READ_APB_REG(ADAC_POWER_CTRL_REG2));
		printk("ADAC_MUTE_CTRL_REG1 = %x\n",READ_APB_REG(ADAC_MUTE_CTRL_REG1));
		printk("ADAC_DAC_ADC_MIXER = %x\n",READ_APB_REG(ADAC_DAC_ADC_MIXER));
		printk("ADAC_PLAYBACK_VOL_CTRL_LSB = %x\n",READ_APB_REG(ADAC_PLAYBACK_VOL_CTRL_LSB));
		printk("ADAC_PLAYBACK_VOL_CTRL_MSB = %x\n",READ_APB_REG(ADAC_PLAYBACK_VOL_CTRL_MSB));
		printk("ADAC_STEREO_HS_VOL_CTRL_LSB = %x\n",READ_APB_REG(ADAC_STEREO_HS_VOL_CTRL_LSB));
		printk("ADAC_STEREO_HS_VOL_CTRL_MSB = %x\n",READ_APB_REG(ADAC_STEREO_HS_VOL_CTRL_MSB));
		printk("AIU_AUDAC_CTRL0 = %x\n",READ_MPEG_REG(AIU_AUDAC_CTRL0));
		printk("AIU_AUDAC_CTRL1 = %x\n",READ_MPEG_REG(AIU_AUDAC_CTRL1));
		printk("AIU_CLK_CTRL_MORE = %x\n",READ_MPEG_REG(AIU_CLK_CTRL_MORE));
		printk("HHI_AUD_CLK_CNTL = %x\n",READ_MPEG_REG(HHI_AUD_CLK_CNTL));
		printk("HHI_AUD_PLL_CNTL = %x\n",READ_MPEG_REG(HHI_AUD_PLL_CNTL));
#endif		
		break;		/* SNDRV_PCM_TRIGGER_START */
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
	case SNDRV_PCM_TRIGGER_STOP:
		// TODO
		printk("aml_pcm_trigger: SNDRV_PCM_TRIGGER_STOP\n");
		s->active = 0;
		audio_enable_ouput(0);
		break;

	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		// TODO
		printk("aml_pcm_trigger: SNDRV_PCM_TRIGGER_RESUME\n");
		s->active = 1;
		audio_enable_ouput(1);
		
		break;

	default:
		ret = -EINVAL;
	}
	spin_unlock(&s->lock);
	return ret;
}

static snd_pcm_uframes_t aml_pcm_pointer(
	struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct aml_runtime_data *prtd = runtime->private_data;
	audio_stream_t *s = &prtd->s[substream->stream];
	
	unsigned int addr, ptr;
	
	if(substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
			ptr = read_i2s_rd_ptr();
	    addr = ptr - s->I2S_addr;
	    return bytes_to_frames(runtime, addr);
	}else{
			// TODO
	}
	

	return 0;
}

static void aml_pcm_timer_callback(unsigned long data)
{
    struct snd_pcm_substream *substream = (struct snd_pcm_substream *)data;
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct aml_runtime_data *prtd = runtime->private_data;
		audio_stream_t *s = &prtd->s[substream->stream];

    unsigned int last_ptr, size;
	
		if(substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
				if(s->active == 1){
						spin_lock(&s->lock);
						last_ptr = read_i2s_rd_ptr();
						if (last_ptr < s->last_ptr) {
				        size = runtime->dma_bytes + last_ptr - (s->last_ptr);
				    } else {
				        size = last_ptr - (s->last_ptr);
				    }
    				s->last_ptr = last_ptr;
    				s->size += bytes_to_frames(substream->runtime, size);
    				if (s->size >= runtime->period_size) {
				        s->size %= runtime->period_size;
				        spin_unlock(&s->lock);
				        snd_pcm_period_elapsed(substream);
				        spin_lock(&s->lock);
				    }
				    mod_timer(&timer, jiffies + 1);
   					spin_unlock(&s->lock);
				}else{
						 mod_timer(&timer, jiffies + 1);
				}
		}else{
				// TODO
		}    
}


static int aml_pcm_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct aml_runtime_data *prtd;
	int ret = 0;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
		snd_soc_set_runtime_hwparams(substream, &aml_pcm_hardware);
	}else{
		snd_soc_set_runtime_hwparams(substream, &aml_pcm_hardware);
	}
	
	/* ensure that buffer size is a multiple of period size */
	ret = snd_pcm_hw_constraint_integer(runtime,
						SNDRV_PCM_HW_PARAM_PERIODS);
	if (ret < 0)
		goto out;

	prtd = kzalloc(sizeof(struct aml_runtime_data), GFP_KERNEL);
	if (prtd == NULL) {
		ret = -ENOMEM;
		goto out;
	}
	
	prtd->s[SNDRV_PCM_STREAM_PLAYBACK].id = "Audio out";
  prtd->s[SNDRV_PCM_STREAM_PLAYBACK].stream_id =
      SNDRV_PCM_STREAM_PLAYBACK;
  prtd->s[SNDRV_PCM_STREAM_CAPTURE].id = "Audio in";
  prtd->s[SNDRV_PCM_STREAM_CAPTURE].stream_id =
 			SNDRV_PCM_STREAM_CAPTURE;
 			
	prtd->pcm = substream->pcm;
	
	runtime->private_data = prtd;

	spin_lock_init(&prtd->s[0].lock);
	
	timer.function = &aml_pcm_timer_callback;
  timer.data = (unsigned long)substream;
  init_timer(&timer);
    
 out:
	return ret;
}

static int aml_pcm_close(struct snd_pcm_substream *substream)
{
	struct aml_runtime_data *prtd = substream->runtime->private_data;
	
	del_timer_sync(&timer);
	
	kfree(prtd);
	
	return 0;
}


int aml_pcm_copy(struct snd_pcm_substream *substream, int channel,
		    snd_pcm_uframes_t pos,
		    void __user *buf, snd_pcm_uframes_t count)
{
		unsigned short *tfrom, *to, *left, *right;
    int res = 0;
    int n;
    int i = 0, j = 0;
    
   // register unsigned  int vol =(audio_mixer_control.output_volume*(1<<VOLUME_SHIFT))/VOLUME_SCALE;
    struct snd_pcm_runtime *runtime = substream->runtime;
    char *hwbuf = runtime->dma_area + frames_to_bytes(runtime, pos);
 
    tfrom = (unsigned short *)buf;
    to = (unsigned short *)hwbuf;
    n = frames_to_bytes(runtime, count);

    if (access_ok(VERIFY_READ, buf, frames_to_bytes(runtime, count)))
    { 
	    left = to;
	    right = to + 16;
	    if (pos % 16) {
	        printk("audio data unligned\n");
	    }
	    for (j = 0; j < n; j += 64) {
	        for (i = 0; i < 16; i++) {
              *left++ = (*tfrom++) ;
              *right++ = (*tfrom++);
	         }
	        left += 16;
	        right += 16;
	    }
	    tfrom = (unsigned short *)buf;   		
   }
   else{
   		res=-EFAULT;
   }
   
    return res;
}

int aml_pcm_silence(struct snd_pcm_substream *substream, int channel, 
		       snd_pcm_uframes_t pos, snd_pcm_uframes_t count)
{
		char* ppos;
		int n;
		struct snd_pcm_runtime *runtime = substream->runtime;
		
		n = frames_to_bytes(runtime, count);
		ppos = runtime->dma_area + frames_to_bytes(runtime, pos);
		memset(ppos, 0, n);
		
		return 0;
}
		       		    
static struct snd_pcm_ops aml_pcm_ops = {
	.open		= aml_pcm_open,
	.close		= aml_pcm_close,
	.ioctl		= snd_pcm_lib_ioctl,
	.hw_params	= aml_pcm_hw_params,
	.hw_free	= aml_pcm_hw_free,
	.prepare	= aml_pcm_prepare,
	.trigger	= aml_pcm_trigger,
	.pointer	= aml_pcm_pointer,
	.copy 		= aml_pcm_copy,
	.silence	=	aml_pcm_silence,
};


/*--------------------------------------------------------------------------*\
 * ASoC platform driver
\*--------------------------------------------------------------------------*/
static u64 aml_pcm_dmamask = 0xffffffff;

static int aml_pcm_new(struct snd_card *card,
	struct snd_soc_dai *dai, struct snd_pcm *pcm)
{
	int ret = 0;
	if (!card->dev->dma_mask)
		card->dev->dma_mask = &aml_pcm_dmamask;
	if (!card->dev->coherent_dma_mask)
		card->dev->coherent_dma_mask = 0xffffffff;

	if (dai->playback.channels_min) {
		ret = aml_pcm_preallocate_dma_buffer(pcm,
			SNDRV_PCM_STREAM_PLAYBACK);
		if (ret)
			goto out;
	}

	if (dai->capture.channels_min) {
		pr_debug("aml-pcm:"
				"Allocating PCM capture DMA buffer\n");
		ret = aml_pcm_preallocate_dma_buffer(pcm,
			SNDRV_PCM_STREAM_CAPTURE);
		if (ret)
			goto out;
	}
	
 out:
	return ret;
}

static void aml_pcm_free_dma_buffers(struct snd_pcm *pcm)
{
	struct snd_pcm_substream *substream;
	struct snd_dma_buffer *buf;
	int stream;
	for (stream = 0; stream < 2; stream++) {
		substream = pcm->streams[stream].substream;
		if (!substream)
			continue;

		buf = &substream->dma_buffer;
		if (!buf->area)
			continue;
		dma_free_coherent(pcm->card->dev, buf->bytes,
				  buf->area, buf->addr);
		buf->area = NULL;
	}
}

#ifdef CONFIG_PM
static int aml_pcm_suspend(struct snd_soc_dai *dai)
{
	struct snd_pcm_runtime *runtime = dai->runtime;
	struct aml_runtime_data *prtd;
	struct aml_pcm_dma_params *params;
	if (!runtime)
		return 0;

	prtd = runtime->private_data;
	params = prtd->params;

	/* disable the PDC and save the PDC registers */
	// TODO
	

	return 0;
}

static int aml_pcm_resume(struct snd_soc_dai *dai)
{
	struct snd_pcm_runtime *runtime = dai->runtime;
	struct aml_runtime_data *prtd;
	struct aml_pcm_dma_params *params;
	if (!runtime)
		return 0;

	prtd = runtime->private_data;
	params = prtd->params;

	/* restore the PDC registers and enable the PDC */
	// TODO
	
	return 0;
}
#else
#define aml_pcm_suspend	NULL
#define aml_pcm_resume	NULL
#endif

#ifdef CONFIG_DEBUG_FS

static struct dentry *debugfs_root;
static struct dentry *debugfs_regs;

static int regs_open_file(struct inode *inode, struct file *file)
{
	return 0;
}

/**
 *	cat regs
 */
static ssize_t regs_read_file(struct file *file, char __user *user_buf,
			       size_t count, loff_t *ppos)
{
	ssize_t ret;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;
		
	ret = sprintf(buf, "Usage: \n"
										 "	echo base reg val >regs\t(set the register)\n"
										 "	echo base reg >regs\t(show the register)\n"
										 "	base -> c(cbus), x(aix), p(apb), h(ahb) \n"
									);
		
	if (ret >= 0)
		ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);
	kfree(buf);	
	
	return ret;
}

static int read_regs(char base, int reg)
{
	int val = 0;
	switch(base){
		case 'c':
			val = READ_CBUS_REG(reg);
			break;
		case 'x':
			val = READ_AXI_REG(reg);
			break;
		case 'p':
			val = READ_APB_REG(reg);
			break;
		case 'h':
			val = READ_AHB_REG(reg);
			break;
		default:
			break;
	};
	printk("\tReg %x = %x\n", reg, val);
	return val;
}

static void write_regs(char base, int reg, int val)
{
	switch(base){
		case 'c':
			WRITE_CBUS_REG(reg, val);
			break;
		case 'x':
			WRITE_AXI_REG(reg, val);
			break;
		case 'p':
			WRITE_APB_REG(reg, val);
			break;
		case 'h':
			WRITE_AHB_REG(reg, val);
			break;
		default:
			break;
	};
	printk("Write reg:%x = %x\n", reg, val);
}
static ssize_t regs_write_file(struct file *file,
		const char __user *user_buf, size_t count, loff_t *ppos)
{
	char buf[32];
	int buf_size = 0;
	char *start = buf;
	unsigned long reg, value;
	int step = 1;
	char base;
	
	buf_size = min(count, (sizeof(buf)-1));
	
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;
	buf[buf_size] = 0;
	while (*start == ' ')
		start++;
		
	base = *start;
	start ++;
	if(!(base =='c' || base == 'x' || base == 'p' || base == 'h')){
		return -EINVAL;
	}
	
	while (*start == ' ')
		start++;
		
	reg = simple_strtoul(start, &start, 16);
	
	while (*start == ' ')
		start++;
		
	if (strict_strtoul(start, 16, &value))
	{
			read_regs(base, reg);
			return -EINVAL;
	}
	
	write_regs(base, reg, value);
	
	return buf_size;
}

static const struct file_operations regs_fops = {
	.open = regs_open_file,
	.read = regs_read_file,
	.write = regs_write_file,
};

static void aml_pcm_init_debugfs()
{
		debugfs_root = debugfs_create_dir("aml_pcm_debug",NULL);
		if (IS_ERR(debugfs_root) || !debugfs_root) {
			printk("aml_pcm: Failed to create debugfs directory\n");
			debugfs_root = NULL;
		}
		
		debugfs_regs = debugfs_create_file("regs", 0644, debugfs_root, NULL, &regs_fops);
		if(!debugfs_regs){
			printk("aml_pcm: Failed to create debugfs file\n");
		}
}
static void aml_pcm_cleanup_debugfs()
{
	debugfs_remove_recursive(debugfs_root);
}
#else
static void aml_pcm_init_debugfs()
{
}
static void aml_pcm_cleanup_debugfs()
{
}
#endif
struct snd_soc_platform aml_soc_platform = {
	.name		= "aml-audio",
	.pcm_ops 	= &aml_pcm_ops,
	.pcm_new	= aml_pcm_new,
	.pcm_free	= aml_pcm_free_dma_buffers,
	.suspend	= aml_pcm_suspend,
	.resume		= aml_pcm_resume,
};

EXPORT_SYMBOL_GPL(aml_soc_platform);

static int __init aml_alsa_audio_init(void)
{
		aml_pcm_init_debugfs();
		
		return snd_soc_register_platform(&aml_soc_platform);
}

static void __exit aml_alsa_audio_exit(void)
{
		aml_pcm_cleanup_debugfs();
    snd_soc_unregister_platform(&aml_soc_platform);
}

module_init(aml_alsa_audio_init);
module_exit(aml_alsa_audio_exit);

MODULE_AUTHOR("AMLogic, Inc.");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("AML driver for ALSA");

//module_param(id, charp, 0444);
MODULE_PARM_DESC(id, "ID string for AML soundcard.");
