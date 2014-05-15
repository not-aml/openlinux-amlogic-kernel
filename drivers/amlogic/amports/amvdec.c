/*
 * AMLOGIC Audio/Video streaming port driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the named License,
 * or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Author:  Tim Yao <timyao@amlogic.com>
 *
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/amports/vformat.h>

#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

#include <mach/am_regs.h>
#include <mach/power_gate.h>
#include "amvdec.h"

#define MC_SIZE (4096 * 4)
static void amvdec_pg_enable(bool enable)
{
    ulong timeout;

    if (enable) {
        CLK_GATE_ON(MDEC_CLK_PIC_DC);
        CLK_GATE_ON(MDEC_CLK_DBLK);
        CLK_GATE_ON(MC_CLK);
        CLK_GATE_ON(IQIDCT_CLK);
        //CLK_GATE_ON(VLD_CLK);
        CLK_GATE_ON(AMRISC);
    }
    else {
        CLK_GATE_OFF(AMRISC);

        timeout = jiffies + HZ/10;

        while (READ_MPEG_REG(MDEC_PIC_DC_STATUS) != 0) {
            if (time_after(jiffies, timeout)) {
                WRITE_MPEG_REG_BITS(MDEC_PIC_DC_CTRL, 1, 0, 1);
                WRITE_MPEG_REG_BITS(MDEC_PIC_DC_CTRL, 0, 0, 1);
                READ_MPEG_REG(MDEC_PIC_DC_STATUS);
                READ_MPEG_REG(MDEC_PIC_DC_STATUS);
                READ_MPEG_REG(MDEC_PIC_DC_STATUS);
                break;
            }
        }

        CLK_GATE_OFF(MDEC_CLK_PIC_DC);

        timeout = jiffies + HZ/10;

        while (READ_MPEG_REG(DBLK_STATUS) & 1) {
            if (time_after(jiffies, timeout)) {
                WRITE_MPEG_REG(DBLK_CTRL, 3);
                WRITE_MPEG_REG(DBLK_CTRL, 0);
                READ_MPEG_REG(DBLK_STATUS);
                READ_MPEG_REG(DBLK_STATUS);
                READ_MPEG_REG(DBLK_STATUS);
                break;
            }
        }
        CLK_GATE_OFF(MDEC_CLK_DBLK);

        timeout = jiffies + HZ/10;

        while (READ_MPEG_REG(MC_STATUS0) & 1) {
            if (time_after(jiffies, timeout)) {
                SET_MPEG_REG_MASK(MC_CTRL1, 0x9);
                CLEAR_MPEG_REG_MASK(MC_CTRL1, 0x9);
                READ_MPEG_REG(MC_STATUS0);
                READ_MPEG_REG(MC_STATUS0);
                READ_MPEG_REG(MC_STATUS0);
                break;
            }
        }
        CLK_GATE_OFF(MC_CLK);

        timeout = jiffies + HZ/10;
        while (READ_MPEG_REG(DCAC_DMA_CTRL) & 0x8000) {
            if (time_after(jiffies, timeout)) {
                break;
            }
        }

        WRITE_MPEG_REG(RESET0_REGISTER, 4);
        READ_MPEG_REG(RESET0_REGISTER);
        READ_MPEG_REG(RESET0_REGISTER);
        READ_MPEG_REG(RESET0_REGISTER);
        CLK_GATE_OFF(IQIDCT_CLK);

        //CLK_GATE_OFF(VLD_CLK);
    }
}


s32 amvdec_loadmc(const u32 *p)
{
    ulong timeout;
    void *mc_addr;
    dma_addr_t mc_addr_map;
    s32 ret = 0;

    mc_addr = kmalloc(MC_SIZE, GFP_KERNEL);
    if (!mc_addr)
        return -ENOMEM;
		
    memcpy(mc_addr, p, MC_SIZE);

    mc_addr_map = dma_map_single(NULL, mc_addr, MC_SIZE, DMA_TO_DEVICE);

    WRITE_MPEG_REG(MPSR, 0);
    WRITE_MPEG_REG(CPSR, 0);

    /* Read CBUS register for timing */
    timeout = READ_MPEG_REG(MPSR);
    timeout = READ_MPEG_REG(MPSR);

    timeout = jiffies + HZ;

    WRITE_MPEG_REG(IMEM_DMA_ADR, mc_addr_map);
    WRITE_MPEG_REG(IMEM_DMA_COUNT,0x1000);
    WRITE_MPEG_REG(IMEM_DMA_CTRL,(0x8000 | (7 << 16)));

    while(READ_MPEG_REG(IMEM_DMA_CTRL) & 0x8000) {
        if (time_before(jiffies, timeout)) {
            schedule();
        }
        else {
            printk("vdec load mc error\n");
            ret = -EBUSY;
            break;
        }
    }

    dma_unmap_single(NULL, mc_addr_map, MC_SIZE, DMA_TO_DEVICE);

    kfree(mc_addr);

    return ret;
}

void amvdec_start(void)
{
    /* additional cbus dummy register reading for timing control */
    READ_MPEG_REG(RESET0_REGISTER);
    READ_MPEG_REG(RESET0_REGISTER);
    READ_MPEG_REG(RESET0_REGISTER);
    READ_MPEG_REG(RESET0_REGISTER);

    WRITE_MPEG_REG(RESET0_REGISTER, RESET_VCPU | RESET_CCPU);

    READ_MPEG_REG(RESET0_REGISTER);
    READ_MPEG_REG(RESET0_REGISTER);
    READ_MPEG_REG(RESET0_REGISTER);

    WRITE_MPEG_REG(MPSR, 0x0001);
}

void amvdec_stop(void)
{
    ulong timeout = jiffies + HZ;

    WRITE_MPEG_REG(MPSR, 0);
    WRITE_MPEG_REG(CPSR, 0);

    while (READ_MPEG_REG(IMEM_DMA_CTRL) & 0x8000) {
        if (time_after(jiffies, timeout)) {
            break;
        }
    }

    WRITE_MPEG_REG(RESET0_REGISTER, RESET_VCPU | RESET_CCPU);

    /* additional cbus dummy register reading for timing control */
    READ_MPEG_REG(RESET0_REGISTER);
    READ_MPEG_REG(RESET0_REGISTER);
    READ_MPEG_REG(RESET0_REGISTER);
    READ_MPEG_REG(RESET0_REGISTER);

}

void amvdec_enable(void)
{
    amvdec_pg_enable(true);
}

void amvdec_disable(void)
{
    amvdec_pg_enable(false);
}

#ifdef CONFIG_PM
int amvdec_suspend(struct platform_device *dev, pm_message_t event)
{
    amvdec_pg_enable(false);
	
    return 0;
}

int amvdec_resume(struct platform_device *dev)
{
    amvdec_pg_enable(true);
    return 0;
}
#endif

EXPORT_SYMBOL(amvdec_loadmc);
EXPORT_SYMBOL(amvdec_start);
EXPORT_SYMBOL(amvdec_stop);
EXPORT_SYMBOL(amvdec_enable);
EXPORT_SYMBOL(amvdec_disable);
#ifdef CONFIG_PM
EXPORT_SYMBOL(amvdec_suspend);
EXPORT_SYMBOL(amvdec_resume);
#endif

MODULE_DESCRIPTION("Amlogic Video Decoder Utility Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tim Yao <timyao@amlogic.com>");
