#ifndef __KDEV_WIFI_H__
#define __KDEV_WIFI_H__

#include "kdev_driver_wifi.h"
#include "kdev_wifi_config.h"

/**@brief The maximum length of SSID.
*/
#define WIFI_MAX_LENGTH_OF_SSID             (32)

/**@brief MAC address length.
*/
#define WIFI_MAC_ADDRESS_LENGTH             (6)

/**@brief The maximum length of passphrase used in WPA-PSK and WPA2-PSK encryption types.
*/
#define WIFI_LENGTH_PASSPHRASE              (64)


#define WIFI_MODE_STA_ONLY    1
#define WIFI_MODE_AP_ONLY     2
#define WIFI_MODE_AP_STA      3 

#define WIFI_MODE_STA 0
#define WIFI_MODE_AP 1

#define TCP_MODE_NORMAL 0
#define TCP_MODE_SSL 1

/**@defgroup WIFI_STRUCT Structure
* @{
*/
/** @brief This enumeration defines the wireless authentication mode to indicate the Wi-Fi device’s authentication attribute.
*/
typedef enum {
    WIFI_AUTH_MODE_OPEN = 0,                        /**< Open mode.     */
    //WIFI_AUTH_MODE_SHARED,                          /**< Not supported. */
    WIFI_AUTH_MODE_AUTO_WEP,                        /**< Not supported. */
   // WIFI_AUTH_MODE_WPA,                             /**< Not supported. */
    WIFI_AUTH_MODE_WPA_PSK,                         /**< WPA_PSK.       */
    //WIFI_AUTH_MODE_WPA_None,                        /**< Not supported. */
   // WIFI_AUTH_MODE_WPA2,                            /**< Not supported. */
    WIFI_AUTH_MODE_WPA2_PSK,                        /**< WPA2_PSK.      */
    //WIFI_AUTH_MODE_WPA_WPA2,                        /**< Not supported. */
    WIFI_AUTH_MODE_WPA_PSK_WPA2_PSK,                /**< Mixture mode.  */
} wifi_auth_mode_t;

typedef struct 
{
	int32_t sock;
	int32_t af;
	int32_t type;
	int32_t protocol;
	int32_t rc;
	
}socket_info;

#if 0
/** @brief This structure is the Wi-Fi configuration for initialization in STA mode.
*/
typedef struct {
    uint8_t ssid[WIFI_MAX_LENGTH_OF_SSID];    /**< The SSID of the target AP. */
    uint8_t ssid_length;                      /**< The length of the SSID. */
    uint8_t bssid_present;                    /**< The BSSID is present if it is set to 1. Otherwise, it is set to 0. */
    uint8_t bssid[WIFI_MAC_ADDRESS_LENGTH];   /**< The MAC address of the target AP. */
    uint8_t password[WIFI_LENGTH_PASSPHRASE]; /**< The password of the target AP. */
    uint8_t password_length;                  /**< The length of the password. If the length is 64, the password is regarded as PMK. */
} wifi_sta_config_t;


/** @brief This structure is the Wi-Fi configuration for initialization in AP mode.
*/
typedef struct {
    uint8_t ssid[WIFI_MAX_LENGTH_OF_SSID];    /**< The SSID of the AP. */
    uint8_t ssid_length;                      /**< The length of the SSID. */
    uint8_t password[WIFI_LENGTH_PASSPHRASE]; /**< The password of the AP. */
    uint8_t password_length;                  /**< The length of the password. */
    wifi_auth_mode_t auth_mode;               /**< The authentication mode. */
 
    uint8_t channel;                          /**< The channel. */   
} wifi_ap_config_t;

/** @brief Wi-Fi configuration for initialization.
*/
typedef struct {
    uint8_t opmode;                          /**< The operation mode. The value should be #WIFI_MODE_STA_ONLY, #WIFI_MODE_AP_ONLY, #WIFI_MODE_REPEATER or #WIFI_MODE_MONITOR*/
    wifi_sta_config_t sta_config;            /**< The configurations for the STA. It should be set when the OPMODE is #WIFI_MODE_STA_ONLY or #WIFI_MODE_REPEATER. */
    wifi_ap_config_t ap_config;              /**< The configurations for the AP. It should be set when the OPMODE is #WIFI_MODE_AP_ONLY or #WIFI_MODE_REPEATER. */
} wifi_config_t;
#endif

uint8_t kdev_WIFI_DV_Initialize (void);
void kdev_WIFI_DV_Uninitialize (void) ;

ARM_DRIVER_VERSION kdev_WIFI_GetVersion (void);

void kdev_WIFI_GetCapabilities (void);

int32_t kdev_WIFI_Initialize_Uninitialize (bool wifi_init);

int32_t kdev_WIFI_GetModuleInfo (char* info_buf,uint32_t info_len) ;

int32_t kdev_WIFI_SetOption_BSSID(uint32_t interface,uint8_t *bssid_buf);

int32_t kdev_WIFI_GetOption_BSSID(uint32_t interface, uint8_t *bssid,uint32_t *bssid_len);

int32_t kdev_WIFI_SetOption_TX_POWER (uint32_t interface,uint32_t power) ;
int32_t kdev_WIFI_GetOption_TX_POWER (uint32_t interface,uint32_t *power_buf);

int32_t kdev_WIFI_SetOption_LP_TIMER (uint32_t interface,uint32_t time) ;

int32_t kdev_WIFI_GetOption_LP_TIMER (uint32_t interface,uint32_t *time_buf);

int32_t kdev_WIFI_SetOption_DTIM (uint32_t interface,uint32_t dtime) ;
int32_t kdev_WIFI_GetOption_DTIM (uint32_t interface,uint32_t *dtime_buf);

int32_t kdev_WIFI_SetOption_BEACON (uint32_t beacon);
int32_t kdev_WIFI_GetOption_BEACON (uint32_t *beacon_buf);

int32_t kdev_WIFI_SetOption_MAC (uint32_t interface,uint8_t *mac_buf) ;
int32_t kdev_WIFI_GetOption_MAC (uint32_t interface,uint32_t *mac_buf);

int32_t kdev_WIFI_SetOption_IP_info (uint32_t interface,uint32_t option, uint32_t *buf);
int32_t kdev_WIFI_GetOption_IP_info (uint32_t interface,uint32_t option, uint32_t *ip_buf);

int32_t kdev_WIFI_SetOption_DNS(uint32_t interface,uint32_t option, uint32_t *dns_buf);
int32_t kdev_WIFI_GetOption_DNS (uint32_t interface,uint32_t option, uint32_t *dns_buf);

int32_t kdev_WIFI_SetOption_IP_DHCP(uint32_t interface,bool on_flag);
int32_t kdev_WIFI_GetOption_IP_DHCP (uint32_t interface,bool on_flag);

int32_t kdev_WIFI_SetOption_IP_DHCP_POOL(uint32_t interface,uint8_t* ip_begin, uint8_t *ip_end);
int32_t kdev_WIFI_GetOption_IP_DHCP_POOL (uint32_t interface,uint8_t* ip_begin, uint8_t *ip_end);

int32_t kdev_WIFI_SetOption_IP_DHCP_RELEASE_TIME(uint32_t interface,uint32_t time);
int32_t kdev_WIFI_GetOption_IP_DHCP_RELEASE_TIME(uint32_t interface,uint32_t *time);

int32_t kdev_WIFI_Scan(ARM_WIFI_SCAN_INFO_t *scan_info, uint32_t max_num);
int32_t kdev_WIFI_Activate_Deactivate (uint32_t interface,ARM_WIFI_CONFIG_t *wifi_config, bool act_flag) ;
int32_t kdev_WIFI_IsConnected (void) ;
int32_t kdev_WIFI_GetNetInfo(ARM_WIFI_NET_INFO_t *net_info);

int32_t kdev_socket_creat(int32_t family,int32_t type,int32_t proto);
int32_t kdev_socket_bind(int32_t s, const uint8_t *ip, uint32_t ip_len, uint16_t port);
int32_t kdev_socket_listen(int32_t s, int32_t backlog);
int32_t kdev_socket_accept(int32_t s, uint8_t *ip, uint32_t *ip_len, uint16_t *port);
int32_t kdev_socket_connect(int32_t s, const uint8_t *ip, uint32_t ip_len, uint16_t port,uint32_t mode);
int32_t kdev_socket_setsockopt(int32_t s, int32_t opt_id, const void *opt_val, uint32_t opt_len);
int32_t kdev_socket_getsockopt(int32_t s,int32_t opt_id, void *opt_val, uint32_t *opt_len);
int32_t kdev_socket_recv(int32_t s, void *buf, uint32_t len);
int32_t kdev_socket_send(int32_t s, const void *buf, uint32_t len);
int32_t kdev_socket_sendto(int32_t s, const void *buf, uint32_t len, const uint8_t *ip, uint32_t ip_len, uint16_t port);
int32_t kdev_socket_recvfrom(int32_t s, void *buf, uint32_t len, uint8_t *from, uint32_t *from_len, uint16_t *port);
int32_t kdev_socketclose(int32_t s);
int32_t kdev_socket_get_host_by_name(const char *name, int32_t af, uint8_t *ip, uint32_t *ip_len);

#endif
