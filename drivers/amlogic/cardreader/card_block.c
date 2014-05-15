/*
 * Block driver for media (i.e., flash cards)
 */

#include <linux/moduleparam.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/hdreg.h>
#include <linux/kdev_t.h>
#include <linux/blkdev.h>
#include <linux/mutex.h>
#include <linux/scatterlist.h>
#include <linux/err.h>
#include <linux/genhd.h>
#include <linux/cardreader/card_block.h>

#include <asm/system.h>
#include <asm/uaccess.h>

#include <mach/am_regs.h>
#include <linux/dma-mapping.h>
#include <asm/cacheflush.h>
#include <linux/sched.h>

static int major;
#define CARD_SHIFT	3
#define CARD_QUEUE_EXIT		(1 << 0)
#define CARD_QUEUE_SUSPENDED	(1 << 1)

#define CARD_QUEUE_BOUNCESZ	(512*256)

#define CARD_NUM_MINORS	(256 >> CARD_SHIFT)
static unsigned long dev_use[CARD_NUM_MINORS / (8 * sizeof(unsigned long))];

static int card_blk_issue_rq(struct card_queue *cq, struct request *req);
static int card_blk_probe(struct memory_card *card);
static int card_blk_prep_rq(struct card_queue *cq, struct request *req);

struct card_blk_data {
	spinlock_t lock;
	struct gendisk *disk;
	struct card_queue queue;

	unsigned int usage;
	unsigned int block_bits;
	unsigned int read_only;
};

static DEFINE_MUTEX(open_lock);

static unsigned card_thread_sleep_flag = 0;
static struct completion card_thread_complete;
static wait_queue_head_t card_thread_wq;
static struct semaphore card_thread_sem;

struct card_queue_list {
	int cq_num;
	unsigned cq_flag;
	struct card_queue *cq;
	struct card_queue_list *cq_next;
};
static struct card_queue_list *card_queue_head = NULL;

void card_cleanup_queue(struct card_queue *cq)
{
	struct card_queue_list *cq_node_current = card_queue_head;
	struct card_queue_list *cq_node_prev = NULL;

	do {
		if (cq_node_current->cq == cq)
			break;

		cq_node_prev = cq_node_current;
		cq_node_current = cq_node_current->cq_next;
	} while (cq_node_current != NULL);

	if (cq_node_current == card_queue_head) {
		if (cq_node_current->cq_next == NULL) {
			cq->flags |= CARD_QUEUE_EXIT;
			wake_up_interruptible(&card_thread_wq);
			wait_for_completion(&card_thread_complete);
		} else {
			card_queue_head->cq_num = 0;
		}
		card_queue_head = card_queue_head->cq_next;
	} else {
		cq_node_prev->cq_next = cq_node_current->cq_next;
	}

	kfree(cq->sg);
	cq->sg = NULL;

	blk_cleanup_queue(cq->queue);

	cq->card = NULL;

	if (cq_node_current == card_queue_head)
		card_queue_head = NULL;
	kfree(cq_node_current);
	cq_node_current = NULL;
}

static struct card_blk_data *card_blk_get(struct gendisk *disk)
{
	struct card_blk_data *card_data;

	mutex_lock(&open_lock);
	card_data = disk->private_data;
	if (card_data && card_data->usage == 0)
		card_data = NULL;
	if (card_data)
		card_data->usage++;
	mutex_unlock(&open_lock);

	return card_data;
}

static void card_blk_put(struct card_blk_data *card_data)
{
	mutex_lock(&open_lock);
	card_data->usage--;
	if (card_data->usage == 0) {
		put_disk(card_data->disk);
		card_cleanup_queue(&card_data->queue);
		kfree(card_data);
	}
	mutex_unlock(&open_lock);
}

static int card_blk_open(struct block_device *bdev, fmode_t mode)
{
	struct card_blk_data *card_data;
	int ret = -ENXIO;

	card_data = card_blk_get(bdev->bd_disk);
	if (card_data) {
		if (card_data->usage == 2)
			check_disk_change(bdev);
		ret = 0;

		if ((mode & FMODE_WRITE) && card_data->read_only)
			ret = -EROFS;
	}

	return ret;
}

static int card_blk_release(struct gendisk *disk, fmode_t mode)
{
	struct card_blk_data *card_data = disk->private_data;

	card_blk_put(card_data);
	return 0;
}

static int card_blk_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	geo->cylinders = get_capacity(bdev->bd_disk) / (4 * 16);
	geo->heads = 4;
	geo->sectors = 16;
	return 0;
}

static struct block_device_operations card_ops = {
	.open = card_blk_open,
	.release = card_blk_release,
	.getgeo = card_blk_getgeo,
	.owner = THIS_MODULE,
};

static inline int card_claim_card(struct memory_card *card)
{
	if(cr_mon.card_status[card->card_type] == CARD_REMOVED)
		return ENODEV;
	return __card_claim_host(card->host, card);
}

static int card_prep_request(struct request_queue *q, struct request *req)
{
	struct card_queue *cq = q->queuedata;
	int ret = BLKPREP_KILL;

	if (blk_special_request(req)) {
		/*
		 * Special commands already have the command
		 * blocks already setup in req->special.
		 */
		BUG_ON(!req->special);

		ret = BLKPREP_OK;
	} else if (blk_fs_request(req) || blk_pc_request(req)) {
		/*
		 * Block I/O requests need translating according
		 * to the protocol.
		 */
		ret = cq->prep_fn(cq, req);
	} else {
		/*
		 * Everything else is invalid.
		 */
		blk_dump_rq_flags(req, "CARD bad request");
	}

	if (ret == BLKPREP_OK)
		req->cmd_flags |= REQ_DONTPREP;

	return ret;
}

static void card_request(struct request_queue *q)
{
	struct card_queue *cq = q->queuedata;
	struct card_queue_list *cq_node_current = card_queue_head;

	do {
		if (cq_node_current->cq == cq) {
			cq_node_current->cq_flag = 1;
			break;
		}

		cq_node_current = cq_node_current->cq_next;
	} while (cq_node_current != NULL);

	if (card_thread_sleep_flag) {
		card_thread_sleep_flag = 0;
		wake_up_interruptible(&card_thread_wq);
	}
}

void card_queue_suspend(struct card_queue *cq)
{
	struct request_queue *q = cq->queue;
	unsigned long flags;

	if (!(cq->flags & CARD_QUEUE_SUSPENDED)) {
		cq->flags |= CARD_QUEUE_SUSPENDED;

		spin_lock_irqsave(q->queue_lock, flags);
		blk_stop_queue(q);
		spin_unlock_irqrestore(q->queue_lock, flags);

		down(&card_thread_sem);
	}
}

void card_queue_resume(struct card_queue *cq)
{
	struct request_queue *q = cq->queue;
	unsigned long flags;

	if (cq->flags & CARD_QUEUE_SUSPENDED) {
		cq->flags &= ~CARD_QUEUE_SUSPENDED;

		up(&card_thread_sem);

		spin_lock_irqsave(q->queue_lock, flags);
		blk_start_queue(q);
		spin_unlock_irqrestore(q->queue_lock, flags);
	}
}

static int card_queue_thread(void *d)
{
	struct card_queue *cq = d;
	struct request_queue *q = cq->queue;
	struct card_queue_list *cq_node_current;
	DECLARE_WAITQUEUE(wait, current);

	daemonize("card_queue_thread");
	/*
	 * Set iothread to ensure that we aren't put to sleep by
	 * the process freezing.  We handle suspension ourselves.
	 */
	current->flags |= PF_MEMALLOC;

	complete(&card_thread_complete);

	down(&card_thread_sem);
	add_wait_queue(&card_thread_wq, &wait);
	do {
		struct request *req = NULL;

		spin_lock_irq(q->queue_lock);
		cq_node_current = card_queue_head;
		while (cq_node_current != NULL) {
			cq = cq_node_current->cq;
			q = cq->queue;
			if (cq_node_current->cq_flag) {
				req = blk_fetch_request(q);
				if (req)
					break;

				cq_node_current->cq_flag = 0;
			}

			cq_node_current = cq_node_current->cq_next;
		}

		cq->req = req;
		spin_unlock_irq(q->queue_lock);

		if (!req) {
			if (cq->flags & CARD_QUEUE_EXIT)
				break;

			if (cq_node_current == NULL) {
				up(&card_thread_sem);
				card_thread_sleep_flag = 1;
				interruptible_sleep_on(&card_thread_wq);
				//schedule();
				down(&card_thread_sem);
			}
			continue;
		}

		cq->issue_fn(cq, req);
	} while (1);
	remove_wait_queue(&card_thread_wq, &wait);
	up(&card_thread_sem);

	complete_and_exit(&card_thread_complete, 0);

	return 0;
}

#define CONFIG_CARD_BLOCK_BOUNCE 1

#ifdef CONFIG_CARD_BLOCK_BOUNCE

static dma_addr_t card_desc[PAGE_SIZE>>2];
/*
 * ndmacpy - memcpy with ndma module
 */
void card_ndma_copy(void *to, void *from, size_t n, int to_buffer)
{
	dma_addr_t dma_desc=0;

	BUG_ON(n > CARD_QUEUE_BOUNCESZ);
	
	// Add a descriptor to SDRAM at locations vdesc
	//CONTROL: Descriptor Entry 0
	card_desc[0] = (0<<30)|(1<<8);/*thread id 0 & no break*/
	//SOURCE POINTER: Descriptor Entry 1
	card_desc[1] = (dma_addr_t)from;
	//DESTINATION POINTER: Descriptor Entry 2
	card_desc[2] = (dma_addr_t)to;
	//Bytes to Transfer: Descriptor Entry 3
	card_desc[3] = n;

	dmac_map_area(card_desc, PAGE_SIZE, DMA_TO_DEVICE);
	dma_desc = virt_to_dma(NULL, card_desc);

	dmac_map_area(phys_to_virt(from), n, DMA_FROM_DEVICE);
	dmac_map_area(phys_to_virt(to), n, DMA_FROM_DEVICE);
	
	//reset table add reg
	WRITE_CBUS_REG(NDMA_TABLE_ADD_REG, 0);
	
	//set thread 0 start descriptor addr
	WRITE_CBUS_REG(NDMA_THREAD_TABLE_START0, dma_desc);
	//set thread 0 end descriptor addr
	WRITE_CBUS_REG(NDMA_THREAD_TABLE_END0, dma_desc+0x10);
	
	//init thread 0
	WRITE_CBUS_REG(NDMA_THREAD_REG, 0x1<<24|0x1<<8|1);

	// Indicate we just added a descriptor
	WRITE_CBUS_REG(NDMA_TABLE_ADD_REG, 1);
	// Enable the DMA Engine
	WRITE_CBUS_REG(NDMA_CNTL_REG0, 1<<NDMA_ENABLE| 0x63<<16);

	// Wait for DMA to complete (or use interrupts)
	while(READ_CBUS_REG(NDMA_CNTL_REG0) & (1<<NDMA_STATUS)){
		asm("nop");
	}
	
	return ;
}

/**
 * sg_miter_next - proceed mapping iterator to the next mapping
 * @miter: sg mapping iter to proceed
 *
 * Description:
 *   Proceeds @miter@ to the next mapping.  @miter@ should have been
 *   started using sg_miter_start().  On successful return,
 *   @miter@->page, @miter@->addr and @miter@->length point to the
 *   current mapping.
 *
 * Context:
 *   IRQ disabled if SG_MITER_ATOMIC.  IRQ must stay disabled till
 *   @miter@ is stopped.  May sleep if !SG_MITER_ATOMIC.
 *
 * Returns:
 *   true if @miter contains the next mapping.  false if end of sg
 *   list is reached.
 */
bool card_sg_miter_next(struct sg_mapping_iter *miter)
{
	unsigned int off, len;

	/* check for end and drop resources from the last iteration */
	if (!miter->__nents)
		return false;

	sg_miter_stop(miter);

	/* get to the next sg if necessary.  __offset is adjusted by stop */
	while (miter->__offset == miter->__sg->length) {
		if (--miter->__nents) {
			miter->__sg = sg_next(miter->__sg);
			miter->__offset = 0;
		} else
			return false;
	}

	/* map the next page */
	off = miter->__sg->offset + miter->__offset;
	len = miter->__sg->length - miter->__offset;

	miter->page = nth_page(sg_page(miter->__sg), off >> PAGE_SHIFT);
	off &= ~PAGE_MASK;
	//miter->length = min_t(unsigned int, len, PAGE_SIZE - off);
	/*no need to copy every page, so copy every sg->length*/
	miter->length = len;
	
	miter->consumed = miter->length;

	if (miter->__flags & SG_MITER_ATOMIC)
		miter->addr = kmap_atomic(miter->page, KM_BIO_SRC_IRQ) + off;
	else
		miter->addr = kmap(miter->page) + off;

	return true;
}

static size_t card_sg_copy_buffer(struct scatterlist *sgl, unsigned int nents,
			     void *buf, size_t buflen, int to_buffer)
{
	unsigned int offset = 0;
	struct sg_mapping_iter miter;
	unsigned long flags;
	unsigned int sg_flags = SG_MITER_ATOMIC;

	if (to_buffer)
		sg_flags |= SG_MITER_FROM_SG;
	else
		sg_flags |= SG_MITER_TO_SG;

	sg_miter_start(&miter, sgl, nents, sg_flags);

	local_irq_save(flags);

	while (card_sg_miter_next(&miter) && offset < buflen) {
		unsigned int len;

		len = min(miter.length, buflen - offset);

		if (to_buffer)
			card_ndma_copy(buf + offset, virt_to_phys(miter.addr), len, to_buffer);
		else
			card_ndma_copy(virt_to_phys(miter.addr), buf + offset, len, to_buffer);

		offset += len;
	}
	
	sg_miter_stop(&miter);

	local_irq_restore(flags);
	
	return offset;
}


/**
 * sg_copy_from_buffer_for_sd - Copy from a linear buffer to an SG list
 * @sgl:		 The SG list
 * @nents:		 Number of SG entries
 * @buf:		 Where to copy from
 * @buflen:		 The number of bytes to copy
 *
 * Returns the number of copied bytes.
 *
 **/
static size_t card_sg_copy_from_buffer(struct scatterlist *sgl, unsigned int nents,
			   void *buf, size_t buflen)
{
	return card_sg_copy_buffer(sgl, nents, buf, buflen, 0);
}

/**
 * sg_copy_to_buffer - Copy from an SG list to a linear buffer
 * @sgl:		 The SG list
 * @nents:		 Number of SG entries
 * @buf:		 Where to copy to
 * @buflen:		 The number of bytes to copy
 *
 * Returns the number of copied bytes.
 *
 **/
static size_t card_sg_copy_to_buffer(struct scatterlist *sgl, unsigned int nents,
			 void *buf, size_t buflen)
{
	return card_sg_copy_buffer(sgl, nents, buf, buflen, 1);
}



/*
 * Prepare the sg list(s) to be handed of to the host driver
 */
static unsigned int card_queue_map_sg(struct card_queue *cq)
{
	unsigned int sg_len;
	size_t buflen;
	struct scatterlist *sg;
	int i;

	if (!cq->bounce_buf)
		return blk_rq_map_sg(cq->queue, cq->req, cq->sg);

	BUG_ON(!cq->bounce_sg);

	sg_len = blk_rq_map_sg(cq->queue, cq->req, cq->bounce_sg);

	cq->bounce_sg_len = sg_len;

	buflen = 0;
	for_each_sg(cq->bounce_sg, sg, sg_len, i)
		buflen += sg->length;

	sg_init_one(cq->sg, cq->bounce_buf, buflen);

	return 1;
}

extern unsigned char *sd_mmc_phy_buf;

/*
 * If writing, bounce the data to the buffer before the request
 * is sent to the host driver
 */
static void card_queue_bounce_pre(struct card_queue *cq)
{
	unsigned long flags;

	if (!cq->bounce_buf || (cq->bounce_sg_len==1))
		return;

	if (rq_data_dir(cq->req) != WRITE)
		return;
	
	local_irq_save(flags);
		
	card_sg_copy_to_buffer(cq->bounce_sg, cq->bounce_sg_len,
		sd_mmc_phy_buf, cq->sg[0].length);	
	
	local_irq_restore(flags);
}

/*
 * If reading, bounce the data from the buffer after the request
 * has been handled by the host driver
 */
static void card_queue_bounce_post(struct card_queue *cq)
{
	unsigned long flags;
	
	if (!cq->bounce_buf || (cq->bounce_sg_len==1))
		return;

	if (rq_data_dir(cq->req) != READ)
		return;

	local_irq_save(flags);
	
	card_sg_copy_from_buffer(cq->bounce_sg, cq->bounce_sg_len,
		sd_mmc_phy_buf, cq->sg[0].length);
		
	local_irq_restore(flags);
	
	bio_flush_dcache_pages(cq->req->bio);
}

extern unsigned char *sd_mmc_buf;
/*
 * Alloc bounce buf for read/write numbers of pages in one request
 */
static int card_init_bounce_buf(struct card_queue *cq, 
			struct memory_card *card)
{
	int ret=0;
	unsigned int bouncesz;

	bouncesz = CARD_QUEUE_BOUNCESZ;

	if (bouncesz >= PAGE_CACHE_SIZE) {
	//	cq->bounce_buf = kmalloc(bouncesz, GFP_KERNEL);
		cq->bounce_buf = sd_mmc_buf;
		if (!cq->bounce_buf) {
			printk(KERN_WARNING "%s: unable to "
				"allocate bounce buffer\n", card->name);
		}
	}

	if (cq->bounce_buf) {
		blk_queue_bounce_limit(cq->queue, BLK_BOUNCE_HIGH);
		blk_queue_max_hw_sectors(cq->queue, bouncesz / 512);
		blk_queue_physical_block_size(cq->queue, bouncesz);
		blk_queue_max_segments(cq->queue, bouncesz / PAGE_CACHE_SIZE);
		blk_queue_max_segment_size(cq->queue, bouncesz);

		cq->queue->queuedata = cq;
		cq->req = NULL;
	
		cq->sg = kmalloc(sizeof(struct scatterlist),
			GFP_KERNEL);
		if (!cq->sg) {
			ret = -ENOMEM;
			blk_cleanup_queue(cq->queue);
			return ret;
		}
		sg_init_table(cq->sg, 1);

		cq->bounce_sg = kmalloc(sizeof(struct scatterlist) *
			bouncesz / PAGE_CACHE_SIZE, GFP_KERNEL);
		if (!cq->bounce_sg) {
			ret = -ENOMEM;
			kfree(cq->sg);
			cq->sg = NULL;
			blk_cleanup_queue(cq->queue);
			return ret;
		}
		sg_init_table(cq->bounce_sg, bouncesz / PAGE_CACHE_SIZE);
	}

	return 0;
}

#else

static unsigned int card_queue_map_sg(struct card_queue *cq)
{
}

static void card_queue_bounce_pre(struct card_queue *cq)
{
}

static void card_queue_bounce_post(struct card_queue *cq)
{
}

static int card_init_bounce_buf(struct card_queue *cq, 
			struct memory_card *card)
{
}

#endif

int card_init_queue(struct card_queue *cq, struct memory_card *card,
		    spinlock_t * lock)
{
	struct card_host *host = card->host;
	u64 limit = BLK_BOUNCE_HIGH;
	int ret, card_quene_num;
	struct card_queue_list *cq_node_current;
	struct card_queue_list *cq_node_prev = NULL;

	if (host->parent->dma_mask && *host->parent->dma_mask)
		limit = *host->parent->dma_mask;

	cq->card = card;
	cq->queue = blk_init_queue(card_request, lock);
	if (!cq->queue)
		return -ENOMEM;

	blk_queue_prep_rq(cq->queue, card_prep_request);
	card_init_bounce_buf(cq, card);
	
	if(!cq->bounce_buf){
		blk_queue_bounce_limit(cq->queue, limit);
		blk_queue_max_hw_sectors(cq->queue, host->max_sectors);
		//blk_queue_max_hw_phys_segments(cq->queue, host->max_phys_segs);
		blk_queue_max_segments(cq->queue, host->max_hw_segs);
		blk_queue_max_segment_size(cq->queue, host->max_seg_size);

		cq->queue->queuedata = cq;
		cq->req = NULL;

		cq->sg = kmalloc(sizeof(struct scatterlist) * host->max_phys_segs, GFP_KERNEL);
		if (!cq->sg) {
			ret = -ENOMEM;
			blk_cleanup_queue(cq->queue);
			return ret;
		}
	}

	if (card_queue_head == NULL)
	{
		card_queue_head = kmalloc(sizeof(struct card_queue_list), GFP_KERNEL);
		if (card_queue_head == NULL) 
		{
			ret = -ENOMEM;
			kfree(card_queue_head);
			card_queue_head = NULL;
			return ret;
		}
		card_queue_head->cq = cq;
		card_queue_head->cq_num = 0;
		card_queue_head->cq_flag = 0;
		card_queue_head->cq_next = NULL;

		init_completion(&card_thread_complete);
		init_waitqueue_head(&card_thread_wq);
		init_MUTEX(&card_thread_sem);
		ret = kernel_thread(card_queue_thread, cq, CLONE_KERNEL | SIGCHLD);
		if (ret >= 0)
		{
			wait_for_completion(&card_thread_complete);
			init_completion(&card_thread_complete);
			ret = 0;
			return ret;
		}
	} 
	else
	{
		card_quene_num = 0;
		cq_node_current = card_queue_head;
		do
		{
			card_quene_num = cq_node_current->cq_num;
			cq_node_prev = cq_node_current;
			cq_node_current = cq_node_current->cq_next;
		} while (cq_node_current != NULL);

		cq_node_current = kmalloc(sizeof(struct card_queue_list), GFP_KERNEL);
		if (cq_node_current == NULL)
		{
			ret = -ENOMEM;
			kfree(cq_node_current);
			cq_node_current = NULL;
			return ret;
		}
		cq_node_prev->cq_next = cq_node_current;
		cq_node_current->cq = cq;
		cq_node_current->cq_next = NULL;
		cq_node_current->cq_num = (++card_quene_num);
		cq_node_current->cq_flag = 0;

		ret = 0;
		return ret;
	}

	return ret;
}

static struct card_blk_data *card_blk_alloc(struct memory_card *card)
{
	struct card_blk_data *card_data;
	int devidx, ret;

	devidx = find_first_zero_bit(dev_use, CARD_NUM_MINORS);
	if (devidx >= CARD_NUM_MINORS)
		return ERR_PTR(-ENOSPC);
	__set_bit(devidx, dev_use);

	card_data = kmalloc(sizeof(struct card_blk_data), GFP_KERNEL);
	if (!card_data) {
		ret = -ENOMEM;
		return ERR_PTR(ret);
	}

	memset(card_data, 0, sizeof(struct card_blk_data));

	card_data->block_bits = 9;

	card_data->disk = alloc_disk(1 << CARD_SHIFT);
	if (card_data->disk == NULL) {
		ret = -ENOMEM;
		kfree(card_data);
		return ERR_PTR(ret);
	}

	spin_lock_init(&card_data->lock);
	card_data->usage = 1;

	ret = card_init_queue(&card_data->queue, card, &card_data->lock);
	if (ret) {
		put_disk(card_data->disk);
		return ERR_PTR(ret);
	}

	card_data->queue.prep_fn = card_blk_prep_rq;
	card_data->queue.issue_fn = card_blk_issue_rq;
	card_data->queue.data = card_data;

	card_data->disk->major = major;
	card_data->disk->minors = 1 << CARD_SHIFT;
	card_data->disk->first_minor = devidx << CARD_SHIFT;
	card_data->disk->fops = &card_ops;
	card_data->disk->private_data = card_data;
	card_data->disk->queue = card_data->queue.queue;
	card_data->disk->driverfs_dev = &card->dev;

	sprintf(card_data->disk->disk_name, "cardblk%s", card->name);

	blk_queue_logical_block_size(card_data->queue.queue, 1 << card_data->block_bits);

	set_capacity(card_data->disk, card->capacity);

	return card_data;
}

static int card_blk_prep_rq(struct card_queue *cq, struct request *req)
{
	struct card_blk_data *card_data = cq->data;
	int stat = BLKPREP_OK;

	/*
	 * If we have no device, we haven't finished initialising.
	 */
	if (!card_data || !cq->card) {
		printk(KERN_ERR "%s: killing request - no device/host\n", req->rq_disk->disk_name);
		stat = BLKPREP_KILL;
	}

	return stat;
}

static int card_blk_issue_rq(struct card_queue *cq, struct request *req)
{
	struct card_blk_data *card_data = cq->data;
	struct memory_card *card = card_data->queue.card;
	struct card_blk_request brq;
	int ret;

	//printk("card issue request %d sector num: %d\n", req->sector, req->nr_sectors);
	if (card_claim_card(card)) {
		spin_lock_irq(&card_data->lock);
		ret = 1;
		while (ret) {
			ret = __blk_end_request(req, -EIO, (1 << card_data->block_bits));
		}
		spin_unlock_irq(&card_data->lock);
		return 0;
	}

	do {
		brq.crq.cmd = rq_data_dir(req);

		brq.card_data.lba = blk_rq_pos(req);
		brq.card_data.blk_size = 1 << card_data->block_bits;
		brq.card_data.blk_nums = blk_rq_sectors(req);

		brq.card_data.sg = cq->sg;

		brq.card_data.sg_len = card_queue_map_sg(cq);

		if(cq->bounce_sg_len>1)
			brq.crq.buf = cq->bounce_buf;
		else
			brq.crq.buf = req->buffer;
		//printk("%s %d sg_len %d \t", brq.crq.cmd?"W":"R", brq.card_data.blk_nums, cq->bounce_sg_len);

		//brq.card_data.sg_len = blk_rq_map_sg(req->q, req, brq.card_data.sg);

		card->host->card_type = card->card_type;
		
		card_queue_bounce_pre(cq);

		card_wait_for_req(card->host, &brq);
		
		card_queue_bounce_post(cq);
			
		/*
		 *the request issue failed
		 */
		if (brq.card_data.error) {
			card_release_host(card->host);

			spin_lock_irq(&card_data->lock);
			ret = 1;
			while (ret) {
				ret = __blk_end_request(req, -EIO, (1 << card_data->block_bits));
			}
			spin_unlock_irq(&card_data->lock);

			/*add_disk_randomness(req->rq_disk);
			   blkdev_dequeue_request(req);
			   end_that_request_last(req, 0);
			   spin_unlock_irq(&card_data->lock); */

			return 0;
		}
		/*
		 * A block was successfully transferred.
		 */
		spin_lock_irq(&card_data->lock);
		brq.card_data.bytes_xfered = brq.card_data.blk_size * brq.card_data.blk_nums;
		ret = __blk_end_request(req, 0, brq.card_data.bytes_xfered);
		//if(!ret) 
		//{
		/*
		 * The whole request completed successfully.
		 */
		/*add_disk_randomness(req->rq_disk);
		   blkdev_dequeue_request(req);
		   end_that_request_last(req, 1);
		   } */
		spin_unlock_irq(&card_data->lock);
	} while (ret);

	card_release_host(card->host);
	//printk("card request completely %d sector num: %d communiction dir %d\n", req->sector, req->nr_sectors, brq.crq.cmd);
	return 1;
}

static void card_blk_remove(struct memory_card *card)
{
	struct card_blk_data *card_data = card_get_drvdata(card);

	if (card_data) {
		int devidx;

		del_gendisk(card_data->disk);

		/*
		 * I think this is needed.
		 */
		card_data->disk->queue = NULL;

		devidx = card_data->disk->first_minor >> CARD_SHIFT;
		__clear_bit(devidx, dev_use);

		card_blk_put(card_data);
	}
	card_set_drvdata(card, NULL);
}

#ifdef CONFIG_PM
static int card_blk_suspend(struct memory_card *card, pm_message_t state)
{
	struct card_blk_data *card_data = card_get_drvdata(card);

	if (card_data) {
		card_queue_suspend(&card_data->queue);
	}
	return 0;
}

static int card_blk_resume(struct memory_card *card)
{
	struct card_blk_data *card_data = card_get_drvdata(card);

	if (card_data) {
		//mmc_blk_set_blksize(md, card);
		card_queue_resume(&card_data->queue);
	}
	return 0;
}
#else
#define	card_blk_suspend	NULL
#define card_blk_resume		NULL
#endif

static int card_blk_probe(struct memory_card *card)
{
	struct card_blk_data *card_data;

	card_data = card_blk_alloc(card);
	if (IS_ERR(card_data))
		return PTR_ERR(card_data);

	card_set_drvdata(card, card_data);
	add_disk(card_data->disk);

	return 0;
}

static struct card_driver card_driver = {
	.drv = {
		.name = "cardblk",
		},
	.probe = card_blk_probe,
	.remove = card_blk_remove,
	.suspend = card_blk_suspend,
	.resume = card_blk_resume,
};

static int __init card_blk_init(void)
{
	int res = -ENOMEM;

	res = register_blkdev(major, "memorycard");
	if (res < 0) {
		printk(KERN_WARNING
		       "Unable to get major %d for Memory Card media: %d\n",
		       major, res);
		return res;
	}
	if (major == 0)
		major = res;
    printk(KERN_WARNING
		       "Memory Card media Major: %d\n",
		       major);
	return card_register_driver(&card_driver);
}

static void __exit card_blk_exit(void)
{
	card_unregister_driver(&card_driver);
	unregister_blkdev(major, "memorycard");
}

module_init(card_blk_init);
module_exit(card_blk_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Memory Card block device driver");

module_param(major, int, 0444);
MODULE_PARM_DESC(major, "specify the major device number for Memory block driver");
