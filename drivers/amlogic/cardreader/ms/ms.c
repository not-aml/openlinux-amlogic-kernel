/*****************************************************************
**                                                              **
**  Copyright (C) 2004 Amlogic,Inc.                             **
**  All rights reserved                                         **
**        Filename : sd.c /Project:AVOS  driver                 **
**        Revision : 1.0                                        **
**                                                              **
*****************************************************************/
#include <linux/slab.h>

#include <asm/drivers/cardreader/cardreader.h>
#include <asm/drivers/cardreader/card_io.h>

#include "ms_mspro.h"

static MS_MSPRO_Card_Info_t ms_info;
extern MS_MSPRO_Card_Info_t *ms_mspro_info;

extern unsigned ms_force_write_two_times;

unsigned char ms_insert_detector(void)
{
	int ret = ms_mspro_check_insert();
	if(ret)
	{
        return CARD_INSERTED;
    }
    else
    {
        return CARD_REMOVED;
    }
}

extern unsigned char disable_port_switch;
unsigned char ms_open(void)
{
	int ret;
	
	ret = ms_mspro_init(&ms_info);
	if(ret)
	{
		disable_port_switch = 1;
	    ret = ms_mspro_init(&ms_info);
	}
	disable_port_switch = 0;
	if(ret)
		return CARD_UNIT_READY;
	else
		return CARD_UNIT_PROCESSED;
}

unsigned char ms_close(void)
{
	ms_mspro_exit();	
	return CARD_UNIT_PROCESSED;
}

unsigned char ms_read_info(u32 *blk_length, u32 *capacity, u32 *raw_cid)
{
	if(ms_info.inited_flag)
	{
		if(blk_length)
			*blk_length = 512;
		if(capacity)
			*capacity = ms_info.blk_nums;
		if(raw_cid)
			memcpy(raw_cid, &(ms_info.raw_cid), sizeof(ms_info.raw_cid));
		return 0;
	}
	else
		return 1;
}

int ms_ioctl(dev_t dev, int req, void *argp)
{
//	unsigned32 ret=0;
//    int errno;
//    blkdev_request1 *req1 =(blkdev_request1 *) argp;  
//    blkdev_request *r =&(req1->req); 
//    blkdev_sg_buffer* psgbuf ;
//    void * databuf;
//    avfs_status_code status;
//    INT32U capacity = 0;
//
//    switch (req)
//    {
//        case BLKIO_REQUEST:
//        {
//            psgbuf=&(r->bufs[0]);
//            databuf=psgbuf->buffer;
//            switch (r->req)
//            {
//                case BLKDEV_REQ_READ:
//                case BLKDEV_REQ_READ_DEV:
//                	card_get_dev();
//                    ret = ms_mspro_read_data(r->start,r->count*512,(INT8U *)databuf);
//                    if((ret)&&(ms_mspro_check_insert()))
//                    {
//                    	ms_mspro_info->inited_flag = 0;
//                    	ret = ms_mspro_init(&ms_info);
//                    	if(!ret)
//                    		ret = ms_mspro_read_data(r->start,r->count*512,(INT8U *)databuf);
//                    }
//                    card_put_dev();
//                    if(!ret)
//                        status=AVFS_SUCCESSFUL;
//                    else
//                        status=AVFS_IO_ERROR;                   
//                    if(r->req_done)
//                        r->req_done(r->done_arg, status, ret);       
//                    break;
//                case BLKDEV_REQ_WRITE:
//                case BLKDEV_REQ_WRITE_DEV:
//                	card_get_dev();
//                    ret = ms_mspro_write_data(r->start,r->count*512,(INT8U *)databuf);
//                    if((!ret) && ms_force_write_two_times)
//                    	ret = ms_mspro_write_data(r->start,r->count*512,(INT8U *)databuf);
//                    if((ret)&&(ms_mspro_check_insert()))
//                    {
//                    	ms_mspro_info->inited_flag = 0;
//                    	ret = ms_mspro_init(&ms_info);
//                    	if(!ret)
//                    		ret = ms_mspro_write_data(r->start,r->count*512,(INT8U *)databuf);
//                    }
//                    card_put_dev();
//                    if(!ret)
//                        status=AVFS_SUCCESSFUL;
//                    else
//                        status=AVFS_IO_ERROR;                       
//                    if(r->req_done)
//                        r->req_done(r->done_arg, status, ret);       
//                    break;
//                case BLKDEV_REQ_ASYREAD_DEV:
//                case BLKDEV_REQ_ASYREAD_MEM:
//                    break;
//                default:
//                    errno = EBADRQC;
//                    ret=-1;
//                    break;
//            }
//            break;
//        }
//        case BLKIO_GETSIZE:
//        {
//            ms_read_info(NULL, &capacity);
//            r->status = capacity;
//            break;
//        }
//        case BLKIO_GET_TYPE_STR:
//        {
//        	//if(ms_info.card_type == CARD_TYPE_MS)
//        		*(char **)argp = "MS";
//        	//else
//        	//	*(char **)argp = "MSPRO";
//        	break;
//    	}
//    	case BLKIO_GET_DEVSTAT:
//    	{
//    		blkdev_stat_t *info = (blkdev_stat_t *)argp;
//    		ms_mspro_get_info(info);
//    		break;
//    	}   	
//        default:
//        {
//            errno = EBADRQC;
//            ret=-1;
//            break;
//        }
//    }
    return 0;//ret; 
}

void ms_init(void)
{
	ms_io_init();
	ms_mspro_prepare_init();
}
