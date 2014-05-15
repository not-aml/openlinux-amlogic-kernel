#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/amports/timestamp.h>
#include <linux/amports/tsync.h>

#ifdef CONFIG_AM_TIMESYNC_LOG
#define AMLOG
#define LOG_LEVEL_ERROR		0
#define LOG_LEVEL_ATTENTION 1
#define LOG_LEVEL_INFO		2
#define LOG_LEVEL_VAR		amlog_level_tsync
#define LOG_MASK_VAR		amlog_mask_tsync
#endif
#include <linux/amlog.h>
MODULE_AMLOG(AMLOG_DEFAULT_LEVEL, 0, LOG_DEFAULT_LEVEL_DESC, LOG_DEFAULT_MASK_DESC);

//#define DEBUG
#define AVEVENT_FLAG_PARAM  0x01

//#define TSYNC_SLOW_SYNC

typedef enum {
    TSYNC_STAT_PCRSCR_SETUP_NONE,
    TSYNC_STAT_PCRSCR_SETUP_VIDEO,
    TSYNC_STAT_PCRSCR_SETUP_AUDIO
} tsync_stat_t;

const static struct {
    const char *token;
    const u32 token_size;
    const avevent_t event;
    const u32 flag;
} avevent_token[] = 
{
    {"VIDEO_START", 11, VIDEO_START, AVEVENT_FLAG_PARAM},
    {"VIDEO_STOP",  10, VIDEO_STOP,  0},
    {"VIDEO_PAUSE", 11, VIDEO_PAUSE, 0},
    {"VIDEO_TSTAMP_DISCONTINUITY", 26, VIDEO_TSTAMP_DISCONTINUITY, AVEVENT_FLAG_PARAM},
    {"AUDIO_START", 11, AUDIO_START, AVEVENT_FLAG_PARAM},
    {"AUDIO_RESUME",12, AUDIO_RESUME, 0},
    {"AUDIO_STOP",  10, AUDIO_STOP,  0},
    {"AUDIO_PAUSE", 11, AUDIO_PAUSE, 0},
    {"AUDIO_TSTAMP_DISCONTINUITY", 26, AUDIO_TSTAMP_DISCONTINUITY, AVEVENT_FLAG_PARAM},
};

const static char *tsync_mode_str[] = 
{
    "vmaster", "amaster"
};

static spinlock_t lock = SPIN_LOCK_UNLOCKED;
static tsync_mode_t tsync_mode = TSYNC_MODE_AMASTER;
static tsync_stat_t tsync_stat = TSYNC_STAT_PCRSCR_SETUP_NONE;
static int tsync_enable = 1;
static int tsync_abreak = 0;
static int tsync_trickmode = 0;
static unsigned int tsync_av_thresh = AV_DISCONTINUE_THREDHOLD;
static unsigned int tsync_syncthresh = 1;
static int tsync_dec_reset_flag = 0;

void tsync_avevent(avevent_t event, u32 param)
{
    ulong flags;
    u32 t;
    ulong fiq_flag;

    spin_lock_irqsave(&lock, flags);
    
    raw_local_save_flags(fiq_flag);

    local_fiq_disable();

    switch (event) {
    case VIDEO_START:
        if (tsync_enable)
            tsync_mode = TSYNC_MODE_AMASTER;
        else
            tsync_mode = TSYNC_MODE_VMASTER;

    #ifndef TSYNC_SLOW_SYNC
        if (tsync_stat == TSYNC_STAT_PCRSCR_SETUP_NONE) 
    #endif
        {
        #ifndef TSYNC_SLOW_SYNC
            if (tsync_syncthresh && (tsync_mode == TSYNC_MODE_AMASTER))
                timestamp_pcrscr_set(param - VIDEO_HOLD_THRESHOLD);
            else
                timestamp_pcrscr_set(param);
        #else
            timestamp_pcrscr_set(param);
        #endif

            tsync_stat = TSYNC_STAT_PCRSCR_SETUP_VIDEO;
            amlog_level(LOG_LEVEL_INFO, "vpts to scr, apts = 0x%x, vpts = 0x%x\n",
                timestamp_apts_get(),
                timestamp_vpts_get());
        }

        if (tsync_stat == TSYNC_STAT_PCRSCR_SETUP_AUDIO)
        {
            t = timestamp_pcrscr_get();
            if (abs(param-t) > tsync_av_thresh)
            {
                /* if this happens, then play */
                tsync_stat = TSYNC_STAT_PCRSCR_SETUP_VIDEO;
                timestamp_pcrscr_set(param);
            }
        }

        timestamp_pcrscr_enable(1);
        break;

    case VIDEO_STOP:
        tsync_stat = TSYNC_STAT_PCRSCR_SETUP_NONE;
        timestamp_vpts_set(0);
        timestamp_pcrscr_enable(0);
        break;

    case VIDEO_TSTAMP_DISCONTINUITY:
        t = timestamp_pcrscr_get();
    
        if (abs(param - t) > AV_DISCONTINUE_THREDHOLD) {
            /*
             * making system time updated by itself.
             */
            tsync_mode = TSYNC_MODE_VMASTER;
            tsync_stat = TSYNC_STAT_PCRSCR_SETUP_VIDEO;

            timestamp_vpts_set(param);

            timestamp_pcrscr_set(param);

            amlog_level(LOG_LEVEL_ATTENTION, "reset scr from vpts to 0x%x\n", param);

        } else if (tsync_enable)
            tsync_mode = TSYNC_MODE_AMASTER;
        break;

    case AUDIO_TSTAMP_DISCONTINUITY:
        if (!tsync_enable)
            break;

        t = timestamp_pcrscr_get();
    
        amlog_level(LOG_LEVEL_ATTENTION, "AUDIO_TSTAMP_DISCONTINUITY, 0x%x, 0x%x\n", t, param);

        if (abs(param - t) > AV_DISCONTINUE_THREDHOLD) {
            /* switch tsync mode to free run mode,
             * making system time updated by itself.
             */
            tsync_mode = TSYNC_MODE_VMASTER;

            timestamp_apts_set(param);

            amlog_level(LOG_LEVEL_ATTENTION, "apts interrupt: 0x%x\n", param);

        } else
            tsync_mode = TSYNC_MODE_AMASTER;
        break;

    case AUDIO_START:
        if (!tsync_enable)
            break;

        t = timestamp_pcrscr_get();

        amlog_level(LOG_LEVEL_INFO, "[%s]param %d, t %d, tsync_abreak %d\n", 
            __FUNCTION__, param, t, tsync_abreak);

        if (tsync_abreak && (abs(param-t) > TIME_UNIT90K/10)) // 100ms, then wait to match
        {
            break;
        }

        tsync_abreak = 0;
        if (tsync_dec_reset_flag) // after reset, video should be played first
        {
            int vpts = timestamp_vpts_get();
            if (param < vpts)
                timestamp_pcrscr_set(param);
            else
                timestamp_pcrscr_set(vpts);
            tsync_dec_reset_flag = 0;
        }
        else
        {
            timestamp_pcrscr_set(param);
        }
        timestamp_apts_set(param);
        tsync_stat = TSYNC_STAT_PCRSCR_SETUP_AUDIO;

        amlog_level(LOG_LEVEL_INFO, "apts reset scr = 0x%x\n", param);

        timestamp_pcrscr_enable(1);
        break;
        
    case AUDIO_RESUME:
        if (!tsync_enable)
            break;
        timestamp_pcrscr_enable(1);
        break;
        
    case AUDIO_STOP:
        tsync_abreak = 0;
        if (tsync_trickmode)
            tsync_stat = TSYNC_STAT_PCRSCR_SETUP_VIDEO;
        else
            tsync_stat = TSYNC_STAT_PCRSCR_SETUP_NONE;
        break;

    case AUDIO_PAUSE:
        if (!tsync_enable)
            break;

        timestamp_pcrscr_enable(0);
        break;

    case VIDEO_PAUSE:
        timestamp_pcrscr_enable(1-param);
        break;

    default:
        break;
    }

    raw_local_irq_restore(fiq_flag);

    spin_unlock_irqrestore(&lock, flags);
}
EXPORT_SYMBOL(tsync_avevent);

void tsync_audio_break(int audio_break)
{
    tsync_abreak = audio_break;
    return;
}
EXPORT_SYMBOL(tsync_audio_break);

void tsync_trick_mode(int trick_mode)
{
    tsync_trickmode = trick_mode;
    return;
}
EXPORT_SYMBOL(tsync_trick_mode);

void tsync_set_avthresh(unsigned int av_thresh)
{
    tsync_av_thresh = av_thresh;
    return;
}
EXPORT_SYMBOL(tsync_set_avthresh);

void tsync_set_syncthresh(unsigned int sync_thresh)
{
    tsync_syncthresh = sync_thresh;
    return;
}
EXPORT_SYMBOL(tsync_set_syncthresh);

void tsync_set_dec_reset(void)
{
    tsync_dec_reset_flag = 1;
}
EXPORT_SYMBOL(tsync_set_dec_reset);

/*********************************************************/
static ssize_t show_vpts(struct class *class,
                         struct class_attribute *attr,
                         char *buf)
{
    return sprintf(buf, "0x%x\n", timestamp_vpts_get());
}

static ssize_t store_vpts(struct class *class,
                          struct class_attribute *attr,
                          const char *buf,
                          size_t size)
{
    unsigned pts;
    ssize_t r;

    r = sscanf(buf, "0x%x", &pts);
    if (r != 1)
        return -EINVAL;

    timestamp_vpts_set(pts);
    return size;
}

static ssize_t show_apts(struct class *class,
                         struct class_attribute *attr,
                         char *buf)
{
    return sprintf(buf, "0x%x\n", timestamp_apts_get());
}

static ssize_t store_apts(struct class *class,
                          struct class_attribute *attr,
                          const char *buf,
                          size_t size)
{
    unsigned pts, t;
    ssize_t r;
    
    r = sscanf(buf, "0x%x", &pts);
    if (r != 1)
        return -EINVAL;

    timestamp_apts_set(pts);
	
    if (tsync_abreak)
        tsync_abreak = 0;
	
	if (!tsync_enable)
		return size;

    t = timestamp_pcrscr_get();
    if (tsync_mode == TSYNC_MODE_AMASTER)
    {
        if (abs(pts - t) > tsync_av_thresh)
        {
            tsync_mode = TSYNC_MODE_VMASTER;
        }
        else
            timestamp_pcrscr_set(pts);
    }
    else {   
        if (abs(pts - t) <= tsync_av_thresh) {
            tsync_mode = TSYNC_MODE_AMASTER;

            timestamp_pcrscr_set(pts);
        }
    }

    return size;
}

static ssize_t show_pcrscr(struct class *class,
                           struct class_attribute *attr,
                           char *buf)
{
    return sprintf(buf, "0x%x\n", timestamp_pcrscr_get());
}

static ssize_t store_pcrscr(struct class *class,
                            struct class_attribute *attr,
                            const char *buf,
                            size_t size)
{
    unsigned pts;
    ssize_t r;

    r = sscanf(buf, "0x%x", &pts);
    if (r != 1)
        return -EINVAL;

    timestamp_pcrscr_set(pts);

    return size;
}

static ssize_t store_event(struct class *class,
                           struct class_attribute *attr,
                           const char *buf,
                           size_t size)
{
    int i;
        
    for (i = 0; i < ARRAY_SIZE(avevent_token); i++) {
        if (strncmp(avevent_token[i].token, buf, avevent_token[i].token_size) == 0) {
            if (avevent_token[i].flag & AVEVENT_FLAG_PARAM) {
                char *param_str = strchr(buf, ':');
                
                if (!param_str)
                    return -EINVAL;

                tsync_avevent(avevent_token[i].event, 
                              simple_strtoul(param_str + 1, NULL, 16));
            }
            else
                tsync_avevent(avevent_token[i].event, 0);

            return size;
        }
    }
        
    return -EINVAL;
}

static ssize_t show_mode(struct class *class,
                         struct class_attribute *attr,
                         char *buf)
{
    if (tsync_mode <= TSYNC_MODE_AMASTER)
        return sprintf(buf, "%d: %s\n", tsync_mode, tsync_mode_str[tsync_mode]);

    return sprintf(buf, "invalid mode");
}

static ssize_t show_enable(struct class *class,
                           struct class_attribute *attr,
                           char *buf)
{
    if (tsync_enable)
        return sprintf(buf, "1: enabled\n");

    return sprintf(buf, "0: disabled\n");
}

static ssize_t store_enable(struct class *class,
                            struct class_attribute *attr,
                            const char *buf,
                            size_t size)
{
    unsigned mode;
    ssize_t r;

    r = sscanf(buf, "%d", &mode);
    if ((r != 1))
        return -EINVAL;

    tsync_enable = mode ? 1 : 0;

    return size;
}

static struct class_attribute tsync_class_attrs[] = {
    __ATTR(pts_video,  S_IRUGO | S_IWUSR, show_vpts,    store_vpts  ),
    __ATTR(pts_audio,  S_IRUGO | S_IWUSR, show_apts,    store_apts  ),
    __ATTR(pts_pcrscr, S_IRUGO | S_IWUSR, show_pcrscr,  store_pcrscr),
    __ATTR(event,      S_IRUGO | S_IWUSR, NULL,         store_event ),
    __ATTR(mode,       S_IRUGO | S_IWUSR, show_mode,    NULL        ),
    __ATTR(enable,     S_IRUGO | S_IWUSR, show_enable,  store_enable),
    __ATTR_NULL
};

static struct class tsync_class = {
    .name = "tsync",
    .class_attrs = tsync_class_attrs,
};

static int __init tsync_init(void)
{
    int r;

    r = class_register(&tsync_class);

    if (r) {
        amlog_level(LOG_LEVEL_ERROR, "tsync class create fail.\n");
        return r;
    }

    /* init audio pts to -1, others to 0 */
    timestamp_apts_set(-1);
    timestamp_vpts_set(0);
    timestamp_pcrscr_set(0);
    
    return (0);
}

static void __exit tsync_exit(void)
{
    class_unregister(&tsync_class);
}

module_init(tsync_init);
module_exit(tsync_exit);

MODULE_DESCRIPTION("AMLOGIC time sync management driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tim Yao <timyao@amlogic.com>");
