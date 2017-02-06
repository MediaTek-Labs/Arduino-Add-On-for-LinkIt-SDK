#include "utility/ard_tcp.h"
#include "utility/ard_utils.h"

#include "log_dump.h"

#define IS_BUF_AVAIL(x) (tailBuf[x] != headBuf[x])
#define IS_BUF_EMPTY(x) ((tailBuf[x] == 0) && (headBuf[x] == 0))

#define MAX_PBUF_STORED	30
tData pBufStore[MAX_PBUF_STORED][MAX_SOCK_NUM];
unsigned char headBuf[MAX_SOCK_NUM] = {0};
unsigned char tailBuf[MAX_SOCK_NUM] = {0};

udata_t UdpBuf[MAX_SOCK_NUM];

uint8_t getTcpDataByte(uint8_t sock, uint8_t* payload, uint8_t peek)
{
	// ref field in struct pbuf has been used as index pointer for byte data
	tData* p = get_pBuf(sock);

	if(p != NULL){
		if(p->idx < p->len){
			uint8_t* buf = (uint8_t*)p->data;
			if (peek)
				*payload = buf[p->idx];
			else
				*payload = buf[p->idx++];

			if (p->idx == p->len)
				ackAndFreeData(p->pcb, p->len, sock, p->data);
			return 1;
		}else{
			ackAndFreeData(p->pcb, p->len, sock, p->data);
		}
	}
	return 0;
}

uint16_t calcMergeLen(uint8_t sock)
{
	uint16_t len = 0;
	unsigned char index = tailBuf[sock];

	do {
		if(pBufStore[index][sock].data != NULL)
		{
			len += pBufStore[index][sock].len;
			len -= pBufStore[index][sock].idx;  //idx = 0
			//pr_debug(" [%d]: len:%d idx:%d tot:%d\r\n", sock, pBufStore[index][sock].len, pBufStore[index][sock].idx, len);
		}
		++index;
		if(index == MAX_PBUF_STORED)
			index = 0;
	} while(index!=headBuf[sock]);

	return len;
}

uint16_t getAvailTcpDataByte(uint8_t sock)
{
	return calcMergeLen(sock);
}

void freetData(void * buf, uint8_t sock)
{
	void *temp = buf;

	if(buf==NULL){
		pr_debug("Buf == NULL!");
		return;
	}

	pBufStore[tailBuf[sock]][sock].data = NULL;
	pBufStore[tailBuf[sock]][sock].len = 0;
	pBufStore[tailBuf[sock]][sock].idx = 0;
	pBufStore[tailBuf[sock]][sock].pcb = 0;

	if(++tailBuf[sock] == MAX_PBUF_STORED)
		tailBuf[sock] = 0;

	free(temp);
}

void freeAllTcpData(uint8_t sock)
{
	tData* p = NULL;

	do {
		p = get_pBuf(sock);
		if (p != NULL)
			freetData(p->data, sock);
	} while(p != NULL);

	pr_debug("freeAllTcpData\r\n");
}

void ackAndFreeData(void* pcb, int len, uint8_t sock, uint8_t* data)
{
	if(data != NULL)
		freetData(data, sock);
}

int freeTcpData(uint8_t sock)
{
	tData* p = NULL;

	p = get_pBuf(sock);
	if(p != NULL){
		ackAndFreeData(p->pcb, p->len, sock, p->data);
		return 0;
	}

	return -1;
}

tData* get_pBuf(uint8_t sock)
{
	if(IS_BUF_EMPTY(sock))
		return NULL;

	if(IS_BUF_AVAIL(sock)){
		tData* p = &(pBufStore[tailBuf[sock]][sock]);
		return p;
	}

	return NULL;
}

int getTcpData(uint8_t sock, void** payload, uint16_t* len)
{
	tData* p = NULL;

	p = get_pBuf(sock);
	if(p != NULL){
		*payload = p->data;
		*len = p->len;
		return 0;
	}

	return -1;
}

void freetDataIdx(uint8_t idxBuf, uint8_t sock)
{
	if(idxBuf >=MAX_PBUF_STORED){
		pr_debug("idxBuf out of range: %d\r\n", idxBuf);
		return;
	}

	void *buf = pBufStore[idxBuf][sock].data;

	//pr_debug("%p idx:%d\r\n", buf, idxBuf);

	free(buf);

	pBufStore[idxBuf][sock].data = NULL;
	pBufStore[idxBuf][sock].len = 0;
	pBufStore[idxBuf][sock].idx = 0;
	pBufStore[idxBuf][sock].pcb = 0;
}

void init_pBuf(void)
{
	pr_debug("WIFI_init   init_pBuf\r\n");
	memset(pBufStore, 0, sizeof(pBufStore));
	uint8_t i = 0;

	for (; i < MAX_SOCK_NUM; ++i) {
		UdpBuf[i].phead = NULL;
		UdpBuf[i].ptail = NULL;
	}
}

uint8_t* insert_pBuf(struct pbuf* q, uint8_t sock, void* _pcb)
{
	if (q == NULL)
		return NULL;

	if(((headBuf[sock]+1)%MAX_PBUF_STORED) == tailBuf[sock]){
		pr_debug("full\r\n");
		return NULL;
	}

	if(pBufStore[headBuf[sock]][sock].data != NULL)
	{
		pr_debug("Overwriting buffer %p idx:%d!\r\n",
			pBufStore[headBuf[sock]][sock].data, headBuf[sock]);
		freetDataIdx(headBuf[sock], sock);
	}

	uint8_t* p = (uint8_t*)malloc(q->tot_len + 1);
	pr_debug("malloc tot_len %d\r\n", q->tot_len);
	memset(p, 0, q->tot_len+1);
	if (p != NULL) {
		if (pbuf_copy_partial(q, p, q->tot_len,0) != q->tot_len) {
			pr_debug("pbuf_copy_partial failed: src:%p, dst:%p, len:%d\r\n", q, p, q->tot_len);
			free(p);
			p = NULL;
			return p;
		}
		//pr_debug("Insert[%d]: [head=%d,tail=%d]\r\n", sock, headBuf[sock], tailBuf[sock]);
		pBufStore[headBuf[sock]][sock].data = p;
		pBufStore[headBuf[sock]][sock].len = q->tot_len;
		pBufStore[headBuf[sock]][sock].idx = 0;
		pBufStore[headBuf[sock]][sock].pcb = _pcb;
		headBuf[sock]++;

		if(headBuf[sock] == MAX_PBUF_STORED)
			headBuf[sock] = 0;
		if(headBuf[sock] == tailBuf[sock])
		{
			pr_debug("Avoid to Overwrite data [%d-%d]!\r\n", headBuf[sock], tailBuf[sock]);
			if(headBuf[sock] != 0)
				--headBuf[sock];
			else
				headBuf[sock] = MAX_PBUF_STORED-1;
		}
	}

	return p;
}

int8_t insertBuf(uint8_t sock, uint8_t* buf, uint16_t len)
{
	uData *node = NULL;
	node = (uData *)malloc(sizeof(uData));
	if(node == NULL)
		return -1;

	node->len = len;
	node->next = NULL;

	uint8_t *temp = NULL;
	temp = malloc(len+1);
	if(temp == NULL){
		free(node);
		return -1;
	}
	memcpy(temp, buf, len);
	temp[len] = '\0';
	node->data = temp;

	if((UdpBuf[sock].phead == NULL) && (UdpBuf[sock].ptail == NULL)){
		UdpBuf[sock].phead = node;
		UdpBuf[sock].ptail = node;
	}else{
		UdpBuf[sock].ptail->next = node;
		UdpBuf[sock].ptail = node;
	}
	return 0;

}

uint8_t* mergeBuf(uint8_t sock, uint8_t** buf, uint16_t* _len)
{
	if(UdpBuf[sock].phead== NULL)
		return NULL;

	uData *temp = UdpBuf[sock].phead;
	uint16_t len = 0;
	while(temp != NULL){
		len += temp->len;
		temp = temp->next;
	}
	//pr_debug("mergeBuf  datalen %d\r\n", len);
	uint8_t *p = (uint8_t *)malloc(len+1);
	uint8_t *_p = p;
	if(p != NULL){
		temp = UdpBuf[sock].phead;
		while(temp != NULL){
			if(temp->data != NULL){
				memcpy(p, temp->data, temp->len);
				p += temp->len;
				temp = temp->next;
			}
		}
	}
	*p = '\0';
	if(buf != NULL)
		*buf = _p;
	if(_len != NULL)
		*_len = len;

	return _p;
}

void clearBuf(uint8_t sock)
{
	if(UdpBuf[sock].phead != NULL){
		uData *temp = UdpBuf[sock].phead;
		uData *temp1 = temp->next;

		while(temp->next != NULL){
			if(temp->data != NULL){
				free(temp->data);
				temp->data = NULL;
			}
			free(temp);
			temp = temp1;
			temp1 = temp1->next;
		}
		if(temp->data != NULL){
			free(temp->data);
			temp->data = NULL;
		}
		free(temp);
		UdpBuf[sock].phead = NULL;
		UdpBuf[sock].ptail = NULL;
	}
}
