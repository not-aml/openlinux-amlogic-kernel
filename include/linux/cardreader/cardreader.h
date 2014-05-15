/*******************************************************************
 * 
 *  Copyright C 2005 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: 
 *
 *  Author: Eric Zhang
 *  Created: 2/20/2006
 *
 *******************************************************************/
#ifndef CARD__READER_H
#define CARD__READER_H

#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/list.h>
/**
 * @file cardreader.h
 * @addtogroup Card
 */
/*@{*/

#define CARD_INSERTED           1
#define CARD_REMOVED            0

#define CARD_REGISTERED         1
#define CARD_UNREGISTERED       0

#define CARD_UNIT_NOT_READY     0
#define CARD_UNIT_PROCESSING    1
#define CARD_UNIT_PROCESSED     2
#define CARD_UNIT_READY         3

#define CARD_EVENT_NOT_INSERTED     0
#define CARD_EVENT_INSERTED     	1
#define CARD_EVENT_POST_INSERTED    2
#define CARD_EVENT_NOT_REMOVED      0
#define CARD_EVENT_REMOVED     		1

typedef enum {
	CARD_XD_PICTURE = 0,
	CARD_MEMORY_STICK,
	CARD_MEMORY_STICK_MICRO,
    CARD_SECURE_DIGITAL,
    CARD_INAND,
    CARD_SMART_MEDIA,
    CARD_COMPACT_FLASH,   
    CARD_TYPE_UNKNOW
} CARD_TYPE_t;

#define CARD_MAX_UNIT           (CARD_TYPE_UNKNOW+1)

#define CARD_STRING_LEN			   13
#define CARD_MS_NAME_STR           "ms"
#define CARD_SD_NAME_STR           "sd"
#define CARD_XD_NAME_STR           "xd"
#define CARD_CF_NAME_STR           "cf"
#define CARD_UNKNOW_NAME_STR       "xx"
#define CARD_5IN1CARD_NAME_STR     "5in1"

 /**/ typedef unsigned char (*CARD_DETECTOR) (void);	// INSERTED: 1  REMOVED: 0
 /**/ typedef unsigned char (*CARD_PROCESS) (void);
 /**/ typedef int (*CARD_OPERATION) (unsigned long lba, unsigned long byte_cnt, unsigned char *data_buf);
 /**/ typedef int (*CARD_IOCTL) (unsigned dev, int req, void *argp);
    
typedef struct _card_reader_monitor {
	unsigned time;
	unsigned char slot_detector;
	unsigned card_slot_mode;

	unsigned char card_register_flag[CARD_MAX_UNIT];
	char card_power_off_flag[CARD_MAX_UNIT];
	char card_in_event_status[CARD_MAX_UNIT];
	char card_out_event_status[CARD_MAX_UNIT];
	char card_status[CARD_MAX_UNIT];
	unsigned blk_length[CARD_MAX_UNIT];	// block length byte
	unsigned capacity[CARD_MAX_UNIT];	// capacity in block unit
	unsigned char unit_state[CARD_MAX_UNIT];
	unsigned raw_cid[CARD_MAX_UNIT][4];
	char name[CARD_MAX_UNIT][CARD_STRING_LEN];	

	CARD_DETECTOR card_detector[CARD_MAX_UNIT];
	CARD_PROCESS card_insert_process[CARD_MAX_UNIT];
	CARD_PROCESS card_remove_process[CARD_MAX_UNIT];
	CARD_OPERATION card_read_data[CARD_MAX_UNIT];
	CARD_OPERATION card_write_data[CARD_MAX_UNIT];
}CARD_READER_MONITOR;
//extern wait_queue_head_t     sdio_wait_event;

extern CARD_READER_MONITOR cr_mon;
extern struct completion sdio_int_complete;
extern void sdio_open_host_interrupt(unsigned int_resource);
extern void sdio_clear_host_interrupt(unsigned int_resource);
extern unsigned sdio_check_interrupt(void);
extern void sdio_close_host_interrupt(unsigned int_resource);
extern void sdio_cmd_int_handle(void);
extern void sdio_timeout_int_handle(void);
/*@}*/
#endif // CARD__READER_H
