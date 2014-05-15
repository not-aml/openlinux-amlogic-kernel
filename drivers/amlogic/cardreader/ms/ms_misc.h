#ifndef _H_MS_MISC
#define _H_MS_MISC

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
    
#include <asm/arch/am_regs.h>
#include <asm/cacheflush.h>
#include <asm/delay.h>
    
#define ms_get_timer_tick()         READ_ISA_REG(IREG_TIMER_E_COUNT)	//unit: 10us or 1/100ms, max: 0 ~ 0xFFFFFF
#define MS_MAX_TIMER_TICK           0xFFFFFF
#define TIMER_1US					1
#define TIMER_10US					(10*TIMER_1US)
#define TIMER_1MS					(100*TIMER_10US)
void ms_start_timer(unsigned long time_value);
int ms_check_timer(void);
int ms_check_timeout(void);

#define MS_MSPRO_DEBUG
#define Debug_Printf				printk
extern const unsigned short ms_crc_table[];

//#define inline _Inline
    
//Definition to use block address 0x3400000
//#define AMLOGIC_CHIP_SUPPORT
    
#ifdef AMLOGIC_CHIP_SUPPORT
#ifdef AVOS
#define WRITE_BYTE_TO_FIFO(DATA)	{WRITE_MPEG_REG(HFIFO_DATA,DATA);while((READ_MPEG_REG(BFIFO_LEVEL)>>8) >= 120){}}
#else				/*  */
#define WRITE_BYTE_TO_FIFO(DATA)    {Wr(HFIFO_DATA,DATA);while((Rd(BFIFO_LEVEL)>>8) >= 120){}}
#endif				/*  */
#endif				/*  */
    
#define MS_MSPRO_HW_CONTROL
#define MS_MSPRO_SW_CONTROL
    
#define MS_MSPRO_ALLOC_MEMORY
    
#ifdef MS_MSPRO_ALLOC_MEMORY
#define ms_mspro_malloc				kmalloc
#define ms_mspro_free				kfree
#endif				/*  */
    
//Definition for debug
#if ((!defined __ROM_) || (defined __ROM_ && defined __ROMDBG_))
#define MS_MSPRO_DEBUG
    //#define MS_MSPRO_CRC_CHECK
#endif				/*  */
#define MS_MSPRO_CRC_CHECK
//Delay time in 1 us
void ms_delay_us(unsigned long num_us);

//Delay time in 1 ms
void ms_delay_ms(unsigned long num_ms);

//Maximum 20Mhz, Period = 50ns
#define ms_clk_delay_serial_low()
#define ms_clk_delay_serial_high()
//Maximum 40Mhz, Period = 25ns
#define ms_clk_delay_parallel_low()
#define ms_clk_delay_parallel_high()
    
#define ms_clk_serial_low()         	{ms_set_clk_low();ms_clk_delay_serial_low();}
#define ms_clk_serial_high()        	{ms_set_clk_high();ms_clk_delay_serial_high();}
#define ms_clk_parallel_low()       	{ms_set_clk_low();ms_clk_delay_parallel_low();}
#define ms_clk_parallel_high()      	{ms_set_clk_high();ms_clk_delay_parallel_high();}
unsigned short ms_verify_crc16(unsigned char *ptr, unsigned int len);
unsigned short ms_cal_crc16(unsigned char *ptr, unsigned int len);

#endif				//_H_MS_MISC
