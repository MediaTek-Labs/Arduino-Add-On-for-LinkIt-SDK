#ifndef _ARD_UTILS_H_
#define _ARD_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct udpdata{
	uint8_t		*data;
	uint16_t	len;
	struct udpdata	*next;
}uData;

typedef struct{
	uData		*phead;
	uData		*ptail;
}udata_t;

typedef struct sData
{
	uint8_t		*data;
	uint16_t	len;
	uint16_t	idx;
	void		*pcb;
}tData;

uint8_t getTcpDataByte(uint8_t sock, uint8_t* payload, uint8_t peek);
uint16_t calcMergeLen(uint8_t sock);
uint16_t getAvailTcpDataByte(uint8_t sock);

void freetData(void * buf, uint8_t sock);
void freeAllTcpData(uint8_t sock);

void ackAndFreeData(void* pcb, int len, uint8_t sock, uint8_t* data);
int freeTcpData(uint8_t sock);
tData* get_pBuf(uint8_t sock);
int getTcpData(uint8_t sock, void** payload, uint16_t* len);
void freetDataIdx(uint8_t idxBuf, uint8_t sock);
void init_pBuf(void);

uint8_t* insert_pBuf(struct pbuf* q, uint8_t sock, void* _pcb);

int8_t insertBuf(uint8_t sock, uint8_t* buf, uint16_t len);
uint8_t* mergeBuf(uint8_t sock, uint8_t** buf, uint16_t* _len);
void clearBuf(uint8_t sock);

#ifdef __cplusplus
}
#endif

#endif
