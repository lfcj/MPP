/*
 * Copyright (C) 2014 Markus Hoffmann <mackone@inf.fu-berlin.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. This LICENSE in it's most up to date version can be
 * found under: https://www.gnu.org/licenses/lgpl-2.1.html
 */


#ifndef NTP_H
#define NTP_H

#include "CC3000.h"

//=========================================================================
// board_lib
//=========================================================================
#include "led.h"
#include "main.h"
#include "usart.h"
#include "rtc.h"

//=========================================================================
// standart_lib
//=========================================================================
#include "string.h"
#include "stdbool.h"
#include "stdlib.h"

typedef struct ntp_packet // Size: 48 Bytes
{
	uint8_t Mode :3; //		Mode 				3: Client, 4: Server
	uint8_t version :3; //	Version Number 		1-4 currently 4 [RFC 5905 p 19]
	uint8_t li :2; //		Leap Indicator 		0: no warning, 1: 2: 3: clock unsync
	uint8_t pr_Stratum; //	Peer Clock Stratum	16: unsynchronized
	uint8_t pr_pollInt; //	Polling Interval	6-10
	uint8_t pr_clkPrec; //	Clock Precision		250 0,015625 sec
	uint32_t rt_Delay; //	Root Delay
	uint32_t rt_disper; //	Root Dispersion		66192 = 1,01 sec
	uint32_t ref_ID; // 	Reference ID
	uint64_t ts_ref; //		Reference Timestamp
	uint64_t ts_org; // 	Original Timestamp
	uint64_t ts_rx; // 		Receive Timestamp
	uint64_t ts_tx; //		Transimit Timestamp
} ntp_packet_t;
//=== Globale Externe Variablen (Zustandsvariablen) ===========================
extern char NTP_Server_Name[];
extern ntp_packet_t ntp_pkt;
extern uint32_t last_NTP_Sync;
extern _Bool retryNTP;
extern _Bool ntp_Zeit_aktualisiert;

//=== Externe Funktionen (Schnittstelle) ==================================
void ntp_read_callback(char* rx, uint16_t rx_len, sockaddr* from,
		uint16_t socket_Position);

unsigned long get_NTP_Time(char* SNTP_Server_Name);
void init_ntp_packet(void);
void init_NTPServer(void);
void ntp2stream(ntp_packet_t* p, char * rx);
void stream2ntp(ntp_packet_t* p, char * rx);
void uint32_stream(unsigned char * to, uint16_t offset, uint32_t* from);
void stream_uint64(char * from, uint16_t offset, uint64_t* to);
#endif
