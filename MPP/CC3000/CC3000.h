/*
 * Copyright (C) 2014 Markus Hoffmann <mackone@inf.fu-berlin.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. This LICENSE in it's most up to date version can be
 * found under: https://www.gnu.org/licenses/lgpl-2.1.html
 */


#ifndef CC3000_H
#define CC3000_H

//=========================================================================
// CC3000 Driver
//=========================================================================
#include "cc3000_common.h"
#include "cc3000_hal.h"
#include "evnt_handler.h"
#include "hci.h"
#include "host_driver_version.h"
#include "netapp.h"
#include "nvmem.h"
#include "os.h"
#include "security.h"
#include "socket.h"
#include "spi.h"
#include "wlan.h"
#include "patch.h"

//eigene Erweiterungen



//#include "rtc.h"

//=========================================================================
// board_lib
//=========================================================================
#include "led.h"
#include "main.h"
#include "usart.h"

//=========================================================================
// standart_lib
//=========================================================================
#include "string.h"
#include "stdbool.h"

//=========================================================================
// standart_lib
//=========================================================================


#define WLAN_PROFILE_SIZE (80)
#define WLAN_ALL_PROFILES_SIZE (560)
#define WLAN_PROFILE_COUNT (7)
#define NETWORKS_ARRAY_MAX (30)
#define CC3000_MAX_SOCK_COUNT (4)

#define RRtype_Question (1);
#define RRtype_Answer (2);
#define RRtype_Authority (3);
#define RRtype_Additional (4);

typedef struct _profiles {
	unsigned long ulSecurityType; // 	4 Bytes: WLAN_SEC_UNSEC,WLAN_SEC_WEP,WLAN_SEC_WPA,WLAN_SEC_WPA2
	unsigned char ucSSID[32]; // 		32 Bytes: SSID up to 32 bytes
	unsigned long SSIDlen; // 			4 Bytes Length of SSID
	unsigned char ucBSSID[6]; // 		6 Bytes: BSSID up to 6 bytes
	unsigned long BSSIDlen; //			4 Bytes Length of BSSID
	unsigned long ulPriority; // 		4 Bytes profile priority. Lowest priority:0
	unsigned long ulPairwiseCypher; // 	4 Bytes ulPairwiseCipher_Or_TxKeyLen  key length for WEP security
	unsigned long ulGroupCypher; // 	4 Bytes ulGroupCipher_TxKeyIndex  WEP key index
	unsigned long ulKeyMgmt; // 		4 Bytes 			ulKeyMgmt 		KEY management
	unsigned char ucSecurityKey[32]; // 32 Bytes assumed ucPf_OrKey		security key
	unsigned long ulPassPhraseLen; // 	4 Bytes security key length for WPA\WPA2
} profile_t;

typedef struct _ipConfig {
	char cIP[4]; // 		4 Bytes: IP Address
	char cSubnet[4]; // 	4 Bytes: Subnet Mask
	char cGateway[4]; // 	4 Bytes: Gateway Address
	char cDHCP[4]; // 		4 Bytes: DHCP Address
	char cDNS[4]; // 		4 Bytes: DNS Address
} ipConfig_t;

typedef struct _scanresults {
	long lNumNetworks; // 		4 Bytes: number of networks found
	long lNetScanStatus; // 	4 Bytes: The status of the scan:
						 // 		0 - agged results,
						 // 		1 - results valid,
						 // 		2 - no results
						 //  	- 56 bytes: Result entry, where the bytes are arranged as
						 // follows:
	_Bool bValid :1; //			1 bit: 	is result valid = 1 or not Valid = 0
	uint8_t sRSSI :7; // 		7 bits: 	RSSI value;
	uint8_t sSecMode :2; // 	2 bits: 	securityMode of the AP: 0 - Open, 1 - WEP, 2 WPA, 3 WPA2
	uint8_t lNameLength :6; // 	6 bits: 	SSID name length
	short sTime; // 			2 bytes: the time at which the entry has entered into scans result table
	char strSSID[32]; // 		32 bytes:SSID name
	char strBSSID[6]; // 		6 bytes: BSSID
} scanresults;

typedef struct _network {
	unsigned char bValid :1; // 	1 byte:		is result valid = 1 or not Valid = 0
	unsigned int uiRSSI :7; // 		1 byte:		RSSI value;
	unsigned int uiSecMode; // 		1 byte:		securityMode of the AP: 0 - Open, 1 - WEP, 2 WPA, 3 WPA2
	unsigned int uiNameLength; // 	1 byte:		SSID name length
	char strSSID[32]; // 			32 bytes:	SSID name
} network_t;

typedef struct _profile {
	unsigned long ulSecType; // 	4  Bytes: 	securityMode of the AP: 0 - Open, 1 - WEP, 2 - WPA, 3 - WPA2
	char strSSID[32]; // 			32 Bytes:	SSID name
	unsigned long ulSSIDlen; //		4  Bytes: 	SSID name length
	unsigned char strKey[32]; // 	32 Bytes: 	Key
	unsigned long ulKeyLen; // 		4  Bytes: 	Key length
	unsigned char ucPriority; // 	1  Byte: 	Verbindungspriorität
} wlan_profile_t;

typedef struct ipInfo {
	int16_t family; // e.g. AF_INET
	uint16_t port; // e.g. htons(3490)
	unsigned char addr[4];
} ipInfo_t;

//====== Zugangsdaten =========================================================

extern char* ssid;
extern char* wLANkey;

extern char tx_daten[2048];
extern unsigned char rx_daten[2048];

extern wlan_profile_t apProfiles[WLAN_PROFILE_COUNT];
extern network_t visibleNetworks[NETWORKS_ARRAY_MAX];
extern profile_t profile;
extern ipConfig_t ipConfig; //							Enthält die Informationen über IP, Subnet, DNS, Gateway
extern const unsigned char smartconfigkey[]; //			AES Schlüssel für die SmartConfig Routine
ipInfo_t senderIP; // 									Wird beim Empfang eines Pakets gesetzt umd die Sender IP zu erhalten

//====== CC3000 Zustandsvariablen =============================================
extern bool CC3000_DHCP_done;
extern bool CC3000_is_Connected;
extern bool CC3000_is_ON;
extern bool CC3000_SmartConfig_active;
extern bool CC3000_SmartConfig_finished;
extern bool CC3000_SmartConfig_stop;
extern unsigned long CC3000_NetScanLength;
extern unsigned long CC3000_SOCKET_WAIT_DISCONNECT;
extern char buffer_out[1024];
//====== Sockets ==============================================================
extern unsigned long nfds;
extern sockaddr sockets[];
extern unsigned long socket_Handles[];
extern unsigned long handle_mDNS;
extern unsigned long handle_HTTP;
extern unsigned long handle_NTP;
extern unsigned long handle_Protokoll;
extern unsigned long handle_HTTP_Client;
extern unsigned long handle_NetBIOS;
extern unsigned long handle_DB;
extern sockaddr my_IP;
extern sockaddr S1_mDNS;
extern sockaddr S2_TCP;
extern sockaddr S3_NTP;
extern sockaddr S4_unused;
extern sockaddr S_NetBIOS;
extern sockaddr S_ConnectedClient;
extern socklen_t s_ConnectedLen;
extern _Bool S3_WaitForAnswer;
extern _Bool S_ClientConnected;


extern char rx_buf[][2048]; // 	Array der Empfangspuffer
extern char tx_buf[][2048]; // 	Array der Empfangspuffer

//====== Funktionen
//===== Initialisierung
void CC3000_Init(void);
void CC3000_Reset(void);

//===== NVMEM
signed long read_NVMEM_Profiles(wlan_profile_t profiles[]);
signed long write_NVMEM_Profiles(unsigned int position, unsigned int ssidLen,
		char* SSID, unsigned int keyLen, char* Key, unsigned long SecurityType);
signed long deleteAll_NVMEM_Profiles(void);
signed long delete_NVMEM_Profile(unsigned int position);

//===== Service Funktionen der Lib
void CC3000_UsynchCallback(long lEventType, char *pcData,
		unsigned char ucLength);
const unsigned char *sendWLFWPatch(unsigned long *Length);
const unsigned char *sendDriverPatch(unsigned long *Length);
const unsigned char *sendBootLoaderPatch(unsigned long *Length);
void StartSmartConfig(void);
unsigned long CC3000_openSocket(unsigned char ip1, unsigned char ip2,
		unsigned char ip3, unsigned char ip4, uint16_t port, _Bool isTCP,
		_Bool isServer, _Bool bindRequired, _Bool shouldConnect, void (*read_cb)(char* rx, uint16_t rx_len,
				sockaddr* from, uint16_t socket_Pos),
		long (*write_cb)(char* tx, uint16_t tx_len,
				 uint16_t socket_Position), void (*except_cb)(uint16_t pos));
unsigned long CC3000_openSocketul(unsigned long ip_l, uint16_t port,
		_Bool isTCP, _Bool isServer, _Bool bindRequired, _Bool shouldConnect,
		void (*read_cb)(char* rx, uint16_t rx_len,
				sockaddr* from, uint16_t socket_Pos), long (*write_cb)(char* tx, uint16_t tx_len,
						 uint16_t socket_Position),
		void (*except_cb)(uint16_t pos));
unsigned long CC3000_openClientSocket(sockaddr* client, uint16_t pos,
		void (*read_cb)(char* rx, uint16_t rx_len, sockaddr* from,
				uint16_t socket_Pos),
		long (*write_cb)(char* tx, uint16_t tx_len, uint16_t socket_Position),
		void (*except_cb)(uint16_t pos));
unsigned long CC3000_closeSocket(uint16_t socketNr);

//===== Ausgabe Funktionen
void print_WLANprofiles(wlan_profile_t profiles[]);
void CC3000_printIPconfig(void);
unsigned long CC3000_ScanNetworks(network_t nets[]);
unsigned long createNVMEMentries(void);
void eraseArray(uint16_t* array);




long CC3000_select(void);

void stream_uint16(unsigned char * from, uint16_t offset, uint16_t* to);
void stream_uint32(unsigned char * from, uint16_t offset, uint32_t* to);
void printRAW_Packet(char * rx, uint16_t readresult);

#endif
