/*****************************************************************
**                                                              **
**  Copyright (C) 2004 Amlogic,Inc.                             **
**  All rights reserved                                         **
**        Filename : cardreader.c /Project:  driver         	**
**        Revision : 1.0                                        **
**                                                              **
*****************************************************************/  
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/pagemap.h>
#include <linux/platform_device.h>
#include <linux/cardreader/cardreader.h>
#include <linux/cardreader/card_block.h>
    
#include <mach/am_regs.h>
#include <mach/irqs.h>
#include <mach/card_io.h>

#define card_list_to_card(l)	container_of(l, struct memory_card, node)
static DEFINE_MUTEX(init_lock);

struct amlogic_card_host 
{
	struct card_host *card;
	int present;
 
	/*
	* Flag indicating when the command has been sent. This is used to
	* work out whether or not to send the stop
	*/ 
	unsigned int flags;
	/* flag for current bus settings */ 
	unsigned bus_mode;
	/* Latest in the scatterlist that has been enabled for transfer, but not freed */ 
	int in_use_index;
	/* Latest in the scatterlist that has been enabled for transfer */ 
	int transfer_index;
};

CARD_READER_MONITOR cr_mon;

//wait_queue_head_t     sdio_wait_event;

static int card_reader_monitor(void *arg);
void card_detect_change(struct card_host *host, unsigned long delay);
struct card_host *card_alloc_host(int extra, struct device *dev);
static void card_setup(struct card_host *host);
static void amlogic_card_request(struct card_host *card, struct card_blk_request *brq);
static struct memory_card *card_find_card(struct card_host *host, u32 * raw_cid);

void card_reader_initialize(struct card_host *card) 
{
	unsigned card_num;

	for (card_num = CARD_XD_PICTURE; card_num < CARD_MAX_UNIT; card_num++) {	
		cr_mon.unit_state[card_num] = CARD_UNIT_NOT_READY;		
		cr_mon.blk_length[card_num] = 0;	
		cr_mon.capacity[card_num] = 0;	
	}
	
	cr_mon.card_detector[CARD_TYPE_UNKNOW] = NULL;	
	cr_mon.card_insert_process[CARD_TYPE_UNKNOW] = NULL;	
	cr_mon.card_remove_process[CARD_TYPE_UNKNOW] = NULL;	
	cr_mon.card_read_data[CARD_TYPE_UNKNOW] = NULL;	
	cr_mon.card_write_data[CARD_TYPE_UNKNOW] = NULL;	
	strcpy(cr_mon.name[CARD_TYPE_UNKNOW], CARD_UNKNOW_NAME_STR);	//   "/dev/disk/xx"

}

static int card_reader_init(struct card_host *card) 
{	
	int ret;	

	card_reader_initialize(card);	
	ret = kernel_thread(card_reader_monitor, card, CLONE_KERNEL | SIGCHLD);
	if (ret < 0)	
		printk("card creat process failed\n");
	else	
		printk("card creat process sucessful\n");
	return 0;
} 

static int card_reader_monitor(void *data)
{
    unsigned card_type, card_4in1_init_type;
    struct card_host *card_host = (struct card_host *)data;
    struct memory_card *card;
    card_4in1_init_type = 0;

	daemonize("card_read_monitor");
	
	while(1) {
		msleep(200);

		mutex_lock(&init_lock);
		for(card_type=CARD_XD_PICTURE; card_type<CARD_MAX_UNIT; card_type++) {
			if(cr_mon.card_detector[card_type] && (!cr_mon.card_power_off_flag[card_type]))
				cr_mon.card_status[card_type] = (*cr_mon.card_detector[card_type])();

	    	if((cr_mon.card_status[card_type] == CARD_INSERTED) && (cr_mon.unit_state[card_type] != CARD_UNIT_READY) && ((card_type == CARD_COMPACT_FLASH) ||(cr_mon.slot_detector == CARD_REMOVED)||(cr_mon.card_slot_mode == CARD_SLOT_DISJUNCT))) {
				if(cr_mon.card_insert_process[card_type])
					cr_mon.unit_state[card_type] = (*cr_mon.card_insert_process[card_type])();
				else
					cr_mon.unit_state[card_type] = CARD_UNIT_READY;

				if(cr_mon.unit_state[card_type] == CARD_UNIT_PROCESSED) {
					if(cr_mon.card_slot_mode == CARD_SLOT_4_1) {
	                	if (card_type != CARD_COMPACT_FLASH) {
	                		cr_mon.slot_detector = CARD_INSERTED;
	                		card_4in1_init_type = card_type;
	                	}
					}
					cr_mon.unit_state[card_type] = CARD_UNIT_READY;
					cr_mon.card_register_flag[card_type] = CARD_REGISTERED;
					card_host->card_type = card_type;
					card_detect_change(card_host, 0);
	            }
	        }
	        else if((cr_mon.card_status[card_type] == CARD_REMOVED) && (cr_mon.unit_state[card_type] != CARD_UNIT_NOT_READY)) {
				if(cr_mon.card_slot_mode == CARD_SLOT_4_1) {                       
					if (card_type == card_4in1_init_type) 
						cr_mon.slot_detector = CARD_REMOVED;
				}

				if(cr_mon.card_remove_process[card_type])
					cr_mon.unit_state[card_type] = (*cr_mon.card_remove_process[card_type])();
				else
					cr_mon.unit_state[card_type] = CARD_UNIT_NOT_READY;

				if(cr_mon.unit_state[card_type] == CARD_UNIT_PROCESSED) {
					cr_mon.unit_state[card_type] = CARD_UNIT_NOT_READY;

					if(cr_mon.card_register_flag[card_type] == CARD_REGISTERED) {
						cr_mon.card_register_flag[card_type] = CARD_UNREGISTERED;
						card = card_find_card(card_host, cr_mon.raw_cid[card_type]);
						if(card) {
							list_del(&card->node);
							card_remove_card(card);
						}
					}
					
	            }
	        }
		}
		mutex_unlock(&init_lock);
	}

    return 0;
}

static void card_deselect_cards(struct card_host *host) 
{	
	if (host->card_selected)		
		host->card_selected = NULL;	
}

/*
 * Check whether cards we already know about are still present.
 * We do this by requesting status, and checking whether a card
 * responds.
 *
 * A request for status does not cause a state change in data
 * transfer mode.
 */ 
static void card_check_cards(struct card_host *host) 
{
	//struct list_head *l, *n;
	card_deselect_cards(host);

	/*list_for_each_safe(l, n, &host->cards)
	{
		struct memory_card *card = card_list_to_card(l);
	       
		if(card->card_type == host->card_type)
	       continue;
	       
		card->state = CARD_STATE_DEAD;
	} */ 
} 

int __card_claim_host(struct card_host *host, struct memory_card *card)
{
	DECLARE_WAITQUEUE(wait, current);
	unsigned long flags;
	int err = 0;

	add_wait_queue(&host->wq, &wait);
	spin_lock_irqsave(&host->lock, flags);
	while (1) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		if (host->card_busy == NULL)
			break;
		spin_unlock_irqrestore(&host->lock, flags);
		schedule();
		spin_lock_irqsave(&host->lock, flags);
	}
	set_current_state(TASK_RUNNING);
	host->card_busy = card;
	spin_unlock_irqrestore(&host->lock, flags);
	remove_wait_queue(&host->wq, &wait);

	if (card != (void *)-1) {
		host->card_selected = card;
	}

	return err;
}

EXPORT_SYMBOL(__card_claim_host);

/*
 * Locate a Memory card on this Memory host given a raw CID.
 */ 
static struct memory_card *card_find_card(struct card_host *host, u32 * raw_cid) 
{
	struct memory_card *card;
	
	list_for_each_entry(card, &host->cards, node) {
		if (memcmp(card->raw_cid, raw_cid, sizeof(card->raw_cid)) == 0)		
			return card;
	}

	return NULL;
}

static struct memory_card *card_alloc_card(struct card_host *host, u32 * raw_cid) 
{
	struct memory_card *card;

	card = kmalloc(sizeof(struct memory_card), GFP_KERNEL);
	if (!card)
		return ERR_PTR(-ENOMEM);

	card_init_card(card, host);
	memcpy(card->raw_cid, raw_cid, sizeof(card->raw_cid));

	return card;
}

/**
 *	card_add_host - initialise host hardware
 *	@host: card host
 */ 
int card_add_host(struct card_host *host) 
{
	int ret;
	ret = card_add_host_sysfs(host);
	/*if (ret == 0)
	{
		card_detect_change(host, 0);
	} */ 
    
	return ret;
}

/**
 *	card_remove_host - remove host hardware
 *	@host: card host
 *
 *	Unregister and remove all cards associated with this host,
 *	and power down the CARD bus.
 */ 
void card_remove_host(struct card_host *host) 
{
	struct list_head *l, *n;

	list_for_each_safe(l, n, &host->cards) {	
		struct memory_card *card = card_list_to_card(l);
		card_remove_card(card);
	} 

	card_remove_host_sysfs(host);
} 

/**
 *	card_free_host - free the host structure
 *	@host: card host
 *
 *	Free the host once all references to it have been dropped.
 */ 
void card_free_host(struct card_host *host) 
{
	card_flush_scheduled_work();

	card_free_host_sysfs(host);
} 

static void card_discover_cards(struct card_host *host) 
{
	struct memory_card *card;
	int err;
	
	card = card_find_card(host, cr_mon.raw_cid[host->card_type]);
	if (!card) {	
		card = card_alloc_card(host, cr_mon.raw_cid[host->card_type]);
		if (IS_ERR(card)) {	
			err = PTR_ERR(card);		
		}	
		list_add(&card->node, &host->cards);
	}

	card->capacity = cr_mon.capacity[host->card_type];
	card->card_type = host->card_type;	
	card->state &= (~CARD_STATE_DEAD);
}

static void card_setup(struct card_host *host) 
{
	card_discover_cards(host);
} 

/**
 *	card_release_host - release a host
 *	@host: card host to release
 *
 *	Release a CARD host, allowing others to claim the host
 *	for their operations.
 */ 
void card_release_host(struct card_host *host) 
{
	unsigned long flags;

	BUG_ON(host->card_busy == NULL);
	spin_lock_irqsave(&host->lock, flags);
	host->card_busy = NULL;
	spin_unlock_irqrestore(&host->lock, flags);
	wake_up(&host->wq);
} 

static void card_reader_rescan(struct work_struct *work) 
{	
	struct list_head *l, *n;
	struct card_host *host = container_of(work, struct card_host, detect);

	card_claim_host(host);

	card_setup(host);

	card_check_cards(host);

	card_release_host(host);

	list_for_each_safe(l, n, &host->cards) {
		struct memory_card *card = card_list_to_card(l);
		/*
		* If this is a new and good card, register it.
		*/ 
		if ((!(card->state & CARD_STATE_PRESENT)) && (!(card->state & CARD_STATE_DEAD))) {
			if (card_register_card(card, cr_mon.name[host->card_type]))	
				card->state = CARD_STATE_DEAD;
			else	
				card->state = CARD_STATE_PRESENT;	
		}
		
		/*
		* If this card is dead, destroy it.
		*/ 
		if (card->state == CARD_STATE_DEAD) {	
			list_del(&card->node);
			card_remove_card(card);
		}
	}
}

struct card_host *card_alloc_host(int extra, struct device *dev) 
{	
	struct card_host *host;

	host = card_alloc_host_sysfs(extra, dev);	
	if (host) {	
		spin_lock_init(&host->lock);
		init_waitqueue_head(&host->wq);	
		INIT_LIST_HEAD(&host->cards);
		INIT_WORK(&host->detect, card_reader_rescan);
		
		    /*
		     * By default, hosts do not support SGIO or large requests.
		     * They have to set these according to their abilities.
		     */ 
		host->max_hw_segs = 1;
		host->max_phys_segs = 1;
		host->max_sectors = 1 << (PAGE_CACHE_SHIFT - 5);
		host->max_seg_size = PAGE_CACHE_SIZE;
		
		host->max_req_size = 512*256;	/*for CONFIG_CARD_BLOCK_BOUNCE fix me*/
		printk("card max_req_size is %dK \n", host->max_req_size/1024);
	}

	return host;

}

int card_wait_for_req(struct card_host *host, struct card_blk_request *brq) 
{
	WARN_ON(host->card_busy == NULL);
	host->ops->request(host, brq);	
	return 0;
}
EXPORT_SYMBOL(card_wait_for_req);

/**
 *	card_detect_change - process change of state on a memory card socket
 *	@host: host which changed state.
 *	@delay: optional delay to wait before detection (jiffies)
 *
 *	All we know is that card(s) have been inserted or removed
 *	from the socket(s).  We don't know which socket or cards.
 */ 
void card_detect_change(struct card_host *host, unsigned long delay) 
{
	/*if (delay)
		card_schedule_delayed_work(&host->detect, delay);
	else */ 
	card_schedule_work(&host->detect);
} 
EXPORT_SYMBOL(card_detect_change);

static struct card_host_ops amlogic_card_ops = { 
	.request = amlogic_card_request, 
};

static int amlogic_card_probe(struct platform_device *pdev) 
{
	struct card_host *card;
	struct amlogic_card_host *host;
	int ret;	

	card = card_alloc_host(sizeof(struct amlogic_card_host), &pdev->dev);
	if (!card) {	
		printk("Failed to allocate card host\n");	
		return -ENOMEM;	
	}

	card->ops = &amlogic_card_ops;
	host = card_priv(card);	
	host->card = card;	
	host->bus_mode = 0;
	//host->board = pdev->dev.platform_data;
  
	platform_set_drvdata(pdev, card);
	/*
	* Add host to CARD layer
	*/ 
	ret = card_add_host(card);
	/*
	* monitor card insertion/removal if we can
	*/ 
	ret = card_reader_init(card);
	if (ret) {
		card_free_host(card);
		return ret;	
	}

	return 0;
}

/*
 * Remove a device
 */ 
static int amlogic_card_remove(struct platform_device *pdev) 
{
	struct card_host *card = platform_get_drvdata(pdev);
	struct amlogic_card_host *host;
	
	if (!card)	
		return -1;

	host = card_priv(card);
	card_remove_host(card);	
	card_free_host(card);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static void amlogic_card_request(struct card_host *card, struct card_blk_request *brq)
{
	int ret;
	unsigned int lba, byte_cnt;
	unsigned char *data_buf;

	lba = brq->card_data.lba;
	byte_cnt = brq->card_data.blk_size * brq->card_data.blk_nums;
	data_buf = brq->crq.buf;

	mutex_lock(&init_lock);
	if(brq->crq.cmd == READ) {
		ret = cr_mon.card_read_data[card->card_type](lba, byte_cnt, data_buf);
				brq->card_data.error = ret;
	}
	else if(brq->crq.cmd == WRITE) {
		ret = cr_mon.card_write_data[card->card_type](lba, byte_cnt, data_buf);
				brq->card_data.error = ret;
	}
	mutex_unlock(&init_lock);
}

static struct platform_driver amlogic_card_driver = { 
	.probe = amlogic_card_probe, 
	.remove = amlogic_card_remove, 
	.driver =
	    {
			.name = "AMLOGIC_CARD", 
			.owner = THIS_MODULE, 
		}, 
};

static int __init amlogic_card_init(void) 
{	
	return platform_driver_register(&amlogic_card_driver);
}

static void __exit amlogic_card_exit(void) 
{	
	platform_driver_unregister(&amlogic_card_driver);
} 

module_init(amlogic_card_init);


module_exit(amlogic_card_exit);


MODULE_DESCRIPTION("Amlogic Memory Card Interface driver");

MODULE_LICENSE("GPL");

