#ifndef __CARD_BLOCK_H
#define __CARD_BLOCK_H

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/device.h>
#include "cardreader.h"

#define card_get_drvdata(c)		dev_get_drvdata(&(c)->dev)
#define card_set_drvdata(c,d)	dev_set_drvdata(&(c)->dev, d)

#define CARD_NAME_LEN            6

struct memory_card;
struct card_blk_request;

struct card_driver {
	struct device_driver drv;
	int (*probe)(struct memory_card *);
	void (*remove)(struct memory_card *);
	int (*suspend)(struct memory_card *, pm_message_t);
	int (*resume)(struct memory_card *);
};

struct memory_card {
	struct list_head	node;		/* node in hosts devices list */
	struct card_host	*host;		/* the host this device belongs to */
	struct device		dev;		/* the device */
	unsigned int		state;		/* (our) card state */
	unsigned int		capacity;	/* card capacity*/
	u32					raw_cid[4]; /*card raw cid */
	CARD_TYPE_t			card_type;	/* card type*/
	char				name[CARD_NAME_LEN];
};

struct card_queue {
	struct memory_card		*card;
	//struct completion	thread_complete;
	//wait_queue_head_t	thread_wq;
	//struct semaphore	thread_sem;
	unsigned int		flags;
	struct request		*req;
	int			(*prep_fn)(struct card_queue *, struct request *);
	int			(*issue_fn)(struct card_queue *, struct request *);
	void			*data;
	struct request_queue	*queue;
	struct scatterlist	*sg;
	
	char			*bounce_buf;
	struct scatterlist	*bounce_sg;
	unsigned int		bounce_sg_len;
};

struct card_request {
	unsigned int cmd;
	unsigned char *buf;
};

struct card_data {
	unsigned int		timeout_ns;	/* data timeout (in ns, max 80ms) */
	unsigned int		timeout_clks;	/* data timeout (in clocks) */
	unsigned int 		lba;			/*logic block address*/
	unsigned int		blk_size;		/* data block size */
	unsigned int		blk_nums;		/* number of blocks */
	unsigned int		error;		/* data error */
	unsigned int		flags;

	unsigned int		bytes_xfered;

	struct card_request	*crq;		/* associated request */

	unsigned int		sg_len;		/* size of scatter list */
	struct scatterlist	*sg;		/* I/O scatter list */
};

struct card_host_ops {
	void	(*request)(struct card_host *host, struct card_blk_request *req);
};

struct card_host {
	struct device		*parent;
	struct device	class_dev;
	int			index;
	const struct card_host_ops *ops;
	u32			ocr_avail;

	unsigned long		caps;		/* Host capabilities */

	/* host specific block data */
	unsigned int		max_req_size;	/* maximum number of bytes in one req */
	unsigned int		max_seg_size;	/* see blk_queue_max_segment_size */
	unsigned short		max_hw_segs;	/* see blk_queue_max_hw_segments */
	unsigned short		max_phys_segs;	/* see blk_queue_max_phys_segments */
	unsigned short		max_sectors;	/* see blk_queue_max_sectors */
	unsigned short		unused;

	unsigned int		mode;		/* current card mode of host */

	struct list_head	cards;		/* devices attached to this host */

	wait_queue_head_t	wq;
	spinlock_t		lock;		/* card_busy lock */
	struct memory_card		*card_busy;	/* the MEMORY card claiming host */
	struct memory_card		*card_selected;	/* the selected MEMORY card */

	CARD_TYPE_t			card_type;	/* card type*/
	
	struct work_struct	detect;

	unsigned long		private[0] ____cacheline_aligned;
};

struct card_blk_request {
	struct card_request	crq;
	struct card_data	card_data;
};

#define CARD_STATE_PRESENT	(1<<0)		/* present in sysfs */
#define CARD_STATE_DEAD		(1<<1)		/* device no longer in stack */
#define CARD_STATE_BAD		(1<<2)		/* unrecognised device */
#define CARD_STATE_READONLY	(1<<3)		/* card is read-only */

#define card_hostname(x)	(dev_name(&(x)->class_dev))

static inline void *card_priv(struct card_host *host)
{
	return (void *)host->private;
}

extern int __card_claim_host(struct card_host *host, struct memory_card *card);

static inline void card_claim_host(struct card_host *host)
{
	__card_claim_host(host, (struct memory_card *)-1);
}

extern void card_init_card(struct memory_card *card, struct card_host *host);
extern int card_register_card(struct memory_card *card, char *card_name);
extern void card_remove_card(struct memory_card *card);
extern int card_add_host_sysfs(struct card_host *host);
extern void card_flush_scheduled_work(void);
extern void card_remove_host_sysfs(struct card_host *host);
extern int card_wait_for_req(struct card_host *host, struct card_blk_request *brq);
extern void card_release_card(struct device *dev);
extern int card_register_driver(struct card_driver *drv);
extern void card_unregister_driver(struct card_driver *drv);
extern void card_free_host_sysfs(struct card_host *host);
extern struct card_host *card_alloc_host_sysfs(int extra, struct device *dev);
extern int card_schedule_work(struct work_struct *work);
extern int card_schedule_delayed_work(struct delayed_work *work, unsigned long delay);
extern void card_release_host(struct card_host *host);

#endif
