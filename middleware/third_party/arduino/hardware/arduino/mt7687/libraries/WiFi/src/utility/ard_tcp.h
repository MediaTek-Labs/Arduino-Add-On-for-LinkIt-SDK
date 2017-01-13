#ifndef _ARD_TCP_H_
#define _ARD_TCP_H_

#include "FreeRTOS.h"
#include "task.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "netif/etharp.h"
#include "lwip/tcp.h"

#define MAX_CLIENT_ACCEPTED			4
#define MAX_SOCK_NUM				4
#define TCP_MODE				0
#define UDP_MODE				1

#define TTCP_MODE_TRANSMIT			0
#define TTCP_MODE_RECEIVE			1
#define MAX_MODE_NUM				2

#define NO_VALID_ID				-1

typedef struct eRemoteClient{
	uint32_t ipaddr;
	uint16_t port;
}tRemoteClient;

typedef struct ttcp {
	ip_addr_t addr;
	uint16_t port;
	uint8_t sock;
	int mode;
	int udp;
	uint8_t buff_sent[MAX_CLIENT_ACCEPTED];
	xSemaphoreHandle *insert_xMutex;

	uint32_t buflen;
	char     *payload[MAX_CLIENT_ACCEPTED];
	uint32_t left[MAX_CLIENT_ACCEPTED];

	/* TCP specific */
	struct   tcp_pcb* tpcb[MAX_CLIENT_ACCEPTED];
	struct   tcp_pcb* lpcb;
	uint8_t  pending_close[MAX_CLIENT_ACCEPTED];

	/* UDP specific */
	struct udp_pcb* upcb;
}ttcp_t;

#define GET_TCP_MODE(X)	((X!=NULL)?((struct ttcp*)(X))->mode:0)

#define IS_VALID_SOCK(SOCK) ((SOCK>=0)&&(SOCK<MAX_SOCK_NUM))

#define Mode2Str(_Mode) ((_Mode==0)?"TRANSMIT":"RECEIVE")

#define ProtMode2Str(_protMode) ((_protMode==0)?"TCP":"UDP")

#define FREE_PAYLOAD_ID(TTCP,ID) do {		\
	if (TTCP->payload[ID]){			\
		free(TTCP->payload[ID]);	\
		TTCP->payload[ID] = NULL; }	\
}while(0)

struct tcp_pcb * getFirstClient(struct ttcp* _ttcp);
uint8_t getStateTcp(void* p, uint8_t client);
void* getTTCP(uint8_t sock, uint8_t mode);

void ard_tcp_stop(void* ttcp);

void clearMapSockTcp(uint8_t sock, uint8_t mode);
void setMapSockMode(uint8_t sock, void* _ttcp, uint8_t _tcp_mode);
void clearMapSockMode(uint8_t sock, void* _ttcp, uint8_t _tcp_mode);
void setMapSock(uint8_t sock, void* _ttcp);
void initMapSockTcp(void);
int getSock(void * _ttcp);

int8_t insertNewClientConn(struct ttcp* _ttcp, struct tcp_pcb *newpcb);
int8_t get_client_connId(struct ttcp* _ttcp, struct tcp_pcb *newpcb);
int8_t removeNewClientConn(struct ttcp* _ttcp, struct tcp_pcb *newpcb);

int8_t sendTcpData(void* p, uint8_t* buf, uint16_t len);
int8_t isDataSent(void* p);

void setRemoteClient(uint16_t sock, uint32_t _ipaddr, uint16_t _port);
tRemoteClient* getRemoteClient(uint16_t sock);
void getRemoteData(uint8_t sock, uint8_t mode, tRemoteClient* remoteData);
int8_t sendUdpData(void* ttcp, uint8_t* buf, uint16_t len);

int ard_tcp_start(ip_addr_t addr, struct ttcp* ttcp, uint16_t port, int mode, uint16_t buflen,int udp,uint8_t sock);

#endif
