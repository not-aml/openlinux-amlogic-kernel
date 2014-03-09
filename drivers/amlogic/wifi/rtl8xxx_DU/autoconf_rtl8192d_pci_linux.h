/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/

/*
 * Public  General Config
 */
#define AUTOCONF_INCLUDED
#define RTL871X_MODULE_NAME "92DE"
#define DRV_NAME "rtl8192de"

#define CONFIG_PCI_HCI	1

#define CONFIG_RTL8192D	1

#define PLATFORM_LINUX	1

//#define CONFIG_IOCTL_CFG80211 1
#ifdef CONFIG_IOCTL_CFG80211
	//#define RTW_USE_CFG80211_STA_EVENT /* Indecate new sta asoc through cfg80211_new_sta */
	#define CONFIG_CFG80211_FORCE_COMPATIBLE_2_6_37_UNDER
	//#define CONFIG_DEBUG_CFG80211 1
	#define CONFIG_SET_SCAN_DENY_TIMER
#endif

/*
 * Internal  General Config
 */
//#define CONFIG_PWRCTRL	1
//#define CONFIG_H2CLBK 1

#define CONFIG_EMBEDDED_FWIMG	1

#define CONFIG_R871X_TEST	1

#define CONFIG_80211N_HT	1

#define CONFIG_RECV_REORDERING_CTRL	1

//#define CONFIG_TCP_CSUM_OFFLOAD_RX	1

//#define CONFIG_DRVEXT_MODULE	1

#ifndef CONFIG_MP_INCLUDED
	//#define CONFIG_IPS	1
	//#define CONFIG_LPS	1
	//#define CONFIG_BT_COEXIST  	1
	//#define SUPPORT_HW_RFOFF_DETECTED	1
#else
	#define CONFIG_MP_IWPRIV_SUPPORT	1
#endif

#define CONFIG_AP_MODE 1
#ifdef CONFIG_AP_MODE
	#define CONFIG_NATIVEAP_MLME 1
	#ifndef CONFIG_NATIVEAP_MLME
		#define CONFIG_HOSTAPD_MLME	1
	#endif			
	#define CONFIG_FIND_BEST_CHANNEL	1
#endif

//	Added by Albert 20110314
#define CONFIG_P2P	1
#ifdef CONFIG_P2P
	//Added by Albert 20110812
	//The CONFIG_WFD is for supporting the Wi-Fi display
	#define CONFIG_WFD

	#ifndef CONFIG_WIFI_TEST
		#define CONFIG_P2P_REMOVE_GROUP_INFO
	#endif
	//#define CONFIG_DBG_P2P

	#define CONFIG_P2P_PS
	//#define CONFIG_P2P_IPS
#endif

//	Added by Kurt 20110511
//#define CONFIG_TDLS 1

#define CONFIG_SKB_COPY	1//for amsdu

#define CONFIG_DFS	1

#define CONFIG_LED
#ifdef CONFIG_LED
	#define CONFIG_SW_LED
#endif //CONFIG_LED


#define CONFIG_NEW_SIGNAL_STAT_PROCESS
//#define CONFIG_SIGNAL_DISPLAY_DBM //display RX signal with dbm
#define RTW_NOTCH_FILTER 0 /* 0:Disable, 1:Enable,*/

/*
 * Interface  Related Config
 */


/*
 * HAL  Related Config
 */

#define RTL8192C_RX_PACKET_NO_INCLUDE_CRC	1

#define DISABLE_BB_RF	0	

#define RTL8191C_FPGA_NETWORKTYPE_ADHOC 0

#define TX_POWER_FOR_5G_BAND				1	//For 5G band TX Power

#define RTL8192D_EASY_SMART_CONCURRENT	0

#define RTL8192D_DUAL_MAC_MODE_SWITCH	0

#define SWLCK   1

#ifdef CONFIG_MP_INCLUDED
	#define MP_DRIVER	1
#else
	#define MP_DRIVER	0
#endif

#define CONFIG_80211D

#define CONFIG_ATTEMPT_TO_FIX_AP_BEACON_ERROR

/*
 * Debug  Related Config
 */
//#define CONFIG_DEBUG_RTL871X

#define DBG	0

#define CONFIG_DEBUG_RTL819X

#define CONFIG_PROC_DEBUG 1

//#define MEMORY_LEAK_DEBUG

