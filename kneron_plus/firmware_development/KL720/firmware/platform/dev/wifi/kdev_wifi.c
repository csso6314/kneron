/*-----------------------------------------------------------------------------
 *      Name:         kdev_wifi.c
 *      Purpose:      WiFi driver interface
 *----------------------------------------------------------------------------
 *      Copyright(c) KNERON 
 *----------------------------------------------------------------------------*/

/*
  Known limitations:
  - Bypass mode and functionality is not SUPPORTED
  - Set/GetOption API does not test IPv6 options
  - SetOption operation is not tested, only API is tested
    (BSSID, MAC, static IP operation testing would require dedicated hardware
     with manual check of results on dedicated hardware)
  - WPS operation is not tested (not Station nor AP)
    (WPS functional testing would require dedicated hardware
     with manual check of results WPS AP, WPS on Station could
     be checked by comparing parameters with expected result (configured))
  - WiFi sockets tested in blocking mode only
  - WiFi sockets not tested for IPv6
*/

#include "kdev_wifi.h"
#include "cmsis_os2.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kmdw_console.h"

#define GET_SYSTICK() osKernelGetSysTimerCount()
#define SYSTICK_MICROSEC(microsec) (((uint64_t)microsec *  osKernelGetSysTimerFreq()) / 1000000)


/* Register Driver_WiFi# */
extern ARM_DRIVER_WIFI         ARM_Driver_WiFi_(DRV_WIFI);
static ARM_DRIVER_WIFI* wifi_drv = &ARM_Driver_WiFi_(DRV_WIFI);

/* Local variables */
static uint8_t                  powered   = 0U;
static uint8_t                  connected = 0U;

static uint8_t volatile         event;


static ARM_WIFI_SignalEvent_t   event_func;
static ARM_WIFI_CAPABILITIES    cap;

#if 0
/* String representation of Driver return codes */
static const char *str_ret[] = {
  "ARM_DRIVER_OK",
  "ARM_DRIVER_ERROR",
  "ARM_DRIVER_ERROR_BUSY",
  "ARM_DRIVER_ERROR_TIMEOUT",
  "ARM_DRIVER_ERROR_UNSUPPORTED",
  "ARM_DRIVER_ERROR_PARAMETER",
  "ARM_DRIVER_ERROR_SPECIFIC"
};
#endif



/* WiFi event */
static void kdev_WIFI_DrvEvent (uint32_t evt, void *arg) {
  (void)arg;

  event |= evt;
}


/*-----------------------------------------------------------------------------
 *      Functions
 *----------------------------------------------------------------------------*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup wifi_funcs WiFi control
\brief WiFi Functions
\details
The WiFi function  include the following interface:
- API interface compliance.
- Some of the control and management operations.
- Socket operation with various transfer sizes and communication parameters.
- Socket performance.
*/

/* Helper function that initializes and powers on WiFi Module if not initialized and powered */
static int32_t init_and_power_on (void) {

  if (powered == 0U) {
    if ((wifi_drv->Initialize   (event_func)     == ARM_DRIVER_OK) && 
        (wifi_drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK)) {
      powered = 1U;
    } else {
      return 0;
    }
  }

  return 1;
}

/* Helper function that is called before tests start executing */
uint8_t kdev_WIFI_DV_Initialize (void) {

  ///sscanf(WIFI_SOCKET_SERVER_IP, "%hhu.%hhu.%hhu.%hhu", &ip_socket_server[0], &ip_socket_server[1], &ip_socket_server[2], &ip_socket_server[3]);

  cap = wifi_drv->GetCapabilities();

  event_func = NULL;
  if ((cap.event_eth_rx_frame   != 0U) ||
      (cap.event_ap_connect     != 0U) ||
      (cap.event_ap_disconnect  != 0U)) {
    event_func = kdev_WIFI_DrvEvent;
  }

  if ((wifi_drv->SocketCreate        != NULL) &&
      (wifi_drv->SocketBind          != NULL) &&
      (wifi_drv->SocketListen        != NULL) &&
      (wifi_drv->SocketAccept        != NULL) &&
      (wifi_drv->SocketConnect       != NULL) &&
      (wifi_drv->SocketRecv          != NULL) &&
      (wifi_drv->SocketRecvFrom      != NULL) &&
      (wifi_drv->SocketSend          != NULL) &&
      (wifi_drv->SocketSendTo        != NULL) &&
      (wifi_drv->SocketGetSockName   != NULL) &&
      (wifi_drv->SocketGetPeerName   != NULL) &&
      (wifi_drv->SocketGetOpt        != NULL) &&
      (wifi_drv->SocketSetOpt        != NULL) &&
      (wifi_drv->SocketClose         != NULL) &&
      (wifi_drv->SocketGetHostByName != NULL)) {
				return 1;
  }
	return 0;
}

/* function should be excuted after stop wi-fi */
void kdev_WIFI_DV_Uninitialize (void) {

  if (connected != 0U) {
    if (wifi_drv->Deactivate (0U) == ARM_DRIVER_OK) {
      connected = 0U;
    }
  }
  if (powered != 0U) {
    if ((wifi_drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK) && 
        (wifi_drv->Uninitialize ()              == ARM_DRIVER_OK)) {
      powered = 0U;
    }
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/* WiFi Control function */
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup wifi_ctrl WiFi Control
\ingroup wifi_funcs
\details
These tests verify API and operation of the WiFi control functions.
@{
*/

/**
\brief Function: WIFI_GetVersion
\details
The Function \b WIFI_GetVersion verifies the WiFi Driver \b GetVersion function.
\code
ARM_DRIVER_VERSION (*GetVersion) (void);
\endcode
*/
ARM_DRIVER_VERSION kdev_WIFI_GetVersion (void) {
  ARM_DRIVER_VERSION ver;

  ver = wifi_drv->GetVersion();

  if((ver.api >= ARM_DRIVER_VERSION_MAJOR_MINOR(1,0)) && (ver.drv >= ARM_DRIVER_VERSION_MAJOR_MINOR(1,0)))
	{
		printf("WIFI driver version mismatch\n");
	}

  //snprintf(msg_buf, sizeof(msg_buf), "[INFO] Driver API version %d.%d, Driver version %d.%d", (ver.api >> 8), (ver.api & 0xFFU), (ver.drv >> 8), (ver.drv & 0xFFU));
  //printf("%s\n",msg_buf);
	return ver;
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: WIFI_GetCapabilities
\details
The Function \b WIFI_GetCapabilities verifies the WiFi Driver \b GetCapabilities function.
\code
ARM_WIFI_CAPABILITIES (*GetCapabilities) (void);
\endcode
*/
void kdev_WIFI_GetCapabilities (void) {

  cap = wifi_drv->GetCapabilities();
	
	if((cap.station_ap != 0U) || (cap.station != 0U) || (cap.ap != 0U)){
			printf("At least 1 mode must be supported \n");	
	}
 
  if (cap.wps_station != 0U) {
    if((cap.station_ap != 0U) || (cap.station != 0U))
		{
			printf( "If WPS for station is supported version of station mode of operation must be supported also\n");
		}
  }

  if (cap.wps_ap != 0U) {
    if((cap.station_ap != 0U) || (cap.ap != 0U))
		{
			printf( "If WPS for AP is supported version of AP mode of operation must be supported also\n");
		}
  }

  if ((cap.event_ap_connect != 0U) || (cap.event_ap_disconnect != 0U)) {
    if((cap.station_ap != 0U) || (cap.ap != 0U))
			printf("If events for AP are supported version of AP mode of operation must be supported also\n");
  }

  if (cap.event_eth_rx_frame != 0U) {
    if(cap.bypass_mode != 0U)
		{printf("If event for Ethernet Rx frame is supported, bypass mode must be supported also\n");}
	}
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: WIFI_Initialize/Uninitialize
\details
The Function \b WIFI_Initialize_Uninitialize verifies the WiFi Driver \b Initialize and \b Uninitialize functions.
\code
int32_t (*Initialize) (ARM_WIFI_SignalEvent_t cb_event);
\endcode
and
\code
int32_t (*Uninitialize) (void);
\endcode
excuting sequence:
 
*/
int32_t kdev_WIFI_Initialize_Uninitialize (bool wifi_init) {
  int32_t ret;
	
  if ((cap.event_eth_rx_frame  != 0U) ||
      (cap.event_ap_connect    != 0U) ||
      (cap.event_ap_disconnect != 0U)) {
			event_func = kdev_WIFI_DrvEvent;
  }
	
  if(wifi_init == true)
  {	
    ret = wifi_drv->Initialize(event_func);  //Initialize with callback (if driver supports it)
    if(ret != ARM_DRIVER_OK)
    {
      err_msg("wifi init error:%d\n",ret);
      return ret;
    }
		
    ret = wifi_drv->PowerControl(ARM_POWER_FULL); //Power on
    if(ret != ARM_DRIVER_OK)
    {
      err_msg("wifi PowerControl  ARM_POWER_FULL error:%d\n",ret);
      //return ret;
    }
		else
		{
			powered = 1u;
		}
  }
	
  else
  {
      ret = wifi_drv->Uninitialize(); //Uninitialize
      if(ret != ARM_DRIVER_OK)
      {
        err_msg("wifi Uninitialize error:%d\n",ret);
        return ret;
      }

      ret = wifi_drv->PowerControl(ARM_POWER_OFF); //Power off
      if(ret != ARM_DRIVER_OK)
      {
        err_msg("wifi PowerControl  ARM_POWER_OFF error:%d\n",ret);
        //return ret;
      }
  }

	return ret;
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: WIFI_PowerControl
\details
The Function \b WIFI_PowerControl verifies the WiFi Driver \b PowerControl function.
\code
int32_t (*PowerControl) (ARM_POWER_STATE state);
\endcode
Testing sequence:
 - Power off
 - Initialize with callback (if driver supports it)
 - Power off
 - Power on
 - Scan
 - Power low
 - Power off
 - Uninitialize

---------------reserved--------------

static int32_t kdev_WIFI_PowerControl (ARM_POWER_STATE state) {
  int32_t ret;

  ret = wifi_drv->PowerControl(state);
  if(ret != ARM_DRIVER_OK)
  {
    err_msg("wifi powercontrol failed:%s \n",str_ret[-ret]);
  }
  return ret;
  
}
*/
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: WIFI_GetModuleInfo
\details
The Function \b WIFI_GetModuleInfo verifies the WiFi Driver \b GetModuleInfo function.
\code
int32_t (*GetModuleInfo) (char *module_info, uint32_t max_len);
\endcode
*/
int32_t kdev_WIFI_GetModuleInfo (char* info_buf,uint32_t info_len) {
  int32_t ret;

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  if(info_buf == NULL || (info_len == 0))
  {
    ret = ARM_DRIVER_ERROR_PARAMETER;
    return ret;
  }

  ret = wifi_drv->GetModuleInfo(info_buf, info_len);
  if (ret != ARM_DRIVER_OK)
  {
    err_msg("GetModuleInfo () returned %s", str_ret[-ret]);
  }
  
  return ret;
}
/**
@}
*/
// End of wifi_ctrl

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/* WiFi Management  */
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup wifi_mgmt WiFi Management
\ingroup wifi_funcs
\details
These tests verify API and operation of the WiFi management functions.
@{
*/


/*
para:
  interface: station:0 AP:1
*/
int32_t kdev_WIFI_SetOption_BSSID(uint32_t interface,uint8_t *bssid_buf)
{
  //uint8_t u8_arr[8] __ALIGNED(4)={0};
  uint8_t  bssid[7]  __ALIGNED(4);
  //uint8_t  not_suported;
  int32_t ret = 0;

  

  if((interface > 1) || (bssid_buf == NULL))
  {
    err_msg("parameters error \n");
    return -1;
  }
	
	memset(bssid,0,7);
  sscanf((const char *)bssid_buf, "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx", &bssid[0], &bssid[1], &bssid[2], &bssid[3], &bssid[4], &bssid[5]);
		
  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  //memcpy(u8_arr,bssid,6);
  ret = wifi_drv->SetOption(interface, ARM_WIFI_BSSID, bssid, 6);
  if(ret != ARM_DRIVER_OK)
  {
    err_msg("set bssid error :%s\n",str_ret[-ret]);
    //return ret;
  }

  return ret;
}

int32_t kdev_WIFI_GetOption_BSSID(uint32_t interface, uint8_t *bssid,uint32_t *bssid_len)
{
  //uint8_t u8_arr[8] __ALIGNED(4)={0};
  //uint8_t  not_suported;
  int32_t ret = 0;

  if((interface > 1) || (bssid == NULL))
  {
    err_msg("parameters error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  //memcpy(u8_arr,bssid,6);
  ret = wifi_drv->GetOption(interface, ARM_WIFI_BSSID, bssid, bssid_len);
  if(ret != ARM_DRIVER_OK)
  {
    err_msg("set bssid error :%s\n",str_ret[-ret]);
    //return ret;
  }
  else
  {
    if(*bssid_len != 6)
    {
      err_msg("set bssid LENGTH error :%d\n",bssid_len);
    }
  }

  return ret;
}

int32_t kdev_WIFI_SetOption_TX_POWER (uint32_t interface,uint32_t power) 
{
  int32_t ret = 0;

  if(interface > 1)
  {
    err_msg("set txpower para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  ret = wifi_drv->SetOption ( interface, ARM_WIFI_TX_POWER, &power, 4U);
  if( ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Driver set tx power failed:%s\n",str_ret[-ret]);
  }

  return ret ;
}

int32_t kdev_WIFI_GetOption_TX_POWER (uint32_t interface,uint32_t *power_buf)
{
  int32_t ret = 0;
  uint32_t power_len = 4U;

  if(interface > 1)
  {
    err_msg("set txpower para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  ret = wifi_drv->GetOption ( interface, ARM_WIFI_TX_POWER, power_buf, &power_len);
  if( ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Driver set tx power failed:%s\n",str_ret[-ret]);
  }

  return ret ;
} 

int32_t kdev_WIFI_SetOption_LP_TIMER (uint32_t interface,uint32_t time) 
{
  int32_t ret = 0;

  if(interface > 1)
  {
    err_msg("set lp timer para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  ret = wifi_drv->SetOption ( interface, ARM_WIFI_LP_TIMER, &time, 4U);
  if( ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Driver set lp timer failed:%s\n",str_ret[-ret]);
  }

  return ret ;
}

int32_t kdev_WIFI_GetOption_LP_TIMER (uint32_t interface,uint32_t *time_buf)
{
  int32_t ret = 0;
  uint32_t time_len = 4U;

  if(interface > 1)
  {
    err_msg("get lp timer para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  ret = wifi_drv->GetOption ( interface, ARM_WIFI_LP_TIMER, time_buf, &time_len);
  if( ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Driver get lp timer failed:%s\n",str_ret[-ret]);
  }

  return ret ;
} 

int32_t kdev_WIFI_SetOption_DTIM (uint32_t interface,uint32_t dtime) 
{
  int32_t ret = 0;

  if(interface > 1)
  {
    err_msg("set dtim para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  ret = wifi_drv->SetOption ( interface, ARM_WIFI_DTIM, &dtime, 4U);
  if( ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Driver set dtim failed:%s\n",str_ret[-ret]);
  }

  return ret ;
}

int32_t kdev_WIFI_GetOption_DTIM (uint32_t interface,uint32_t *dtime_buf)
{
  int32_t ret = 0;
  uint32_t dtim_len = 4U;

  if(interface > 1)
  {
    err_msg("get dtim para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  ret = wifi_drv->GetOption ( interface, ARM_WIFI_DTIM, dtime_buf, &dtim_len);
  if( ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Driver get dtim failed:%s\n",str_ret[-ret]);
  }

  return ret ;
} 

/*only used in AP mode*/
int32_t kdev_WIFI_SetOption_BEACON (uint32_t beacon) 
{
  int32_t ret = 0;
  
  
  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  ret = wifi_drv->SetOption ( 1u, ARM_WIFI_BEACON, &beacon, 4U);
  if( ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Driver set beacon failed:%s\n",str_ret[-ret]);
  }

  return ret ;
}

int32_t kdev_WIFI_GetOption_BEACON (uint32_t *beacon_buf)
{
  int32_t ret = 0;
  uint32_t beacon_len = 4U;
  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  ret = wifi_drv->GetOption ( 1U, ARM_WIFI_BEACON, beacon_buf, &beacon_len);
  if( ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Driver get beacon failed:%s\n",str_ret[-ret]);
  }

  return ret ;
} 

int32_t kdev_WIFI_SetOption_MAC (uint32_t interface,uint8_t *mac_buf) 
{
  int32_t ret = 0;
  uint8_t mac[7] __ALIGNED(4);

  if(interface > 1 || (mac_buf == NULL))
  {
    err_msg("set mac para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  sscanf((const char *)mac_buf, "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

  ret = wifi_drv->SetOption ( interface, ARM_WIFI_MAC, mac, 6U);
  if( ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Driver set mac failed:%s\n",str_ret[-ret]);
  }

  return ret ;
}

int32_t kdev_WIFI_GetOption_MAC (uint32_t interface,uint32_t *mac_buf)
{
  int32_t ret = 0;
  uint32_t mac_len = 6U;

  if(interface > 1 || (mac_buf == NULL))
  {
    err_msg("get mac para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  ret = wifi_drv->GetOption ( interface, ARM_WIFI_MAC, mac_buf, &mac_len);
  if( ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Driver get mac failed:%s\n",str_ret[-ret]);
  }
  else
  {
    if(mac_len != 6)
    {
      err_msg("get mac length failed :%d\n",mac_len);
    }
  }

  return ret ;
} 

/*buf string formate ："xxx.xxx.xxx.xxx"
  optinon:
    ARM_WIFI_IP
    ARM_WIFI_IP_SUBNET_MASK
    ARM_WIFI_IP_GATEWAY
*/
int32_t kdev_WIFI_SetOption_IP_info (uint32_t interface,uint32_t option, uint32_t *buf)
{
  int32_t ret = 0;
  uint8_t ip[4] = {0};

  if(interface > 1 || (buf == NULL))
  {
    err_msg("set ip para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  
  if(sscanf((const char *)buf, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) != 4)
  {
      err_msg("[FAILED] IP formate error \n");
      return -1;
  }

  ret = wifi_drv->SetOption ( interface, option, ip, 4U);
  if( ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Drive SET IP failed:%s\n",str_ret[-ret]);
  }
  return ret ;
} 

/*
  optinon:
    ARM_WIFI_IP
    ARM_WIFI_IP_SUBNET_MASK
    ARM_WIFI_IP_GATEWAY
*/
int32_t kdev_WIFI_GetOption_IP_info (uint32_t interface,uint32_t option, uint32_t *ip_buf)
{
  int32_t ret = 0;
  uint32_t ip_len = 4U;
 

  if(interface > 1 || (ip_buf == NULL))
  {
    err_msg("get ip  para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  
  ret = wifi_drv->GetOption ( interface, option, ip_buf, &ip_len);
  if( ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Drive GET IP info failed:%s\n",str_ret[-ret]);
  }
  return ret ;
} 

/*
  OPTION:
    ARM_WIFI_IP_DNS1
    ARM_WIFI_IP_DNS2
*/
int32_t kdev_WIFI_SetOption_DNS(uint32_t interface,uint32_t option, uint32_t *dns_buf)
{
  int32_t ret = 0;
  uint8_t ip[4] = {0};

  if(interface > 1 || (dns_buf == NULL))
  {
    err_msg("set dns para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }


  if(sscanf((const char *)dns_buf, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) != 4)
  {
    err_msg("[FAILED] IP formate error \n");
    return -1;
  }

  ret = wifi_drv->SetOption ( interface, option, ip, 4U);
  if( ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Drive SET DNS failed:%s\n",str_ret[-ret]);
  }
  return ret ;
}

/*
  optinon:
    ARM_WIFI_IP
    ARM_WIFI_IP_SUBNET_MASK
    ARM_WIFI_IP_GATEWAY
*/
int32_t kdev_WIFI_GetOption_DNS (uint32_t interface,uint32_t option, uint32_t *dns_buf)
{
  int32_t ret = 0;
  uint32_t dns_len = 4U;
 

  if(interface > 1 || (dns_buf == NULL))
  {
    err_msg("get dns  para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  
  ret = wifi_drv->GetOption (interface, option, dns_buf, &dns_len);
  if( ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Drive GET dns info failed:%s\n",str_ret[-ret]);
  }
  return ret ;
} 


int32_t kdev_WIFI_SetOption_IP_DHCP(uint32_t interface,bool on_flag)
{
  uint32_t on = on_flag;
  int32_t ret ;
  
  if(interface > 1 )
  {
    err_msg("set dhcp  para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  ret = wifi_drv->SetOption (interface, ARM_WIFI_IP_DHCP, &on,   4U);
  if(ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Drive set dhcp(%d) info failed:%s\n",on,str_ret[-ret]);
  }
  return ret;
}


int32_t kdev_WIFI_GetOption_IP_DHCP (uint32_t interface,bool on_flag)
{
  int32_t ret = 0;
  uint32_t len = 4U;
  uint32_t dhcp_flag ;
 

  if(interface > 1)
  {
    err_msg("get dns  para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  
  ret = wifi_drv->GetOption (interface, ARM_WIFI_IP_DHCP, &dhcp_flag, &len);
  if( ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Drive GET dhcp option failed:%s\n",str_ret[-ret]);
  }
  return ret ;
} 

int32_t kdev_WIFI_SetOption_IP_DHCP_POOL(uint32_t interface,uint8_t* ip_begin, uint8_t *ip_end)
{
  int32_t ret ;
  uint8_t buf[5] = {0};
  
  if(interface > 1 || (ip_begin == NULL) || (ip_end == NULL))
  {
    err_msg("set dhcp  para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  sscanf((const char *)ip_begin, "%hhu.%hhu.%hhu.%hhu", &buf[0], &buf[1], &buf[2], &buf[3]);
  ret = wifi_drv->SetOption (interface, ARM_WIFI_IP_DHCP_POOL_BEGIN, buf,   4U);
  if(ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Drive set dhcp pool begin failed:%s\n",str_ret[-ret]);
    return ret;
  }
  
  memset(buf, 0 ,5);
  sscanf((const char *)ip_end, "%hhu.%hhu.%hhu.%hhu", &buf[0], &buf[1], &buf[2], &buf[3]);
  ret = wifi_drv->SetOption (interface, ARM_WIFI_IP_DHCP_POOL_END, buf,  4U);
  if(ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Drive set dhcp pool begin failed:%s\n",str_ret[-ret]);
  }
  return ret;
}


int32_t kdev_WIFI_GetOption_IP_DHCP_POOL (uint32_t interface,uint8_t* ip_begin, uint8_t *ip_end)
{
  int32_t ret = 0;
  uint32_t len = 4U;
 

  if(interface > 1 || (ip_begin != NULL) || (ip_end != NULL))
  {
    err_msg("get dns  para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  
  ret = wifi_drv->GetOption (interface, ARM_WIFI_IP_DHCP_POOL_BEGIN, ip_begin, &len);
  if( ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Drive GET dhcp pool begin failed:%s\n",str_ret[-ret]);
    return ret ;
  }
  
  ret = wifi_drv->GetOption (interface, ARM_WIFI_IP_DHCP_POOL_END, ip_end, &len);
  if( ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Drive GET dhcp pool end failed:%s\n",str_ret[-ret]);
  }
  return ret ;
} 

/*just used for AP mode*/
int32_t kdev_WIFI_SetOption_IP_DHCP_RELEASE_TIME(uint32_t interface,uint32_t time)
{
  int32_t ret ;
  uint32_t utime ;
  
  utime = time;
  if(interface > 1 )
  {
    err_msg("set dhcp  release time para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  
  ret = wifi_drv->SetOption (interface, ARM_WIFI_IP_DHCP_LEASE_TIME, &utime,   4U);
  if(ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Drive set dhcp release time failed:%s\n",str_ret[-ret]);
  }
  
  return ret;
}


int32_t kdev_WIFI_GetOption_IP_DHCP_RELEASE_TIME(uint32_t interface,uint32_t *time)
{
  int32_t ret = 0;
  uint32_t len = 4U;
  
  if(interface > 1)
  {
    err_msg("get dns  release time para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed");
    return -1;
  }

  
  ret = wifi_drv->GetOption (interface, ARM_WIFI_IP_DHCP_LEASE_TIME, time, &len);
  if( ret != ARM_DRIVER_OK)
  {
    err_msg("[FAILED] Drive GET release time failed:%s\n",str_ret[-ret]);
  }
  
  
  return ret ;
} 


/*----------------------------------------------------------------------------
wifi management interface
WIFI_Scan
WIFI_Activate_Deactivate
WIFI_IsConnected
WIFI_GetNetInfo
----------------------------------------------------------------------------*/

/*note: 0< max_num <=10*/
int32_t kdev_WIFI_Scan(ARM_WIFI_SCAN_INFO_t *scan_info, uint32_t max_num)
{
  int32_t ret;

  if( scan_info == NULL || (max_num == 0))
  {
    err_msg("scan para error \n");
    return -1;
  }

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed\n");
    return -1;
  }

   ret = wifi_drv->Scan(scan_info, max_num);
   if(ret > 0)
   {
     err_msg("scan find wifi num :%d\n",ret);
   }else
   {
     err_msg("not find wifi \n");
   }

   return ret; 
}

int32_t kdev_WIFI_Activate_Deactivate (uint32_t interface,ARM_WIFI_CONFIG_t *wifi_config, bool act_flag) 
{
  int32_t ret;

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed \n");
    return -1;
  }

  if(act_flag)
  {
    if(wifi_config == NULL )
    {
      err_msg("scan para error \n");
      return -1;
    }

    ret = wifi_drv->Activate(interface, wifi_config);
    if(ret != ARM_DRIVER_OK)
    {
       err_msg("[FAILED] wifi activate failed:%s\n",str_ret[-ret]);
       return ret;
    }
  }
  else
  {
    ret = wifi_drv->Deactivate(interface);
    if(ret != ARM_DRIVER_OK)
    {
       err_msg("[FAILED] wifi deactivate failed:%s\n",str_ret[-ret]);
    }
  }
  return ret;
}

int32_t kdev_WIFI_IsConnected (void) 
{
  uint32_t conn;

  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed \n");
    return -1;
  }

  conn = wifi_drv->IsConnected();

  return conn;
}

int32_t kdev_WIFI_GetNetInfo(ARM_WIFI_NET_INFO_t *net_info)
{
  int32_t ret ;

  if(net_info == NULL )
  {
    err_msg("GET net infomation para error \n");
    return -1;
  }
  if (init_and_power_on () == 0) {
    err_msg("[FAILED] Driver initialization and power on failed \n");
    return -1;
  }

  ret = wifi_drv->GetNetInfo(net_info);
  if(ret != ARM_DRIVER_OK)
  {
      err_msg("[FAILED] wifi deactivate failed:%s\n",str_ret[-ret]);
  }

  return ret;
}

/*--------------------------------------------------------------------------
wifi socket api interface

---------------------------------------------------------------------------*/

/**
 * It is a replacement of DRIVER socket(). The parameters are same.
 *						
 * @return				pointer to created socket cast as a int32_t or SOCKET_ERROR
 */
int32_t kdev_socket_creat(int32_t family,int32_t type,int32_t proto)
{
  return wifi_drv->SocketCreate(family, type, proto);
}
/**
 * It is a replacement of DRIVER bind(). The parameters are same.
 *						
 * @return				0 if OK, else one of the socket error codes
 */
int32_t kdev_socket_bind(int32_t s, const uint8_t *ip, uint32_t ip_len, uint16_t port)
{
  return wifi_drv->SocketBind(s, ip, ip_len, port);
}
/**
 * It is a replacement of DRIVER listen(). The parameters are same.
 *						
 * @return				0 if OK, else one of the socket error codes
 */
int32_t kdev_socket_listen(int32_t s, int32_t backlog)
{
  return  wifi_drv->SocketListen(s, backlog);
}
/**
 * It is a replacement of DRIVER accept(). The parameters are same.
 *						
 * @return				pointer to socket cast as a int32_t or SOCKET_ERROR
 */
int32_t kdev_socket_accept(int32_t s, uint8_t *ip, uint32_t *ip_len, uint16_t *port)
{
  return wifi_drv->SocketAccept(s, ip, ip_len, port);
}
/**
 * It is a replacement of DRIVER connect(). The parameters are same.
 *						
 * @return				0 if ok, else one of the socket error codes
 */
int32_t kdev_socket_connect(int32_t s, const uint8_t *ip, uint32_t ip_len, uint16_t port,uint32_t mode)
{
  return wifi_drv->SocketConnect(s, ip, ip_len, port,mode);
}
/**
 * It is a replacement of DRIVER setsockopt(). The parameters are same.
 *						
 * @return				0 if ok, else one of the socket error codes
 */
int32_t kdev_socket_setsockopt(int32_t s, int32_t opt_id, const void *opt_val, uint32_t opt_len)
{
  return wifi_drv->SocketSetOpt(s, opt_id, opt_val,opt_len);
} 
/**
 * It is a replacement of DRIVER getsockopt(). The parameters are same.
 *						
 * @return				0 if ok, else one of the socket error codes
 */
int32_t kdev_socket_getsockopt(int32_t s,int32_t opt_id, void *opt_val, uint32_t *opt_len)
{
  return wifi_drv->SocketGetOpt(s, opt_id, opt_val, opt_len);
}
/**
 * It is a replacement of DRIVER recv(). The parameters are same.
 *						
 * @return				bytes we receive
 */
int32_t kdev_socket_recv(int32_t s, void *buf, uint32_t len)
{
  return wifi_drv->SocketRecv(s, buf, len);
}
/**
 * It is a replacement of DRIVER send(). The parameters are same.
 *						
 * @return				bytes we send
 */
int32_t kdev_socket_send(int32_t s, const void *buf, uint32_t len)
{
  return wifi_drv->SocketSend(s, buf, len);
}
/**
 * It is a replacement of DRIVER sendto(). The parameters are same.
 *						
 * @return				bytes we send
 */
int32_t kdev_socket_sendto(int32_t s, const void *buf, uint32_t len, const uint8_t *ip, uint32_t ip_len, uint16_t port)
{
  return wifi_drv->SocketSendTo(s, buf, len, ip, ip_len, port);
}
/**
 * It is a replacement of DRIVER recvfrom(). The parameters are same.
 *						
 * @return				bytes we recv
 */
int32_t kdev_socket_recvfrom(int32_t s, void *buf, uint32_t len, uint8_t *from, uint32_t *from_len, uint16_t *port)
{
  return wifi_drv->SocketRecvFrom(s, buf, len, from,from_len,port);
}
/**
 * It is a replacement of DRIVER close(). The parameters are same.
 *						
 * @return				0 on okay orerror code
 */
int32_t kdev_socketclose(int32_t s)
{
  return wifi_drv->SocketClose(s);
}

/**
 * It is a replacement of DRIVER GetHostByName(). The parameters are same.
 *						
 * @return				0 on okay orerror code
 */
int32_t kdev_socket_get_host_by_name(const char *name, int32_t af, uint8_t *ip, uint32_t *ip_len)
{
  return wifi_drv->SocketGetHostByName(name,  af,  ip,  ip_len);
}

#if 0
/* Helper function for execution of socket test function in the worker thread */
static int32_t th_execute (osThreadId_t *id, uint32_t sig, uint32_t tout) {
  osThreadFlagsSet (id, sig);
  if (osThreadFlagsWait (TH_OK | TH_TOUT, osFlagsWaitAny, tout) == TH_OK) {
    /* Success, completed in time */
    return (1);
  }
  /* If function timeout expired prepare output message */
  snprintf(msg_buf, sizeof(msg_buf), "[FAILED] Execution timeout (%d ms)", tout);
  return (0);
}

#define TH_EXECUTE(sig,tout) do {                                               \
                               io.xid++;                                        \
                               rval = th_execute (worker, sig, tout);           \
                               if (rval == 0) {                                 \
                                 /* Msg was prepared in th_execute function */  \
                                 err_msg("%s",msg_buf);                \
                               }                                                \
                             } while (0)
#endif
