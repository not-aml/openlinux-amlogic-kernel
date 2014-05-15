
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/slab.h>

//#include <asm/dsp/audiodsp_control.h>
#include "audiodsp_control.h"

#include <linux/firmware.h>
#include <linux/major.h>
#include <linux/device.h>


#include "audiodsp_module.h"
#include "dsp_control.h"
#include "dsp_microcode.h"
#include <linux/dma-mapping.h>

static  int auidodsp_microcode_insert(struct audiodsp_priv*priv,struct auidodsp_microcode *pmcode)
{
	unsigned long flags;
	
	if(pmcode==NULL)
		return -1;
	spin_lock_irqsave(&priv->mcode_lock, flags);
	list_add_tail(&pmcode->list,&priv->mcode_list);
	priv->mcode_id++;
	pmcode->id=priv->mcode_id;
	spin_unlock_irqrestore(&priv->mcode_lock, flags);
	return priv->mcode_id;
}

struct auidodsp_microcode *  audiodsp_find_supoort_mcode(struct audiodsp_priv*priv,int fmt)
{
	struct auidodsp_microcode *pmcode=NULL;
	struct auidodsp_microcode *p=NULL;
	unsigned long flags;
	spin_lock_irqsave(&priv->mcode_lock, flags);
	list_for_each_entry(p,&priv->mcode_list,list)
		{
			if(p->fmt & fmt)
			{
				pmcode=p;
				break;
			}
		}
	spin_unlock_irqrestore(&priv->mcode_lock, flags);
	return pmcode;
}
static struct auidodsp_microcode *  audiodsp_find_mcode_by_name(struct audiodsp_priv*priv,char *name)
{
	struct auidodsp_microcode *pmcode=NULL;
	struct auidodsp_microcode *p=NULL;
	unsigned long flags;
	spin_lock_irqsave(&priv->mcode_lock, flags);
	list_for_each_entry(p,&priv->mcode_list,list)
		{
			if(memcmp(p->file_name,name,strlen(name))==0)
			{
				pmcode=p;
				break;
			}
		}
	spin_unlock_irqrestore(&priv->mcode_lock, flags);
	return pmcode;
}

 int auidodsp_microcode_load(struct audiodsp_priv*priv,struct auidodsp_microcode *pmcode)
{
	
	const struct firmware *firmware;
	int err=0;
    unsigned dsp_code_text_start = 0;
	priv->micro_dev = device_create(priv->class,
					    NULL, MKDEV(AUDIODSP_MAJOR, 1),
					    NULL, "audiodsp1");
	if(priv->micro_dev ==NULL)
		return -1;
	if((err=request_firmware(&firmware,pmcode->file_name, priv->micro_dev))<0)
		{
		DSP_PRNT("can't load the %s,err=%d\n",pmcode->file_name,err);
		goto error1;
		}
	if(firmware->size>priv->code_mem_size)
		{
		DSP_PRNT("not enough memory size for audiodsp code\n");
		err=ENOMEM;
		goto release;
		}
    if(priv->dsp_is_started)
        dsp_code_text_start = 0x66c;//after dsp is running,only load from the text section of the microcode.
	memcpy((char *)((unsigned)priv->p+dsp_code_text_start), (char*)firmware->data+dsp_code_text_start,firmware->size-dsp_code_text_start);

	pmcode->code_size=firmware->size;
	DSP_PRNT("load mcode size=%d\n,load addr 0x%x mcode name %s",firmware->size,pmcode->code_start_addr,pmcode->file_name);
release:	
	release_firmware(firmware);

error1:
	device_destroy(priv->class, MKDEV(AUDIODSP_MAJOR, 1));
	return err;
}

 int auidodsp_microcode_register(struct audiodsp_priv*priv,int fmt,char *filename)
{
	struct auidodsp_microcode *pmcode;
	int len;
	pmcode=audiodsp_find_mcode_by_name(priv,filename);
	if(pmcode!=NULL)
		{
			DSP_PRNT("Have register the firmware code before=%s\n",filename);
			DSP_PRNT("Refresh the firmware settings now\n");
			pmcode->fmt=fmt;
			pmcode->code_start_addr=priv->dsp_code_start;
			len=min(64,(int)strlen(filename));
			memcpy(pmcode->file_name,filename,len);
			pmcode->file_name[len]='\0';
			return pmcode->id;
		}
	else
		{
			pmcode=kmalloc(sizeof(struct auidodsp_microcode ),GFP_KERNEL);
			if(pmcode==NULL)
				return -ENOMEM;
			pmcode->fmt=fmt;
			pmcode->code_start_addr=priv->dsp_code_start;
			len=min(64,(int)strlen(filename));
			memcpy(pmcode->file_name,filename,len);
			pmcode->file_name[len]='\0';
			pmcode->id=auidodsp_microcode_insert(priv,pmcode);
			if(pmcode->id<0)
				{
				kfree(pmcode);
				return -EIO;
				}

		}
	
	
	return 0;

}

  int auidodsp_microcode_free(struct audiodsp_priv*priv)
  {
  	unsigned long flags;
	struct auidodsp_microcode *pmcode;
	struct list_head  *list,*head;
	local_irq_save(flags);
	head=&priv->mcode_list;
	while(!list_empty(head))
		{
		list=head->prev;
		pmcode=list_entry(list, struct auidodsp_microcode, list);
		list_del(list);
		kfree(pmcode);
		}
	local_irq_restore(flags);
	return 0;
  }
