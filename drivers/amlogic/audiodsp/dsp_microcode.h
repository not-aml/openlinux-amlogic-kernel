
#ifndef AUDIODSP_MICROCODE_HEADER
#define AUDIODSP_MICROCODE_HEADER
#include "audiodsp_module.h"
struct auidodsp_microcode
{
	int 	id;
	int 	fmt;//support format;
	struct list_head list;
	unsigned long code_start_addr;
	unsigned long code_size;
	char file_name[64];
};



extern int auidodsp_microcode_register(struct audiodsp_priv*priv,int fmt,char *filename);
extern struct auidodsp_microcode *  audiodsp_find_supoort_mcode(struct audiodsp_priv*priv,int fmt);
extern int auidodsp_microcode_load(struct audiodsp_priv*priv,struct auidodsp_microcode *pmcode);
int auidodsp_microcode_free(struct audiodsp_priv*priv);
 #endif

