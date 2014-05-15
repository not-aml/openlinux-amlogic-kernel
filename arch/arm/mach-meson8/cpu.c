/*
 * arch/arm/mach-meson8/cpu.c
 *
 * Copyright (C) 2013 Amlogic, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <plat/io.h>
#include <plat/cpu.h>
#include <mach/io.h>
#include <mach/am_regs.h>
#include <linux/printk.h>
#include <linux/string.h>

static int meson_cpu_version[MESON_CPU_VERSION_LVL_MAX];
int __init meson_cpu_version_init(void)
{
	unsigned int  *version_map;	
	unsigned int version;	

	meson_cpu_version[MESON_CPU_VERSION_LVL_MAJOR] = 
		aml_read_reg32(P_ASSIST_HW_REV);

	version_map = (unsigned int *)IO_BOOTROM_BASE;
	version = version_map[1];
	printk(KERN_INFO "chip version=%x\n", version);	
	switch (version) {		
		case 0x000025e2:			
			meson_cpu_version[MESON_CPU_VERSION_LVL_MINOR] = 0xA;		
			break;		
		default:/*changed?*/			
			meson_cpu_version[MESON_CPU_VERSION_LVL_MINOR] = 0xB;
			break;
	}	

	return 0;
}
EXPORT_SYMBOL(meson_cpu_version_init);
int get_meson_cpu_version(int level)
{
	if(level >= 0 && level <= MESON_CPU_VERSION_LVL_MAX)
		return meson_cpu_version[level];
	return 0;
}
EXPORT_SYMBOL(get_meson_cpu_version);