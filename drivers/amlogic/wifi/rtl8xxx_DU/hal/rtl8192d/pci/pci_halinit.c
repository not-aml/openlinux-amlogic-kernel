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
#define _HCI_HAL_INIT_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <rtw_efuse.h>

#include <rtl8192d_hal.h>
#include <rtl8192d_led.h>

#if defined (PLATFORM_LINUX) && defined (PLATFORM_WINDOWS)

#error "Shall be Linux or Windows, but not both!\n"

#endif

#ifndef CONFIG_PCI_HCI

#error "CONFIG_PCI_HCI shall be on!\n"

#endif

#include <pci_ops.h>
#include <pci_hal.h>
#include <pci_osintf.h>


// For Two MAC FPGA verify we must disable all MAC/BB/RF setting
#define FPGA_UNKNOWN		0
#define FPGA_2MAC			1
#define FPGA_PHY			2
#define FPGA_TYPE			FPGA_PHY

#if FPGA_TYPE == FPGA_2MAC
#define HAL_FW_ENABLE		0
#define HAL_MAC_ENABLE		0
#define HAL_BB_ENABLE		0
#define HAL_RF_ENABLE		0
#else
#define HAL_FW_ENABLE		1
#define HAL_MAC_ENABLE		1
#define HAL_BB_ENABLE		1
#define HAL_RF_ENABLE		1

#define FPGA_RF_UNKOWN	0
#define FPGA_RF_8225		1
#define FPGA_RF_0222D		2
#define FPGA_RF 				FPGA_RF_0222D

#endif

#if (RTL8192D_DUAL_MAC_MODE_SWITCH == 1)
extern BOOLEAN	GlobalFirstConfigurationForNormalChip;
#endif

extern atomic_t GlobalMutexForGlobalAdapterList;

//add mutex to solve the problem that reading efuse and power on/fw download do 
//on the same time 
extern atomic_t GlobalMutexForMac0_2G_Mac1_5G;
extern atomic_t GlobalMutexForPowerAndEfuse;
extern atomic_t GlobalMutexForPowerOnAndPowerOff;

//
//	Description:
//		Config HW adapter information into initial value.
//
//	Assumption:
//		1. After Auto load fail(i.e, check CR9346 fail)
//
//	Created by Roger, 2008.10.21.
//
static	VOID
ConfigAdapterInfo8192DForAutoLoadFail(
	IN PADAPTER			Adapter
)
{
	EEPROM_EFUSE_PRIV	*pEEPROM = GET_EEPROM_EFUSE_PRIV(Adapter);
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	u16	i;
	u8	sMacAddr[6] = {0x00, 0xE0, 0x4C, 0x81, 0x92, 0x00};
	u8	*hwinfo = 0;
#if (RTL8192D_DUAL_MAC_MODE_SWITCH == 1)
	u1Byte 			MacPhyCrValue = 0;
	PADAPTER		BuddyAdapter = Adapter->BuddyAdapter;
	HAL_DATA_TYPE	*pHalDataBuddyAdapter;
#endif

	DBG_8192C("====> ConfigAdapterInfo8192CForAutoLoadFail\n");
	// Marked by tynli. Suggested by Alfred.
	//rtw_write8(Adapter, REG_SYS_ISO_CTRL+1, 0xE8); // Isolation signals from Loader
	//rtw_udelay_os(10000);
	//rtw_write8(Adapter, REG_APS_FSMCO, 0x02); // Enable Loader Data Keep
	
	// Initialize IC Version && Channel Plan
	pHalData->EEPROMVID = 0;
	pHalData->EEPROMDID = 0;		
	pHalData->EEPROMChannelPlan = 0;
	pHalData->EEPROMCustomerID = 0;
	//cosa pHalData->bIgnoreDiffRateTxPowerOffset = FALSE;

	DBG_8192C("EEPROM VID = 0x%4x\n", pHalData->EEPROMVID);
	DBG_8192C("EEPROM DID = 0x%4x\n", pHalData->EEPROMDID);	
	DBG_8192C("EEPROM Customer ID: 0x%2x\n", pHalData->EEPROMCustomerID);
	DBG_8192C("EEPROM ChannelPlan = 0x%4x\n", pHalData->EEPROMChannelPlan);
	//cosa RT_TRACE(COMP_INIT, DBG_LOUD, ("IgnoreDiffRateTxPowerOffset = %d\n", pHalData->bIgnoreDiffRateTxPowerOffset));
       //
	//<Roger_Notes> In this case, we random assigh MAC address here. 2008.10.15.
	//
	//Initialize Permanent MAC address
	//if(!Adapter->bInHctTest)
       // sMacAddr[5] = (u1Byte)GetRandomNumber(1, 254);	
	for(i = 0; i < 6; i++)
		pEEPROM->mac_addr[i] = sMacAddr[i];
	
	//NicIFSetMacAddress(Adapter, Adapter->PermanentAddress);

	DBG_8192C("Permanent Address = %02x-%02x-%02x-%02x-%02x-%02x\n", 
	pEEPROM->mac_addr[0], pEEPROM->mac_addr[1], 
	pEEPROM->mac_addr[2], pEEPROM->mac_addr[3], 
	pEEPROM->mac_addr[4], pEEPROM->mac_addr[5]);
	
	//
	// Read tx power index from efuse or eeprom
	//
	//ReadTxPowerInfoFromHWPG(Adapter, pEEPROM->bautoload_fail_flag, hwinfo);
	rtl8192d_ReadTxPowerInfo(Adapter, hwinfo, pEEPROM->bautoload_fail_flag);
	
	//
	// Read Bluetooth co-exist and initialize
	//
#ifdef CONFIG_BT_COEXIST
	ReadBluetoothCoexistInfoFromHWPG(Adapter, pEEPROM->bautoload_fail_flag, hwinfo);
#endif
	
	//
	// Read EEPROM Version && Channel Plan
	//
	// Default channel plan  =0
	pHalData->EEPROMChannelPlan = 0;
	pHalData->EEPROMVersion = 1;		// Default version is 1
	pHalData->bTXPowerDataReadFromEEPORM = _FALSE;

	// 20100318 Joseph: These two variable is set in the beginning of the ReadAdapterInfo8192C().
	pHalData->rf_type = RTL819X_DEFAULT_RF_TYPE;	// default : 1T2R

#if (RTL8192D_DUAL_MAC_MODE_SWITCH == 1)
//SMSP->DMSP/DMDP, must to wait the other adapter complete 
//the wait 1s
	CheckInModeSwitchProcess(Adapter);


//Normal chip case for set MacPhyMode
	ACQUIRE_GLOBAL_MUTEX(GlobalMutexForGlobalAdapterList);
	if(GlobalFirstConfigurationForNormalChip)
	{
		RELEASE_GLOBAL_MUTEX(GlobalMutexForGlobalAdapterList);
		PHY_ReadMacPhyMode92D(Adapter, _TRUE);
		PHY_ConfigMacPhyMode92D( Adapter);
		ACQUIRE_GLOBAL_MUTEX(GlobalMutexForGlobalAdapterList);
		GlobalFirstConfigurationForNormalChip = _FALSE;
		RELEASE_GLOBAL_MUTEX(GlobalMutexForGlobalAdapterList);
	}
	else
	{
		RELEASE_GLOBAL_MUTEX(GlobalMutexForGlobalAdapterList);			
		PHY_ReadMacPhyMode92D(Adapter, _TRUE);
	}
	
#else
	PHY_ReadMacPhyMode92D(Adapter, _TRUE);
	PHY_ConfigMacPhyMode92D(Adapter);
#endif

	PHY_ConfigMacPhyModeInfo92D(Adapter);
	rtl8192d_ResetDualMacSwitchVariables(Adapter);

	pHalData->rf_chip = RF_6052;
	pHalData->EEPROMCustomerID = 0;

	DBG_8192C("EEPROM Customer ID: 0x%2x\n", pHalData->EEPROMCustomerID);
			
	pHalData->EEPROMBoardType = EEPROM_Default_BoardType;	
	DBG_8192C("BoardType = %#x\n", pHalData->EEPROMBoardType);

//	pHalData->LedStrategy = SW_LED_MODE0;

//1 JOSEPH_REVISE
	//InitRateAdaptive(Adapter);
	
	DBG_8192C("<==== ConfigAdapterInfo8192CForAutoLoadFail\n");
}

static VOID
HalCustomizedBehavior8192D(
	PADAPTER			Adapter
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);	
	EEPROM_EFUSE_PRIV	*pEEPROM = GET_EEPROM_EFUSE_PRIV(Adapter);
	struct led_priv	*pledpriv = &(Adapter->ledpriv);
	
	pledpriv->LedStrategy = SW_LED_MODE7; //Default LED strategy.

	switch(pHalData->CustomerID)
	{

		case RT_CID_DEFAULT:
			break;

		case RT_CID_TOSHIBA:
			pHalData->CurrentChannel = 10;
			//pHalData->EEPROMRegulatory = 1;
			break;

		case RT_CID_CCX:
			//pMgntInfo->IndicateByDeauth = _TRUE;
			break;

		case RT_CID_819x_Lenovo:
			// Customize Led mode	
			pledpriv->LedStrategy = SW_LED_MODE7;
			// Customize  Link any for auto connect
			// This Value should be set after InitializeMgntVariables
			//pMgntInfo->bAutoConnectEnable = FALSE;
			DBG_8192C("RT_CID_819x_Lenovo \n");
			break;

		case RT_CID_819x_HP:
			break;

		case RT_CID_819x_Acer:
			break;			

		case RT_CID_WHQL:
			//Adapter->bInHctTest = TRUE;
			break;

		case RT_CID_819x_PRONETS:
			pledpriv->LedStrategy = SW_LED_MODE9; // Customize Led mode	
			break;			
	
		default:
			DBG_8192C("Unkown hardware Type \n");
			break;
	}
	DBG_8192C("HalCustomizedBehavior8192D(): RT Customized ID: 0x%02X\n", pHalData->CustomerID);

	if(pHalData->EEPROMSMID >= 0x6000 && pHalData->EEPROMSMID < 0x7fff)
	{
		if(pHalData->CurrentWirelessMode != WIRELESS_MODE_B)
			pHalData->CurrentWirelessMode = WIRELESS_MODE_G;
	}
}

//
//	Description:
//		Read HW adapter information through EEPROM 93C46.
//		Or For EFUSE 92S .And Get and Set 92D MACPHY mode and Band Type.
//		MacPhyMode:DMDP,SMSP.
//		BandType:2.4G,5G.
//
//	Assumption:
//		1. Boot from EEPROM and CR9346 regiser has verified.
//		2. PASSIVE_LEVEL (USB interface)
//
static VOID ReadMacPhyModeFromPROM92DE(
		IN PADAPTER			Adapter,
		IN u8*				Content
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u8	MacPhyCrValue = Content[EEPROM_MAC_FUNCTION];
#if (RTL8192D_EASY_SMART_CONCURRENT == 1)	
	PADAPTER		BuddyAdapter= Adapter->BuddyAdapter;
#endif
		
	if(MacPhyCrValue & BIT3)//function mode
	{
		pHalData->MacPhyMode92D = SINGLEMAC_SINGLEPHY;
		DBG_8192C("MacPhyMode SINGLEMAC_SINGLEPHY \n");
	}
	else
	{
		pHalData->MacPhyMode92D = DUALMAC_DUALPHY;
		DBG_8192C("MacPhyMode DUALMAC_DUALPHY \n");
	}
		
}

/****************************************************************
	Function: syn concurrentMode for dual band
	2011/03/24 by sherry
	
*****************************************************************/
static VOID 
Rtl8192DSetTwoStaConcurrentMode(
	PADAPTER		Adapter
)
{
		PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
#if (RTL8192D_EASY_SMART_CONCURRENT == 1)
		PADAPTER 	BuddyAdapter = Adapter->BuddyAdapter;

		RT_TRACE(COMP_INIT,DBG_LOUD,("bSupportSTAAndAPSmartConcurrent %d \n",Adapter->DualMacEasyConcurrentControl.bSupportSTAAndAPSmartConcurrent));
		if(!Adapter->DualMacEasyConcurrentControl.bSupportSTAAndAPSmartConcurrent)
		{
			if(BuddyAdapter != NULL)
			{
				RT_TRACE(COMP_INIT,DBG_LOUD,("CurrentTwoStaConcurrentMode %d \n",Adapter->DualMacEasyConcurrentControl.CurrentTwoStaConcurrentMode));
				RT_TRACE(COMP_INIT,DBG_LOUD,("BuddyAdapter CurrentTwoStaConcurrentMode %d \n",BuddyAdapter->DualMacEasyConcurrentControl.CurrentTwoStaConcurrentMode));
				if(Adapter->DualMacEasyConcurrentControl.CurrentTwoStaConcurrentMode != BuddyAdapter->DualMacEasyConcurrentControl.CurrentTwoStaConcurrentMode)
				{
					RT_TRACE(COMP_EASY_CONCURRENT,DBG_LOUD,("ReadAdapterInfo(): MAC 0 and MAC 1 has different easy concurrent mode \n"));
					pHalData->MacPhyMode92D = GET_HAL_DATA(BuddyAdapter)->MacPhyMode92D;
					Adapter->DualMacEasyConcurrentControl.CurrentTwoStaConcurrentMode = BuddyAdapter->DualMacEasyConcurrentControl.CurrentTwoStaConcurrentMode;
				}
				else
				{
					RT_TRACE(COMP_EASY_CONCURRENT,DBG_LOUD,("ReadAdapterInfo(): MAC 0 and MAC 1 has same easy concurrent mode \n"));
					if(Adapter->DualMacEasyConcurrentControl.CurrentTwoStaConcurrentMode == RT_SECONDARY_DISABLE)
						pHalData->MacPhyMode92D = DUALMAC_SINGLEPHY;
					else
						pHalData->MacPhyMode92D = DUALMAC_DUALPHY;
				}
			}
			else
			{
				pHalData->MacPhyMode92D = DUALMAC_SINGLEPHY;
			}
		}
		else
		{
			pHalData->MacPhyMode92D = DUALMAC_SINGLEPHY;
		}	
#endif			
}

//
//	Description:
//		Read HW adapter information through EEPROM 93C46.
//		Or For EFUSE 92S .And Get and Set 92D MACPHY mode and Band Type.
//		MacPhyMode:DMDP,SMSP.
//		BandType:2.4G,5G.
//
//	Assumption:
//		1. Boot from EEPROM and CR9346 regiser has verified.
//		2. PASSIVE_LEVEL (USB interface)
//
static VOID Read92DMacPhyModeandBandType(
		IN PADAPTER			Adapter,
		IN u8*				Content
)
{
	//PMGNT_INFO		pMgntInfo = &(Adapter->MgntInfo);
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
#if (RTL8192D_DUAL_MAC_MODE_SWITCH == 1)
	PADAPTER		BuddyAdapter = Adapter->BuddyAdapter;
	u1Byte			MacPhyCrValue = 0;
	HAL_DATA_TYPE	*pHalDataBuddyAdapter;
	int				i;
#endif

#if (RTL8192D_DUAL_MAC_MODE_SWITCH == 1)
//SMSP->DMSP/DMDP, must to wait the other adapter complete 
//the wait 1s
	CheckInModeSwitchProcess(Adapter);
	
//Normal Chip,modify MACPHYCtrl definition
//Dual Phy/Dual Mac/Super Mac does not autoload 
	ACQUIRE_GLOBAL_MUTEX(GlobalMutexForGlobalAdapterList);
	if(GlobalFirstConfigurationForNormalChip)
	{
		RELEASE_GLOBAL_MUTEX(GlobalMutexForGlobalAdapterList);
		ReadMacPhyModeFromPROM92DE(Adapter, Content);
#if (RTL8192D_EASY_SMART_CONCURRENT == 1)
		if(pHalData->MacPhyMode92D != SINGLEMAC_SINGLEPHY)
			Rtl8192DSetTwoStaConcurrentMode(Adapter);
#endif						
		PHY_ConfigMacPhyMode92D(Adapter);

		ACQUIRE_GLOBAL_MUTEX(GlobalMutexForGlobalAdapterList);
		GlobalFirstConfigurationForNormalChip = _FALSE;
		RELEASE_GLOBAL_MUTEX(GlobalMutexForGlobalAdapterList);
	}
	else
	{
		RELEASE_GLOBAL_MUTEX(GlobalMutexForGlobalAdapterList);			
		PHY_ReadMacPhyMode92D(Adapter, _FALSE);
#if (RTL8192D_EASY_SMART_CONCURRENT == 1)
		RT_TRACE(COMP_INIT,DBG_LOUD,("pHalData->MacPhyMode92 aaaa  %d \n",pHalData->MacPhyMode92D));
		if(pHalData->MacPhyMode92D != SINGLEMAC_SINGLEPHY)
			Rtl8192DSetTwoStaConcurrentMode(Adapter);
#endif				
	}	
		
#else
		
	ReadMacPhyModeFromPROM92DE(Adapter, Content);
	PHY_ConfigMacPhyMode92D(Adapter);		
		

#endif
	PHY_ConfigMacPhyModeInfo92D(Adapter);	
	rtl8192d_ResetDualMacSwitchVariables(Adapter);

}

static	VOID
_ReadAdapterInfo8192DE(
	IN PADAPTER			Adapter
	)
{
	EEPROM_EFUSE_PRIV	*pEEPROM = GET_EEPROM_EFUSE_PRIV(Adapter);
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	u16			i,usValue;
	u16			EEPROMId;
	u8			tempval;
	u8			hwinfo[HWSET_MAX_SIZE];
	u8			MAC_ADDR;
	//DBG_8192C("====> _ReadAdapterInfo8192DE\n");

	if (pEEPROM->EepromOrEfuse == EEPROM_93C46)
	{	// Read frin EEPROM
		// Marked by tynli. Suggested by Alfred.
		//PlatformEFIOWrite1Byte(Adapter, REG_SYS_ISO_CTRL+1, 0xE8); // Isolation signals from Loader
		//PlatformStallExecution(10000);
		//PlatformEFIOWrite1Byte(Adapter, REG_APS_FSMCO, 0x02); // Enable Loader Data Keep

		//RT_TRACE(COMP_INIT, DBG_LOUD, ("EEPROM\n"));
		// Read all Content from EEPROM or EFUSE!!!
		//for(i = 0; i < HWSET_MAX_SIZE; i += 2)
		//{
		//	usValue = EF2Byte(ReadEEprom(Adapter, (u16) (i>>1)));
		//	*((u16 *)(&hwinfo[i])) = usValue;
		//}	
	}
	else if (pEEPROM->EepromOrEfuse == EEPROM_BOOT_EFUSE)
	{	// Read from EFUSE	
		//DBG_8192C("EFUSE\n");

		// Read EFUSE real map to shadow!!
		ACQUIRE_GLOBAL_MUTEX(GlobalMutexForPowerAndEfuse);
		EFUSE_ShadowMapUpdate(Adapter, EFUSE_WIFI, _FALSE);
		RELEASE_GLOBAL_MUTEX(GlobalMutexForPowerAndEfuse);
		_rtw_memcpy((void*)hwinfo, (void*)pEEPROM->efuse_eeprom_data, HWSET_MAX_SIZE);
	}

	// Print current HW setting map!!
	//RT_PRINT_DATA(COMP_INIT, DBG_LOUD, ("MAP \n"), hwinfo, HWSET_MAX_SIZE);			

	// Checl 0x8129 again for making sure autoload status!!
	EEPROMId = le16_to_cpu(*((u16 *)&hwinfo[0]));
	if( EEPROMId != RTL8190_EEPROM_ID )
	{
		DBG_8192C("EEPROM ID(%#x) is invalid!!\n", EEPROMId);
		pEEPROM->bautoload_fail_flag = _TRUE;
	}
	else
	{
		//DBG_8192C("Autoload OK\n");
		pEEPROM->bautoload_fail_flag = _FALSE;
	}	

	//
	if (pEEPROM->bautoload_fail_flag == _TRUE)
	{
		ConfigAdapterInfo8192DForAutoLoadFail(Adapter);
		return;
	}

	pHalData->EEPROMCustomerID = *(u8 *)&hwinfo[EEPROM_CUSTOMER_ID]; 	
	pHalData->EEPROMVID		= EF2Byte(*(u16 *)&hwinfo[EEPROM_VID]);
	pHalData->EEPROMDID		= EF2Byte(*(u16 *)&hwinfo[EEPROM_DID]);
	pHalData->EEPROMSVID	= EF2Byte(*(u16 *)&hwinfo[EEPROM_SVID]);
	pHalData->EEPROMSMID	= EF2Byte(*(u16 *)&hwinfo[EEPROM_SMID]);

	DBG_8192C("EEPROM Customer ID = 0x%4x\n", pHalData->EEPROMCustomerID);
	DBG_8192C("EEPROM VID = 0x%4x\n", pHalData->EEPROMVID);
	DBG_8192C("EEPROM DID = 0x%4x\n", pHalData->EEPROMDID);
	DBG_8192C("EEPROM SVID = 0x%4x\n", pHalData->EEPROMSVID);
	DBG_8192C("EEPROM SMID = 0x%4x\n", pHalData->EEPROMSMID);

	// If the customer ID had been changed by registry, do not cover up by EEPROM.
	if(pHalData->CustomerID == RT_CID_DEFAULT)
	{
		switch(pHalData->EEPROMCustomerID)
		{	
			case EEPROM_CID_DEFAULT:
				if(pHalData->EEPROMDID==0x8193)
				{
					if((pHalData->EEPROMSVID == 0x10EC && pHalData->EEPROMSMID == 0x8190)) //ProNets
						pHalData->CustomerID = RT_CID_819x_PRONETS;
					else if(pHalData->EEPROMSVID == 0x1025) //for Acer
						pHalData->CustomerID = RT_CID_819x_Acer;
					else
						pHalData->CustomerID = RT_CID_DEFAULT;
				}
				else
				{
					pHalData->CustomerID = RT_CID_DEFAULT;
				}
				break;

			case EEPROM_CID_TOSHIBA:       
				pHalData->CustomerID = RT_CID_TOSHIBA;
				break;

			case EEPROM_CID_QMI:
				pHalData->CustomerID = RT_CID_819x_QMI;
				break;

			case EEPROM_CID_WHQL:
				/*Adapter->bInHctTest = TRUE;
				
				pMgntInfo->bSupportTurboMode = FALSE;
				pMgntInfo->bAutoTurboBy8186 = FALSE;
				pMgntInfo->PowerSaveControl.bInactivePs = FALSE;
				pMgntInfo->PowerSaveControl.bIPSModeBackup = FALSE;
				pMgntInfo->PowerSaveControl.bLeisurePs = FALSE;
				pMgntInfo->keepAliveLevel = 0;	
				Adapter->bUnloadDriverwhenS3S4 = FALSE;
				pHalData->bEarlyModeEnable = _FALSE;*/
				break;

			default:
				pHalData->CustomerID = RT_CID_DEFAULT;
				break;

		}
	}

	//If is 92D, Get and Set MacPhy Mode and BandType.
	//And decide the Rf type.
	Read92DMacPhyModeandBandType(Adapter,hwinfo);

	//Read Permanent MAC address
	if(pHalData->interfaceIndex == 0){
		MAC_ADDR = EEPROM_MAC_ADDR_MAC0_92D;
		/*
		for(i = 0; i < 6; i += 2)
		{
			usValue = *(u16 *)&hwinfo[EEPROM_MAC_ADDR_MAC0_92D+i];
			*((u16 *)(&pEEPROM->mac_addr[i])) = usValue;
		}*/
	}
	else{
		MAC_ADDR =EEPROM_MAC_ADDR_MAC1_92D;
		/*for(i = 0; i < 6; i += 2)
		{
			usValue = *(u16 *)&hwinfo[EEPROM_MAC_ADDR_MAC1_92D+i];
			*((u16 *)(&pEEPROM->mac_addr[i])) = usValue;
		}*/
	}
	_rtw_memcpy(pEEPROM->mac_addr, (u8 *)&hwinfo[MAC_ADDR], MAC_ADDR_LEN);

	//NicIFSetMacAddress(Adapter, Adapter->PermanentAddress);

	DBG_8192C("ReadAdapterInfo8192DE(), Permanent Address = %02x-%02x-%02x-%02x-%02x-%02x\n", 
	pEEPROM->mac_addr[0], pEEPROM->mac_addr[1], 
	pEEPROM->mac_addr[2], pEEPROM->mac_addr[3], 
	pEEPROM->mac_addr[4], pEEPROM->mac_addr[5]); 

	//
	// Read tx power index from efuse or eeprom
	//
	//because the efuse is incompleted,we use default value by driver.
	rtl8192d_ReadTxPowerInfo(Adapter,hwinfo, pEEPROM->bautoload_fail_flag);//pHalData->AutoloadFailFlag);

	//
	// Read Bluetooth co-exist and initialize
	//
#ifdef CONFIG_BT_COEXIST
	ReadBluetoothCoexistInfoFromHWPG(Adapter, pEEPROM->bautoload_fail_flag, hwinfo);
#endif

	//Read Channel Plan
	rtl8192d_EfuseParseChnlPlan(Adapter,hwinfo, pEEPROM->bautoload_fail_flag);

	//
	// Read IC Version && Channel Plan
	//
	// Version ID, Channel plan
	//pHalData->EEPROMChannelPlan = *(u1Byte *)&hwinfo[EEPROM_CHANNEL_PLAN];
	pHalData->EEPROMVersion  = EF2Byte(*(u16 *)&hwinfo[EEPROM_VERSION]);
	pHalData->bTXPowerDataReadFromEEPORM = _TRUE;
	//DBG_8192C("EEPROM ChannelPlan = 0x%02x\n", pHalData->EEPROMChannelPlan);

	//
	// Read Customer ID or Board Type!!!
	//
#if 0
	tempval = 0; 
	// 2008/11/14 MH RTL8192SE_PCIE_EEPROM_SPEC_V0.6_20081111.doc
	// 2008/12/09 MH RTL8192SE_PCIE_EEPROM_SPEC_V0.7_20081201.doc
	// Change RF type definition
	if (tempval == 0)	// 2x2           RTL8192SE (QFN68)
		pHalData->rf_type = RF_2T2R;
	else if (tempval == 1)	 // 1x2           RTL8191RE (QFN68)
		pHalData->rf_type = RF_1T2R;
	else if (tempval == 2)	 //  1x2		RTL8191SE (QFN64) Crab
		pHalData->rf_type = RF_1T2R;
	else if (tempval == 3)	 //  1x1 		RTL8191SE (QFN64) Unicorn
		pHalData->rf_type = RF_1T1R;
#endif

	DBG_8192C("EEPROM Customer ID: 0x%02x\n", pHalData->EEPROMCustomerID);

	//pHalData->LedStrategy = SW_LED_MODE0;

	//DBG_8192C("MGNT Customer ID: 0x%02x\n", pHalData->CustomerID);
	
	//DBG_8192C("<==== _ReadAdapterInfo8192DE\n");
}

static int _ReadAdapterInfo8192D(PADAPTER Adapter)
{
	EEPROM_EFUSE_PRIV	*pEEPROM = GET_EEPROM_EFUSE_PRIV(Adapter);
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	struct mlme_priv		*pmlmepriv = &(Adapter->mlmepriv);
	struct registry_priv	*pregistrypriv = &Adapter->registrypriv;
	u8	tmpU1b;

	//DBG_8192C("====> ReadAdapterInfo8192C\n");

	// For debug test now!!!!!
	//PHY_RFShadowRefresh(Adapter);

	// Read IC Version && Channel Plan	
	//rtl8192d_ReadChipVersion(Adapter);

	DBG_8192C("VersionID = 0x%4x\n", pHalData->VersionID);

	tmpU1b = rtw_read8(Adapter, REG_9346CR);
	pHalData->AutoLoadStatusFor8192D = tmpU1b;

	if (tmpU1b & BIT4)
	{
		DBG_8192C("Boot from EEPROM\n");
		pEEPROM->EepromOrEfuse = EEPROM_93C46;
	}
	else 
	{
		DBG_8192C("Boot from EFUSE\n");
		pEEPROM->EepromOrEfuse = EEPROM_BOOT_EFUSE;
	}

	// Autoload OK
	if (tmpU1b & BIT5)
	{
		DBG_8192C("Autoload OK\n");
		pEEPROM->bautoload_fail_flag = _FALSE;
		_ReadAdapterInfo8192DE(Adapter);
	}	
	else
	{ 	// Auto load fail.		
		DBG_8192C("AutoLoad Fail reported from CR9346!!\n");
		pEEPROM->bautoload_fail_flag = _TRUE;
		ConfigAdapterInfo8192DForAutoLoadFail(Adapter);		

		if (pEEPROM->EepromOrEfuse == EEPROM_BOOT_EFUSE)
		{
			EFUSE_ShadowMapUpdate(Adapter, EFUSE_WIFI, _FALSE);
		}
	}	

	if(pHalData->CustomerID == RT_CID_DEFAULT)
	{ // If we have not yet change pMgntInfo->CustomerID in register, 
		switch(pHalData->EEPROMCustomerID)
		{
			case EEPROM_CID_DEFAULT:
				pHalData->CustomerID = RT_CID_DEFAULT;
				break;
			case EEPROM_CID_TOSHIBA:       
				pHalData->CustomerID = RT_CID_TOSHIBA;
				break;

			case EEPROM_CID_CCX:
				pHalData->CustomerID = RT_CID_CCX;
				break;
				
			default:
				// value from RegCustomerID
				break;
		}
	}

	if(pHalData->EEPROMSVID == 0x10EC && pHalData->EEPROMSMID == 0xE020)
		pHalData->CustomerID = RT_CID_819x_Lenovo;

#if 0
	switch(pHalData->BandSet92D){
		case BAND_ON_2_4G:
			pmlmepriv->ChannelPlan = RT_CHANNEL_DOMAIN_TELEC;
			break;
		case BAND_ON_5G:
			pmlmepriv->ChannelPlan = RT_CHANNEL_DOMAIN_FCC;
			break;
		case BAND_ON_BOTH:
			pmlmepriv->ChannelPlan = RT_CHANNEL_DOMAIN_FCC;
			break;
		default:
			break;
	}
#if (RTL8192D_DUAL_MAC_MODE_SWITCH == 1)
	pmlmepriv->ChannelPlan = RT_CHANNEL_DOMAIN_FCC;
#endif
#endif

	//MSG_8192C("RegChannelPlan(%d) EEPROMChannelPlan(%d)", pregistrypriv->channel_plan, pHalData->EEPROMChannelPlan);
	//MSG_8192C("ChannelPlan = %d\n" , pmlmepriv->ChannelPlan);

	HalCustomizedBehavior8192D(Adapter);	

	//DBG_8192C("<==== ReadAdapterInfo8192C\n");

	return _SUCCESS;
}

static void ReadAdapterInfo8192DE(PADAPTER Adapter)
{
	// Read EEPROM size before call any EEPROM function
	//Adapter->EepromAddressSize=Adapter->HalFunc.GetEEPROMSizeHandler(Adapter);
	Adapter->EepromAddressSize = GetEEPROMSize8192D(Adapter);
	
	_ReadAdapterInfo8192D(Adapter);
}


void rtl8192de_interface_configure(_adapter *padapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct dvobj_priv	*pdvobjpriv = adapter_to_dvobj(padapter);
	struct pwrctrl_priv	*pwrpriv = &padapter->pwrctrlpriv;

_func_enter_;

	////close ASPM for AMD defaultly
	pdvobjpriv->const_amdpci_aspm = 0;

	//// ASPM PS mode.
	//// 0 - Disable ASPM, 1 - Enable ASPM without Clock Req, 
	//// 2 - Enable ASPM with Clock Req, 3- Alwyas Enable ASPM with Clock Req,
	//// 4-  Always Enable ASPM without Clock Req.
	//// set defult to RTL8192CE:3 RTL8192E:2
	pdvobjpriv->const_pci_aspm = 0;

	//// Setting for PCI-E device */
	pdvobjpriv->const_devicepci_aspm_setting = 0x03;

	//// Setting for PCI-E bridge */
	pdvobjpriv->const_hostpci_aspm_setting = 0x02;

	//// In Hw/Sw Radio Off situation.
	//// 0 - Default, 1 - From ASPM setting without low Mac Pwr, 
	//// 2 - From ASPM setting with low Mac Pwr, 3 - Bus D3
	//// set default to RTL8192CE:0 RTL8192SE:2
	pdvobjpriv->const_hwsw_rfoff_d3 = 0;

	//// This setting works for those device with backdoor ASPM setting such as EPHY setting.
	//// 0: Not support ASPM, 1: Support ASPM, 2: According to chipset.
	pdvobjpriv->const_support_pciaspm = 1;

	pwrpriv->reg_rfoff = 0;
	pwrpriv->rfoff_reason = 0;

	pHalData->interfaceIndex = pdvobjpriv->InterfaceNumber;

_func_exit_;
}

VOID
DisableInterrupt8192DE (
	IN PADAPTER			Adapter
	)
{
	struct dvobj_priv	*pdvobjpriv = adapter_to_dvobj(Adapter);

	// Because 92SE now contain two DW IMR register range.
	rtw_write32(Adapter, REG_HIMR, IMR8190_DISABLED);	
	//RT_TRACE(COMP_INIT,DBG_LOUD,("***DisableInterrupt8192CE.\n"));
	// From WMAC code
	//PlatformEFIOWrite4Byte(Adapter, REG_HIMR+4,IMR, IMR8190_DISABLED);
	//RTPRINT(FISR, ISR_CHK, ("Disable IMR=%x"));

	rtw_write32(Adapter, REG_HIMRE, IMR8190_DISABLED);	// by tynli

	pdvobjpriv->irq_enabled = 0;

}

VOID
ClearInterrupt8192DE(
	IN PADAPTER			Adapter
	)
{
	u32	tmp = 0;
	tmp = rtw_read32(Adapter, REG_HISR);	
	rtw_write32(Adapter, REG_HISR, tmp);	

	tmp = 0;
	tmp = rtw_read32(Adapter, REG_HISRE);	
	rtw_write32(Adapter, REG_HISRE, tmp);	
}


VOID
EnableInterrupt8192DE(
	IN PADAPTER			Adapter
	)
{
	HAL_DATA_TYPE	*pHalData=GET_HAL_DATA(Adapter);
	struct dvobj_priv	*pdvobjpriv = adapter_to_dvobj(Adapter);

	pdvobjpriv->irq_enabled = 1;

	pHalData->IntrMask[0] = pHalData->IntrMaskToSet[0];
	pHalData->IntrMask[1] = pHalData->IntrMaskToSet[1];

	rtw_write32(Adapter, REG_HIMR, pHalData->IntrMask[0]&0xFFFFFFFF);

	rtw_write32(Adapter, REG_HIMRE, pHalData->IntrMask[1]&0xFFFFFFFF);

}

void
InterruptRecognized8192DE(
	IN	PADAPTER			Adapter,
	OUT	PRT_ISR_CONTENT	pIsrContent
	)
{
	HAL_DATA_TYPE	*pHalData=GET_HAL_DATA(Adapter);

	pIsrContent->IntArray[0] = rtw_read32(Adapter, REG_HISR);	
	pIsrContent->IntArray[0] &= pHalData->IntrMask[0];
	rtw_write32(Adapter, REG_HISR, pIsrContent->IntArray[0]);

	//For HISR extension. Added by tynli. 2009.10.07.
	pIsrContent->IntArray[1] = rtw_read32(Adapter, REG_HISRE);	
	pIsrContent->IntArray[1] &= pHalData->IntrMask[1];
	rtw_write32(Adapter, REG_HISRE, pIsrContent->IntArray[1]);

}

VOID
UpdateInterruptMask8192DE(
	IN	PADAPTER		Adapter,
	IN	u32		AddMSR,
	IN	u32		RemoveMSR
	)
{
	HAL_DATA_TYPE	*pHalData=GET_HAL_DATA(Adapter);

	if( AddMSR )
	{
		pHalData->IntrMaskToSet[0] |= AddMSR;
	}

	if( RemoveMSR )
	{
		pHalData->IntrMaskToSet[0] &= (~RemoveMSR);
	}

	DisableInterrupt8192DE( Adapter );
	EnableInterrupt8192DE( Adapter );
}


static VOID
HwConfigureRTL8192DE(
		IN	PADAPTER			Adapter
		)
{

	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u8	regBwOpMode = 0;
	u32	regRATR = 0, regRRSR = 0;


	//1 This part need to modified according to the rate set we filtered!!
	//
	// Set RRSR, RATR, and BW_OPMODE registers
	//
	switch(pHalData->CurrentWirelessMode)
	{
	case WIRELESS_MODE_B:
		regBwOpMode = BW_OPMODE_20MHZ;
		regRATR = RATE_ALL_CCK;
		regRRSR = RATE_ALL_CCK;
		break;
	case WIRELESS_MODE_A:
		regBwOpMode = BW_OPMODE_5G |BW_OPMODE_20MHZ;
		regRATR = RATE_ALL_OFDM_AG;
		regRRSR = RATE_ALL_OFDM_AG;
		break;
	case WIRELESS_MODE_G:
		regBwOpMode = BW_OPMODE_20MHZ;
		regRATR = RATE_ALL_CCK | RATE_ALL_OFDM_AG;
		regRRSR = RATE_ALL_CCK | RATE_ALL_OFDM_AG;
		break;
	case WIRELESS_MODE_UNKNOWN:
	case WIRELESS_MODE_AUTO:
	case WIRELESS_MODE_N_24G:
		// It support CCK rate by default.
		// CCK rate will be filtered out only when associated AP does not support it.
		regBwOpMode = BW_OPMODE_20MHZ;
		regRATR = RATE_ALL_CCK | RATE_ALL_OFDM_AG | RATE_ALL_OFDM_1SS | RATE_ALL_OFDM_2SS;
		regRRSR = RATE_ALL_CCK | RATE_ALL_OFDM_AG;
		break;
	case WIRELESS_MODE_N_5G:
		regBwOpMode = BW_OPMODE_5G;
		regRATR = RATE_ALL_OFDM_AG | RATE_ALL_OFDM_1SS | RATE_ALL_OFDM_2SS;
		regRRSR = RATE_ALL_OFDM_AG;
		break;
	default:
		break;
	}

	rtw_write8(Adapter, REG_INIRTS_RATE_SEL, 0x8);

	// 2007/02/07 Mark by Emily becasue we have not verify whether this register works	
	//For 92C,which reg?
	rtw_write8(Adapter, REG_BWOPMODE, regBwOpMode);


	// Init value for RRSR.
	rtw_write32(Adapter, REG_RRSR, regRRSR);

	// Set SLOT time
	rtw_write8(Adapter,REG_SLOT, 0x09);

	// Set AMPDU min space
	rtw_write8(Adapter,REG_AMPDU_MIN_SPACE, 0x0);

	// CF-End setting.
	rtw_write16(Adapter,REG_FWHW_TXQ_CTRL, 0x1F80);

	// Set retry limit
	//3vivi, 20100928, especially for DTM, performance_ext, To avoid asoc too long to another AP more than 4.1 seconds.
	//3we find retry 7 times maybe not enough, so we retry more than 7 times to pass DTM.
	//if(!Adapter->bInHctTest)
		rtw_write16(Adapter,REG_RL, 0x0707);
	//else
	//	rtw_write16(Adapter,REG_RL, 0x3030);

	// BAR settings
	rtw_write32(Adapter, REG_BAR_MODE_CTRL, 0x02012802);

	// HW SEQ CTRL
	rtw_write8(Adapter,REG_HWSEQ_CTRL, 0xFF); //set 0x0 to 0xFF by tynli. Default enable HW SEQ NUM.

	// Set Data / Response auto rate fallack retry count
	rtw_write32(Adapter, REG_DARFRC, 0x01000000);
	rtw_write32(Adapter, REG_DARFRC+4, 0x07060504);
	rtw_write32(Adapter, REG_RARFRC, 0x01000000);
	rtw_write32(Adapter, REG_RARFRC+4, 0x07060504);

#ifdef CONFIG_BT_COEXIST
	if(	(pHalData->bt_coexist.BluetoothCoexist) &&
		(pHalData->bt_coexist.BT_CoexistType == BT_CSR) )
	{
		rtw_write32(Adapter, REG_AGGLEN_LMT, 0x97427431);
		//RTPRINT(FBT, BT_TRACE, ("BT write 0x%x = 0x97427431\n", REG_AGGLEN_LMT));
	}
	else
#endif
	{
		// Aggregation threshold
		if(pHalData->MacPhyMode92D == DUALMAC_DUALPHY)
			rtw_write32(Adapter, REG_AGGLEN_LMT, 0xb9726641);
		else if(pHalData->MacPhyMode92D == DUALMAC_SINGLEPHY)
			rtw_write32(Adapter, REG_AGGLEN_LMT, 0x66666641);
		else
			rtw_write32(Adapter, REG_AGGLEN_LMT, 0xb972a841);
	}
	
	// Beacon related, for rate adaptive
	rtw_write8(Adapter, REG_ATIMWND, 0x2);
	// Change  0xff to 0x0A. Advised by TimChen. 2009.01.25. by tynli.
	rtw_write8(Adapter, REG_BCN_MAX_ERR, 0x0a);
	
	
	// 20100211 Joseph: Change original setting of BCN_CTRL(0x550) from 
	// 0x1e(0x2c for test chip) ro 0x1f(0x2d for test chip). Set BIT0 of this register disable ATIM
	// function. Since we do not use HIGH_QUEUE anymore, ATIM function is no longer used.
	// Also, enable ATIM function may invoke HW Tx stop operation. This may cause ping failed
	// sometimes in long run test. So just disable it now.

	pHalData->RegBcnCtrlVal = 0x1f;
	//PlatformAtomicExchange((pu4Byte)(&pHalData->RegBcnCtrlVal), 0x1d);
	
	rtw_write8(Adapter, REG_BCN_CTRL, (u8)(pHalData->RegBcnCtrlVal));

	// TBTT prohibit hold time. Suggested by designer TimChen.
	rtw_write8(Adapter, REG_TBTT_PROHIBIT+1,0xff); // 8 ms

	// 20091211 Joseph: Do not set 0x551[1] suggested by Scott.
	// 
	// Disable BCNQ SUB1 0x551[1]. Suggested by TimChen. 2009.12.04. by tynli.
	// For protecting HW to decrease the TSF value when temporarily the real TSF value 
	// is smaller than the TSF counter.
	//regTmp = PlatformEFIORead1Byte(Adapter, REG_USTIME_TSF);
	//PlatformEFIOWrite1Byte(Adapter, REG_USTIME_TSF, (regTmp|BIT1)); // 8 ms

	rtw_write8(Adapter, REG_PIFS, 0x1C);
	rtw_write8(Adapter, REG_AGGR_BREAK_TIME, 0x16);

	{
		rtw_write16(Adapter, REG_NAV_PROT_LEN, 0x0040);
		rtw_write16(Adapter, REG_PROT_MODE_CTRL, 0x08ff);
	}

	if(!Adapter->registrypriv.wifi_spec)
	{
		//For Rx TP. Suggested by SD1 Richard. Added by tynli. 2010.04.12.
		rtw_write32(Adapter, REG_FAST_EDCA_CTRL, 0x03086666);
	}
	else
	{
		//For WiFi WMM. suggested by timchen. Added by tynli.	
		rtw_write16(Adapter, REG_FAST_EDCA_CTRL, 0x0);
	}

#if FPGA_2MAC
	// ACKTO for IOT issue.
	rtw_write8(Adapter, REG_ACKTO, 0x40);

	// Set Spec SIFS (used in NAV)
	rtw_write16(Adapter,REG_SPEC_SIFS, 0x1010);
	rtw_write16(Adapter,REG_MAC_SPEC_SIFS, 0x1010);

	// Set SIFS for CCK
	rtw_write16(Adapter,REG_SIFS_CTX, 0x1010);	

	// Set SIFS for OFDM
	rtw_write16(Adapter,REG_SIFS_TRX, 0x1010);

#else
	// ACKTO for IOT issue.
	rtw_write8(Adapter, REG_ACKTO, 0x40);

	// Set Spec SIFS (used in NAV)
	//92d all set these register 0x1010, check it later
	rtw_write16(Adapter,REG_SPEC_SIFS, 0x100a);
	rtw_write16(Adapter,REG_MAC_SPEC_SIFS, 0x100a);

	// Set SIFS for CCK
	rtw_write16(Adapter,REG_SIFS_CTX, 0x100a);	

	// Set SIFS for OFDM
	rtw_write16(Adapter,REG_SIFS_TRX, 0x100a);
#endif

	// Set Multicast Address. 2009.01.07. by tynli.
	rtw_write32(Adapter, REG_MAR, 0xffffffff);
	rtw_write32(Adapter, REG_MAR+4, 0xffffffff);

	//Reject all control frame - default value is 0
	rtw_write16(Adapter,REG_RXFLTMAP1,0x0);

	// Accept all data frames
	//rtw_write16(Adapter, REG_RXFLTMAP2, 0xFFFF);

	//Nav limit , suggest by scott
	rtw_write8(Adapter, 0x652, 0x0);

	//For 92C,how to??
	//PlatformEFIOWrite1Byte(Adapter, MLT, 0x8f);

	// Set Contention Window here		

	// Set Tx AGC

	// Set Tx Antenna including Feedback control
		
	// Set Auto Rate fallback control
				
	//
	// For Min Spacing configuration.
	//
//1 JOSEPH_REVISE
#if 0
	Adapter->MgntInfo.MinSpaceCfg = 0x90;	//cosa, asked by scott, for MCS15 short GI, padding patch, 0x237[7:3] = 0x12.
	rtw_hal_set_hwreg(Adapter, HW_VAR_AMPDU_MIN_SPACE, (pu1Byte)(&Adapter->MgntInfo.MinSpaceCfg));	
        switch(pHalData->RF_Type)
	{
		case RF_1T2R:
		case RF_1T1R:
			RT_TRACE(COMP_INIT, DBG_LOUD, ("Initializeadapter: RF_Type%s\n", (pHalData->RF_Type==RF_1T1R? "(1T1R)":"(1T2R)")));
			Adapter->MgntInfo.MinSpaceCfg = (MAX_MSS_DENSITY_1T<<3);						
			break;
		case RF_2T2R:
		case RF_2T2R_GREEN:
			RT_TRACE(COMP_INIT, DBG_LOUD, ("Initializeadapter:RF_Type(2T2R)\n"));
			Adapter->MgntInfo.MinSpaceCfg = (MAX_MSS_DENSITY_2T<<3);			
			break;
	}
	PlatformEFIOWrite1Byte(Adapter, AMPDU_MIN_SPACE, Adapter->MgntInfo.MinSpaceCfg);
#endif 
}

static u32
_LLTWrite(
	IN  PADAPTER	Adapter,
	IN	u32		address,
	IN	u32		data
	)
{
	u32	status = _SUCCESS;
	s32	count = 0;
	u32	value = _LLT_INIT_ADDR(address) | _LLT_INIT_DATA(data) | _LLT_OP(_LLT_WRITE_ACCESS);

	rtw_write32(Adapter, REG_LLT_INIT, value);

	//polling
	do{

		value = rtw_read32(Adapter, REG_LLT_INIT	);
		if(_LLT_NO_ACTIVE == _LLT_OP_VALUE(value)){
			break;
		}

		if(count > POLLING_LLT_THRESHOLD){
			DBG_8192C("Failed to polling write LLT done at address %x!\n", address);
			status = _FAIL;
			break;
		}
	}while(++count);

	return status;

}


static u32
LLT_table_init(
	IN PADAPTER	Adapter
	)
{
	u16	i;
	HAL_DATA_TYPE* pHalData = GET_HAL_DATA(Adapter);
	u8	txpktbuf_bndy;
	u8	maxPage;
	u32	status;
	u32	value32;	//High+low page number
	u8	value8;	//normal page number

	if(pHalData->MacPhyMode92D == SINGLEMAC_SINGLEPHY){
		maxPage = 255;
		txpktbuf_bndy = 246;
		value8 = 0;
		if(Adapter->registrypriv.wifi_spec)
			value32 = 0x80b01c29;
		else
			value32 = 0x80bf0d29;
	}
	else if(pHalData->MacPhyMode92D != SINGLEMAC_SINGLEPHY){
		maxPage = 127;
		txpktbuf_bndy = 123;
		value8 = 0;
		value32 = 0x80720005;//0x805a1010; // 0x804b1010
	}

	// Set reserved page for each queue
	// 11.	RQPN 0x200[31:0]	= 0x80BD1C1C				// load RQPN
	rtw_write8(Adapter,REG_RQPN_NPQ, 0x03); // Justin: reserve 3 pages for normal Q
	rtw_write32(Adapter,REG_RQPN, value32);

	// 12.	TXRKTBUG_PG_BNDY 0x114[31:0] = 0x27FF00F6	//TXRKTBUG_PG_BNDY	
	rtw_write32(Adapter,REG_TRXFF_BNDY, (rtw_read16(Adapter, REG_TRXFF_BNDY+2) << 16|txpktbuf_bndy));

	// 13.	TDECTRL[15:8] 0x209[7:0] = 0xF6				// Beacon Head for TXDMA
	rtw_write8(Adapter,REG_TDECTRL+1, txpktbuf_bndy);

	// 14.	BCNQ_PGBNDY 0x424[7:0] =  0xF6				//BCNQ_PGBNDY
	// 2009/12/03 Why do we set so large boundary. confilct with document V11.
	rtw_write8(Adapter,REG_TXPKTBUF_BCNQ_BDNY, txpktbuf_bndy);
	rtw_write8(Adapter,REG_TXPKTBUF_MGQ_BDNY, txpktbuf_bndy);

	// 15.	WMAC_LBK_BF_HD 0x45D[7:0] =  0xF6			//WMAC_LBK_BF_HD
	rtw_write8(Adapter,0x45D, txpktbuf_bndy);
	
	// Set Tx/Rx page size (Tx must be 128 Bytes, Rx can be 64,128,256,512,1024 bytes)
	// 16.	PBP [7:0] = 0x11								// TRX page size
	rtw_write8(Adapter,REG_PBP, 0x11);		

	// 17.	DRV_INFO_SZ
	rtw_write8(Adapter,REG_RX_DRVINFO_SZ, DRVINFO_SZ);

	// 18.	LLT_table_init(Adapter); 
	for(i = 0 ; i < (txpktbuf_bndy - 1) ; i++){
		status = _LLTWrite(Adapter, i , i + 1);
		if(_SUCCESS != status){
			return status;
		}
	}

	// end of list
	status = _LLTWrite(Adapter, (txpktbuf_bndy - 1), 0xFF); 
	if(_SUCCESS != status){
		return status;
	}

	// Make the other pages as ring buffer
	// This ring buffer is used as beacon buffer if we config this MAC as two MAC transfer.
	// Otherwise used as local loopback buffer. 
	for(i = txpktbuf_bndy ; i < maxPage ; i++){
		status = _LLTWrite(Adapter, i, (i + 1)); 
		if(_SUCCESS != status){
			return status;
		}
	}

	// Let last entry point to the start entry of ring buffer
	status = _LLTWrite(Adapter, maxPage, txpktbuf_bndy);
	if(_SUCCESS != status)
	{
		return status;
	}	
	
	return _SUCCESS;
	
}

static u32 InitMAC(IN	PADAPTER Adapter)
{
	u8	bytetmp;
	u16	wordtmp;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	struct recv_priv	*precvpriv = &Adapter->recvpriv;
	struct xmit_priv	*pxmitpriv = &Adapter->xmitpriv;
	u16	retry = 0;
	u16	tmpU2b = 0;

	DBG_8192C("=======>InitMAC()\n");

	PHY_SetPowerOnFor8192D(Adapter);

	//2009.12.23. Added by tynli. We should disable PCI host L0s. Suggested by SD1 victorh.
	//Merged by sherry 20100713
	//LOS is determined by BIOS
	//RT_DISABLE_HOST_L0S(Adapter);
	
	// 2009/10/13 MH Enable backdoor.
	//PlatformEnable92CEBackDoor(Adapter);

	//
	// 2009/10/13 MH Add for resume sequence of power domain from document of Afred...
	// 2009/12/03 MH Modify according to power document V11. Chapter V.11.
	//	
	// 0.	RSV_CTRL 0x1C[7:0] = 0x00			// unlock ISO/CLK/Power control register
	rtw_write8(Adapter,REG_RSV_CTRL, 0x00);

	//need not to do it.
	//rtw_write8(Adapter, REG_SYS_FUNC_EN, 0xE0);
	rtw_write8(Adapter, REG_LDOA15_CTRL,0x05);

#ifdef CONFIG_BT_COEXIST
	// For Bluetooth Power save
	// 209/12/03 MH The setting is not written power document now. ?????
	if(	(pHalData->bt_coexist.BluetoothCoexist) &&
		(pHalData->bt_coexist.BT_CoexistType == BT_CSR)	)
	{
		u32	value32;
		value32 = rtw_read32(Adapter, REG_APS_FSMCO);
		value32 |= (SOP_ABG|SOP_AMB|XOP_BTCK);
		rtw_write32(Adapter, REG_APS_FSMCO, value32);
	}
#endif

	// 1.	AFE_XTAL_CTRL [7:0] = 0x0F		//enable XTAL
	// 2.	SPS0_CTRL 0x11[7:0] = 0x2b		//enable SPS into PWM mode
	// 3.	delay (1ms) 			//this is not necessary when initially power on
	
	// C.	Resume Sequence
	// a.	SPS0_CTRL 0x11[7:0] = 0x2b
	rtw_write8(Adapter,REG_SPS0_CTRL, 0x2b);
	
	// b.	AFE_XTAL_CTRL [7:0] = 0x0F
	rtw_write8(Adapter, REG_AFE_XTAL_CTRL, 0x0F);
	
	// c.	DRV runs power on init flow

#ifdef CONFIG_BT_COEXIST
	// Temporarily fix system hang problem.
	if(	(pHalData->bt_coexist.BT_Coexist) /*&&
		(pHalData->bt_coexist.BT_CoexistType == BT_CSR_BC4)*/	)
	{
		u32 u4bTmp = rtw_read32(Adapter, REG_AFE_XTAL_CTRL);
	
		//AFE_XTAL_CTRL+2 0x26[1] = 1
		u4bTmp &= (~0x00024800);
		rtw_write32(Adapter, REG_AFE_XTAL_CTRL, u4bTmp);
	}
#endif

	// auto enable WLAN
	// 4.	APS_FSMCO 0x04[8] = 1; //wait till 0x04[8] = 0	
	//Power On Reset for MAC Block
	bytetmp = rtw_read8(Adapter,REG_APS_FSMCO+1) | BIT0;
	rtw_udelay_os(2);
	rtw_write8(Adapter,REG_APS_FSMCO+1, bytetmp);
	//DBG_8192C("Reg0xEC = %x\n", rtw_read32(Adapter, 0xEC));
	rtw_udelay_os(2);

	// 5.	Wait while 0x04[8] == 0 goto 2, otherwise goto 1
	// 2009/12/03 MH The document V11 loop is not the same current code.
	bytetmp = rtw_read8(Adapter,REG_APS_FSMCO+1);
	//DBG_8192C("Reg0xEC = %x\n", rtw_read32(Adapter, 0xEC));
	rtw_udelay_os(2);
	retry = 0;
	//DBG_8192C("################>%s()Reg0xEC:%x:%x\n", __FUNCTION__,rtw_read32(Adapter, 0xEC),bytetmp);
	while((bytetmp & BIT0) && retry < 1000){
		retry++;
		rtw_udelay_os(50);
		bytetmp = rtw_read8(Adapter,REG_APS_FSMCO+1);
		//DBG_8192C("################>%s()Reg0xEC:%x:%x\n", __FUNCTION__,rtw_read32(Adapter, 0xEC),bytetmp);
		rtw_udelay_os(50);
	}

	// Enable Radio off, GPIO, and LED function
	// 6.	APS_FSMCO 0x04[15:0] = 0x0012		//when enable HWPDN
	rtw_write16(Adapter, REG_APS_FSMCO, 0x1012); //tynli_test to 0x1012. SD1. 2009.12.08.

	 // release RF digital isolation
	 // 7.	SYS_ISO_CTRL 0x01[1]	= 0x0;
	//bytetmp = PlatformEFIORead1Byte(Adapter, REG_SYS_ISO_CTRL+1) & ~BIT1; //marked by tynli.
	//PlatformSleepUs(2);
	//Set REG_SYS_ISO_CTRL 0x1=0x82 to prevent wake# problem. Suggested by Alfred.
	// 2009/12/03 MH We need to check this modification
	rtw_write8(Adapter, REG_SYS_ISO_CTRL+1, 0x82);
	rtw_udelay_os(2);		// By experience!!??

#ifdef CONFIG_BT_COEXIST
	// Enable MAC DMA/WMAC/SCHEDULE block
	// 8.	AFE_XTAL_CTRL [17] = 0;			//with BT, driver will set in card disable
	if(	(pHalData->bt_coexist.BT_Coexist) /*&&
		(pHalData->bt_coexist.BT_CoexistType == BT_CSR_BC4)	*/) //tynli_test 2009.12.16.
	{
		bytetmp = rtw_read8(Adapter, REG_AFE_XTAL_CTRL+2) & 0xfd;
		rtw_write8(Adapter, REG_AFE_XTAL_CTRL+2, bytetmp);
	}
#endif

	// Disable REG_CR before enable it to assure reset
	rtw_write16(Adapter,REG_CR, 0x0);

	// Release MAC IO register reset
	rtw_write16(Adapter,REG_CR, 0x2ff);

      	// clear stopping tx/rx dma  
      	rtw_write8(Adapter,	REG_PCIE_CTRL_REG+1, 0x0);

	//rtw_write16(Adapter,REG_CR+2, 0x2);

	//WritePortUchar(0x553, 0xff);  //reset ??? richard 0507
	// What is the meaning???
	//PlatformEFIOWrite1Byte(Adapter,0x553, 0xFF);	
	
	//PlatformSleepUs(1500);
	//bytetmp = PlatformEFIORead1Byte(Adapter,REG_CR);
	//bytetmp |= (BIT6 | BIT7);//(CmdTxEnb|CmdRxEnb);
	//PlatformEFIOWrite1Byte(Adapter,REG_CR, bytetmp);

	// 2009/12/03 MH The section of initialize code does not exist in the function~~~!
	// These par is inserted into function LLT_table_init
	// 11.	RQPN 0x200[31:0]	= 0x80BD1C1C				// load RQPN
	// 12.	TXRKTBUG_PG_BNDY 0x114[31:0] = 0x27FF00F6	//TXRKTBUG_PG_BNDY
	// 13.	TDECTRL[15:8] 0x209[7:0] = 0xF6				// Beacon Head for TXDMA
	// 14.	BCNQ_PGBNDY 0x424[7:0] =  0xF6				//BCNQ_PGBNDY
	// 15.	WMAC_LBK_BF_HD 0x45D[7:0] =  0xF6			//WMAC_LBK_BF_HD
	// 16.	PBP [7:0] = 0x11								// TRX page size
	// 17.	DRV_INFO_SZ = 0x04
	
	//System init
	// 18.	LLT_table_init(Adapter); 
	if(LLT_table_init(Adapter) == _FAIL)
	{
		return _FAIL;
	}

	// Clear interrupt and enable interrupt
	// 19.	HISR 0x124[31:0] = 0xffffffff; 
	//	   	HISRE 0x12C[7:0] = 0xFF
	// NO 0x12c now!!!!!
	rtw_write32(Adapter,REG_HISR, 0xffffffff);
	rtw_write8(Adapter,REG_HISRE, 0xff);
	
	// 20.	HIMR 0x120[31:0] |= [enable INT mask bit map]; 
	// 21.	HIMRE 0x128[7:0] = [enable INT mask bit map]
	// The IMR should be enabled later after all init sequence is finished.

	// ========= PCIE related register setting =======
	// 22.	PCIE configuration space configuration
	// 23.	Ensure PCIe Device 0x80[15:0] = 0x0143 (ASPM+CLKREQ), 
	//          and PCIe gated clock function is enabled.	
	// PCIE configuration space will be written after all init sequence.(Or by BIOS)

	PHY_ConfigMacCoexist_RFPage92D(Adapter);

	//
	// 2009/12/03 MH THe below section is not related to power document Vxx .
	// This is only useful for driver and OS setting.
	//
	// -------------------Software Relative Setting----------------------
	//

	wordtmp = rtw_read16(Adapter,REG_TRXDMA_CTRL);
	wordtmp &= 0xf;
	//wordtmp |= 0xF771;
	// Justin: the normal priority DMA dedicate for Management Queue, don't share this DMA with
	// data (BK,BE,VO,VI) queuq, otherwise management frame may get stuck when TX paused
	wordtmp |= 0xE771;	// HIQ->hi, MGQ->normal, BKQ->low, BEQ->hi, VIQ->low, VOQ->hi

	rtw_write16(Adapter,REG_TRXDMA_CTRL, wordtmp);
	
	// Reported Tx status from HW for rate adaptive.
	// 2009/12/03 MH This should be realtive to power on step 14. But in document V11  
	// still not contain the description.!!!
	rtw_write8(Adapter,REG_FWHW_TXQ_CTRL+1, 0x1F);

	// Set RCR register
	rtw_write32(Adapter,REG_RCR, pHalData->ReceiveConfig);
	
	// Set TCR register
	rtw_write32(Adapter,REG_TCR, pHalData->TransmitConfig);

	// disable earlymode
	rtw_write8(Adapter,0x4d0, 0x0);
		
	//
	// Set TX/RX descriptor physical address(from OS API).
	//
	rtw_write32(Adapter, REG_BCNQ_DESA, (u64)pxmitpriv->tx_ring[BCN_QUEUE_INX].dma & DMA_BIT_MASK(32));
	rtw_write32(Adapter, REG_MGQ_DESA, (u64)pxmitpriv->tx_ring[MGT_QUEUE_INX].dma & DMA_BIT_MASK(32));
	rtw_write32(Adapter, REG_VOQ_DESA, (u64)pxmitpriv->tx_ring[VO_QUEUE_INX].dma & DMA_BIT_MASK(32));
	rtw_write32(Adapter, REG_VIQ_DESA, (u64)pxmitpriv->tx_ring[VI_QUEUE_INX].dma & DMA_BIT_MASK(32));
	rtw_write32(Adapter, REG_BEQ_DESA, (u64)pxmitpriv->tx_ring[BE_QUEUE_INX].dma & DMA_BIT_MASK(32));
	rtw_write32(Adapter, REG_BKQ_DESA, (u64)pxmitpriv->tx_ring[BK_QUEUE_INX].dma & DMA_BIT_MASK(32));
	rtw_write32(Adapter, REG_HQ_DESA, (u64)pxmitpriv->tx_ring[HIGH_QUEUE_INX].dma & DMA_BIT_MASK(32));
	rtw_write32(Adapter, REG_RX_DESA, (u64)precvpriv->rx_ring[RX_MPDU_QUEUE].dma & DMA_BIT_MASK(32));

#ifdef CONFIG_64BIT_DMA
	// 2009/10/28 MH For DMA 64 bits. We need to assign the high 32 bit address
	// for NIC HW to transmit data to correct path.
	rtw_write32(Adapter, REG_BCNQ_DESA+4, 
		((u64)pxmitpriv->tx_ring[BCN_QUEUE_INX].dma)>>32);
	rtw_write32(Adapter, REG_MGQ_DESA+4, 
		((u64)pxmitpriv->tx_ring[MGT_QUEUE_INX].dma)>>32);  
	rtw_write32(Adapter, REG_VOQ_DESA+4, 
		((u64)pxmitpriv->tx_ring[VO_QUEUE_INX].dma)>>32);
	rtw_write32(Adapter, REG_VIQ_DESA+4, 
		((u64)pxmitpriv->tx_ring[VI_QUEUE_INX].dma)>>32);
	rtw_write32(Adapter, REG_BEQ_DESA+4, 
		((u64)pxmitpriv->tx_ring[BE_QUEUE_INX].dma)>>32);
	rtw_write32(Adapter, REG_BKQ_DESA+4,
		((u64)pxmitpriv->tx_ring[BK_QUEUE_INX].dma)>>32);
	rtw_write32(Adapter,REG_HQ_DESA+4,
		((u64)pxmitpriv->tx_ring[HIGH_QUEUE_INX].dma)>>32);
	rtw_write32(Adapter, REG_RX_DESA+4, 
		((u64)precvpriv->rx_ring[RX_MPDU_QUEUE].dma)>>32);


	// 2009/10/28 MH If RX descriptor address is not equal to zero. We will enable
	// DMA 64 bit functuion.
	// Note: We never saw thd consition which the descripto address are divided into
	// 4G down and 4G upper seperate area.
	if (((u64)precvpriv->rx_ring[RX_MPDU_QUEUE].dma)>>32 != 0)
	{
		//DBG_8192C("RX_DESC_HA=%08lx\n", ((u64)priv->rx_ring_dma[RX_MPDU_QUEUE])>>32);
		DBG_8192C("Enable DMA64 bit\n");

		// Check if other descriptor address is zero and abnormally be in 4G lower area.
		if (((u64)pxmitpriv->tx_ring[MGT_QUEUE_INX].dma)>>32)
		{
			DBG_8192C("MGNT_QUEUE HA=0\n");
		}
		
		PlatformEnable92DEDMA64(Adapter);
	}
	else
	{
		DBG_8192C("Enable DMA32 bit\n");
	}
#endif

	// 2009/12/03 MH This should be included in power
	//PlatformEFIOWrite1Byte(Adapter,REG_PCIE_CTRL_REG+3, 0x77);
	// Set to 0x22. Suggested to SD1 Alan. by tynli. 2009.12.10.
	
	rtw_write8(Adapter,REG_PCIE_CTRL_REG+3, 0x33);
	

	// 20100318 Joseph: Reset interrupt migration setting when initialization. Suggested by SD1.
	//migration, 92d just for normal chip, vivi, 20100708
	
	rtw_write32(Adapter, REG_INT_MIG, 0);	
	pHalData->bInterruptMigration = _FALSE;
 	

	// 20090928 Joseph: Add temporarily.
	// Reconsider when to do this operation after asking HWSD.
	bytetmp = rtw_read8(Adapter, REG_APSD_CTRL);
	rtw_write8(Adapter, REG_APSD_CTRL, bytetmp & ~BIT6);
	do{
		retry++;
		bytetmp = rtw_read8(Adapter, REG_APSD_CTRL);
	}while((retry<200) && (bytetmp&BIT7)); //polling until BIT7 is 0. by tynli

	// 2009/10/26 MH For led test.
	// After MACIO reset,we must refresh LED state.
	rtl8192de_gen_RefreshLedState(Adapter);

	//2009.10.19. Reset H2C protection register. by tynli.
	rtw_write32(Adapter, REG_MCUTST_1, 0x0);

	rtl8192d_PHY_InitRxSetting(Adapter);

	//
	// -------------------Software Relative Setting----------------------
	//

	DBG_8192C("<=======InitMAC()\n");

	return _SUCCESS;
	
}

static VOID
EnableAspmBackDoor92DE(IN	PADAPTER Adapter)
{
	struct pwrctrl_priv		*pwrpriv = &Adapter->pwrctrlpriv;

	// 0x70f BIT7 is used to control L0S
	// 20100212 Tynli: Set register offset 0x70f in PCI configuration space to the value 0x23 
	// for all bridge suggested by SD1. Origianally this is only for INTEL.
	// 20100422 Joseph: Set PCI configuration space offset 0x70F to 0x93 to Enable L0s for all platform.
	// This is suggested by SD1 Glayrainx and for Lenovo's request.
	//if(GetPciBridgeVendor(Adapter) == PCI_BRIDGE_VENDOR_INTEL)
		rtw_write8(Adapter, 0x34b, 0x93);
	//else
	//	PlatformEFIOWrite1Byte(Adapter, 0x34b, 0x23);
	rtw_write16(Adapter, 0x350, 0x870c);
	rtw_write8(Adapter, 0x352, 0x1);

	// 0x719 Bit3 is for L1 BIT4 is for clock request 
	// 20100427 Joseph: Disable L1 for Toshiba AMD platform. If AMD platform do not contain
	// L1 patch, driver shall disable L1 backdoor.
	if(pwrpriv->b_support_backdoor)
		rtw_write8(Adapter, 0x349, 0x1b);
	else
		rtw_write8(Adapter, 0x349, 0x03);
	rtw_write16(Adapter, 0x350, 0x2718);
	rtw_write8(Adapter, 0x352, 0x1);
}

//Added just for 92DE testchip.
//When the device manager can't find MAC1 in DMDP mode,we force the MAC1 to show.
VOID PlatformForceEnableDualMac(
		IN		PADAPTER		Adapter	)
{	
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);	
	u32	ulResult=0;
	//PCI configure space 0xe.
	ulResult =MpReadPCIDwordDBI8192D(Adapter,0xc,0);
		
	if(!(ulResult &BIT23) ){
		ulResult |= BIT23; //multi-function bit.
		MpWritePCIDwordDBI8192D(Adapter, 0xc, ulResult, 0);
	}	
}

static u32 rtl8192de_hal_init(PADAPTER Adapter)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	EEPROM_EFUSE_PRIV	*pEEPROM = GET_EEPROM_EFUSE_PRIV(Adapter);
	struct pwrctrl_priv		*pwrpriv = &Adapter->pwrctrlpriv;
	u32	rtStatus = _SUCCESS;
	u8	tmpU1b;
	u8	eRFPath;
	u32	i;
	BOOLEAN bSupportRemoteWakeUp;
#ifdef CONFIG_DUALMAC_CONCURRENT
	PADAPTER			BuddyAdapter = Adapter->pbuddy_adapter;
	PHAL_DATA_TYPE		pHalDataBuddyAdapter;
#endif
	u32	RegRCR;
	
_func_enter_;

	//
	// No I/O if device has been surprise removed
	//
	if (Adapter->bSurpriseRemoved)
	{
		DBG_8192C("rtl8192de_hal_init(): bSurpriseRemoved!\n");
		return _SUCCESS;
	}

	Adapter->init_adpt_in_progress = _TRUE;
	DBG_8192C("=======>MAC%d:InitializeAdapter8192DE()\n",pHalData->interfaceIndex);
	ACQUIRE_GLOBAL_MUTEX(GlobalMutexForPowerAndEfuse);

#ifdef CONFIG_DUALMAC_CONCURRENT
	if(BuddyAdapter != NULL)
	{
		if(BuddyAdapter->bHaltInProgress)
		{
			for(i=0;i<100;i++)
			{
				rtw_usleep_os(1000);
				if(!BuddyAdapter->bHaltInProgress)
					break;
			}

			if(i==100)
			{
				DBG_871X("fail to initialization due to another adapter is in halt \n");
				RELEASE_GLOBAL_MUTEX(GlobalMutexForPowerAndEfuse);
				return _FAIL;
			}
		}
	}
#endif

	//rtl8192de_reset_desc_ring(Adapter);

	//Let the first starting mac load RF parameters in this case,by wl
	if(pHalData->MacPhyMode92D == DUALMAC_DUALPHY 
		&& ((pHalData->interfaceIndex == 0 && pHalData->BandSet92D == BAND_ON_2_4G)
			|| (pHalData->interfaceIndex == 1 && pHalData->BandSet92D == BAND_ON_5G)))
		ACQUIRE_GLOBAL_MUTEX(GlobalMutexForMac0_2G_Mac1_5G);

	//
	// 1. MAC Initialize
	//
	rtStatus = InitMAC(Adapter);
	if(rtStatus != _SUCCESS)
	{
		DBG_8192C("Init MAC failed\n");
		RELEASE_GLOBAL_MUTEX(GlobalMutexForPowerAndEfuse);
		if(pHalData->MacPhyMode92D == DUALMAC_DUALPHY 
			&& ((pHalData->interfaceIndex == 0 && pHalData->BandSet92D == BAND_ON_2_4G)
			|| (pHalData->interfaceIndex == 1 && pHalData->BandSet92D == BAND_ON_5G)))
			RELEASE_GLOBAL_MUTEX(GlobalMutexForMac0_2G_Mac1_5G);
		return rtStatus;
	}

#if (MP_DRIVER != 1)
#if HAL_FW_ENABLE
	rtStatus = FirmwareDownload92D(Adapter, _FALSE);
	RELEASE_GLOBAL_MUTEX(GlobalMutexForPowerAndEfuse);
	if(rtStatus != _SUCCESS)
	{
		DBG_8192C("FwLoad failed\n");
		rtStatus = _SUCCESS;
		Adapter->bFWReady = _FALSE;
		pHalData->fw_ractrl = _FALSE;
		rtw_write8(Adapter, 0x3a, 0x66);

		//return fail only when part number check fail,suggest by alex
		if(0xE0 == rtw_read8(Adapter, 0x1c5))
		{
			if(pHalData->MacPhyMode92D == DUALMAC_DUALPHY 
			&& ((pHalData->interfaceIndex == 0 && pHalData->BandSet92D == BAND_ON_2_4G)
				|| (pHalData->interfaceIndex == 1 && pHalData->BandSet92D == BAND_ON_5G)))
				RELEASE_GLOBAL_MUTEX(GlobalMutexForMac0_2G_Mac1_5G);		
			return rtStatus;
		}
	}
	else
	{
		DBG_8192C("FwLoad SUCCESSFULLY!!!\n");
		Adapter->bFWReady = _TRUE;
		pHalData->fw_ractrl = _TRUE;
	}

	//Init H2C counter. by tynli. 2009.12.09.
	pHalData->LastHMEBoxNum = 0;
	Adapter->pwrctrlpriv.bFwCurrentInPSMode = _FALSE;
#endif
#endif

#if FPGA_RF==FPGA_RF_8225
	pHalData->rf_chip = RF_8225;
	pHalData->rf_type = RF_2T2R;
#elif FPGA_RF==FPGA_RF_0222D
	pHalData->rf_chip = RF_6052;
	//pHalData->RF_Type = (pHalData->VersionID & BIT0)?RF_2T2R:RF_1T1R;	
	//RT_TRACE(COMP_INIT,DBG_LOUD,("The RF_Type:%d",pHalData->RF_Type));
#endif

#if FPGA_TYPE==FPGA_2MAC
	pHalData->rf_chip = RF_PSEUDO_11N; //RF_8225
	pHalData->rf_type = RF_2T2R;	// default : 1T2R
#endif

#ifdef CONFIG_BT_COEXIST
	if(	(pHalData->bt_coexist.BluetoothCoexist) &&
		(pHalData->bt_coexist.BT_CoexistType == BT_CSR)	)
	{
		//bluetooth Temp test, should be removed in the future.
		pHalData->rf_type = RF_1T1R;
		RTPRINT(FBT, BT_TRACE, ("BT temp set RF = 1T1R\n"));
	}
#endif

	tmpU1b = rtw_read8(Adapter,0x605);
	tmpU1b = tmpU1b|0x30;
	rtw_write8(Adapter,0x605,tmpU1b);
	
	if(pHalData->bEarlyModeEnable)
	{
		DBG_8192C("EarlyMode Enabled!!!\n");

		tmpU1b = rtw_read8(Adapter,0x4d0);
		tmpU1b = tmpU1b|0x1f;
		rtw_write8(Adapter,0x4d0,tmpU1b);

		rtw_write8(Adapter,0x4d3,0x80);

		tmpU1b = rtw_read8(Adapter,0x605);
		tmpU1b = tmpU1b|0x40;
		rtw_write8(Adapter,0x605,tmpU1b);
	}

	if(pHalData->bRDGEnable)
	{
		rtw_write8(Adapter,REG_RD_CTRL,0xff);
		rtw_write16(Adapter,REG_RD_NAV_NXT,0x200);
		rtw_write8(Adapter,REG_RD_RESP_PKT_TH,0x05);
	}

	//
	// 2. Initialize MAC/PHY Config by MACPHY_reg.txt
	//
#if (HAL_MAC_ENABLE == 1)
	DBG_8192C("MAC Config Start!\n");
	rtStatus = PHY_MACConfig8192D(Adapter);
	if (rtStatus != _SUCCESS)
	{
		DBG_8192C("MAC Config failed\n");
		if(pHalData->MacPhyMode92D == DUALMAC_DUALPHY 
		&& ((pHalData->interfaceIndex == 0 && pHalData->BandSet92D == BAND_ON_2_4G)
			|| (pHalData->interfaceIndex == 1 && pHalData->BandSet92D == BAND_ON_5G)))
			RELEASE_GLOBAL_MUTEX(GlobalMutexForMac0_2G_Mac1_5G);
		return rtStatus;
	}
	DBG_8192C("MAC Config Finished!\n");

	rtw_write32(Adapter,REG_RCR, rtw_read32(Adapter, REG_RCR)&~(RCR_ADF) );
#endif	// #if (HAL_MAC_ENABLE == 1)

	//
	// 3. Initialize BB After MAC Config PHY_reg.txt, AGC_Tab.txt
	//
#if (HAL_BB_ENABLE == 1)
	DBG_8192C("BB Config Start!\n");
	rtStatus = PHY_BBConfig8192D(Adapter);
	if (rtStatus!= _SUCCESS)
	{
		DBG_8192C("BB Config failed\n");
		if(pHalData->MacPhyMode92D == DUALMAC_DUALPHY 
		&& ((pHalData->interfaceIndex == 0 && pHalData->BandSet92D == BAND_ON_2_4G)
			|| (pHalData->interfaceIndex == 1 && pHalData->BandSet92D == BAND_ON_5G)))
			RELEASE_GLOBAL_MUTEX(GlobalMutexForMac0_2G_Mac1_5G);
		return rtStatus;
	}
	DBG_8192C("BB Config Finished!\n");
#endif	// #if (HAL_BB_ENABLE == 1)

#ifdef CONFIG_DUALMAC_CONCURRENT
	if(pHalData->bSlaveOfDMSP)
	{
		DBG_8192C("slave of dmsp close phy1 \n");
		PHY_StopTRXBeforeChangeBand8192D(Adapter);
	}
#endif

	//
	// 4. Initiailze RF RAIO_A.txt RF RAIO_B.txt
	//
	// 2007/11/02 MH Before initalizing RF. We can not use FW to do RF-R/W.
	//pHalData->Rf_Mode = RF_OP_By_SW_3wire;
#if (HAL_RF_ENABLE == 1)		
	DBG_8192C("RF Config started!\n");

	// set before initialize RF, suggest by yn
	PHY_SetBBReg(Adapter, rFPGA0_AnalogParameter4, 0x00f00000,  0xf);

	rtStatus = PHY_RFConfig8192D(Adapter);
	if(rtStatus != _SUCCESS)
	{
		DBG_8192C("RF Config failed\n");
		if(pHalData->MacPhyMode92D == DUALMAC_DUALPHY 
		&& ((pHalData->interfaceIndex == 0 && pHalData->BandSet92D == BAND_ON_2_4G)
			|| (pHalData->interfaceIndex == 1 && pHalData->BandSet92D == BAND_ON_5G)))
			RELEASE_GLOBAL_MUTEX(GlobalMutexForMac0_2G_Mac1_5G);
		return rtStatus;
	}
	DBG_8192C("RF Config Finished!\n");

	// After read predefined TXT, we must set BB/MAC/RF register as our requirement
	//After load BB,RF params,we need do more for 92D.
#ifdef CONFIG_DUALMAC_CONCURRENT
	if(!pHalData->bSlaveOfDMSP)
#endif
		PHY_UpdateBBRFConfiguration8192D(Adapter, _FALSE);
	// set default value after initialize RF, 
	PHY_SetBBReg(Adapter, rFPGA0_AnalogParameter4, 0x00f00000,  0);

#ifdef CONFIG_DUALMAC_CONCURRENT
	if(BuddyAdapter != NULL)
	{
		pHalDataBuddyAdapter = GET_HAL_DATA(BuddyAdapter);
		if(pHalData->bSlaveOfDMSP)
		{
			pHalData->RfRegChnlVal[0]  = pHalDataBuddyAdapter->RfRegChnlVal[0] ;
			pHalData->RfRegChnlVal[1]  = pHalDataBuddyAdapter->RfRegChnlVal[1] ;
		}
		else
		{
			pHalData->RfRegChnlVal[0] = PHY_QueryRFReg(Adapter, (RF_RADIO_PATH_E)0, RF_CHNLBW, bRFRegOffsetMask);
			pHalData->RfRegChnlVal[1] = PHY_QueryRFReg(Adapter, (RF_RADIO_PATH_E)1, RF_CHNLBW, bRFRegOffsetMask);
		}
	}
	else
	{
		pHalData->RfRegChnlVal[0] = PHY_QueryRFReg(Adapter, (RF_RADIO_PATH_E)0, RF_CHNLBW, bRFRegOffsetMask);
		pHalData->RfRegChnlVal[1] = PHY_QueryRFReg(Adapter, (RF_RADIO_PATH_E)1, RF_CHNLBW, bRFRegOffsetMask);
	}
#else
	pHalData->RfRegChnlVal[0] = PHY_QueryRFReg(Adapter, (RF_RADIO_PATH_E)0, RF_CHNLBW, bRFRegOffsetMask);
	pHalData->RfRegChnlVal[1] = PHY_QueryRFReg(Adapter, (RF_RADIO_PATH_E)1, RF_CHNLBW, bRFRegOffsetMask);
#endif

#ifdef CONFIG_DUALMAC_CONCURRENT
	if(!pHalData->bSlaveOfDMSP)
#endif
	{
		/*---- Set CCK and OFDM Block "ON"----*/
		if(pHalData->CurrentBandType92D == BAND_ON_2_4G)
			PHY_SetBBReg(Adapter, rFPGA0_RFMOD, bCCKEn, 0x1);
		PHY_SetBBReg(Adapter, rFPGA0_RFMOD, bOFDMEn, 0x1);
	}

#if (MP_DRIVER == 0)
	// Set to 20MHz by default
	if(pHalData->interfaceIndex == 0)
	{
#ifdef CONFIG_DUALMAC_CONCURRENT
		if(!pHalData->bSlaveOfDMSP)
#endif
			//rFPGA0_AnalogParameter2: cck clock select,  set to 20MHz by default
	    		PHY_SetBBReg(Adapter, rFPGA0_AnalogParameter2, BIT10|BIT11, 3);
	}
	else	//Mac1
	{
#ifdef CONFIG_DUALMAC_CONCURRENT
		if(!pHalData->bSlaveOfDMSP)
#endif
		{			
			PHY_SetBBReg(Adapter, rFPGA0_AnalogParameter2, BIT11|BIT10, 3);
		}
	}
#endif	// #if (MP_DRIVER == 0)

#endif	// #if (HAL_RF_ENABLE == 1)	

	pHalData->CurrentWirelessMode = WIRELESS_MODE_AUTO;

	//3 Set Hardware(MAC default setting.)
	HwConfigureRTL8192DE(Adapter);

	//3 Set Wireless Mode
	// TODO: Emily 2006.07.13. Wireless mode should be set according to registry setting and RF type
	//Default wireless mode is set to "WIRELESS_MODE_N_24G|WIRELESS_MODE_G", 
	//and the RRSR is set to Legacy OFDM rate sets. We do not include the bit mask 
	//of WIRELESS_MODE_B currently. Emily, 2006.11.13
	//For wireless mode setting from mass. 
	//if(Adapter->ResetProgress == RESET_TYPE_NORESET)
	//	Adapter->HalFunc.SetWirelessModeHandler(Adapter, Adapter->RegWirelessMode);

	//3Security related
	//-----------------------------------------------------------------------------
	// Set up security related. 070106, by rcnjko:
	// 1. Clear all H/W keys.
	// 2. Enable H/W encryption/decryption.
	//-----------------------------------------------------------------------------
	// 92SE not enable security now
	{
		u8 SECR_value = 0x0;

		invalidate_cam_all(Adapter);

		SECR_value |= (BIT6|BIT7);

		// Joseph debug: MAC_SEC_EN need to be set
		rtw_write8(Adapter, REG_CR+1, (rtw_read8(Adapter, REG_CR+1)|BIT1));

		rtw_write8(Adapter, REG_SECCFG, SECR_value);
	}

	pHalData->CurrentChannel = 6;//default set to 6

	//
	// Read EEPROM TX power index and PHY_REG_PG.txt to capture correct
	// TX power index for different rate set.
	//
#ifdef CONFIG_DUALMAC_CONCURRENT
	if(!pHalData->bSlaveOfDMSP)
#endif
	{
		/* Get original hw reg values */
		rtl8192d_PHY_GetHWRegOriginalValue(Adapter);
		/* Write correct tx power index */
		PHY_SetTxPowerLevel8192D(Adapter, pHalData->CurrentChannel);
	}

	//2=======================================================
	// RF Power Save
	//2=======================================================
#if 1
	// Fix the bug that Hw/Sw radio off before S3/S4, the RF off action will not be executed 
	// in MgntActSet_RF_State() after wake up, because the value of pHalData->eRFPowerState 
	// is the same as eRfOff, we should change it to eRfOn after we config RF parameters.
	// Added by tynli. 2010.03.30.
	pwrpriv->rf_pwrstate = rf_on;

	// 20100326 Joseph: Copy from GPIOChangeRFWorkItemCallBack() function to check HW radio on/off.
	// 20100329 Joseph: Revise and integrate the HW/SW radio off code in initialization.
	tmpU1b = rtw_read8(Adapter, REG_MAC_PINMUX_CFG)&(~BIT3);
	rtw_write8(Adapter, REG_MAC_PINMUX_CFG, tmpU1b);
	tmpU1b = rtw_read8(Adapter, REG_GPIO_IO_SEL);
	DBG_8192C("GPIO_IN=%02x\n", tmpU1b);
	pwrpriv->rfoff_reason |= (tmpU1b & BIT3) ? 0 : RF_CHANGE_BY_HW;
	pwrpriv->rfoff_reason |= (pwrpriv->reg_rfoff) ? RF_CHANGE_BY_SW : 0;

	if(pwrpriv->rfoff_reason & RF_CHANGE_BY_HW)
		pwrpriv->b_hw_radio_off = _TRUE;

	if(pwrpriv->rfoff_reason > RF_CHANGE_BY_PS)
	{ // H/W or S/W RF OFF before sleep.
		DBG_8192C("InitializeAdapter8192DE(): Turn off RF for RfOffReason(%d) ----------\n", pwrpriv->rfoff_reason);
		//MgntActSet_RF_State(Adapter, rf_off, pwrpriv->rfoff_reason, _TRUE);
	}
	else
	{
		pwrpriv->rf_pwrstate = rf_on;
		pwrpriv->rfoff_reason = 0;

		DBG_8192C("InitializeAdapter8192DE(): Turn on  ----------\n");

		// LED control
		rtw_led_control(Adapter, LED_CTL_POWER_ON);

		//
		// If inactive power mode is enabled, disable rf while in disconnected state.
		// But we should still tell upper layer we are in rf on state.
		// 2007.07.16, by shien chang.
		//
		//if(!Adapter->bInHctTest)
			//IPSEnter(Adapter);
	}
#endif

	// Fix the bug that when the system enters S3/S4 then tirgger HW radio off, after system
	// wakes up, the scan OID will be set from upper layer, but we still in RF OFF state and scan
	// list is empty, such that the system might consider the NIC is in RF off state and will wait 
	// for several seconds (during this time the scan OID will not be set from upper layer anymore)
	// even though we have already HW RF ON, so we tell the upper layer our RF state here.
	// Added by tynli. 2010.04.01.
	//DrvIFIndicateCurrentPhyStatus(Adapter);

	if(Adapter->registrypriv.hw_wps_pbc)
	{
		tmpU1b = rtw_read8(Adapter, GPIO_IO_SEL);
		tmpU1b &= ~(HAL_8192C_HW_GPIO_WPS_BIT);
		rtw_write8(Adapter, GPIO_IO_SEL, tmpU1b);	//enable GPIO[2] as input mode
	}

	//
	// Execute TX power tracking later
	//

	hal_init_macaddr(Adapter);

	// Joseph. Turn on the secret lock of ASPM.
	EnableAspmBackDoor92DE(Adapter);

#ifdef CONFIG_BT_COEXIST
	_InitBTCoexist(Adapter);
#endif

	rtl8192d_InitHalDm(Adapter);

	//EnableInterrupt8192CE(Adapter);

#if (MP_DRIVER == 1)
	Adapter->mppriv.channel = pHalData->CurrentChannel;
	MPT_InitializeAdapter(Adapter, Adapter->mppriv.channel);
#else
#ifdef CONFIG_DUALMAC_CONCURRENT
	if(!pHalData->bSlaveOfDMSP)
#endif
	{
		if(pwrpriv->rf_pwrstate == rf_on)
		{
			//rtl8192d_PHY_IQCalibrate(Adapter);
			rtl8192d_dm_CheckTXPowerTracking(Adapter);
			rtl8192d_PHY_LCCalibrate(Adapter);
			if(pHalData->MacPhyMode92D == DUALMAC_DUALPHY 
			&& ((pHalData->interfaceIndex == 0 && pHalData->BandSet92D == BAND_ON_2_4G)
				|| (pHalData->interfaceIndex == 1 && pHalData->BandSet92D == BAND_ON_5G)))
				RELEASE_GLOBAL_MUTEX(GlobalMutexForMac0_2G_Mac1_5G);
			//5G and 2.4G must wait sometime to let RF LO ready
			//by sherry 2010.06.28
#if SWLCK == 0
			if(pHalData->MacPhyMode92D == DUALMAC_DUALPHY)
			{
				u32 tmpRega, tmpRegb;
				for(i=0;i<10000;i++)
				{
					rtw_udelay_os(MAX_STALL_TIME);

					tmpRega = PHY_QueryRFReg(Adapter, (RF_RADIO_PATH_E)RF_PATH_A, 0x2a, bMaskDWord);
					//tmpRegb = PHY_QueryRFReg(Adapter, (RF_RADIO_PATH_E)RF_PATH_B, 0x2a, bMaskDWord);

					if(((tmpRega&BIT11)==BIT11))//&&((tmpRegb&BIT11)==BIT11))
						break;
				}
			}
#endif
		}
	}
#endif

#if 0
	//WoWLAN setting. by tynli.
	rtw_hal_get_def_var(Adapter, HAL_DEF_WOWLAN , &bSupportRemoteWakeUp);
	if(bSupportRemoteWakeUp) // WoWLAN setting. by tynli.
	{
		u1Byte	u1bTmp;
		u1Byte	i;
#if 0
		u4Byte	u4bTmp;

		//Disable L2 support
		u4bTmp = PlatformEFIORead4Byte(Adapter, REG_PCIE_CTRL_REG);
		u4bTmp &= ~(BIT17);
		 PlatformEFIOWrite4Byte(Adapter, REG_PCIE_CTRL_REG, u4bTmp);
#endif

		// enable Rx DMA. by tynli.
		u1bTmp = PlatformEFIORead1Byte(Adapter, REG_RXPKT_NUM+2);
		u1bTmp &= ~(BIT2);
		PlatformEFIOWrite1Byte(Adapter, REG_RXPKT_NUM+2, u1bTmp);

		if(pPSC->WoWLANMode == eWakeOnMagicPacketOnly)
		{
			//Enable magic packet and WoWLAN function in HW.
			PlatformEFIOWrite1Byte(Adapter, REG_WOW_CTRL, WOW_MAGIC);
		}
		else if (pPSC->WoWLANMode == eWakeOnPatternMatchOnly)
		{
			//Enable pattern match and WoWLAN function in HW.
			PlatformEFIOWrite1Byte(Adapter, REG_WOW_CTRL, WOW_WOMEN);
		}
		else if (pPSC->WoWLANMode == eWakeOnBothTypePacket)
		{
			//Enable magic packet, pattern match, and WoWLAN function in HW.
			PlatformEFIOWrite1Byte(Adapter, REG_WOW_CTRL, WOW_MAGIC|WOW_WOMEN);
		}
		
		PlatformClearPciPMEStatus(Adapter);

		if(ADAPTER_TEST_STATUS_FLAG(Adapter, ADAPTER_STATUS_FIRST_INIT))
		{
			//Reset WoWLAN register and related data structure at the first init. 2009.06.18. by tynli.
			ResetWoLPara(Adapter);
		}
		else
		{
			if(pPSC->WoWLANMode > eWakeOnMagicPacketOnly)
			{
				//Rewrite WOL pattern and mask to HW.
				for(i=0; i<(MAX_SUPPORT_WOL_PATTERN_NUM-2); i++)
				{
					rtw_hal_set_hwreg(Adapter, HW_VAR_WF_MASK, (pu1Byte)(&i)); 
					rtw_hal_set_hwreg(Adapter, HW_VAR_WF_CRC, (pu1Byte)(&i)); 
				}
			}
		}
	}
#endif

	// 2009/06/10 MH For 92S 1*1=1R/ 1*2&2*2 use 2R. We default set 1*1 use radio A
	// So if you want to use radio B. Please modify RF path enable bit for correct signal
	// strength calculate.
	// For 5G band,These params must be set.
	if (pHalData->rf_type == RF_1T1R){
		pHalData->bRFPathRxEnable[0] = _TRUE;
	}
	else{
		pHalData->bRFPathRxEnable[0] = pHalData->bRFPathRxEnable[1] = _TRUE;
	}

	PHY_InitPABias92D(Adapter);

	pHalData->RegFwHwTxQCtrl = rtw_read8(Adapter, REG_FWHW_TXQ_CTRL+2);

	Adapter->init_adpt_in_progress = _FALSE;

/*{
	DBG_8192C("===== Start Dump Reg =====");
	for(i = 0 ; i <= 0xeff ; i+=4)
	{
		if(i%16==0)
			DBG_8192C("\n%04x: ",i);
		DBG_8192C("0x%08x ",rtw_read32(Adapter, i));
	}
	DBG_8192C("\n ===== End Dump Reg =====\n");
}*/

_func_exit_;

	return rtStatus;
}

//
// 2009/10/13 MH Acoording to documetn form Scott/Alfred....
// This is based on version 8.1.
//
static VOID
PowerOffAdapter8192DE(
	IN	PADAPTER			Adapter	
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u8	u1bTmp;
	u32	u4bTmp;
	
	DBG_8192C("=======>PowerOffAdapter8192DE()\n");
	// XVI.1.1 PCIe Card Disable
	
	// A.	Target 

	// a.	WLAN all disable (RF off, A15 off, DCORE reset)
	// b.	MCU reset
	// c.	A33 disable (AFE bandgap and m-bias on, others disable)
	// d.	XTAL off
	// e.	PON on
	// f.		HCI D3 mode (the same as S3 state)
	// g.	REG can be accessed by host. Resume by register control.


	// B.	Off Sequence 

	//
	// 2009/10/13 MH Refer to document RTL8191C power sec V8.1 sequence.
	// Chapter 6.1 for power card disable.
	//
	// A. Ensure PCIe Device 0x80[15:0] = 0x0143 (ASPM+CLKREQ), and PCIe gated 
	// clock function is enabled.
	rtw_pci_enable_aspm(Adapter);

#if 1
	//tynli_test. by sd1 Jonbon. Turn off then resume, system will restart.
#ifdef CONFIG_DUALMAC_CONCURRENT
	rtw_write8(Adapter, REG_RF_CTRL, (rtw_read8(Adapter, REG_RF_CTRL) & 0xFC));
#else
	rtw_write8(Adapter, REG_RF_CTRL, 0x00);
#endif
#endif

	//0x87c:Switch  PIN 3:PAGE_2G-1 ,PIN17:ANTSWB from output mode to input mode.
	PHY_SetBBReg(Adapter, rFPGA0_XCD_RFParameter, BIT3, 0);
	if(pHalData->interfaceIndex != 0)
		PHY_SetBBReg(Adapter, rFPGA0_XCD_RFParameter, BIT15, 0);
	
	//0x20:value 05-->04
	rtw_write8(Adapter, REG_LDOA15_CTRL,0x04);

	//  ==== Reset digital sequence   ======
	if((rtw_read8(Adapter, REG_MCUFWDL)&BIT1) && 
		Adapter->bFWReady) //8051 RAM code
	{
		rtl8192d_FirmwareSelfReset(Adapter);
	}
	
	// f.	SYS_FUNC_EN 0x03[7:0]=0x51		// reset MCU, MAC register, DCORE
	rtw_write8(Adapter, REG_SYS_FUNC_EN+1, 0x51);

	// g.	MCUFWDL 0x80[1:0]=0				// reset MCU ready status
	rtw_write8(Adapter, REG_MCUFWDL, 0x00);

	//  ==== Pull GPIO PIN to balance level and LED control ======

	// h.		GPIO_PIN_CTRL 0x44[31:0]=0x000		// 
	rtw_write32(Adapter, REG_GPIO_PIN_CTRL, 0x00000000);

	// i.		Value = GPIO_PIN_CTRL[7:0]
	//This do nth.....FIX ME!!!
	u1bTmp = rtw_read8(Adapter, REG_GPIO_PIN_CTRL);

#ifdef CONFIG_BT_COEXIST
	if((pHalData->bt_coexist.BT_Coexist) &&
		((pHalData->bt_coexist.BT_CoexistType == BT_CSR_BC4)||
		(pHalData->bt_coexist.BT_CoexistType == BT_CSR_BC8)))
	{ // when BT COEX exist
		//j. GPIO_PIN_CTRL 0x44[31:0] = 0x00F30000 | (value <<8); //write external PIN level 
		rtw_write32(Adapter, REG_GPIO_PIN_CTRL, 0x00F30000| (u1bTmp <<8));
	}
	else
#endif
	{ //Without BT COEX
		//j.	GPIO_PIN_CTRL 0x44[31:0] = 0x00FF0000 | (value <<8); //write external PIN level 
		rtw_write32(Adapter, REG_GPIO_PIN_CTRL, 0x00FF0000| (u1bTmp <<8));
	}

	// k.	GPIO_MUXCFG 0x42 [15:0] = 0x0780
	rtw_write16(Adapter, REG_GPIO_IO_SEL, 0x0790);

	// l.	LEDCFG 0x4C[15:0] = 0x8080
	rtw_write16(Adapter, REG_LEDCFG0, 0x8080);	

	//  ==== Disable analog sequence ===

	// m.	AFE_PLL_CTRL[7:0] = 0x80			//disable PLL
	rtw_write8(Adapter, REG_AFE_PLL_CTRL, 0x80);

	// n.	SPS0_CTRL 0x11[7:0] = 0x22			//enter PFM mode
	rtw_write8(Adapter, REG_SPS0_CTRL, 0x23);

#ifdef CONFIG_BT_COEXIST
	if(	(pHalData->bt_coexist.BT_Coexist) /*&&
		(pHalData->bt_coexist.BT_CoexistType == BT_CSR_BC4)	*/)
	{ // when BT COEX exist
		u4bTmp = rtw_read32(Adapter, REG_AFE_XTAL_CTRL);

		//AFE_XTAL_CTRL+2 0x26[9:7] = 3b'111
		u4bTmp |= 0x03800000;
		rtw_write32(Adapter, REG_AFE_XTAL_CTRL, u4bTmp);

		//AFE_XTAL_CTRL+2 0x26[1] = 1
		u4bTmp |= 0x00020000;
		rtw_write32(Adapter, REG_AFE_XTAL_CTRL, u4bTmp);

		//AFE_XTAL_CTRL 0x24[14] = 1
		u4bTmp |= 0x00004000;
		rtw_write32(Adapter, REG_AFE_XTAL_CTRL, u4bTmp);

		//AFE_XTAL_CTRL 0x24[11] = 1
		u4bTmp |= 0x00000800;
		rtw_write32(Adapter, REG_AFE_XTAL_CTRL, u4bTmp);
	}
	else
#endif
	{
		// o.	AFE_XTAL_CTRL 0x24[7:0] = 0x0E		// disable XTAL, if No BT COEX
		rtw_write8(Adapter, REG_AFE_XTAL_CTRL, 0x0e);
	}
	
	// p.	RSV_CTRL 0x1C[7:0] = 0x0E			// lock ISO/CLK/Power control register
	rtw_write8(Adapter, REG_RSV_CTRL, 0x0e);
	
	//  ==== interface into suspend ===

	// q.	APS_FSMCO[15:8] = 0x58				// PCIe suspend mode
	//PlatformEFIOWrite1Byte(Adapter, REG_APS_FSMCO+1, 0x58);
	// by tynli. Suggested by SD1.
	// According to power document V11, we need to set this value as 0x18. Otherwise, we
	// may not L0s sometimes. This indluences power consumption. Bases on SD1's test,
	// set as 0x00 do not affect power current. And if it is set as 0x18, they had ever
	// met auto load fail problem. 2009/12/03 MH/Tylin/Alan add the description.
	rtw_write8(Adapter, REG_APS_FSMCO+1, 0x10); //tynli_test. SD1 2009.12.08.

	//DBG_8192C("In PowerOff,reg0x%x=%X\n",REG_SPS0_CTRL,rtw_read8(Adapter, REG_SPS0_CTRL));
	// r.	Note: for PCIe interface, PON will not turn off m-bias and BandGap 
	// in PCIe suspend mode. 


	// 2009/10/16 MH SD1 Victor need the test for isolation.
	// tynli_test set BIT0 to 1. moved to shutdown
	//rtw_write8(Adapter, 0x0, PlatformEFIORead1Byte(Adapter, 0x0)|BIT0);

	// 0x17[7] 1b': power off in process  0b' : power off over
	ACQUIRE_GLOBAL_MUTEX(GlobalMutexForPowerOnAndPowerOff);
	u1bTmp=rtw_read8(Adapter, REG_POWER_OFF_IN_PROCESS);
	u1bTmp&=(~BIT7);
	rtw_write8(Adapter, REG_POWER_OFF_IN_PROCESS, u1bTmp);
	RELEASE_GLOBAL_MUTEX(GlobalMutexForPowerOnAndPowerOff);

	// 2009/10/13 MH Disable 92SE card disable sequence.
	DBG_8192C("<=======PowerOffAdapter8192CE()\n");
	
}	// PowerOffAdapter8192CE

//
// Description: For WoWLAN, when D0 support PME, we should clear PME status from 0x81
//			to 0x01 to prevent S3/S4 hang. Suggested by SD1 Jonbon/Isaac.
//
//	2009.04. by tynli.
static VOID
PlatformClearPciPMEStatus(
	IN		PADAPTER		Adapter	
)
{
	struct dvobj_priv	*pdvobjpriv = adapter_to_dvobj(Adapter);
	struct pci_dev 	*pdev = pdvobjpriv->ppcidev;
	u8	value_offset, value, tmp;
	int	result;

	result = pci_read_config_byte(pdev, 0x34, &value_offset);

	DBG_8192C("PlatformClearPciPMEStatus(): PCI configration 0x34 = 0x%2x\n", value_offset);

	if(result != 0) //returns how many bytes of caller-supplied data it wrote
	{
		DBG_8192C("PlatformClearPciPMEStatus() Failed!(%d)\n",result);
	}
	else
	{
		do
		{
			if(value_offset == 0x00) //end of pci capability
			{
				value = 0xff;
				break;
			}

			result = pci_read_config_byte(pdev, value_offset, &value);

			DBG_8192C("PlatformClearPciPMEStatus(): in pci configration1, value_offset%x = %x\n", value_offset, value);

			if(result != 1) //returns how many bytes of caller-supplied data it wrote
			{
				DBG_8192C("PlatformClearPciPMEStatus() Failed!(%d)\n",result);
			}
			else
			{
				if(value == 0x01)
					break;
				else
				{
					value_offset = value_offset + 0x1;
					result = pci_read_config_byte(pdev, value_offset, &value);
					//DBG_8192C("PlatformClearPciPMEStatus(): in pci configration2, value_offset%x = %x\n", value_offset, value);
					value_offset = value;
				}
			}
		}while(_TRUE);
		
		if(value == 0x01)
		{
			value_offset = value_offset + 0x05;
			result = pci_read_config_byte(pdev, value_offset, &value);
			//DBG_8192C("*** 1 PME value = %x \n", value);
			if(value & BIT7)
			{
				tmp = value | BIT7;
				pci_write_config_byte(pdev, value_offset, tmp);

				pci_read_config_byte(pdev, value_offset, &tmp);
				//DBG_8192C("*** 2 PME value = %x \n", tmp);
				DBG_8192C("PlatformClearPciPMEStatus(): Clear PME status 0x%2x to 0x%2x\n", value_offset, tmp);
			}
			else
				DBG_8192C("PlatformClearPciPMEStatus(): PME status(0x%2x) = 0x%2x\n", value_offset, value);
		}
	}

	//DBG_8192C("PlatformClearPciPMEStatus(): PME_status offset = %x, EN = %x\n", value_offset, PCIClkReq);

}

static u32 rtl8192de_hal_deinit(PADAPTER Adapter)
{
	u8	u1bTmp = 0;
	u8	OpMode;
	u8	bSupportRemoteWakeUp = _FALSE;
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	struct pwrctrl_priv		*pwrpriv = &Adapter->pwrctrlpriv;
	
_func_enter_;


	if (Adapter->bHaltInProgress == _TRUE)
	{
		DBG_8192C("====> Abort rtl8192de_hal_deinit()\n");
		return _FAIL;
	}

	Adapter->bHaltInProgress = _TRUE;

	//
	// No I/O if device has been surprise removed
	//
	if (Adapter->bSurpriseRemoved)
	{
		Adapter->bHaltInProgress = _FALSE;
		return _SUCCESS;
	}

	OpMode = 0;
	rtw_hal_set_hwreg(Adapter, HW_VAR_MEDIA_STATUS, (u8 *)(&OpMode));

	Adapter->bDriverIsGoingToUnload = _TRUE;
	//if(/*Adapter->bDriverIsGoingToUnload || */pwrpriv->rfoff_reason > RF_CHANGE_BY_PS) {
	//	rtw_led_control(Adapter, LED_CTL_POWER_OFF);
	//}

	RT_SET_PS_LEVEL(pwrpriv, RT_RF_OFF_LEVL_HALT_NIC);

	//rtw_hal_get_def_var(Adapter, HAL_DEF_WOWLAN, &bSupportRemoteWakeUp);

	// Without supporting WoWLAN or the driver is in awake (D0) state, we should 
	// call PowerOffAdapter8192CE() to run the power sequence. 2009.04.23. by tynli.
	if(!bSupportRemoteWakeUp )//||!pMgntInfo->bPwrSaveState) 
	{
		// 2009/10/13 MH For power off test.	

		//Power sequence for each MAC.
		//a. stop tx DMA		
		//b. close RF
		//c. clear rx buf
		//d. stop rx DMA
		//e.  reset MAC

		//a. stop tx DMA
		rtw_write8(Adapter,	REG_PCIE_CTRL_REG+1, 0xFE);
		rtw_udelay_os(50); 

		//Not do this. Need power reset.
		//// b.	TXPAUSE 0x522[7:0] = 0xFF			//Pause MAC TX queue
		//PlatformEFIOWrite1Byte(Adapter, REG_TXPAUSE, 0xFF);
		///c. ========RF OFF sequence==========		
#if (RTL8192D_DUAL_MAC_MODE_SWITCH == 1)
		if(!pHalData->bSlaveOfDMSP)
#endif
		{
			//0x88c[23:20] = 0xf.
			PHY_SetBBReg(Adapter, rFPGA0_AnalogParameter4, 0x00f00000, 0xf);
			PHY_SetRFReg(Adapter, RF_PATH_A, 0x00, bRFRegOffsetMask, 0x00);

			// 	APSD_CTRL 0x600[7:0] = 0x40
			rtw_write8(Adapter, REG_APSD_CTRL, 0x40);

			//Close antenna 0,0xc04,0xd04
			PHY_SetBBReg(Adapter, rOFDM0_TRxPathEnable, bMaskByte0, 0);
			PHY_SetBBReg(Adapter, rOFDM1_TRxPathEnable, bDWord, 0);
			//before BB reset should do clock gated
			rtw_write32(Adapter, rFPGA0_XCD_RFParameter, rtw_read32(Adapter, rFPGA0_XCD_RFParameter)|(BIT31));
			// 	SYS_FUNC_EN 0x02[7:0] = 0xE2		//reset BB state machine
			rtw_write8(Adapter, REG_SYS_FUNC_EN, 0xE2);
			//Mac0 can not do Global reset. Mac1 can do.
			if(pHalData->interfaceIndex == 1){
				//	SYS_FUNC_EN 0x02[7:0] = 0xE0		//reset BB state machine		
				rtw_write8(Adapter, REG_SYS_FUNC_EN, 0xE0);
			}
		//====================================================
			rtw_udelay_os(50); 
		}
		//d. 		
		// stop tx/rx dma before disable REG_CR (0x100) to fix dma hang issue when disable/enable device.
		rtw_write8(Adapter,	REG_PCIE_CTRL_REG+1, 0xff);
		rtw_udelay_os(50); 
		rtw_write8(Adapter,	REG_CR, 0x0);

		DBG_8192C("==> HaltAdapter8192DE(): MAC:%d Do power off.......\n",pHalData->interfaceIndex);

#if 0
		if(IS_HARDWARE_TYPE_8192C(Adapter)
			||(IS_HARDWARE_TYPE_8192D(Adapter)
				&&pHalData->MacPhyMode92D!=SINGLEMAC_SINGLEPHY
				&& CanGotoPowerOff92D(Adapter))
			||(IS_HARDWARE_TYPE_8192D(Adapter)
				&&pHalData->MacPhyMode92D == SINGLEMAC_SINGLEPHY)	)
#endif
		if(PHY_CheckPowerOffFor8192D(Adapter))
			PowerOffAdapter8192DE(Adapter);
	}
	else
	{
		u8	bSleep = _TRUE;

		//stop send beacon for 92d fpag  and A-cut 
		rtw_write8(Adapter, REG_BCN_CTRL, 0x04);

		// stop tx/rx dma before disable REG_CR (0x100) to fix dma hang issue when disable/enable device.
		rtw_write8(Adapter, REG_PCIE_CTRL_REG+1, 0xff);

		//RxDMA
		//tynli_test 2009.12.16.
		u1bTmp = rtw_read8(Adapter, REG_RXPKT_NUM+2);
		rtw_write8(Adapter, REG_RXPKT_NUM+2, u1bTmp|BIT2);	

		//PlatformDisableASPM(Adapter);
		//rtw_hal_set_hwreg(Adapter, HW_VAR_SWITCH_EPHY_WoWLAN, (u8 *)&bSleep);

		//tynli_test. 2009.12.17.
		u1bTmp = rtw_read8(Adapter, REG_SPS0_CTRL);
		rtw_write8(Adapter, REG_SPS0_CTRL, (u1bTmp|BIT1));

		//
		rtw_write8(Adapter, REG_APS_FSMCO+1, 0x0);

		PlatformClearPciPMEStatus(Adapter);

		// tynli_test for normal chip wowlan. 2010.01.26. Suggested by Sd1 Isaac and designer Alfred.
		rtw_write8(Adapter, REG_SYS_CLKR, (rtw_read8(Adapter, REG_SYS_CLKR)|BIT3));

		//prevent 8051 to be reset by PERST#
		rtw_write8(Adapter, REG_RSV_CTRL, 0x20);
		rtw_write8(Adapter, REG_RSV_CTRL, 0x60);
			
	}

	rtl8192de_gen_RefreshLedState(Adapter);

	Adapter->bHaltInProgress = _FALSE;

_func_exit_;

	return _SUCCESS;
}

static void StopTxBeacon(_adapter *padapter)
{
	HAL_DATA_TYPE*	pHalData = GET_HAL_DATA(padapter);
	u8 			tmp1Byte = 0;

 	
	tmp1Byte = rtw_read8(padapter, REG_FWHW_TXQ_CTRL+2);
	rtw_write8(padapter, REG_FWHW_TXQ_CTRL+2, tmp1Byte & (~BIT6));
	rtw_write8(padapter, REG_BCN_MAX_ERR, 0xff);
	//CheckFwRsvdPageContent(padapter);  // 2010.06.23. Added by tynli.

	tmp1Byte = rtw_read8(padapter, REG_BCN_CTRL);
	tmp1Byte &= 0xF7;
	//PlatformAtomicExchange((pu4Byte)(&GET_HAL_DATA(Adapter)->RegBcnCtrlVal), tmp1Byte);
	rtw_write8(padapter, REG_BCN_CTRL, (u8)(pHalData->RegBcnCtrlVal));
	 
	
}

static void ResumeTxBeacon(_adapter *padapter)
{
	HAL_DATA_TYPE*	pHalData = GET_HAL_DATA(padapter);	
	u8 			tmp1Byte = 0;

	
	tmp1Byte = rtw_read8(padapter, REG_BCN_CTRL);
	tmp1Byte |= 0x08;
	//PlatformAtomicExchange((pu4Byte)(&GET_HAL_DATA(Adapter)->RegBcnCtrlVal), tmp1Byte);
	rtw_write8(padapter, REG_BCN_CTRL, (u8)(pHalData->RegBcnCtrlVal));
	 
	tmp1Byte = rtw_read8(padapter, REG_FWHW_TXQ_CTRL+2);
	rtw_write8(padapter, REG_FWHW_TXQ_CTRL+2, tmp1Byte | BIT6);
	rtw_write8(padapter, REG_BCN_MAX_ERR, 0x0a);
	
	
	 
}

static void hw_var_set_macaddr(PADAPTER Adapter, u8 variable, u8* val)
{
	u8 idx = 0;
	u32 reg_macid;

#ifdef CONFIG_CONCURRENT_MODE
	if(Adapter->iface_type == IFACE_PORT1)
	{
		reg_macid = REG_MACID1;
	}
	else
#endif
	{
		reg_macid = REG_MACID;
	}

	for(idx = 0 ; idx < 6; idx++)
	{
		rtw_write8(Adapter, (reg_macid+idx), val[idx]);
	}
	
}

void SetHwReg8192DE(PADAPTER Adapter, u8 variable, u8* val)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;

_func_enter_;

	switch(variable)
	{
		case HW_VAR_MEDIA_STATUS:
			{
				u8 val8;

				val8 = rtw_read8(Adapter, MSR)&0x0c;
				val8 |= *((u8 *)val);
				rtw_write8(Adapter, MSR, val8);
			}
			break;
		case HW_VAR_MEDIA_STATUS1:
			{
				u8 val8;
				
				val8 = rtw_read8(Adapter, MSR)&0x03;
				val8 |= *((u8 *)val) <<2;
				rtw_write8(Adapter, MSR, val8);
			}
			break;
		case HW_VAR_SET_OPMODE:
			{
				u8	val8;
				u8	mode = *((u8 *)val);

				if((mode == _HW_STATE_STATION_) || (mode == _HW_STATE_NOLINK_))
				{
					StopTxBeacon(Adapter);
					rtw_write8(Adapter,REG_BCN_CTRL, 0x18);
					UpdateInterruptMask8192DE(Adapter, 0, RT_BSS_INT_MASKS);
				}
				else if((mode == _HW_STATE_ADHOC_) /*|| (mode == _HW_STATE_AP_)*/)
				{
					rtw_write16(Adapter, REG_CR, rtw_read16(Adapter, REG_CR)|BIT(8));
					ResumeTxBeacon(Adapter);
					rtw_write8(Adapter,REG_BCN_CTRL, 0x1a);
				}
				else if(mode == _HW_STATE_AP_)
				{
					rtw_write16(Adapter, REG_CR, rtw_read16(Adapter, REG_CR)|BIT(8));
				
					ResumeTxBeacon(Adapter);
					
					rtw_write8(Adapter, REG_BCN_CTRL, 0x12);

					
					//Set RCR
					//rtw_write32(padapter, REG_RCR, 0x70002a8e);//CBSSID_DATA must set to 0
					rtw_write32(Adapter, REG_RCR, 0x7000228e);//CBSSID_DATA must set to 0
					//enable to rx data frame				
					rtw_write16(Adapter, REG_RXFLTMAP2, 0xFFFF);
					//enable to rx ps-poll
					rtw_write16(Adapter, REG_RXFLTMAP1, 0x0400);

					//Beacon Control related register for first time 
					rtw_write8(Adapter, REG_BCNDMATIM, 0x02); // 2ms		
					rtw_write8(Adapter, REG_DRVERLYINT, 0x05);// 5ms
					//rtw_write8(Adapter, REG_BCN_MAX_ERR, 0xFF);
					rtw_write8(Adapter, REG_ATIMWND, 0x0a); // 10ms
					rtw_write16(Adapter, REG_BCNTCFG, 0x00);
					rtw_write16(Adapter, REG_TBTT_PROHIBIT, 0x6404);		
	
					//reset TSF		
					rtw_write8(Adapter, REG_DUAL_TSF_RST, BIT(0));

					//enable TSF Function for if1
					rtw_write8(Adapter, REG_BCN_CTRL, (EN_BCN_FUNCTION | EN_TXBCN_RPT));
			
					//enable update TSF for if1
					rtw_write8(Adapter, REG_BCN_CTRL, rtw_read8(Adapter, REG_BCN_CTRL)&(~BIT(4)));			
					
					UpdateInterruptMask8192DE(Adapter, RT_BSS_INT_MASKS, 0 );
					
				}

				val8 = rtw_read8(Adapter, MSR)&0x0c;
				val8 |= mode;
				rtw_write8(Adapter, MSR, val8);
			}
			break;
		case HW_VAR_BSSID:
			{
				u8	idx = 0;
				for(idx = 0 ; idx < 6; idx++)
				{
					rtw_write8(Adapter, (REG_BSSID+idx), val[idx]);
				}
			}
			break;
		case HW_VAR_BASIC_RATE:
			{
				u16			BrateCfg = 0;
				u8			RateIndex = 0;
#ifdef CONFIG_DUALMAC_CONCURRENT
				PADAPTER	BuddyAdapter = Adapter->pbuddy_adapter;
#endif
				
				// 2007.01.16, by Emily
				// Select RRSR (in Legacy-OFDM and CCK)
				// For 8190, we select only 24M, 12M, 6M, 11M, 5.5M, 2M, and 1M from the Basic rate.
				// We do not use other rates.
				HalSetBrateCfg( Adapter, val, &BrateCfg );

				if(pHalData->CurrentBandType92D == BAND_ON_2_4G)
				{
					//CCK 2M ACK should be disabled for some BCM and Atheros AP IOT
					//because CCK 2M has poor TXEVM
					//CCK 5.5M & 11M ACK should be enabled for better performance
					pHalData->BasicRateSet = BrateCfg = (BrateCfg |0xd )& 0x15d;
					BrateCfg |= 0x1; // default enable 1M ACK rate
				}
				else // 5G
				{
					pHalData->BasicRateSet &= 0xFF0;
					BrateCfg |= 0x10; // default enable 6M ACK rate
				}

				DBG_8192C("HW_VAR_BASIC_RATE: BrateCfg(%#x)\n", BrateCfg);

				// Set RRSR rate table.
				rtw_write8(Adapter, REG_RRSR, BrateCfg&0xff);
				rtw_write8(Adapter, REG_RRSR+1, (BrateCfg>>8)&0xff);
				rtw_write8(Adapter, REG_RRSR+2, rtw_read8(Adapter, REG_RRSR+2)&0xf0);

				//if(pHalData->FirmwareVersion > 0xe)
				//{
				//	SetRTSRateWorkItemCallback(Adapter);
				//}
				//else
				//{
					// Set RTS initial rate
					while(BrateCfg > 0x1)
					{
						BrateCfg = (BrateCfg>> 1);
						RateIndex++;
					}
					rtw_write8(Adapter, REG_INIRTS_RATE_SEL, RateIndex);
				//}

				if(check_fwstate(&Adapter->mlmepriv, WIFI_AP_STATE) == _TRUE)
				{
					DBG_8192C("%s(): pHalData->bNeedIQK = _TRUE\n",__FUNCTION__);
					pHalData->bNeedIQK = _TRUE; //for 92D IQK
				}
#ifdef CONFIG_DUALMAC_CONCURRENT
				if((BuddyAdapter !=NULL) && (pHalData->bSlaveOfDMSP))
				{
					if(check_fwstate(&BuddyAdapter->mlmepriv, WIFI_AP_STATE) == _TRUE)
						GET_HAL_DATA(BuddyAdapter)->bNeedIQK = _TRUE; //for 92D IQK
				}
#endif
			}
			break;
		case HW_VAR_TXPAUSE:
			rtw_write8(Adapter, REG_TXPAUSE, *((u8 *)val));	
			break;
		case HW_VAR_BCN_FUNC:
			if(*((u8 *)val))
			{
				rtw_write8(Adapter, REG_BCN_CTRL, (EN_BCN_FUNCTION | EN_TXBCN_RPT));
			}
			else
			{
				rtw_write8(Adapter, REG_BCN_CTRL, rtw_read8(Adapter, REG_BCN_CTRL)&(~(EN_BCN_FUNCTION | EN_TXBCN_RPT)));
			}
			break;
		case HW_VAR_CORRECT_TSF:
			{
				u64	tsf;
				struct mlme_ext_priv	*pmlmeext = &Adapter->mlmeextpriv;
				struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

				//tsf = pmlmeext->TSFValue - ((u32)pmlmeext->TSFValue % (pmlmeinfo->bcn_interval*1024)) -1024; //us
				tsf = pmlmeext->TSFValue - rtw_modular64(pmlmeext->TSFValue, (pmlmeinfo->bcn_interval*1024)) -1024; //us

				if(((pmlmeinfo->state&0x03) == WIFI_FW_ADHOC_STATE) || ((pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE))
				{				
					//pHalData->RegTxPause |= STOP_BCNQ;BIT(6)
					//rtw_write8(Adapter, REG_TXPAUSE, (rtw_read8(Adapter, REG_TXPAUSE)|BIT(6)));
					StopTxBeacon(Adapter);
				}

				//disable related TSF function
				rtw_write8(Adapter, REG_BCN_CTRL, rtw_read8(Adapter, REG_BCN_CTRL)&(~BIT(3)));
							
				rtw_write32(Adapter, REG_TSFTR, tsf);
				rtw_write32(Adapter, REG_TSFTR+4, tsf>>32);

				//enable related TSF function
				rtw_write8(Adapter, REG_BCN_CTRL, rtw_read8(Adapter, REG_BCN_CTRL)|BIT(3));
				
							
				if(((pmlmeinfo->state&0x03) == WIFI_FW_ADHOC_STATE) || ((pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE))
				{
					//pHalData->RegTxPause  &= (~STOP_BCNQ);
					//rtw_write8(Adapter, REG_TXPAUSE, (rtw_read8(Adapter, REG_TXPAUSE)&(~BIT(6))));
					ResumeTxBeacon(Adapter);
				}
			}
			break;
		case HW_VAR_CHECK_BSSID:
			if(*((u8 *)val))
			{
				rtw_write32(Adapter, REG_RCR, rtw_read32(Adapter, REG_RCR)|RCR_CBSSID_DATA|RCR_CBSSID_BCN);
			}
			else
			{
				u32	val32;

				val32 = rtw_read32(Adapter, REG_RCR);
				val32 &= ~(RCR_CBSSID_DATA | RCR_CBSSID_BCN);
				rtw_write32(Adapter, REG_RCR, val32);
			}
			break;
		case HW_VAR_MLME_DISCONNECT:
			{
				//Set RCR to not to receive data frame when NO LINK state
				//rtw_write32(Adapter, REG_RCR, rtw_read32(padapter, REG_RCR) & ~RCR_ADF);
				rtw_write16(Adapter, REG_RXFLTMAP2,0x00);

				//reset TSF
				rtw_write8(Adapter, REG_DUAL_TSF_RST, (BIT(0)|BIT(1)));

				//disable update TSF
				rtw_write8(Adapter, REG_BCN_CTRL, rtw_read8(Adapter, REG_BCN_CTRL)|BIT(4));	
			}
			break;
		case HW_VAR_MLME_SITESURVEY:
			{
				u32	value_rcr, rcr_clear_bit, reg_bcn_ctl;
				u16	value_rxfltmap2;
				struct mlme_priv *pmlmepriv=&(Adapter->mlmepriv);

				reg_bcn_ctl = REG_BCN_CTRL;

#ifdef CONFIG_FIND_BEST_CHANNEL

#ifdef CONFIG_TDLS
				// TDLS will clear RCR_CBSSID_DATA bit for connection.
				if ( Adapter->tdlsinfo.setup_state & TDLS_LINKED_STATE )
					rcr_clear_bit = RCR_CBSSID_BCN;
				else
#endif // CONFIG_TDLS
					rcr_clear_bit = (RCR_CBSSID_BCN | RCR_CBSSID_DATA);

				// Recieve all data frames
				value_rxfltmap2 = 0xFFFF;

#else /* CONFIG_FIND_BEST_CHANNEL */

				rcr_clear_bit = RCR_CBSSID_BCN;

				//config RCR to receive different BSSID & not to receive data frame
				value_rxfltmap2 = 0;

#endif /* CONFIG_FIND_BEST_CHANNEL */

				value_rcr = rtw_read32(Adapter, REG_RCR);

				if(*((u8 *)val))//under sitesurvey
				{
					value_rcr &= ~(rcr_clear_bit);
					rtw_write32(Adapter, REG_RCR, value_rcr);

					rtw_write16(Adapter, REG_RXFLTMAP2, value_rxfltmap2);

					if (check_fwstate(pmlmepriv, WIFI_STATION_STATE | WIFI_ADHOC_STATE |WIFI_ADHOC_MASTER_STATE)) {
						//disable update TSF
						rtw_write8(Adapter, reg_bcn_ctl, rtw_read8(Adapter, reg_bcn_ctl)|BIT(4));
					}
				}
				else//sitesurvey done
				{
					if (check_fwstate(pmlmepriv, _FW_LINKED))
					{
						//enable to rx data frame
						rtw_write16(Adapter, REG_RXFLTMAP2,0xFFFF);
					}

					if (check_fwstate(pmlmepriv, WIFI_STATION_STATE | WIFI_ADHOC_STATE |WIFI_ADHOC_MASTER_STATE)) {
						//enable update TSF
						rtw_write8(Adapter, reg_bcn_ctl, rtw_read8(Adapter, reg_bcn_ctl)&(~BIT(4)));
					}

					value_rcr |= rcr_clear_bit;
					rtw_write32(Adapter, REG_RCR, value_rcr);
				}
			}
			break;
		case HW_VAR_MLME_JOIN:
			{
				u8	RetryLimit = 0x30;
				u8	type = *((u8 *)val);
				struct mlme_priv	*pmlmepriv = &Adapter->mlmepriv;
#ifdef CONFIG_DUALMAC_CONCURRENT
				PADAPTER	BuddyAdapter = Adapter->pbuddy_adapter;
#endif

				if(type == 0) // prepare to join
				{
					//enable to rx data frame.Accept all data frame
					//rtw_write32(padapter, REG_RCR, rtw_read32(padapter, REG_RCR)|RCR_ADF);
					rtw_write16(Adapter, REG_RXFLTMAP2,0xFFFF);

					rtw_write32(Adapter, REG_RCR, rtw_read32(Adapter, REG_RCR)|RCR_CBSSID_DATA|RCR_CBSSID_BCN);

					if(check_fwstate(pmlmepriv, WIFI_STATION_STATE) == _TRUE)
					{
						RetryLimit = (pHalData->CustomerID == RT_CID_CCX) ? 7 : 48;
					}
					else // Ad-hoc Mode
					{
						RetryLimit = 0x7;
					}

					DBG_8192C("%s(): pHalData->bNeedIQK = _TRUE\n",__FUNCTION__);
					pHalData->bNeedIQK = _TRUE; //for 92D IQK
#ifdef CONFIG_DUALMAC_CONCURRENT
					if((BuddyAdapter !=NULL) && (pHalData->bSlaveOfDMSP))
					{
						GET_HAL_DATA(BuddyAdapter)->bNeedIQK = _TRUE; //for 92D IQK
					}
#endif
				}
				else if(type == 1) //joinbss_event call back when join res < 0
				{
					rtw_write16(Adapter, REG_RXFLTMAP2,0x00);
				}
				else if(type == 2) //sta add event call back
				{
					//enable update TSF
					rtw_write8(Adapter, REG_BCN_CTRL, rtw_read8(Adapter, REG_BCN_CTRL)&(~BIT(4)));

					if(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE|WIFI_ADHOC_MASTER_STATE))
					{
						//fixed beacon issue for 8191su...........
						//rtw_write8(Adapter,0x542 ,0x02);
						RetryLimit = 0x7;
					}
				}

				rtw_write16(Adapter, REG_RL, RetryLimit << RETRY_LIMIT_SHORT_SHIFT | RetryLimit << RETRY_LIMIT_LONG_SHIFT);
			}
			break;
		case HW_VAR_ON_RCR_AM:
			rtw_write32(Adapter, REG_RCR, rtw_read32(Adapter, REG_RCR)|RCR_AM);
			DBG_871X("%s, %d, RCR= %x \n", __FUNCTION__,__LINE__, rtw_read32(Adapter, REG_RCR));
			break;
		case HW_VAR_OFF_RCR_AM:
			rtw_write32(Adapter, REG_RCR, rtw_read32(Adapter, REG_RCR)& (~RCR_AM));
			DBG_871X("%s, %d, RCR= %x \n", __FUNCTION__,__LINE__, rtw_read32(Adapter, REG_RCR));
			break;
		case HW_VAR_BEACON_INTERVAL:
			rtw_write16(Adapter, REG_BCN_INTERVAL, *((u16 *)val));
			break;
		case HW_VAR_SLOT_TIME:
			{
				u8	u1bAIFS, aSifsTime;
				struct mlme_ext_priv	*pmlmeext = &Adapter->mlmeextpriv;
				struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

				DBG_8192C("Set HW_VAR_SLOT_TIME: SlotTime(%#x)\n", val[0]);
				rtw_write8(Adapter, REG_SLOT, val[0]);

				if(pmlmeinfo->WMM_enable == 0)
				{
					if(pmlmeext->cur_wireless_mode == WIRELESS_11B)
						aSifsTime = 10;
					else
						aSifsTime = 16;
					
					u1bAIFS = aSifsTime + (2 * pmlmeinfo->slotTime);
					
					// <Roger_EXP> Temporary removed, 2008.06.20.
					rtw_write8(Adapter, REG_EDCA_VO_PARAM, u1bAIFS);
					rtw_write8(Adapter, REG_EDCA_VI_PARAM, u1bAIFS);
					rtw_write8(Adapter, REG_EDCA_BE_PARAM, u1bAIFS);
					rtw_write8(Adapter, REG_EDCA_BK_PARAM, u1bAIFS);
				}
			}
			break;
		case HW_VAR_ACK_PREAMBLE:
			{
				u8	regTmp;
				u8	bShortPreamble = *( (PBOOLEAN)val );
				// Joseph marked out for Netgear 3500 TKIP channel 7 issue.(Temporarily)
				regTmp = (pHalData->nCur40MhzPrimeSC)<<5;
				//regTmp = 0;
				if(bShortPreamble)
					regTmp |= 0x80;

				rtw_write8(Adapter, REG_RRSR+2, regTmp);
			}
			break;
		case HW_VAR_SEC_CFG:
			rtw_write8(Adapter, REG_SECCFG, *((u8 *)val));
			break;
		case HW_VAR_DM_FLAG:
			pdmpriv->DMFlag = *((u8 *)val);
			break;
		case HW_VAR_DM_FUNC_OP:
			if(val[0])
			{// save dm flag
				pdmpriv->DMFlag_tmp = pdmpriv->DMFlag;
			}
			else
			{// restore dm flag
				pdmpriv->DMFlag = pdmpriv->DMFlag_tmp;
			}
			break;
		case HW_VAR_DM_FUNC_SET:
			pdmpriv->DMFlag |= *((u8 *)val);
			break;
		case HW_VAR_DM_FUNC_CLR:
			pdmpriv->DMFlag &= *((u8 *)val);
			break;
		case HW_VAR_CAM_EMPTY_ENTRY:
			{
				u8	ucIndex = *((u8 *)val);
				u8	i;
				u32	ulCommand=0;
				u32	ulContent=0;
				u32	ulEncAlgo=CAM_AES;

				for(i=0;i<CAM_CONTENT_COUNT;i++)
				{
					// filled id in CAM config 2 byte
					if( i == 0)
					{
						ulContent |=(ucIndex & 0x03) | ((u16)(ulEncAlgo)<<2);
						//ulContent |= CAM_VALID;
					}
					else
					{
						ulContent = 0;
					}
					// polling bit, and No Write enable, and address
					ulCommand= CAM_CONTENT_COUNT*ucIndex+i;
					ulCommand= ulCommand | CAM_POLLINIG|CAM_WRITE;
					// write content 0 is equall to mark invalid
					rtw_write32(Adapter, WCAMI, ulContent);  //delay_ms(40);
					//RT_TRACE(COMP_SEC, DBG_LOUD, ("CAM_empty_entry(): WRITE A4: %lx \n",ulContent));
					rtw_write32(Adapter, RWCAM, ulCommand);  //delay_ms(40);
					//RT_TRACE(COMP_SEC, DBG_LOUD, ("CAM_empty_entry(): WRITE A0: %lx \n",ulCommand));
				}
			}
			break;
		case HW_VAR_CAM_INVALID_ALL:
			rtw_write32(Adapter, RWCAM, BIT(31)|BIT(30));
			break;
		case HW_VAR_CAM_WRITE:
			{
				u32	cmd;
				u32	*cam_val = (u32 *)val;
				rtw_write32(Adapter, WCAMI, cam_val[0]);
				
				cmd = CAM_POLLINIG | CAM_WRITE | cam_val[1];
				rtw_write32(Adapter, RWCAM, cmd);
			}
			break;
		case HW_VAR_AC_PARAM_VO:
			rtw_write32(Adapter, REG_EDCA_VO_PARAM, ((u32 *)(val))[0]);
			break;
		case HW_VAR_AC_PARAM_VI:
			rtw_write32(Adapter, REG_EDCA_VI_PARAM, ((u32 *)(val))[0]);
			break;
		case HW_VAR_AC_PARAM_BE:
			pHalData->AcParam_BE = ((u32 *)(val))[0];
			rtw_write32(Adapter, REG_EDCA_BE_PARAM, ((u32 *)(val))[0]);
			break;
		case HW_VAR_AC_PARAM_BK:
			rtw_write32(Adapter, REG_EDCA_BK_PARAM, ((u32 *)(val))[0]);
			break;
		case HW_VAR_ACM_CTRL:
			{
				u8	acm_ctrl = *((u8 *)val);
				u8	AcmCtrl = rtw_read8( Adapter, REG_ACMHWCTRL);

				if(acm_ctrl > 1)
					AcmCtrl = AcmCtrl | 0x1;

				if(acm_ctrl & BIT(3))
					AcmCtrl |= AcmHw_VoqEn;
				else
					AcmCtrl &= (~AcmHw_VoqEn);

				if(acm_ctrl & BIT(2))
					AcmCtrl |= AcmHw_ViqEn;
				else
					AcmCtrl &= (~AcmHw_ViqEn);

				if(acm_ctrl & BIT(1))
					AcmCtrl |= AcmHw_BeqEn;
				else
					AcmCtrl &= (~AcmHw_BeqEn);

				DBG_871X("[HW_VAR_ACM_CTRL] Write 0x%X\n", AcmCtrl );
				rtw_write8(Adapter, REG_ACMHWCTRL, AcmCtrl );
			}
			break;
		case HW_VAR_AMPDU_MIN_SPACE:
			{
				u8	MinSpacingToSet;
				u8	SecMinSpace;

				MinSpacingToSet = *((u8 *)val);
				if(MinSpacingToSet <= 7)
				{
					switch(Adapter->securitypriv.dot11PrivacyAlgrthm)
					{
						case _NO_PRIVACY_:
						case _AES_:
							SecMinSpace = 0;
							break;

						case _WEP40_:
						case _WEP104_:
						case _TKIP_:
						case _TKIP_WTMIC_:
						default:
							SecMinSpace = 7;
							break;
					}

					if(MinSpacingToSet < SecMinSpace){
						MinSpacingToSet = SecMinSpace;
					}

					//RT_TRACE(COMP_MLME, DBG_LOUD, ("Set HW_VAR_AMPDU_MIN_SPACE: %#x\n", Adapter->MgntInfo.MinSpaceCfg));
					rtw_write8(Adapter, REG_AMPDU_MIN_SPACE, (rtw_read8(Adapter, REG_AMPDU_MIN_SPACE) & 0xf8) | MinSpacingToSet);
				}
			}
			break;
		case HW_VAR_AMPDU_FACTOR:
			{
				u8	FactorToSet;
				u32	RegToSet;
				u8	*pTmpByte = NULL;
				u8	index = 0;

				if(pHalData->MacPhyMode92D == DUALMAC_DUALPHY)
					RegToSet = 0xb9726641;
				else if(pHalData->MacPhyMode92D == DUALMAC_SINGLEPHY)
					RegToSet = 0x66666641;
				else
					RegToSet = 0xb972a841;

				FactorToSet = *((u8 *)val);
				if(FactorToSet <= 3)
				{
					FactorToSet = (1<<(FactorToSet + 2));
					if(FactorToSet>0xf)
						FactorToSet = 0xf;

					for(index=0; index<4; index++)
					{
						pTmpByte = (u8 *)(&RegToSet) + index;

						if((*pTmpByte & 0xf0) > (FactorToSet<<4))
							*pTmpByte = (*pTmpByte & 0x0f) | (FactorToSet<<4);
					
						if((*pTmpByte & 0x0f) > FactorToSet)
							*pTmpByte = (*pTmpByte & 0xf0) | (FactorToSet);
					}

					rtw_write32(Adapter, REG_AGGLEN_LMT, RegToSet);
					//RT_TRACE(COMP_MLME, DBG_LOUD, ("Set HW_VAR_AMPDU_FACTOR: %#x\n", FactorToSet));
				}
			}
			break;
		case HW_VAR_SET_RPWM:
			{
				u8	RpwmVal = (*(u8 *)val);
				RpwmVal = RpwmVal & 0xf;

				/*if(pHalData->PreRpwmVal & BIT7) //bit7: 1
				{
					PlatformEFIOWrite1Byte(Adapter, REG_USB_HRPWM, (*(pu1Byte)val));
					pHalData->PreRpwmVal = (*(pu1Byte)val);
				}
				else //bit7: 0
				{
					PlatformEFIOWrite1Byte(Adapter, REG_USB_HRPWM, ((*(pu1Byte)val)|BIT7));
					pHalData->PreRpwmVal = ((*(pu1Byte)val)|BIT7);
				}*/
				FillH2CCmd92D(Adapter, H2C_PWRM, 1, (u8 *)(&RpwmVal));
			}
			break;
		case HW_VAR_H2C_FW_PWRMODE:
			rtl8192d_set_FwPwrMode_cmd(Adapter, (*(u8 *)val));
			break;
		case HW_VAR_H2C_FW_JOINBSSRPT:
			rtl8192d_set_FwJoinBssReport_cmd(Adapter, (*(u8 *)val));
			break;
#ifdef CONFIG_P2P_PS
		case HW_VAR_H2C_FW_P2P_PS_OFFLOAD:
			{
				u8	p2p_ps_state = (*(u8 *)val);
				rtl8192d_set_p2p_ps_offload_cmd(Adapter, p2p_ps_state);
			}
			break;
#endif // CONFIG_P2P_PS
		case HW_VAR_INITIAL_GAIN:
			{				
				DIG_T	*pDigTable = &pdmpriv->DM_DigTable;					
				u32 		rx_gain = ((u32 *)(val))[0];
				
				if(rx_gain == 0xff){//restore rx gain
					pDigTable->CurIGValue = pDigTable->BackupIGValue;
					PHY_SetBBReg(Adapter, rOFDM0_XAAGCCore1, 0x7f,pDigTable->CurIGValue );
					PHY_SetBBReg(Adapter, rOFDM0_XBAGCCore1, 0x7f,pDigTable->CurIGValue);
				}
				else{
					pDigTable->BackupIGValue = pDigTable->CurIGValue;					
					PHY_SetBBReg(Adapter, rOFDM0_XAAGCCore1, 0x7f,rx_gain );
					PHY_SetBBReg(Adapter, rOFDM0_XBAGCCore1, 0x7f,rx_gain);
					pDigTable->CurIGValue = (u8)rx_gain;
				}
			}
			break;
		case HW_VAR_EFUSE_BYTES: // To set EFUE total used bytes, added by Roger, 2008.12.22.
			pHalData->EfuseUsedBytes = *((u16 *)val);			
			break;
		case HW_VAR_FIFO_CLEARN_UP:
			{
				#define RW_RELEASE_EN		BIT18
				#define RXDMA_IDLE			BIT17
				
				struct pwrctrl_priv *pwrpriv = &Adapter->pwrctrlpriv;
				u8 trycnt = 100;	
				
				//pause tx
				rtw_write8(Adapter,REG_TXPAUSE,0xff);
			
				//keep sn
				Adapter->xmitpriv.nqos_ssn = rtw_read16(Adapter,REG_NQOS_SEQ);

				//RX DMA stop
				rtw_write32(Adapter,REG_RXPKT_NUM,(rtw_read32(Adapter,REG_RXPKT_NUM)|RW_RELEASE_EN));
				do{
					if(!(rtw_read32(Adapter,REG_RXPKT_NUM)&RXDMA_IDLE))
						break;
				}while(trycnt--);
				if(trycnt ==0)				
					DBG_8192C("Stop RX DMA failed...... \n");
					
				//RQPN Load 0
				rtw_write16(Adapter,REG_RQPN_NPQ,0x0);
				rtw_write32(Adapter,REG_RQPN,0x80000000);
				rtw_mdelay_os(10);
				
	
			}
			break;
		case HW_VAR_BCN_VALID:
			//BCN_VALID, BIT16 of REG_TDECTRL = BIT0 of REG_TDECTRL+2, write 1 to clear, Clear by sw
			rtw_write8(Adapter, REG_TDECTRL+2, rtw_read8(Adapter, REG_TDECTRL+2) | BIT0); 
			break;
		case HW_VAR_MAC_ADDR:
			hw_var_set_macaddr(Adapter, variable, val);			
			break;
		default:
			break;
	}

_func_exit_;
}

void GetHwReg8192DE(PADAPTER Adapter, u8 variable, u8* val)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

_func_enter_;

	switch(variable)
	{
		case HW_VAR_BASIC_RATE:
			*((u16 *)(val)) = pHalData->BasicRateSet;
		case HW_VAR_TXPAUSE:
			val[0] = rtw_read8(Adapter, REG_TXPAUSE);
			break;
		case HW_VAR_BCN_VALID:
			//BCN_VALID, BIT16 of REG_TDECTRL = BIT0 of REG_TDECTRL+2
			val[0] = (BIT0 & rtw_read8(Adapter, REG_TDECTRL+2))?_TRUE:_FALSE;
			break;
		case HW_VAR_DM_FLAG:
			val[0] = pHalData->dmpriv.DMFlag;
			break;
		case HW_VAR_RF_TYPE:
			val[0] = pHalData->rf_type;
			break;
		case HW_VAR_FWLPS_RF_ON:
			{
				//When we halt NIC, we should check if FW LPS is leave.
				u32	valRCR;
				
				if(Adapter->pwrctrlpriv.rf_pwrstate == rf_off)
				{
					// If it is in HW/SW Radio OFF or IPS state, we do not check Fw LPS Leave,
					// because Fw is unload.
					val[0] = _TRUE;
				}
				else
				{
					valRCR = rtw_read32(Adapter, REG_RCR);
					valRCR &= 0x00070000;
					if(valRCR)
						val[0] = _FALSE;
					else
						val[0] = _TRUE;
				}
			}
			break;
		case HW_VAR_EFUSE_BYTES: // To get EFUE total used bytes, added by Roger, 2008.12.22.
			*((u16 *)(val)) = pHalData->EfuseUsedBytes;	
			break;
		default:
			break;
	}

_func_exit_;
}

//
//	Description: 
//		Query setting of specified variable.
//
u8
GetHalDefVar8192DE(
	IN	PADAPTER				Adapter,
	IN	HAL_DEF_VARIABLE		eVariable,
	IN	PVOID					pValue
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u8			bResult = _TRUE;

	switch(eVariable)
	{
		case HAL_DEF_UNDERCORATEDSMOOTHEDPWDB:
			*((int *)pValue) = pHalData->dmpriv.UndecoratedSmoothedPWDB;
			break;
		case HAL_DEF_DRVINFO_SZ:
			*(( u32*)pValue) = DRVINFO_SZ;
			break;
		case HAL_DEF_MAX_RECVBUF_SZ:
			*(( u32*)pValue) = MAX_RECVBUF_SZ;
			break;
		case HAL_DEF_RX_PACKET_OFFSET:
			*(( u32*)pValue) = RXDESC_SIZE + DRVINFO_SZ;
			break;
		default:
			//RT_TRACE(COMP_INIT, DBG_WARNING, ("GetHalDefVar8192CUsb(): Unkown variable: %d!\n", eVariable));
			bResult = _FALSE;
			break;
	}

	return bResult;
}


//
//	Description:
//		Change default setting of specified variable.
//
u8
SetHalDefVar8192DE(
	IN	PADAPTER				Adapter,
	IN	HAL_DEF_VARIABLE		eVariable,
	IN	PVOID					pValue
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u8			bResult = _TRUE;

	switch(eVariable)
	{
		default:
			//RT_TRACE(COMP_INIT, DBG_TRACE, ("SetHalDefVar819xUsb(): Unkown variable: %d!\n", eVariable));
			bResult = _FALSE;
			break;
	}

	return bResult;
}

void UpdateHalRAMask8192DE(PADAPTER padapter, u32 mac_id)
{
	u32	value[2];
	u8	init_rate=0;
	u8	networkType, raid;	
	u32	mask;
	u8	shortGIrate = _FALSE;
	int	supportRateNum = 0;
	struct sta_info	*psta;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX 		*cur_network = &(pmlmeinfo->network);
#ifdef CONFIG_BT_COEXIST
	struct btcoexist_priv	*pbtpriv = &(pHalData->bt_coexist);
#endif

	if (mac_id >= NUM_STA) //CAM_SIZE
	{
		return;
	}

	psta = pmlmeinfo->FW_sta_info[mac_id].psta;
	if(psta == NULL)
	{
		return;
	}

	switch (mac_id)
	{
		case 0: // for infra mode
			supportRateNum = rtw_get_rateset_len(cur_network->SupportedRates);
			networkType = judge_network_type(padapter, cur_network->SupportedRates, supportRateNum);
			//pmlmeext->cur_wireless_mode = networkType;
			raid = networktype_to_raid(networkType);

			mask = update_supported_rate(cur_network->SupportedRates, supportRateNum);
			mask |= (pmlmeinfo->HT_enable)? update_MSC_rate(&(pmlmeinfo->HT_caps)): 0;
			mask |= ((raid<<28)&0xf0000000);

			if (support_short_GI(padapter, &(pmlmeinfo->HT_caps)))
			{
				shortGIrate = _TRUE;
			}

			break;

		case 1://for broadcast/multicast
			supportRateNum = rtw_get_rateset_len(pmlmeinfo->FW_sta_info[mac_id].SupportedRates);
			if(pmlmeext->cur_wireless_mode & WIRELESS_11B)
				networkType = WIRELESS_11B;
			else
				networkType = WIRELESS_11G;
			raid = networktype_to_raid(networkType);

			mask = update_basic_rate(cur_network->SupportedRates, supportRateNum);
			mask |= ((raid<<28)&0xf0000000);

			break;

		default: //for each sta in IBSS
#ifdef CONFIG_TDLS
			if(psta->tdls_sta_state & TDLS_LINKED_STATE)
			{
				shortGIrate = update_sgi_tdls(padapter, psta);
				mask = update_mask_tdls(padapter, psta);
				raid = mask>>28;
				break;
			}
			else
#endif //CONFIG_TDLS
			{
				supportRateNum = rtw_get_rateset_len(pmlmeinfo->FW_sta_info[mac_id].SupportedRates);
				networkType = judge_network_type(padapter, pmlmeinfo->FW_sta_info[mac_id].SupportedRates, supportRateNum) & 0xf;
				//pmlmeext->cur_wireless_mode = networkType;
				raid = networktype_to_raid(networkType);

				mask = update_supported_rate(cur_network->SupportedRates, supportRateNum);
				mask |= ((raid<<28)&0xf0000000);

				//todo: support HT in IBSS

				break;
			}
	}

#if 0
	//
	// Modify rate adaptive bitmap for BT coexist.
	//
	if( (pHalData->bt_coexist.BluetoothCoexist) &&
		(pHalData->bt_coexist.BT_CoexistType == BT_CSR) &&
		(pHalData->bt_coexist.BT_CUR_State) &&
		(pHalData->bt_coexist.BT_Ant_isolation) &&
		((pHalData->bt_coexist.BT_Service==BT_SCO)||
		(pHalData->bt_coexist.BT_Service==BT_Busy)) )
		mask &= 0x0fffcfc0;
	else
		mask &= 0x0FFFFFFF;
#endif

	init_rate = get_highest_rate_idx(mask)&0x3f;

	if(pHalData->fw_ractrl == _TRUE)
	{
		value[0] = mask;
		value[1] = mac_id | (shortGIrate?0x20:0x00) | 0x80;

		DBG_871X("update raid entry, mask=0x%x, arg=0x%x\n", value[0], value[1]);

		FillH2CCmd92D(padapter, H2C_RA_MASK, 5, (u8 *)(&value));
	}
	else
	{
		if (shortGIrate==_TRUE)
			init_rate |= BIT(6);

		rtw_write8(padapter, (REG_INIDATA_RATE_SEL+mac_id), init_rate);
	}


	//set ra_id
	psta->raid = raid;
	psta->init_rate = init_rate;

	//set correct initial date rate for each mac_id
	pdmpriv->INIDATA_RATE[mac_id] = init_rate;	
}

void SetBeaconRelatedRegisters8192DE(PADAPTER padapter)
{
	u32	value32;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	//
	// ATIM window
	//
	rtw_write16(padapter, REG_ATIMWND, 0x2);
	
	//
	// Beacon interval (in unit of TU).
	//
	rtw_write16(padapter, REG_BCN_INTERVAL, pmlmeinfo->bcn_interval);
	//2008.10.24 added by tynli for beacon changed.
	//PHY_SetBeaconHwReg( Adapter, BcnInterval );

	//
	// DrvErlyInt (in unit of TU). (Time to send interrupt to notify driver to change beacon content)
	//
	//PlatformEFIOWrite1Byte(Adapter, BCN_DMA_INT_92C+1, 0xC);

	//
	// BcnDMATIM(in unit of us). Indicates the time before TBTT to perform beacon queue DMA 
	//
	//PlatformEFIOWrite2Byte(Adapter, BCN_DMATIM_92C, 256); // HWSD suggest this value 2006.11.14

	//
	// Force beacon frame transmission even after receiving beacon frame from other ad hoc STA
	//
	//PlatformEFIOWrite2Byte(Adapter, BCN_ERRTH_92C, 100); // Reference from WMAC code 2006.11.14
	//suggest by wl, 20090902

	rtw_write16(padapter, REG_BCNTCFG, 0x660f);

	//For throughput
	//PlatformEFIOWrite2Byte(Adapter,TBTT_PROHIBIT_92C,0x0202);
	//suggest by wl, 20090902
	//PlatformEFIOWrite1Byte(Adapter,REG_RXTSF_OFFSET_CCK, 0x30);	
	//PlatformEFIOWrite1Byte(Adapter,REG_RXTSF_OFFSET_OFDM, 0x30);
	
	// Suggested by TimChen. 2009.01.25.
	// Rx RF to MAC data path time.
	rtw_write8(padapter,REG_RXTSF_OFFSET_CCK, 0x20);	//0x18
	if(GET_HAL_DATA(padapter)->CurrentBandType92D == BAND_ON_5G)
		rtw_write8(padapter,REG_RXTSF_OFFSET_OFDM, 0x30);
	else
		rtw_write8(padapter,REG_RXTSF_OFFSET_OFDM, 0x20); //0x18

	rtw_write8(padapter,0x606, 0x30);

	pHalData->RegBcnCtrlVal |= BIT3;
	rtw_write8(padapter, REG_BCN_CTRL, (u8)(pHalData->RegBcnCtrlVal));

	//
	// Update interrupt mask for IBSS.
	//
	UpdateInterruptMask8192DE( padapter, RT_IBSS_INT_MASKS, 0 );

	ResumeTxBeacon(padapter);

	//rtw_write8(padapter, 0x422, rtw_read8(padapter, 0x422)|BIT(6));
	
	//rtw_write8(padapter, 0x541, 0xff);

	//rtw_write8(padapter, 0x542, rtw_read8(padapter, 0x541)|BIT(0));

	rtw_write8(padapter, REG_BCN_CTRL, rtw_read8(padapter, REG_BCN_CTRL)|BIT(1));

}

static void rtl8192de_init_default_value(_adapter * padapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	struct pwrctrl_priv *pwrctrlpriv = &padapter->pwrctrlpriv;

	pHalData->CurrentWirelessMode = WIRELESS_MODE_AUTO;

	//init default value
	pHalData->fw_ractrl = _FALSE;
	if(!pwrctrlpriv->bkeepfwalive)
		pHalData->LastHMEBoxNum = 0;

	pHalData->bEarlyModeEnable = 0;
	pHalData->pwrGroupCnt = 0;

	//init dm default value
	pdmpriv->TM_Trigger = 0;
	//pdmpriv->binitialized = _FALSE;
	pdmpriv->prv_traffic_idx = 3;

	rtl8192d_PHY_ResetIQKResult(padapter);

	//
	// Set TCR-Transmit Control Register. The value is set in InitializeAdapter8190Pci()
	//
	pHalData->TransmitConfig = CFENDFORM | BIT12 | BIT13;

	//
	// Set RCR-Receive Control Register . The value is set in InitializeAdapter8190Pci().
	//
	pHalData->ReceiveConfig = (\
		//RCR_APPFCS	
		// | RCR_APWRMGT
		// |RCR_ADD3
		// | RCR_ADF |
		RCR_AMF | RCR_APP_MIC| RCR_APP_ICV
		| RCR_AICV | RCR_ACRC32	// Accept ICV error, CRC32 Error
		| RCR_AB | RCR_AM			// Accept Broadcast, Multicast	 
     		| RCR_APM 					// Accept Physical match
     		//| RCR_AAP					// Accept Destination Address packets
     		| RCR_APP_PHYST_RXFF		// Accept PHY status
     		| RCR_HTC_LOC_CTRL
		//(pHalData->EarlyRxThreshold<<RCR_FIFO_OFFSET)	);
		);

	//
	// Set Interrupt Mask Register
	//
	// Make reference from WMAC code 2006.10.02, maybe we should disable some of the interrupt. by Emily
	pHalData->IntrMask[0]	= (u32)(			\
								IMR_ROK			|
								IMR_VODOK		|
								IMR_VIDOK 		|
								IMR_BEDOK 		|
								IMR_BKDOK		|
//								IMR_TBDER		|
								IMR_MGNTDOK 	|
//								IMR_TBDOK		|
								IMR_HIGHDOK 	|
								IMR_BDOK		|
//								IMR_ATIMEND		|
								IMR_RDU			|
								IMR_RXFOVW		|
								IMR_BcnInt		|
								IMR_PSTIMEOUT	| // P2P PS Timeout
//								IMR_TXFOVW		|
//								IMR_TIMEOUT1		|
//								IMR_TIMEOUT2		|
//								IMR_BCNDOK1		|
//								IMR_BCNDOK2		|
//								IMR_BCNDOK3		|
//								IMR_BCNDOK4		|
//								IMR_BCNDOK5		|
//								IMR_BCNDOK6		|
//								IMR_BCNDOK7		|
//								IMR_BCNDOK8		|
//								IMR_BCNDMAINT1	|
//								IMR_BCNDMAINT2	|
//								IMR_BCNDMAINT3	|
//								IMR_BCNDMAINT4	|
//								IMR_BCNDMAINT5	|
//								IMR_BCNDMAINT6	|
								0);
	pHalData->IntrMask[1] 	= (u32)(\
//								IMR_WLANOFF		|
//								IMR_OCPINT		|
//								IMR_CPWM		|
								IMR_C2HCMD		|
//								IMR_RXERR		|
//								IMR_TXERR		|
								0);

	pHalData->IntrMaskToSet[0] = pHalData->IntrMask[0];
	pHalData->IntrMaskToSet[1] = pHalData->IntrMask[1];

}

void rtl8192de_set_hal_ops(_adapter * padapter)
{
	struct hal_ops	*pHalFunc = &padapter->HalFunc;

_func_enter_;

	padapter->HalData = rtw_zmalloc(sizeof(HAL_DATA_TYPE));
	if(padapter->HalData == NULL){
		DBG_8192C("cant not alloc memory for HAL DATA \n");
	}
	//_rtw_memset(padapter->HalData, 0, sizeof(HAL_DATA_TYPE));

	pHalFunc->hal_init = &rtl8192de_hal_init;
	pHalFunc->hal_deinit = &rtl8192de_hal_deinit;

	//pHalFunc->free_hal_data = &rtl8192d_free_hal_data;

	pHalFunc->inirp_init = &rtl8192de_init_desc_ring;
	pHalFunc->inirp_deinit = &rtl8192de_free_desc_ring;

	pHalFunc->init_xmit_priv = &rtl8192de_init_xmit_priv;
	pHalFunc->free_xmit_priv = &rtl8192de_free_xmit_priv;

	pHalFunc->init_recv_priv = &rtl8192de_init_recv_priv;
	pHalFunc->free_recv_priv = &rtl8192de_free_recv_priv;

#ifdef CONFIG_SW_LED
	pHalFunc->InitSwLeds = &rtl8192de_InitSwLeds;
	pHalFunc->DeInitSwLeds = &rtl8192de_DeInitSwLeds;
#else //case of hw led or no led
	pHalFunc->InitSwLeds = NULL;
	pHalFunc->DeInitSwLeds = NULL;	
#endif //CONFIG_SW_LED

	//pHalFunc->dm_init = &rtl8192d_init_dm_priv;
	//pHalFunc->dm_deinit = &rtl8192d_deinit_dm_priv;

	pHalFunc->init_default_value = &rtl8192de_init_default_value;
	pHalFunc->intf_chip_configure = &rtl8192de_interface_configure;
	pHalFunc->read_adapter_info = &ReadAdapterInfo8192DE;

	pHalFunc->enable_interrupt = &EnableInterrupt8192DE;
	pHalFunc->disable_interrupt = &DisableInterrupt8192DE;
	pHalFunc->interrupt_handler = &rtl8192de_interrupt;

	//pHalFunc->set_bwmode_handler = &PHY_SetBWMode8192D;
	//pHalFunc->set_channel_handler = &PHY_SwChnl8192D;

	//pHalFunc->hal_dm_watchdog = &rtl8192d_HalDmWatchDog;

	pHalFunc->SetHwRegHandler = &SetHwReg8192DE;
	pHalFunc->GetHwRegHandler = &GetHwReg8192DE;
  	pHalFunc->GetHalDefVarHandler = &GetHalDefVar8192DE;
 	pHalFunc->SetHalDefVarHandler = &SetHalDefVar8192DE;

	pHalFunc->UpdateRAMaskHandler = &UpdateHalRAMask8192DE;
	pHalFunc->SetBeaconRelatedRegistersHandler = &SetBeaconRelatedRegisters8192DE;

	//pHalFunc->Add_RateATid = &rtl8192d_Add_RateATid;

	pHalFunc->hal_xmit = &rtl8192de_hal_xmit;
	pHalFunc->mgnt_xmit = &rtl8192de_mgnt_xmit;
        pHalFunc->hal_xmitframe_enqueue = &rtl8192de_hal_xmitframe_enqueue;

	//pHalFunc->read_bbreg = &rtl8192d_PHY_QueryBBReg;
	//pHalFunc->write_bbreg = &rtl8192d_PHY_SetBBReg;
	//pHalFunc->read_rfreg = &rtl8192d_PHY_QueryRFReg;
	//pHalFunc->write_rfreg = &rtl8192d_PHY_SetRFReg;

#ifdef CONFIG_HOSTAPD_MLME
	pHalFunc->hostap_mgnt_xmit_entry = &rtl8192de_hostap_mgnt_xmit_entry;
#endif

	rtl8192d_set_hal_ops(pHalFunc);

_func_exit_;

}

