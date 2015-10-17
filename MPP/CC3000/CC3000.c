/*
 * Copyright (C) 2014 Markus Hoffmann <mackone@inf.fu-berlin.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. This LICENSE in it's most up to date version can be
 * found under: https://www.gnu.org/licenses/lgpl-2.1.html
 */


#include "CC3000.h"

//========================================================================
// Variablen global
//========================================================================

//========================================================================
// Zustandsvariablen für die externe Verwendung
//========================================================================
bool CC3000_DHCP_done = false; //				 DHCP INfo bekommen -> Senden/Empfangen jetzt möglich
bool CC3000_is_Connected = false; //			 Ist CC3000 verbunden ACHTUNG: erst Bei DHCP Done bereit zum Senden/Empfangen
bool CC3000_is_ON = false; //					 Ist der CC3000 eingeschaltet
bool CC3000_SmartConfig_active = false; //		 Ist SmartConfig gerade aktiviert
bool CC3000_SmartConfig_finished = false; //	 Zeigt an ob die SmartConfig Daten empfangen wurden.
bool CC3000_SmartConfig_stop = false; //		 Soll der SmartConfig Prozess abgebrochen werden
unsigned long CC3000_NetScanLength = 1000; // Zeit in ms die das Netz nach Access Points durchsucht werden soll.
unsigned long CC3000_SOCKET_WAIT_DISCONNECT = 0; // Falls eine BSD Socket auf Trennung wartet ist der Wert die Socketnummer
int check = 0;

profile_t profile;
ipConfig_t ipConfig = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0,
		0, 0 } };
// SmartConfig Key (smartconfigAES16)
const unsigned char smartconfigkey[] = { 0x73, 0x6d, 0x61, 0x72, 0x74, 0x63,
		0x6f, 0x6e, 0x66, 0x69, 0x67, 0x41, 0x45, 0x53, 0x31, 0x36 };
char buffer_out[1024]; // Text Ausgabe Puffer

//========================================================================
// Zugangsdaten
//========================================================================
char* ssid = "MPP_IoT";
char* wLANkey = "Qa1Sw2Er!";

//========================================================================
// Select Infos
//========================================================================
cc3000_fd_set readds;
cc3000_fd_set writeds;
cc3000_fd_set exceptsds;
timeval timeout;
ipInfo_t senderIP; //	Info des Senders (IP und port)

sockaddr sockets[CC3000_MAX_SOCK_COUNT]; //	Array mit allen Socket
sockaddr from[CC3000_MAX_SOCK_COUNT];
char rx_buf[CC3000_MAX_SOCK_COUNT][2048]; // 	Array der Empfangspuffer
char tx_buf[CC3000_MAX_SOCK_COUNT][2048]; // 	Array der Empfangspuffer

unsigned long socket_Handles[CC3000_MAX_SOCK_COUNT]; //	Array mit den Socket Handes

uint16_t socket_Type[CC3000_MAX_SOCK_COUNT]; //	Speicher den socket Typ (UDP, TCP, RAW...)

void (*read_cbs[CC3000_MAX_SOCK_COUNT])(char* rx, uint16_t rx_len,
		sockaddr* from, uint16_t socket_Pos); // 	Array mit den Funktionen für select->read

long (*write_cbs[CC3000_MAX_SOCK_COUNT])(char* tx, uint16_t tx_len,
		uint16_t socket_Position); // 	Array mit den Funktionen für select->write

void (*except_cbs[CC3000_MAX_SOCK_COUNT])(uint16_t socket_Pos); // 	Array mit den Funktionen für select->exception

sockaddr my_IP;

unsigned long handle_mDNS = 99; // 	SocketHandler #1
//sockaddr S1_mDNS; // 				Broadcast Socket on 224.0.0.251:5353

unsigned long handle_HTTP = 99; // 	SocketHandler #2
//sockaddr S2_TCP; // 				TCP Socket for listening on Port 80

unsigned long handle_NTP = 99; // 	SocketHandler #3 NTP
//sockaddr S3_NTP; // 				Für NTP Abfrage
_Bool S3_WaitForAnswer = false; // 	Wird auf eine Anfrage gewartet

unsigned long handle_Protokoll = 99; // 	Socket Handler #4
//sockaddr S4_unused; // 				TCP Socket for ServerConnection on Port 1075
_Bool S4_WaitForAnswer = false;

unsigned long handle_NetBIOS = 99; // 	SocketHandler #2
//sockaddr S_NetBIOS; // 				TCP Socket for listening on Port 80

unsigned long handle_HTTP_Client = 99; // Handle für Clients die sich mit dem Modul verbinden
sockaddr S_ConnectedClient;
socklen_t s_ConnectedLen = sizeof(sockaddr);
_Bool S_ClientConnected = false;

unsigned long handle_DB = 99; // Handle für die Datenbank Verbindung

//========================================================================
// mDNS
//========================================================================

//========================================================================
// TCP Client
//========================================================================
long tcp_socket; // file descriptor
long udp_socket; // udp socket handle
signed char flag = 0; // Return Wert Funktion

char tx_daten[2048];
unsigned char rx_daten[2048];

sockaddr tSocketAddr; // Structure Socketadresse
unsigned int port = 1075; // IoT_Port

long frx = 0;
long trx = 0;
unsigned long smtpServerIP = 0;

//========================================================================
// UDP Socket
//========================================================================

char cc3000_out[1224]; // Text Ausgabe Puffer

unsigned char strScanresults[50];
long wlanStatus = 0;
unsigned long zu = 1;

uint32_t intervale[16] = { 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000,
		2000, 2000, 2000, 2000, 2000, 2000, 2000 };

wlan_profile_t apProfiles[WLAN_PROFILE_COUNT]; // Shadow of saved Profiles
network_t visibleNetworks[NETWORKS_ARRAY_MAX]; // Sichtbare Netzwerke


void CC3000_Init(void) {
	uint16_t i = 0;
//	unsigned long clearip[4] = { 0, 0, 0, 0 };
	char buffi[40];
	unsigned char version[20];

	// Variablen Initialisieren
	memset(&sockets, 0, sizeof(sockets));
	for (i = 0; i < CC3000_MAX_SOCK_COUNT; i++) {
		socket_Handles[i] = 99;
		memset(tx_buf[i],0,2048);
		memset(rx_buf[i],0,2048);
		memset(&from[i],0,sizeof(sockaddr));
	}

	// Modul starten
	uart_send("WLAN an\r\n");
	wlan_init(CC3000_UsynchCallback, (tFWPatches) sendWLFWPatch,
			(tDriverPatches) sendDriverPatch,
			(tBootLoaderPatches) sendBootLoaderPatch, ReadWlanInterruptPin,
			WlanInterruptEnable, WlanInterruptDisable, WriteWlanPin);
	wlan_start(0);
	uart_send("\r\n=====WLAN-Version============================");
	nvmem_read_sp_version(version);
	sprintf(buffi, "\r\n==== ID %d BUILD %d\r\n", version[0], version[1]);
	uart_send(buffi);
	uart_send("=============================================\r\n");

	// Modul patchen wenn nötig
	if (version[1] < 26) {
		wlan_stop();
		patch();
		wait_uSek(5000000);
	}
	wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE);
	wlan_ioctl_set_connection_policy(0, 0, 0);
	CC3000_Reset();
}

void CC3000_Reset(void) {
	if (CC3000_is_ON) {
		uart_send("WLAN Reset\r\n");
		wlan_stop();
		wlan_start(0);
	} else {
		wlan_start(0);
	}
	wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE);
}

void CC3000_UsynchCallback(long lEventType, char *pcData,
		unsigned char ucLength) {
	switch (lEventType) {
	case HCI_EVNT_WLAN_BASE:
		uart_send("HCI_EVNT_WLAN_BASE\r\n");
		break;
	case HCI_EVNT_WLAN_CONNECT:
		uart_send("HCI_EVNT_WLAN_CONNECT\r\n");
		break;
	case HCI_EVNT_WLAN_DISCONNECT:
		uart_send("HCI_EVNT_WLAN_DISCONNECT\r\n");
		break;
	case HCI_EVNT_WLAN_IOCTL_ADD_PROFILE:
		uart_send("HCI_EVNT_WLAN_IOCTL_ADD_PROFILE\r\n");
		break;
	case HCI_EVNT_SOCKET:
		uart_send("HCI_EVNT_SOCKET\r\n");
		break;
	case HCI_EVNT_BIND:
		uart_send("HCI_EVNT_BIND\r\n");
		break;
	case HCI_EVNT_RECV:
		uart_send("HCI_EVNT_RECV\r\n");
		break;
	case HCI_EVNT_ACCEPT:
		uart_send("HCI_EVNT_ACCEPT\r\n");
		break;
	case HCI_EVNT_LISTEN:
		uart_send("HCI_EVNT_LISTEN\r\n");
		break;
	case HCI_EVNT_CONNECT:
		uart_send("HCI_EVNT_CONNECT\r\n");
		break;
	case HCI_EVNT_SELECT:
		uart_send("HCI_EVNT_SELECT\r\n");
		break;
	case HCI_EVNT_CLOSE_SOCKET:
		uart_send("HCI_EVNT_CLOSE_SOCKET\r\n");
		break;
//	case HCI_CMND_BIND:
//		//TODO
//		uart_send("HCI_EVNT_CLOSE_SOCKET\r\n");
//		break;
	case HCI_EVNT_RECVFROM:
		uart_send("HCI_EVNT_RECVFROM\r\n");
		break;
	case HCI_EVNT_SETSOCKOPT:
		uart_send("HCI_EVNT_SETSOCKOPT\r\n");
		break;
	case HCI_EVNT_GETSOCKOPT:
		uart_send("HCI_EVNT_GETSOCKOPT\r\n");
		break;
	case HCI_EVNT_BSD_GETHOSTBYNAME:
		uart_send("HCI_EVNT_BSD_GETHOSTBYNAME\r\n");
		break;
	case HCI_EVNT_SEND:
		uart_send("HCI_EVNT_SEND\r\n");
		break;
	case HCI_EVNT_WRITE:
		uart_send("HCI_EVNT_WRITE\r\n");
		break;
	case HCI_EVNT_SENDTO:
		uart_send("HCI_EVNT_SENDTO\r\n");
		break;
	case HCI_EVNT_PATCHES_REQ:
		uart_send("HCI_EVNT_PATCHES_REQ\r\n");
		break;
	case HCI_EVNT_UNSOL_BASE:
		uart_send("HCI_EVNT_UNSOL_BASE\r\n");
		break;
	case HCI_EVNT_WLAN_UNSOL_BASE:
		uart_send("HCI_EVNT_WLAN_UNSOL_BASE\r\n");
		break;
	case HCI_EVNT_DATA_UNSOL_FREE_BUFF:
		uart_send("HCI_EVNT_DATA_UNSOL_FREE_BUFF\r\n");
		break;
	case HCI_EVNT_NVMEM_CREATE_ENTRY:
		uart_send("HCI_EVNT_NVMEM_CREATE_ENTRY\r\n");
		break;
	case HCI_EVNT_NVMEM_SWAP_ENTRY:
		uart_send("HCI_EVNT_NVMEM_SWAP_ENTRY\r\n");
		break;
	case HCI_EVNT_NVMEM_READ:
		uart_send("HCI_EVNT_NVMEM_READ\r\n");
		break;
	case HCI_EVNT_NVMEM_WRITE:
		uart_send("HCI_EVNT_NVMEM_WRITE\r\n");
		break;
	case HCI_EVNT_READ_SP_VERSION:
		uart_send("HCI_EVNT_READ_SP_VERSION\r\n");
		break;
	case HCI_EVNT_INPROGRESS:
		uart_send("HCI_EVNT_INPROGRESS\r\n");
		break;
	case HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE:
		uart_send("HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE\r\n");
		CC3000_SmartConfig_finished = true;
		break;
	case HCI_EVNT_WLAN_UNSOL_CONNECT:
		uart_send("HCI_EVNT_WLAN_UNSOL_CONNECT\r\n");
		LED_GR_ON;
//		check = 1;
		CC3000_is_Connected = true;
		break;
	case HCI_EVNT_WLAN_UNSOL_DISCONNECT:
		uart_send("HCI_EVNT_WLAN_UNSOL_DISCONNECT\r\n");
		LED_GR_OFF;
		LED_RT_ON;
		CC3000_is_Connected = false;
		CC3000_DHCP_done = false;
		memset(&ipConfig, 0, sizeof(ipConfig));
		break;
	case HCI_EVNT_WLAN_UNSOL_DHCP:
		uart_send("HCI_EVNT_WLAN_UNSOL_DHCP\r\n");

		ipConfig.cIP[0] = pcData[3];
		ipConfig.cIP[1] = pcData[2];
		ipConfig.cIP[2] = pcData[1];
		ipConfig.cIP[3] = pcData[0];
		ipConfig.cSubnet[0] = pcData[7];
		ipConfig.cSubnet[1] = pcData[6];
		ipConfig.cSubnet[2] = pcData[5];
		ipConfig.cSubnet[3] = pcData[4];
		ipConfig.cGateway[0] = pcData[11];
		ipConfig.cGateway[1] = pcData[10];
		ipConfig.cGateway[2] = pcData[9];
		ipConfig.cGateway[3] = pcData[8];
		ipConfig.cDHCP[0] = pcData[15];
		ipConfig.cDHCP[1] = pcData[14];
		ipConfig.cDHCP[2] = pcData[13];
		ipConfig.cDHCP[3] = pcData[12];
		ipConfig.cDNS[0] = pcData[19];
		ipConfig.cDNS[1] = pcData[18];
		ipConfig.cDNS[2] = pcData[17];
		ipConfig.cDNS[3] = pcData[16];
		CC3000_printIPconfig();
		if (ipConfig.cIP[0] != 0) {
			// verbunden und IP erhalten
			CC3000_DHCP_done = 1;
//			mdns_State = PROBEING;
//			iot_dev.bStateChanged = true;
//			iot_dev.uiState = IOT_STATE_mDNS_PROBING;
//			iot_dev.bWLANreadyToSend = true;
		} else {
//			// nicht verbunden IP = 0.0.0.0
			CC3000_DHCP_done = false;
//			mdns_State = INAKTIVE;
//			iot_dev.bStateChanged = true;
//			iot_dev.uiState = IOT_STATE_UNCONNECTED;
//			iot_dev.bWLANreadyToSend = false;
		}
		break;
	case HCI_EVNT_WLAN_UNSOL_INIT:
		uart_send("HCI_EVNT_WLAN_UNSOL_INIT\r\n");
		break;
	case HCI_EVNT_WLAN_TX_COMPLETE:
		uart_send("HCI_EVNT_WLAN_TX_COMPLETE\r\n");
		break;
	case HCI_EVNT_WLAN_ASYNC_PING_REPORT:
		uart_send("HCI_EVNT_WLAN_ASYNC_PING_REPORT\r\n");

		break;
	case HCI_EVNT_WLAN_KEEPALIVE:
		uart_send("HCI_EVNT_WLAN_KEEPALIVE\r\n");
		break;
	case HCI_EVENT_CC3000_CAN_SHUT_DOWN:
//		uart_send("HCI_EVENT_CC3000_CAN_SHUT_DOWN\r\n");
		//iot_dev.bWLANreadyToSend = true;
		break;
	case HCI_EVNT_BSD_TCP_CLOSE_WAIT:
		uart_send("HCI_EVNT_BSD_TCP_CLOSE_WAIT\r\n");
		CC3000_SOCKET_WAIT_DISCONNECT = (unsigned long) *pcData;
		handle_HTTP_Client = 99;
		break;
	default: {
		char huhu[255];
		sprintf(huhu, "unknown event (%i)\r\n", (int) lEventType);
		uart_send(huhu);
	}
	}
}

const unsigned char *sendWLFWPatch(unsigned long *Length) {
	*Length = 0;
	return NULL;
}

const unsigned char *sendDriverPatch(unsigned long *Length) {
	*Length = 0;
	return NULL;
}

const unsigned char *sendBootLoaderPatch(unsigned long *Length) {
	*Length = 0;
	return NULL;
}

void eraseArray(uint16_t* array) {
	uint8_t length = sizeof(array);
	uint8_t i = 0;
	for (i = 0; i < length; i++) {
		array[i] = 0;
	}
}

//*****************************************************************************
//!  CC3000_ScanNetworks(network_t nets[])
//!
//!  @return    -1 on error else Number of visible Networks
//!             Networks are saved in parameter nets
//!
//!  @brief     Saves all visible Networks inside of the provided Parameter nets
//! 			Total Amount of Networks are NETWORKS_ARRAY_MAX
//*****************************************************************************
unsigned long CC3000_ScanNetworks(network_t nets[]) {
	char buffer[100];
	unsigned int position = 0;
	scanresults* pScanRes;

	wlanStatus = wlan_ioctl_set_scan_params(1, // Scan intervall im mSek 2600 200 pro Kanal 13 Kanäle
			100, // min Verweilzeit in mSek
			100, // max Verweilzeit in mSek
			5, // Anzahl Proben
			0x1ff, // Channelmask bitweise aufwärts bis 13
			-80, // RSSI Threshold
			0, // NRS Threshold
			205, // Tx Power
			intervale // pointer Array 16 Kanäle Timeout in mSek
			);

	if (!wlanStatus) {
		uart_send("WLAN SCAN START\r\n");
	} else {
		uart_send("WLAN SCAN !!!ERROR!!!\r\n");
	}
	wait_uSek(CC3000_NetScanLength * 1000);
	wlanStatus = wlan_ioctl_set_scan_params(0,
			100, // min Verweilzeit in mSek
			100, // max Verweilzeit in mSek
			5, // Anzahl Proben
			0x1ff, // Channelmask bitweise aufwärts bis 13
			-80, // RSSI Threshold
			0, // NRS Threshold
			205, // Tx Power
			intervale // pointer Array 16 Kanäle Timeout in mSek
			);
	if (!wlanStatus) {
		uart_send("WLAN SCAN STOP\r\n");
	} else {
		uart_send("WLAN SCAN !!!ERROR!!!\r\n");
	}
	do {
		wlan_ioctl_get_scan_results(1, strScanresults);

		pScanRes = (scanresults*) strScanresults;

		if ((pScanRes->lNetScanStatus == 1) & (pScanRes->lNumNetworks > 0)) {
			nets[position].bValid = pScanRes->bValid;
			strcpy(nets[position].strSSID, pScanRes->strSSID);
			nets[position].uiRSSI = pScanRes->sRSSI;
			nets[position].uiSecMode = pScanRes->sSecMode;
			position++;
			sprintf(buffer, "Nr: %2i RSSI: %4i SSID: %s SecMode %1i\r\n",
					position, pScanRes->sRSSI, pScanRes->strSSID,
					pScanRes->sSecMode);
			uart_send(buffer);
		}
		if (pScanRes->lNumNetworks == 0) {
			pScanRes->lNumNetworks = -1;
		}

	} while (pScanRes->lNumNetworks > 0);
	return position;
}

unsigned long CC3000_openSocketul(unsigned long ip_l, uint16_t port,
		_Bool isTCP, _Bool isServer, _Bool bindRequired, _Bool shouldConnect,
		void (*read_cb)(char* rx, uint16_t rx_len, sockaddr* from,
				uint16_t socket_Pos),
		long (*write_cb)(char* tx, uint16_t tx_len, uint16_t socket_Position),
		void (*except_cb)(uint16_t pos)) {

	unsigned char ip1 = (ip_l & 0xFF000000) >> 24; // 	First octet of destination IP
	unsigned char ip2 = (ip_l & 0x00FF0000) >> 16; // 	Second Octet of destination IP
	unsigned char ip3 = (ip_l & 0x0000FF00) >> 8; // 	Third Octet of destination IP
	unsigned char ip4 = (ip_l & 0x000000FF); // 			Fourth Octet of destination IP
	return CC3000_openSocket(ip1, ip2, ip3, ip4, port, isTCP, isServer,
			bindRequired, shouldConnect, read_cb, write_cb, except_cb);
}
unsigned long CC3000_openClientSocket(sockaddr* client, uint16_t pos,
		void (*read_cb)(char* rx, uint16_t rx_len, sockaddr* from,
				uint16_t socket_Pos),
		long (*write_cb)(char* tx, uint16_t tx_len, uint16_t socket_Position),
		void (*except_cb)(uint16_t pos)) {
	// Verwaltungsarrays aktualisieren
	socket_Handles[pos] = pos;
	socket_Type[pos] = IPPROTO_TCP;
	sockets[pos] = *client;
	// Callbacks anmelden
	read_cbs[pos] = read_cb;
	write_cbs[pos] = write_cb;
	except_cbs[pos] = except_cb;
	return pos;
}

unsigned long CC3000_openSocket(unsigned char ip1, unsigned char ip2,
		unsigned char ip3, unsigned char ip4, uint16_t port, _Bool isTCP,
		_Bool isServer, _Bool bindRequired, _Bool shouldConnect,
		void (*read_cb)(char* rx, uint16_t rx_len, sockaddr* from,
				uint16_t socket_Pos),
		long (*write_cb)(char* tx, uint16_t tx_len, uint16_t socket_Position),
		void (*except_cb)(uint16_t pos)) {
	long res = 0;
	sockaddr s;
	int handle = 99;
	char feedback[128];
	memset(&s, 0, sizeof(sockaddr));

	// Handle besorgen
	if (isTCP == true) {
		handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	} else {
		handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}
	if (handle < 0) {
		sprintf(feedback, "=> Fehler: Socket konnte nicht erstellt werden\r\n");
		uart_send(feedback);
		return 99;
	} else {
		sprintf(feedback, "=> Socket (%d) erstellt \r\n", handle);
		uart_send(feedback);
	}
	// Socket Verwaltung aktualisieren
	socket_Handles[handle] = handle;
	if (isTCP == true) {
		socket_Type[handle] = IPPROTO_TCP;
	} else {
		socket_Type[handle] = IPPROTO_UDP;
	}
	// Socket Typ
	sockets[handle].sa_family = AF_INET;
	// Socket port
	sockets[handle].sa_data[0] = (port & 0xFF00) >> 8;
	sockets[handle].sa_data[1] = (port & 0x00FF);
	sockets[handle].sa_data[2] = ip1;
	sockets[handle].sa_data[3] = ip2;
	sockets[handle].sa_data[4] = ip3;
	sockets[handle].sa_data[5] = ip4;

	// Callbacks anmelden
	read_cbs[handle] = read_cb;
	write_cbs[handle] = write_cb;
	except_cbs[handle] = except_cb;

	if (shouldConnect == true) {
		res = connect(handle, &sockets[handle], sizeof(sockaddr));
		if (res < 0) {
			sprintf(feedback, "Fehler: Socket(%d) nicht verbunden\r\n", handle);
			uart_send(feedback);
			return 99;
		} else {
			return handle;
		}
	}

	if (bindRequired == false) // Socket soll nicht verbunden werden
		return handle;
	if (isTCP == false) {
		sockets[handle].sa_data[2] = 0;
		sockets[handle].sa_data[3] = 0;
		sockets[handle].sa_data[4] = 0;
		sockets[handle].sa_data[5] = 0;
	}
	res = bind(handle, &sockets[handle], sizeof(sockaddr));
	if (res < 0) { //131.188.3.220
		sprintf(feedback, "Fehler: Socket(%d) nicht gebunden\r\n", handle);
		uart_send(feedback);
		return 99;
	} else {
		sprintf(feedback, "==> Socket (%d) gebunden an IP:%d.%d.%d.%d : %d\r\n",
				handle, ip1, ip2, ip3, ip4, port);
		uart_send(feedback);
	}
	memset(&sockets[handle].sa_data[2], 0, 12);
	if (isServer == true) {
		res = listen(handle, 0);
		if (res < 0) {
			sprintf(feedback,
					"Fehler: Auf Socket (%d) kann nicht gehört werden\r\n",
					handle);
			uart_send(feedback);
			return 99;
		} else {
			sprintf(feedback,
					"==> Socket (%d) bereit zum akzeptieren auf Port %d\r\n",
					handle, port);
			uart_send(feedback);
		}
	}
	sockets[handle].sa_data[2] = ip1;
	sockets[handle].sa_data[3] = ip2;
	sockets[handle].sa_data[4] = ip3;
	sockets[handle].sa_data[5] = ip4;

	return handle;
}
//*****************************************************************************
//
//! CC3000_closeSocket
//!
//!  @param  	socketNr - Position in den Verwaltungs Arrays
//!
//!  @return 	99 - zum markieren der SocketVariablen als unverbunden
//!
//!  @brief  	Schließt die gewünschte Socket und aktualisiert
//!				alle Verwaltungs-Arrays
//
//*****************************************************************************
unsigned long CC3000_closeSocket(uint16_t socketNr) {
	long res = 0;
	if (socketNr != 99) {
		res = closesocket(socketNr);
		if (res == 0) {
			// socket konnte geschlossen werden => Aktualisierung der Verwaltungs-Arrays
			socket_Handles[socketNr] = 99;
			memset(&sockets[socketNr], 0, sizeof(sockaddr));
			socket_Type[socketNr] = 0;
			read_cbs[socketNr] = 0;
			write_cbs[socketNr] = 0;
			except_cbs[socketNr] = 0;
		}
	}
	return 99;
}

void CC3000_printIPconfig(void) {
	sprintf(buffer_out, "IP:      %d.%d.%d.%d\r\n", ipConfig.cIP[0],
			ipConfig.cIP[1], ipConfig.cIP[2], ipConfig.cIP[3]);
	uart_send(buffer_out);
	sprintf(buffer_out, "Subnetz: %d.%d.%d.%d\r\n", ipConfig.cSubnet[0],
			ipConfig.cSubnet[1], ipConfig.cSubnet[2], ipConfig.cSubnet[3]);
	uart_send(buffer_out);
	sprintf(buffer_out, "Gateway: %d.%d.%d.%d\r\n", ipConfig.cGateway[0],
			ipConfig.cGateway[1], ipConfig.cGateway[2], ipConfig.cGateway[3]);
	uart_send(buffer_out);
	sprintf(buffer_out, "DHCP:    %d.%d.%d.%d\r\n", ipConfig.cDHCP[0],
			ipConfig.cDHCP[1], ipConfig.cDHCP[2], ipConfig.cDHCP[3]);
	uart_send(buffer_out);
	sprintf(buffer_out, "DNS:     %d.%d.%d.%d\r\n", ipConfig.cDNS[0],
			ipConfig.cDNS[1], ipConfig.cDNS[2], ipConfig.cDNS[3]);
	uart_send(buffer_out);
}

void CC3000_readProfiles(void) {
	unsigned char profiles[4096];
	long returnValue;
	unsigned long i = 0;
	for (i = 0; i < 10; i++) {
		returnValue = nvmem_read(14, 5, 0, profiles);
		if (returnValue != 0) {
			uart_send("Fehler");
		}
		uart_send((char*) profiles);
	}
}
unsigned long createNVMEMentries(void) {
	unsigned long result = 0;
	unsigned char emptyProfiles[WLAN_ALL_PROFILES_SIZE];
	memset(&emptyProfiles, 0, WLAN_ALL_PROFILES_SIZE);
	// 7 Mal (32 bytes SSID, 32 bytes Key, unsigned long SecurityType
	result = nvmem_create_entry(14, WLAN_ALL_PROFILES_SIZE);
	if (result < 0) {
		uart_send("Erstellen des User Files im NVMEM Fehler!!");
	}
	CC3000_Reset();
	result = nvmem_write(14, WLAN_ALL_PROFILES_SIZE, 0, emptyProfiles);
	if (result < 0) {
		uart_send("Initialisierung des AP Profile fehlgeschlagen");
	}
	return result;
}

//*****************************************************************************
//!  deleteAll_NVMEM_Profiles(void)
//!
//!  @return    -1 on error else 0
//!
//!  @brief      Deletes a Profile in NVMEM at specified position
//*****************************************************************************
signed long deleteAll_NVMEM_Profiles(void) {
	signed long res = 0;
	unsigned int i = 0;
	for (i = 0; i < WLAN_PROFILE_COUNT; i++) {
		delete_NVMEM_Profile(i);
		if (res < 0) {
			uart_send(
					"ERROR: während des Schreibens ins NVMEM ist ein Fehler aufgetreten");
			res = 0;
		}
	}
	return res;
}

//*****************************************************************************
//!  delete_NVMEM_Profile(unsigned int position)
//!
//!  @return    -1 on error else 0
//!
//!  @brief      Deletes a Profile in NVMEM at specified position
//*****************************************************************************
signed long delete_NVMEM_Profile(unsigned int position) {
	signed long res = 0;
	wlan_profile_t profile;
	memset(&profile, 0, sizeof(wlan_profile_t));
	res = write_NVMEM_Profiles(position, 32, profile.strSSID, 32,
			(char*) profile.strKey, 0);
	if (res < 0)
		uart_send(
				"ERROR: während des Schreibens ins NVMEM ist ein Fehler aufgetreten");
	return res;
}

//*****************************************************************************
//!  write_NVMEM_Profiles(wlan_profile_t* profiles)
//!
//!  @return    -1 on error else Number of saved profiles
//!
//!  @brief      Saves all Saved Profiles in Array and returns the count of saved profiles in NVMEM
//*****************************************************************************
signed long write_NVMEM_Profiles(unsigned int position, unsigned int ssidLen,
		char* SSID, unsigned int keyLen, char* Key, unsigned long SecurityType) {
	signed long result = 0;
	wlan_profile_t profile;
	char buffer[140];

	memset(&profile, 0, sizeof(wlan_profile_t));
	strlcpy(profile.strSSID, SSID, ssidLen + 1);
	//strcpy(profile.strSSID, SSID);
	profile.ulSSIDlen = ssidLen;
	strlcpy((char*) profile.strKey, Key, keyLen + 1);
	profile.ulKeyLen = keyLen;
	profile.ulSecType = SecurityType;

	sprintf(buffer, "write Profile to NVMEM at Pos:%i \r\n", position);
	uart_send(buffer);
	sprintf(buffer, "SSID: %-32s Key: %-32s\r\n", profile.strSSID,
			profile.strKey);
	uart_send(buffer);
	unsigned long offset = position * WLAN_PROFILE_SIZE;

	result = nvmem_write(14, WLAN_PROFILE_SIZE, offset,
			(unsigned char *) &profile);
	if (result != 0) {
		uart_send("Error writing profiles to nvmem\r\n");
	}
	//iot_dev.uiProfileCount++;
	return result;

}

//*****************************************************************************
//!  read_NVMEM_Profiles(wlan_profile_t* profiles)
//!
//!  @return    -1 on error else Number of saved profiles
//!
//!  @brief      Saves all Saved Profiles in Array and returns the count of saved profiles in NVMEM
//*****************************************************************************
signed long read_NVMEM_Profiles(wlan_profile_t profiles[]) {
	signed long result = 0;
	unsigned int i = 0;
	uart_send("NVMEM read\r\n");
	result = nvmem_read(14, WLAN_ALL_PROFILES_SIZE, 0,
			(unsigned char *) profiles);
	if (result != WLAN_ALL_PROFILES_SIZE && result != 0) {
		createNVMEMentries();
		result = nvmem_read(14, WLAN_ALL_PROFILES_SIZE, 0,
				(unsigned char *) profiles);
	}
	if (result < 0) {
		uart_send("Error reading profiles from NVMEM\r\n");
		uart_send("Try creating Profile Space\r\n");
		result = 0;
		result = createNVMEMentries();
		if (result == 0) {
			result = nvmem_read(14, WLAN_ALL_PROFILES_SIZE, 0,
					(unsigned char *) profiles);
		}
	}
	result = 0;
	for (i = 0; i < WLAN_PROFILE_COUNT; i++) {
		wlan_profile_t* prof = &profiles[i];
		if ((strcmp(prof->strSSID, ""))) {
			result++;
			uart_send(prof->strSSID);
			uart_send("\r\n");
		}
	}
	return result;
}

//*****************************************************************************
//
//! StartSmartConfig
//!
//!  @param  None
//!
//!  @return none
//!
//!  @brief  The function triggers a smart configuration process on CC3000.
//!			it exists upon completion of the process
//
//*****************************************************************************
void StartSmartConfig(void) {
	volatile unsigned long ulCC3000Connected; // ulCC3000DHCP, OkToDoShutDown;
//			, ulCC3000DHCP_configured;
//volatile unsigned char ucStopSmartConfig;
	const char aucCC3000_prefix[] = { 'T', 'T', 'T' };

//bSmartConfigFinished = 0;
	ulCC3000Connected = 0;
//	ulCC3000DHCP = 0;
//	OkToDoShutDown = 0;
	CC3000_SmartConfig_active = true; // Signalisiert der IOT App dass SmartConfig läuft
	uart_send("=> Start SmatConfig\r\n");
// Reset all the previous configuration
	wlan_ioctl_set_connection_policy(DISABLE, DISABLE, DISABLE);
	wlan_ioctl_del_profile(255);

//Wait until CC3000 is disconnected
	while (ulCC3000Connected == 1) {
		wait_uSek(100);
		hci_unsolicited_event_handler();
	}

// Start blinking LED1 during Smart Configuration process
	LED_RT_ON;
	wlan_smart_config_set_prefix((char*) aucCC3000_prefix);
	LED_RT_OFF;

// Start the SmartConfig start process
	wlan_smart_config_start(1);
	LED_RT_ON;

// Wait for Smart config to finish
	while (CC3000_SmartConfig_finished == false) {
		LED_RT_TOGGLE;
		wait_uSek(200000);
		if (CC3000_SmartConfig_stop == true) {
			CC3000_SmartConfig_stop = false;
			uart_send("SmartConfig abgebrochen durch Benutzer");
			return;
		}
	}
	LED_RT_OFF;
// create new entry for AES encryption key
	nvmem_create_entry(NVMEM_AES128_KEY_FILEID, 16);

// write AES key to NVMEM
	aes_write_key((unsigned char *) (&smartconfigkey[0]));

// Decrypt configuration information and add profile
	wlan_smart_config_process();

// Configure to connect automatically to the AP retrieved in the
// Smart config process
//	wlan_ioctl_set_connection_policy(DISABLE, DISABLE, ENABLE);
	wlan_ioctl_set_connection_policy(DISABLE, DISABLE, DISABLE);

// reset the CC3000
	wlan_stop();

	uart_send("SmartConfig DONE\n\r");
	wait_uSek(100000);
	wlan_start(0);

// Mask out all non-required events
	wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE); //| HCI_EVNT_WLAN_UNSOL_INIT
	CC3000_SmartConfig_active = false;
}

void print_WLANprofiles(wlan_profile_t * profiles) {
	char output[256];
	char secType[30];
	unsigned int i = 0;

	for (i = 0; i < WLAN_PROFILE_COUNT; i++) {
		wlan_profile_t* prof = &profiles[i];
		switch (prof->ulSecType) {
		case 0:
			strcpy(secType, "WLAN OPEN");
			break;
		case 1:
			strcpy(secType, "WLAN WEP");
			break;
		case 2:
			strcpy(secType, "WLAN WPA");
			break;
		case 3:
			strcpy(secType, "WLAN WPA2");
			break;
		}
		if ((strcmp(prof->strSSID, ""))) {
			sprintf(output, "PROFIL %i ==========================\r\n",
					(i + 1));
			uart_send(output);
			sprintf(output,
					"         SSID: %-32s \n         Key:  %-32s \r\n         Security: %-10s\r\n\r\n",
					prof->strSSID, prof->strKey, secType);
			uart_send(output);
		}
	}
	uart_send("===================================\r\n\r\n");
}



long CC3000_select(void) {
	uint16_t i;
	long result = 0; // Fehler des Select beheben!!!!!
	long readresult = 0;
	long sendresult = 0;
	long send_length = 0;
	unsigned long maxSocket = 0; // Ermittelt die größte, benutzte Socket für das select()
	timeout.tv_sec = 0;
	timeout.tv_usec = 500;

	socklen_t fromlen = sizeof(sockaddr);
	;
	//char out[128];

	// Init File Descriptoren
	FD_ZERO_CC3000(&readds);
	FD_ZERO_CC3000(&writeds);
	FD_ZERO_CC3000(&exceptsds);

	// File Descriptoren-Anmeldung wenn socket geöffnet
	for (i = 0; i < CC3000_MAX_SOCK_COUNT; i++) {
		if (socket_Handles[i] != 99) {
			FD_SET_CC3000(socket_Handles[i], &readds);
			FD_SET_CC3000(socket_Handles[i], &writeds);
			FD_SET_CC3000(socket_Handles[i], &exceptsds);
			if (socket_Handles[i] > maxSocket)
				maxSocket = socket_Handles[i];
		}
	}
	// DAS API Select
	result = select(maxSocket + 1, &readds, &writeds, &exceptsds, &timeout);
	//=== SOCKET Handling ==================================================
	for (i = 0; i < CC3000_MAX_SOCK_COUNT; i++) {
		if (socket_Handles[i] != 99) { // Wenn Socket geöffnet
			if (FD_ISSET_CC3000(socket_Handles[i],&exceptsds)) {
				//  *exceptsds - return the sockets which closed recently.
				if (except_cbs[i] != 0) { // 			Wenn Callback angemeldet
					(*except_cbs[i])(i); // 	Verarbeitung durch Callback Funktion
				}
			}
			if (__FD_ISSET(socket_Handles[i],&readds)) {
				// *readsds - return the sockets on which Read request will return without delay with valid data.
				//=== RX Puffer leeren ============================================
				memset(rx_daten, 0, sizeof(rx_daten));
				//=== RX Puffer mit empfangenen Daten füllen ======================
				switch (socket_Type[i]) {
				case IPPROTO_TCP:
					readresult = recv(socket_Handles[i], &rx_daten,
							sizeof(rx_daten), 0);
					memcpy(&from[i], &sockets[i], sizeof(sockaddr));
					break;
				case IPPROTO_UDP:
					readresult = recvfrom(socket_Handles[i], &rx_daten,
							sizeof(rx_daten), 0, &from[i], &fromlen);
					break;
				default:
					uart_send(
							" FEHLER: Protokoll wird beim Empfang nicht unterstützt\r\n==> nur TCP und UDP\r\n");
					continue;
				}
				if (readresult >= 0 && readresult < sizeof(rx_daten)) {
					if (read_cbs[i] != 0) { // 			Wenn Callback angemeldet
						(*read_cbs[i])((char*) rx_daten, readresult, &from[i], i); // 	Verarbeitung durch Callback Funktion
					}
				} else {
					uart_send("Fehler beim Lesen des Pakets\r\n");
				}
			}
			if (FD_ISSET_CC3000(socket_Handles[i],&writeds)) {
				//  *writesds - return the sockets which are ready to write
				//=== Asynchrones Senden im Select ================================
				if (write_cbs[i] != 0) { // 			Wenn Callback angemeldet
					send_length = (*write_cbs[i])(tx_buf[i], 2048, i); // 	Verarbeitung durch Callback Funktion
					if (send_length > 0) {
						switch (socket_Type[i]) {
						case IPPROTO_TCP:
							sendresult = send(socket_Handles[i], &tx_buf[i],
									send_length, 0);
							break;
						case IPPROTO_UDP:
							sendresult = sendto(socket_Handles[i], &tx_buf[i],
									send_length, 0, &from[i], fromlen);
							break;
						default:
							uart_send(
									" FEHLER: Protokoll wird beim Senden nicht unterstützt\r\n==> nur TCP und UDP\r\n");
							continue;
						}
					}
					if (sendresult >= 0){
						memset(tx_buf[i],0,2048);
					}
					send_length = 0;
				}
			}
		}
	}
	return result;
}


void printRAW_Packet(char * rx, uint16_t readresult) {
	uint16_t i = 0;
	uint16_t len = (readresult / 16) + 1;
	char out[60];
	for (i = 0; i < len; i++) {
		sprintf(out,
				"%02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x\r\n",
				*(rx + (16 * i) + 0), *(rx + (16 * i) + 1),
				*(rx + (16 * i) + 2), *(rx + (16 * i) + 3),
				*(rx + (16 * i) + 4), *(rx + (16 * i) + 5),
				*(rx + (16 * i) + 6), *(rx + (16 * i) + 7),
				*(rx + (16 * i) + 8), *(rx + (16 * i) + 9),
				*(rx + (16 * i) + 10), *(rx + (16 * i) + 11),
				*(rx + (16 * i) + 12), *(rx + (16 * i) + 13),
				*(rx + (16 * i) + 14), *(rx + (16 * i) + 15));
		uart_send(out);
	}
}
