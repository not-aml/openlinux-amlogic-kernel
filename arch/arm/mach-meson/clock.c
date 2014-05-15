/*
 *
 * arch/arm/mach-meson/clock.c
 *
 *  Copyright (C) 2010 AMLOGIC, INC.
 *
 * License terms: GNU General Public License (GPL) version 2
 * Define clocks in the app platform.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/init.h>
#include <linux/spinlock.h>

#include <asm/clkdev.h>
#include <mach/clock.h>
#include <mach/hardware.h>
#include <mach/clk_set.h>
#include <mach/am_regs.h>

static DEFINE_SPINLOCK(clockfw_lock);

#ifdef CONFIG_INIT_A9_CLOCK_FREQ
static unsigned long __initdata init_clock=CONFIG_INIT_A9_CLOCK;
#else
static unsigned long __initdata init_clock=0;
#endif

long clk_round_rate(struct clk *clk,unsigned long rate)
{
	if(rate<clk->min)
		return clk->min;
	if(rate>clk->max)
		return clk->max;
	return -1;
}
EXPORT_SYMBOL(clk_round_rate);


unsigned long clk_get_rate(struct clk *clk)
{
    if (!clk)
        return 0;
	if(clk->get_rate)
		return clk->get_rate(clk);
    return clk->rate;
}
EXPORT_SYMBOL(clk_get_rate);

int clk_set_rate(struct clk *clk, unsigned long rate)
{
    unsigned long flags;
    int ret = -EINVAL;

    if (clk == NULL || clk->set_rate==NULL)
        return ret;

    spin_lock_irqsave(&clockfw_lock, flags);

    ret = clk->set_rate(clk, rate);

    spin_unlock_irqrestore(&clockfw_lock, flags);

    return ret;
}
EXPORT_SYMBOL(clk_set_rate);

static unsigned long xtal_get_rate(struct clk *clk)
{
	unsigned long rate;
	rate=get_xtal_clock();/*refresh from register*/
	clk->rate=rate;
	return rate;
}

static int clk_set_rate_sys_pll(struct clk *clk, unsigned long rate)
{
    unsigned long r = rate;
	int ret=-EINVAL;
	
    if (r < 1000)
        r = r * 1000000;
	ret=sys_clkpll_setting(0,r );
    return ret;
}

static int clk_set_rate_other_pll(struct clk *clk, unsigned long rate)
{
    unsigned long r = rate;
	int ret=-EINVAL;
	
    if (r < 1000)
        r = r * 1000000;
	ret=other_pll_setting(0,r );
    return ret;
}

static int clk_set_rate_clk81(struct clk *clk, unsigned long rate)
{
    unsigned long r = rate;
	struct clk *father_clk;
	unsigned long r1;
	int ret=-1;
	
    if (r < 1000)
        r = r * 1000000;

	father_clk = clk_get_sys("clk_other_pll", NULL);
	r1=clk_get_rate(father_clk);
	if(r1!=r*4)
		{
			ret=father_clk->set_rate(father_clk,r*2);
			if(ret!=0)
				return ret;
		}
	clk->rate=r;
	/*for current it is alway equal=otherclk/2*/
	WRITE_MPEG_REG(HHI_MPEG_CLK_CNTL,   // MPEG clk81 set to other/4
						(1 << 12) |                     // select other PLL
						((4 - 1) << 0 ) |    // div1
						(1 << 7 ) |                     // cntl_hi_mpeg_div_en, enable gating
						(1 << 8 )                    // Connect clk81 to the PLL divider output
					);
    return 0;
}


static int clk_set_rate_a9_clk(struct clk *clk, unsigned long rate)
{
    unsigned long r = rate;
	struct clk *father_clk;
	unsigned long r1;
	int ret=-1;
	
    if (r < 1000)
        r = r * 1000000;
	father_clk = clk_get_sys("clk_sys_pll", NULL);
	r1=clk_get_rate(father_clk);
	if(!r1)
		return -1;
	if(r1!=r*2 && r!=0)
		{
			ret=father_clk->set_rate(father_clk,r*2);
			if(ret!=0)
				return ret;
		}
	clk->rate=r;
	/*for current it is alway equal=sys_pll/2*/
	 WRITE_MPEG_REG(HHI_A9_CLK_CNTL,					// A9 clk set to system clock/2
		(0 << 10) |						// 0 - sys_pll_clk, 1 - audio_pll_clk
		(1 << 0 ) |						// 1 - sys/audio pll clk, 0 - XTAL
		(1 << 4 ) |						// APB_CLK_ENABLE
		(1 << 5 ) |						// AT_CLK_ENABLE
		(0 << 2 ) |						// div1
		(1 << 7 ));						// Connect A9 to the PLL divider output
    return 0;
}
static int clk_set_rate_audio_clk(struct clk *clk, unsigned long rate)
{
    unsigned long r = rate;
	int ret=-EINVAL;
	
    if (r < 1000)
        r = r * 1000000;
	ret=audio_pll_setting(0,r );
    return ret;
}

static int clk_set_rate_video_clk(struct clk *clk, unsigned long rate)
{
    unsigned long r = rate;
	int ret=-EINVAL;
	
    if (r < 1000)
        r = r * 1000000;
	ret=video_pll_setting(0,r ,0,0);
    return ret;
}

static struct clk xtal_clk = {
    .name       = "clk_xtal",
    .rate       = 24000000,
    .get_rate	= xtal_get_rate,
    .set_rate   = NULL,
};

static struct clk clk_sys_pll = {
    .name       = "clk_sys_pll",
    .rate       = 1200000000,
    .min		=  200000000,
    .max		= 2000000000,
    .set_rate   = clk_set_rate_sys_pll,
};

static struct clk clk_other_pll = {
    .name       = "clk_other_pll",
    .rate       = 540000000,
    .min		= 200000000,
    .max		= 800000000,
    .set_rate   = clk_set_rate_other_pll,
};


static struct clk clk_ddr_pll = {
    .name       = "clk_ddr",
    .rate       = 400000000,
    .set_rate   = NULL,
};


static struct clk clk81 = {
    .name       = "clk81",
    .rate       = 180000000,
    .min		= 100000000,
    .max		= 400000000,
    .set_rate   = clk_set_rate_clk81,
};


static struct clk a9_clk = {
    .name       = "a9_clk",
    .rate       = 600000000,
    .min		= 100000000,
    .max		=1000000000,
    .set_rate   = clk_set_rate_a9_clk,
};

static struct clk audio_clk = {
    .name       = "audio_clk",
    .rate       = 300000000,
    .min		= 200000000,
    .max		=1000000000,
    .set_rate   = clk_set_rate_audio_clk,
};

static struct clk video_clk = {
    .name       = "video_clk",
    .rate       = 300000000,
    .min		= 100000000,
    .max		= 750000000,
    .set_rate   = clk_set_rate_video_clk,
};




/*
 * Here we only define clocks that are meaningful to
 * look up through clockdevice.
 */
static struct clk_lookup lookups[] = {
	{
			.dev_id = "clk_xtal",
			.clk	= &xtal_clk,
	},
	{
			.dev_id = "clk_sys_pll",
			.clk	= &clk_sys_pll,
	},	
	{
			.dev_id = "clk_other_pll",
			.clk	= &clk_other_pll,
	},
	{
			.dev_id = "clk_ddr_pll",
			.clk	= &clk_ddr_pll,
	},
	{
	        .dev_id = "clk81",
	        .clk    = &clk81,
    },
    {
	        .dev_id = "a9_clk",
	        .clk    = &a9_clk,
    },
    {
	        .dev_id = "audio_clk",
	        .clk    = &audio_clk,
    },
    {
	        .dev_id = "video_clk",
	        .clk    = &video_clk,
    }
};

static int __init meson_clock_init(void)
{
	if(init_clock && init_clock!=a9_clk.rate)
	{
		if(sys_clkpll_setting(0,init_clock<<1)==0)
		{
			a9_clk.rate=init_clock;
			clk_sys_pll.rate=init_clock<<1;
		}
	}

    /* Register the lookups */
	clkdev_add_table(lookups,ARRAY_SIZE(lookups));
    return 0;
}

/* initialize clocking early to be available later in the boot */
core_initcall(meson_clock_init);

unsigned long long clkparse(const char *ptr, char **retptr)
{
	char *endptr;	/* local pointer to end of parsed string */

	unsigned long long ret = simple_strtoull(ptr, &endptr, 0);

	switch (*endptr) {
	case 'G':
	case 'g':
		ret *= 1000;
	case 'M':
	case 'm':
		ret *= 1000;
	case 'K':
	case 'k':
		ret *= 1000;
		endptr++;
	default:
		break;
	}

	if (retptr)
		*retptr = endptr;

	return ret;
}

static int __init a9_clock_setup(char *ptr)
{
	init_clock=clkparse(ptr,0);
	if(sys_clkpll_setting(0,init_clock<<1)==0)
	{
		a9_clk.rate=init_clock;
		clk_sys_pll.rate=init_clock<<1;
	}
	return 0;
}
__setup("a9_clk=",a9_clock_setup);

static int __init clk81_clock_setup(char *ptr)
{
	int clock=clkparse(ptr,0);
    if (other_pll_setting(0, clock*4) == 0) {
        int baudrate = (clock / (115200 * 4)) - 1;

        clk_other_pll.rate = clock*4;
        clk81.rate = clock;

        WRITE_MPEG_REG(HHI_MPEG_CLK_CNTL,   // MPEG clk81 set to other/4
		    (1 << 12) |                     // select other PLL
			((4 - 1) << 0 ) |               // div1
			(1 << 7 ) |                     // cntl_hi_mpeg_div_en, enable gating
			(1 << 8 ));                     // Connect clk81 to the PLL divider output

        CLEAR_CBUS_REG_MASK(UART0_CONTROL, (1 << 19) | 0xFFF);
        SET_CBUS_REG_MASK(UART0_CONTROL, (baudrate & 0xfff));
        CLEAR_CBUS_REG_MASK(UART1_CONTROL, (1 << 19) | 0xFFF);
        SET_CBUS_REG_MASK(UART1_CONTROL, (baudrate & 0xfff));
	}

	return 0;
}
__setup("clk81=",clk81_clock_setup);
