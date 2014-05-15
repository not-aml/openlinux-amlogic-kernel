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
#include <mach/am_regs.h>

#include "amvdec.h"

#define MC_SIZE (4096 * 4)

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
        } else {
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
    WRITE_MPEG_REG(RESET0_REGISTER, RESET_VCPU | RESET_CCPU);

    READ_MPEG_REG(RESET0_REGISTER);
    READ_MPEG_REG(RESET0_REGISTER);
    READ_MPEG_REG(RESET0_REGISTER);

    WRITE_MPEG_REG(MPSR, 0x0001);
}

void amvdec_stop(void)
{
    WRITE_MPEG_REG(MPSR, 0);
    WRITE_MPEG_REG(RESET0_REGISTER, RESET_VCPU | RESET_CCPU);
}

EXPORT_SYMBOL(amvdec_loadmc);
EXPORT_SYMBOL(amvdec_start);
EXPORT_SYMBOL(amvdec_stop);

MODULE_DESCRIPTION("Amlogic Video Decoder Utility Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tim Yao <timyao@amlogic.com>");
