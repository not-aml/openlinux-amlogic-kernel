

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <asm/cacheflush.h>
//#include <asm/arch/am_regs.h>
#include <linux/major.h>
#include <linux/slab.h>

//#include <asm/dsp/audiodsp_control.h>
#include "audiodsp_control.h"	// temp here

#include <asm/uaccess.h>
#include <linux/amports/amstream.h>

#include <mach/am_regs.h>

#include "audiodsp_module.h"
#include "dsp_control.h"
#include "dsp_microcode.h"
#include "dsp_mailbox.h"
#include "dsp_monitor.h"

#include "dsp_codec.h"
#include <linux/dma-mapping.h>



MODULE_DESCRIPTION("AMLOGIC APOLLO Audio dsp driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zhou Zhi <zhi.zhou@amlogic.com>");
MODULE_VERSION("1.0.0");

extern struct audio_info * get_audio_info(void);
void audiodsp_moniter(unsigned long);
static struct audiodsp_priv *audiodsp_p;
#define  DSP_DRIVER_NAME	"auidodsp"
#define  DSP_NAME	"dsp"

int audiodsp_start(void)
{
	struct audiodsp_priv *priv=audiodsp_privdata();
	struct auidodsp_microcode *pmcode;
	struct audio_info *audio_info;
	int ret,i;
	priv->frame_format.valid=0;
	priv->decode_error_count=0;
	priv->last_valid_pts=0;
	pmcode=audiodsp_find_supoort_mcode(priv,priv->stream_fmt);
	if(pmcode==NULL)
		{
		DSP_PRNT("have not find a valid mcode for fmt(0x%x)\n",priv->stream_fmt);
		return -1;
		}
	stop_audiodsp_monitor(priv);
	dsp_stop(priv);
	ret=dsp_start(priv,pmcode);
	if(ret==0){
 		start_audiodsp_monitor(priv);

#ifdef CONFIG_AM_VDEC_REAL
	if(pmcode->fmt == MCODEC_FMT_COOK || pmcode->fmt == MCODEC_FMT_RAAC 
		|| pmcode->fmt == MCODEC_FMT_AMR || pmcode->fmt == MCODEC_FMT_WMA 
		||(pmcode->fmt == MCODEC_FMT_ADPCM))

	{
    		for(i = 0; i< 1000;i++){
                if(DSP_RD(DSP_AUDIOINFO_STATUS) == DSP_AUDIOINFO_READY)//maybe at audiodsp side,INT not enabled yet,so wait a while
                    break;
    		     msleep(1);
            }
            DSP_WD(DSP_AUDIOINFO_STATUS,0);
		    audio_info = get_audio_info();
		    dsp_mailbox_send(priv, 1, M2B_IRQ4_AUDIO_INFO, 0, (const char*)audio_info, sizeof(audio_info));
    }
#endif
     }
	return ret;
}

static int audiodsp_open(struct inode *node, struct file *file)
{
	DSP_PRNT("dsp_open\n");
	return 0;

}

static int audiodsp_ioctl(struct inode *node, struct file *file, unsigned int cmd,
		      unsigned long args)
{
	struct audiodsp_priv *priv=audiodsp_privdata();
	struct audiodsp_cmd *a_cmd;
	char name[64];
	int len;
	int ret=0;
	unsigned long *val=(unsigned long *)args;

	switch(cmd)
		{
		case AUDIODSP_SET_FMT:
			priv->stream_fmt=args;
			break;
		case AUDIODSP_START:
			if(priv->stream_fmt<=0)
				{
				DSP_PRNT("Audio dsp steam format have not set!\n");
				}
			else
				{
				ret=audiodsp_start();
				}
			break;
		case AUDIODSP_STOP:
			//DSP_PRNT("auidodsp command stop\n");
			stop_audiodsp_monitor(priv);
			dsp_stop(priv);
			
			break;
		case AUDIODSP_DECODE_START:
			if(priv->dsp_is_started)
				{
				int waittime=0;
				dsp_codec_start(priv);
				while(!((priv->frame_format.valid & CHANNEL_VALID) &&
					(priv->frame_format.valid & SAMPLE_RATE_VALID) &&
					(priv->frame_format.valid & DATA_WIDTH_VALID)))
					{
					
					waittime++;
					if(waittime>100)
					{
						DSP_PRNT("dsp have not set the codec stream's format details,valid=%x\n",
						priv->frame_format.valid);
						ret=-1;
						break;
					}
					msleep(10);/*wait codec start and decode the format*/
					}
				}
			else
				{
				DSP_PRNT("Audio dsp have not started\n");
				}
			break;
		case AUDIODSP_DECODE_STOP:
			if(priv->dsp_is_started)
				{
				dsp_codec_stop(priv);
				}
			else
				{
				DSP_PRNT("Audio dsp have not started\n");
				}
			
			break;
		case AUDIODSP_REGISTER_FIRMWARE:
			a_cmd=(struct audiodsp_cmd *)args;
		//	DSP_PRNT("register firware,%d,%s\n",a_cmd->fmt,a_cmd->data);
			len=a_cmd->data_len>64?64:a_cmd->data_len;
			copy_from_user(name,a_cmd->data,len);
			name[len]='\0';
			ret=auidodsp_microcode_register(priv,
								a_cmd->fmt,
								name);
			break;
		case AUDIODSP_UNREGISTER_ALLFIRMWARE:
			  auidodsp_microcode_free(priv);
			break;
		case AUDIODSP_GET_CHANNELS_NUM: 
			*val=-1;/*mask data is not valid*/
			if(priv->frame_format.valid & CHANNEL_VALID)
				{
				*val=priv->frame_format.channel_num; 
				}
			break;
		case AUDIODSP_GET_SAMPLERATE: 
			*val=-1;/*mask data is not valid*/
			if(priv->frame_format.valid & SAMPLE_RATE_VALID)
				{
				*val=priv->frame_format.sample_rate; 
				} 
			break;
		case AUDIODSP_GET_BITS_PER_SAMPLE: 
			*val=-1;/*mask data is not valid*/
			if(priv->frame_format.valid & DATA_WIDTH_VALID)
				{
				*val=priv->frame_format.data_width; 
				} 
			break;
		case AUDIODSP_GET_PTS:
			/*val=-1 is not valid*/
			*val=dsp_codec_get_current_pts(priv);
			break;
		default:
			DSP_PRNT("unsupport cmd number%d\n",cmd);
			ret=-1;
		}
	return ret;
}

static int audiodsp_release(struct inode *node, struct file *file)
{
	DSP_PRNT("dsp_release\n");
	return 0;
}



ssize_t audiodsp_read(struct file * file, char __user * ubuf, size_t size,
		  loff_t * loff)
{
	struct audiodsp_priv *priv=audiodsp_privdata();
	unsigned long rp,orp;
	size_t len;
	size_t else_len;
	size_t wlen;
	size_t w_else_len;
	int wait=0;
	 char __user *pubuf=ubuf;
     dma_addr_t buf_map;



	#define MIN_READ		128
	#define PCM_DATA_MIN	128
	#define PCM_DATA_ALGIN(x) (x & (~(PCM_DATA_MIN-1)))
	#define MAX_WAIT		HZ/10
	
	mutex_lock(&priv->stream_buffer_mutex);	
	if(priv->stream_buffer_mem==NULL || !priv->dsp_is_started)
		goto error_out;
	
	do{
			len=dsp_codec_get_bufer_data_len(priv);
			if(len>MIN_READ)
				break;
			else
				{
				if(wait>0)
					break;
				wait++;
				init_completion(&priv->decode_completion);
				wait_for_completion_timeout(&priv->decode_completion, MAX_WAIT);
				}
		}while(len<MIN_READ);
	if(len>priv->stream_buffer_size || len <0)
		{
		DSP_PRNT("audio stream buffer is bad len=%d\n",len);
		goto error_out;
		}
	len=min(len,size);
	len=PCM_DATA_ALGIN(len);
	else_len=len;
	rp=dsp_codec_get_rd_addr(priv);
	orp=rp;
	while(else_len>0)
		{
		
		wlen=priv->stream_buffer_end-rp;
		wlen=min(wlen,else_len);
///		dma_cache_inv((unsigned long)rp,wlen);    
        buf_map = dma_map_single(NULL, (void *)rp, wlen, DMA_FROM_DEVICE);
		w_else_len=copy_to_user((void*)pubuf,(const char *)(rp),wlen);
		if(w_else_len!=0)
			{
			DSP_PRNT("copyed error,%d,%d,[%p]<---[%lx]\n",w_else_len,wlen,pubuf,rp);
			wlen-=w_else_len;
			}      
        dma_unmap_single(NULL, buf_map, wlen, DMA_FROM_DEVICE);
		else_len-=wlen;
		pubuf+=wlen;
		rp=dsp_codec_inc_rd_addr(priv,wlen);
		}
	priv->out_len_after_last_valid_pts+=len;
	mutex_unlock(&priv->stream_buffer_mutex);
	//u32 timestamp_pcrscr_get(void);
	//printk("current pts=%ld,src=%ld\n",dsp_codec_get_current_pts(priv),timestamp_pcrscr_get());
	return len;
error_out:
	mutex_unlock(&priv->stream_buffer_mutex);
	return 0;
}

ssize_t audiodsp_write(struct file * file, const char __user * ubuf, size_t size,
		   loff_t * loff)
{
	struct audiodsp_priv *priv=audiodsp_privdata();
	// int dsp_codec_start( struct audiodsp_priv *priv);
	// int dsp_codec_stop( struct audiodsp_priv *priv);
	auidodsp_microcode_register(priv,
								MCODEC_FMT_COOK,
								"audiodsp_codec_cook.bin");
	priv->stream_fmt=MCODEC_FMT_COOK;
	audiodsp_start();
	dsp_codec_start(priv);
	//dsp_codec_stop(priv);
	
	return size;
}


const static struct file_operations audiodsp_fops = {
	.owner = THIS_MODULE,
	.open =audiodsp_open,
	.read = audiodsp_read,
	.write = audiodsp_write,
	.release = audiodsp_release,
	.ioctl = audiodsp_ioctl,
};
static int audiodsp_get_status(struct adec_status *astatus)
{
	struct audiodsp_priv *priv=audiodsp_privdata();
	if(!astatus)
		return -EINVAL;
	if(priv->frame_format.valid & CHANNEL_VALID)
		astatus->channels=priv->frame_format.channel_num;
	else
		astatus->channels=0;
	if(priv->frame_format.valid & SAMPLE_RATE_VALID)
		astatus->sample_rate=priv->frame_format.sample_rate;
	else
		astatus->sample_rate=0;
	if(priv->frame_format.valid & DATA_WIDTH_VALID)
		astatus->resolution=priv->frame_format.data_width;
	else
		astatus->resolution=0;
	astatus->error_count=priv->decode_error_count;
	astatus->status=priv->dsp_is_started?0:1;
	return 0;
}
static int audiodsp_init_mcode(struct audiodsp_priv *priv)
{
	spin_lock_init(&priv->mcode_lock);
	priv->mcode_id=0;
	priv->dsp_stack_start=0;
	priv->dsp_gstack_start=0;
	priv->dsp_heap_start=0;
	priv->code_mem_size=AUDIO_DSP_MEM_SIZE -REG_MEM_SIZE;
	priv->dsp_code_start=AUDIO_DSP_START_ADDR;
    DSP_PRNT("DSP start addr 0x%x\n",AUDIO_DSP_START_ADDR);
	priv->dsp_stack_size=1024*64;
	priv->dsp_gstack_size=512;
	priv->dsp_heap_size=0;
	priv->stream_buffer_mem=NULL;
	priv->stream_buffer_mem_size=32*1024;
	priv->stream_fmt=-1;
	INIT_LIST_HEAD(&priv->mcode_list);
	init_completion(&priv->decode_completion);
	mutex_init(&priv->stream_buffer_mutex);
	mutex_init(&priv->dsp_mutex);
	priv->last_stream_fmt=-1;
	priv->last_valid_pts=0;
	priv->out_len_after_last_valid_pts=0;
	return 0;
}


int audiodsp_probe(void )
{

	int res=0;
	struct audiodsp_priv *priv;
	priv=kmalloc(sizeof(struct audiodsp_priv),GFP_KERNEL);
	if(priv==NULL)
		{
		DSP_PRNT("Out of memory for audiodsp register\n");
		return -1;
		}
    priv->dsp_is_started=0;
    priv->p = ioremap_nocache(AUDIO_DSP_START_PHY_ADDR, S_1M);
    if(priv->p)
        DSP_PRNT("DSP IOREMAP to addr 0x%x\n",(unsigned)priv->p);
    else
        goto error1;
	audiodsp_p=priv;
	audiodsp_init_mcode(priv);
	res = register_chrdev(AUDIODSP_MAJOR, DSP_NAME, &audiodsp_fops);
	if (res < 0) {
		DSP_PRNT("Can't register  char devie for " DSP_NAME "\n");
		goto error1;
	} else {
		DSP_PRNT("register " DSP_NAME " to char divece(%d)\n",
			  AUDIODSP_MAJOR);
	}
	priv->class = class_create(THIS_MODULE, DSP_DRIVER_NAME);
	if (priv->class == NULL) {
		DSP_PRNT("class_create create error\n");
		res = -EEXIST;
		goto error2;
	}
	priv->dev = device_create(priv->class,
					    NULL, MKDEV(AUDIODSP_MAJOR, 0),
					    NULL, "audiodsp0");
	if(priv->dev==NULL)
		{
		res = -EEXIST;
		goto error3;
		}
	audiodsp_init_mailbox(priv);
	init_audiodsp_monitor(priv);
#ifdef CONFIG_AM_STREAMING	
	set_adec_func(audiodsp_get_status);
#endif
    memset((void*)DSP_REG_OFFSET,0,REG_MEM_SIZE);
    
	return res;

//error4:
	device_destroy(priv->class, MKDEV(AUDIODSP_MAJOR, 0));
error3:
	class_destroy(priv->class);
error2:
	unregister_chrdev(AUDIODSP_MAJOR, DSP_NAME);
error1:
	kfree(priv);
	return res;
}

struct audiodsp_priv *audiodsp_privdata(void)
{
	return audiodsp_p;
}


static int __init audiodsp_init_module(void)
{

	return audiodsp_probe();
}

static void __exit audiodsp_exit_module(void)
{
	struct audiodsp_priv *priv;
	priv=audiodsp_privdata();
#ifdef CONFIG_AM_STREAMING		
	set_adec_func(NULL);
#endif
	dsp_stop(priv);
	stop_audiodsp_monitor(priv);
	audiodsp_release_mailbox(priv);
	release_audiodsp_monitor(priv);
	auidodsp_microcode_free(priv);
    iounmap(priv->p);
	device_destroy(priv->class, MKDEV(AUDIODSP_MAJOR, 0));
	class_destroy(priv->class);
	unregister_chrdev(AUDIODSP_MAJOR, DSP_NAME);
	kfree(priv);
	priv=NULL;
	return;
}

module_init(audiodsp_init_module);
module_exit(audiodsp_exit_module);
