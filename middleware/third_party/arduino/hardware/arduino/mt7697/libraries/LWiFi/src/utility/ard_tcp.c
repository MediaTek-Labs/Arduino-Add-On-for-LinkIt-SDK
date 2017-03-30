#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "wifi_api.h"
#include "lwip/udp.h"
#include "lwip/inet.h"

#include "utility/ard_tcp.h"
#include "utility/ard_utils.h"
#include "utility/wl_definitions.h"

#include "log_dump.h"

void*  mapSockTCP[MAX_SOCK_NUM][MAX_MODE_NUM];
int8_t _connected[MAX_CLIENT_ACCEPTED] = {0,0,0,0};
int8_t currConnId[MAX_CLIENT_ACCEPTED] = {NO_VALID_ID, NO_VALID_ID, NO_VALID_ID, NO_VALID_ID};


void update_currConnId(struct ttcp* _ttcp, uint8_t new_id)
{
	currConnId[_ttcp->sock] = new_id;
}

int8_t get_currConnId(struct ttcp* _ttcp)
{
	return currConnId[_ttcp->sock];
}

struct tcp_pcb * getFirstClient(struct ttcp* _ttcp)
{
	if (_ttcp != NULL) {

		int idx = 0;
		for(; idx < MAX_CLIENT_ACCEPTED; ++idx)
		{
			if(_ttcp->tpcb[idx] != NULL)
				return _ttcp->tpcb[idx];
		}
	}
	return NULL;
}

uint8_t getStateTcp(void* p, uint8_t client)
{
	struct ttcp* _ttcp = (struct ttcp*) p;

	struct tcp_pcb *pcb = getFirstClient(_ttcp);

	if (pcb != NULL) {
		int8_t id = get_client_connId(_ttcp, pcb);
		if(id != NO_VALID_ID)
			update_currConnId(_ttcp, id);
	}

	if ((_ttcp != NULL) && ((pcb != NULL) || (client == 0))) {
		if (client)
			return pcb->state;
		else
			return _ttcp->lpcb->state;

	} else {
		pr_debug("no _ttcp\r\n");
	}

	return CLOSED;
}

void* getTTCP(uint8_t sock, uint8_t mode)
{
	if(sock < MAX_SOCK_NUM)
		return mapSockTCP[sock][mode];
	return NULL;
}

static void cleanSockState_cb(void *ctx)
{
	struct ttcp* _ttcp = ctx;

	if(_ttcp == NULL) return;

	int sock = getSock(_ttcp);
	if(sock != -1)
		clearMapSockTcp(sock, GET_TCP_MODE(_ttcp));
	_connected[sock] = 0;
}

static err_t close_conn_pcb(struct tcp_pcb* tpcb)
{
	err_t err = ERR_OK;
	//pr_debug("clear handler\r\n");

	// flush buffers
	tcp_arg(tpcb, NULL);
	tcp_sent(tpcb, NULL);
	tcp_recv(tpcb, NULL);
	tcp_poll(tpcb, NULL, 10);

	// close socket
	err = tcp_close(tpcb);
	
	//pr_debug("closing tpcb [%p]\r\n", tpcb);
	return err;
}

static int8_t empty_conn_completely(struct ttcp *_ttcp)
{
	int8_t i = 0;

	for (; i<MAX_CLIENT_ACCEPTED; ++i) {
		if (_ttcp->tpcb[i] != NULL)
			return 0;
	}

	return 1;
}

static err_t close_conn(struct ttcp *_ttcp, struct tcp_pcb* tpcb)
{
	if(_ttcp == NULL) return ERR_MEM;

	uint8_t id = get_client_connId(_ttcp, tpcb);
	if (id == NO_VALID_ID)
		return ERR_MEM;

	if (id == get_currConnId(_ttcp))
		update_currConnId(_ttcp, NO_VALID_ID);

	err_t err = close_conn_pcb(_ttcp->tpcb[id]);

	if (err != ERR_OK) {			//close failed, set pending
		pr_debug("Cannot close id:%d-%p put pending\r\n", id, _ttcp->tpcb[id]);
		_ttcp->pending_close[id] = 1;
	} else {						//success
		FREE_PAYLOAD_ID(_ttcp, id);		// free payload buf

		removeNewClientConn(_ttcp, _ttcp->tpcb[id]);

		if (empty_conn_completely(_ttcp))
			clearMapSockMode(_ttcp->sock, _ttcp, TTCP_MODE_TRANSMIT);
		//clear map when last one client remove.

		_ttcp->pending_close[id] = 0;
	}

	return err;
}

static void ard_tcp_destroy(struct ttcp* ttcp)
{
	err_t err = ERR_OK;

	if (TCP_MODE == ttcp->udp) {
		if (ttcp->lpcb) {		//server start failed

			tcp_arg(ttcp->lpcb, NULL);
			tcp_accept(ttcp->lpcb, NULL);
			err = tcp_close(ttcp->lpcb);

			pr_debug("Closing lpcb: state:0x%x err:%d\r\n", ttcp->lpcb->state, err);
		}
	} else {
		if (ttcp->upcb) {
			udp_disconnect(ttcp->upcb);
			udp_remove(ttcp->upcb);
		}
	}

	int8_t i = 0;
	for(; i<MAX_CLIENT_ACCEPTED; ++i)
	{
		if (ttcp->tpcb[i]) {
			tcp_arg(ttcp->tpcb[i], NULL);
			tcp_sent(ttcp->tpcb[i], NULL);
			tcp_recv(ttcp->tpcb[i], NULL);
			tcp_err(ttcp->tpcb[i], NULL);
			//TEMPORAQARY
			//err = tcp_close(ttcp->tpcb[i]);
			pr_debug("Closing tpcb: state:0x%x err:%d\r\n", ttcp->tpcb[i]->state, err);
		}
	}

	uint8_t sock = getSock(ttcp);
	if(sock == -1)
		pr_debug("ttcp already deallocated!\r\n");

	if (ttcp->insert_xMutex != NULL) {
		vSemaphoreDelete(ttcp->insert_xMutex);
		ttcp->insert_xMutex = NULL;
	}

	freeAllTcpData(sock);
	free(ttcp);
	ttcp = NULL;
}

void ard_tcp_stop(void* ttcp)
{
	struct ttcp* _ttcp = (struct ttcp*) ttcp;
	if (_ttcp == NULL) {
		pr_debug("ttcp = NULL!\r\n");
		return;
	}

	if (_ttcp->mode == TTCP_MODE_TRANSMIT) {	//client
		pr_debug("<--stop client-->\r\n");
		update_currConnId(_ttcp, NO_VALID_ID);

		ard_tcp_destroy(_ttcp);
		clearMapSockTcp(getSock(_ttcp), GET_TCP_MODE(_ttcp));
	} else {									//server request client stop
		pr_debug("<--stop server(client)-->\r\n");
		int8_t i = get_currConnId(_ttcp);
		if( i == NO_VALID_ID)
			return ;

		struct tcp_pcb *pcb = _ttcp->tpcb[i];

		if((_ttcp) && (pcb) && (pcb->state!=LAST_ACK) && (pcb->state!=CLOSED))
		{
			// Flush all the data
			err_t err = tcp_output(pcb);
			// if any socket  cannot be close stop the close connection
			//freeAllTcpData(_ttcp->sock);  	//clear recv data
			close_conn(_ttcp, pcb);
			pr_debug("Complete close to client\r\n");
		}
	}
}

void initMapSockTcp(void)
{
	memset(mapSockTCP, 0, sizeof(mapSockTCP));
}

void clearMapSockTcp(uint8_t sock, uint8_t mode)
{
	if(sock < MAX_SOCK_NUM)
		mapSockTCP[sock][mode] = NULL;
}

void setMapSockMode(uint8_t sock, void* _ttcp, uint8_t _tcp_mode)
{
	if ((IS_VALID_SOCK(sock)) && (_ttcp!=NULL))
		mapSockTCP[sock][_tcp_mode] =_ttcp;

	pr_debug("set mapSockTCP[%d][%s]-->[%p]\r\n",sock, Mode2Str(_tcp_mode), _ttcp);
}

void setMapSock(uint8_t sock, void* _ttcp)
{
	setMapSockMode(sock, _ttcp, GET_TCP_MODE(_ttcp));
}

void clearMapSockMode(uint8_t sock, void* _ttcp, uint8_t _tcp_mode)
{
	if ((IS_VALID_SOCK(sock)) && (_ttcp != NULL))
		mapSockTCP[sock][_tcp_mode] = NULL;

	pr_debug("clear mapSockTCP[%d][%s]-->[%p]\r\n",sock, Mode2Str(_tcp_mode), _ttcp);
}

int getSock(void * _ttcp)
{
	if(_ttcp != NULL)
	{
		int8_t i = 0;
		for(; i < MAX_SOCK_NUM; i++)
		{
			if (_ttcp == mapSockTCP[i][GET_TCP_MODE(_ttcp)]) {
				//pr_debug("getSock mapSockTCP[%d][%s]-->[%p]\r\n",i,Mode2Str(GET_TCP_MODE(_ttcp)),_ttcp);
				return i;
			}
		}
	}
	return -1;
}

static void init_clean_conn(struct ttcp* _ttcp)
{
	if (_ttcp != NULL) {
		int8_t i = 0;
		for(; i<MAX_CLIENT_ACCEPTED; ++i)
		{
			_ttcp->tpcb[i] = NULL;
			_ttcp->payload[i] = NULL;
			_ttcp->buff_sent[i] = 0;
			_ttcp->left[i] = 0;
		}
		_ttcp->lpcb = NULL;
		_ttcp->upcb = NULL;
	}
	return;
}

int8_t insertNewClientConn(struct ttcp* _ttcp, struct tcp_pcb *newpcb)
{
	if (_ttcp != NULL) {

		int8_t idx = 0;
		for(; idx < MAX_CLIENT_ACCEPTED; ++idx)
		{
			if ((_ttcp->tpcb[idx] == NULL) || (_ttcp->tpcb[idx] == newpcb))
			{
				pr_debug("insert id=%d tpcb=[%p]\r\n", idx, newpcb);
				_ttcp->tpcb[idx] = newpcb;
				return idx;
			}
		}
		pr_debug("insert tpcb to _ttcp fail\r\n");

	}

	return NO_VALID_ID;
}

int8_t get_client_connId(struct ttcp* _ttcp, struct tcp_pcb *_pcb)
{
	if (_ttcp != NULL) {
		int8_t i = 0;
		for(; i < MAX_CLIENT_ACCEPTED; ++i)
		{
			if (_ttcp->tpcb[i] == _pcb)
				return i;
		}
	}

	pr_debug("No Valid Id for ttcp:%p pcb:%p\r\n",	_ttcp, _pcb);

	return NO_VALID_ID;
}

int8_t removeNewClientConn(struct ttcp* _ttcp, struct tcp_pcb *newpcb)
{
	if (_ttcp != NULL) {
		int i = 0;
		for(; i<MAX_CLIENT_ACCEPTED; ++i)
		{
			if(_ttcp->tpcb[i] == newpcb)
			{
				pr_debug("remove id=%d, tpcb=%p\r\n", i, newpcb);
				_ttcp->tpcb[i] = NULL;
				return i;
			}
		}
	}
	return NO_VALID_ID;
}

static void atcp_init_pend_flags(struct ttcp* _ttcp)
{
	int i = 0;
	for(; i<MAX_CLIENT_ACCEPTED; ++i){
		if(_ttcp) _ttcp->pending_close[i] = 0;
	}
}
static void atcp_conn_err_cb(void *arg, err_t err)
{
	pr_debug("server connection err :%d\r\n", err);
	/*
	    struct ttcp* _ttcp = arg;
	    if(_ttcp == NULL) return;

	    pr_debug("atcp_conn_err_cb  %d!!!!\r\n", err);
	    if(err == ERR_RST)
	    pr_debug("ERR_RST\r\n");

	    if(err == ERR_ABRT){
	    if(_ttcp == NULL)
	    return;

	    int8_t i = currConnId[_ttcp->sock];
	    struct tcp_pcb *pcb = _ttcp->tpcb[i];
	    removeNewClientConn(_ttcp, pcb);
	    FREE_PAYLOAD_ID(_ttcp, i);
	    }
	    */
}

static void atcp_conn_cli_err_cb(void *arg, err_t err)
{
	pr_debug("client connection err :%d\r\n", err);
	/*
	    struct ttcp* _ttcp = arg;
	    if(_ttcp == NULL) return;

	    pr_debug("atcp_conn_cli_err_cb %d!!!!\r\n", err);

	    if(err == ERR_RST)
	    pr_debug("ERR_RST\r\n");

	    if((_ttcp)&&(err == ERR_ABRT)){
	    cleanSockState_cb(_ttcp);

	    int8_t i =  currConnId[_ttcp->sock];
	    FREE_PAYLOAD_ID(_ttcp, i);
	    }
	    */
}

static err_t tcp_send_data_pcb(struct ttcp *ttcp, struct tcp_pcb *pcb)
{
	err_t err = ERR_OK;
	uint32_t len;

	uint8_t id = get_client_connId(ttcp, pcb);
	if(id == NO_VALID_ID)
		return ERR_MEM;

	len = ttcp->left[id];
	ttcp->buff_sent[id] = 0;

	if(len == 0) return ERR_MEM;

	/* We cannot send more data than space available in the send
	    buffer. */
	if(len > tcp_sndbuf(pcb))
		len = tcp_sndbuf(pcb);

	err = tcp_write(pcb, ttcp->payload[id], len, TCP_WRITE_FLAG_COPY);
	// flush immediately
	tcp_output(pcb);
	if (err != ERR_OK) {
		pr_debug("tcp_write failed %p state:%d len:%d err:%d\r\n",
				pcb, pcb->state, len, err);
		ttcp->buff_sent[id] = 0;
	} else {
		ttcp->buff_sent[id] = 1;
		ttcp->left[id] -= len;
	}

	return err;
}

static err_t tcp_data_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
	struct ttcp *_ttcp = arg;

	if(_ttcp == NULL) return ERR_ARG;

	uint8_t id = get_client_connId(_ttcp, pcb);
	if(id == NO_VALID_ID)
		return ERR_MEM;

	_ttcp->buff_sent[id] = 1;

	if ((_ttcp) && (_ttcp->left[id] > 0)) {
		_ttcp->buff_sent[id] = 0;
		tcp_send_data_pcb(_ttcp, pcb);
	}

	return ERR_OK;
}

int8_t sendTcpData(void* p, uint8_t* buf, uint16_t len)
{
	struct ttcp* _ttcp = (struct ttcp*) p;

	if (_ttcp==NULL) {
		pr_debug("ttcp == NULL!\r\n");
		return WL_FAILURE;
	}

	uint8_t id = get_currConnId(_ttcp);
	if (id == NO_VALID_ID)
		return ERR_MEM;
	struct tcp_pcb *pcb = _ttcp->tpcb[id];

	/* don't send more than we have in the payload */
	// payload malloc 1024 btye, buflen = 1024
	if (len > _ttcp->buflen)
		len = _ttcp->buflen;

	//pr_debug("sendTcpData data len %d\r\n", len);

	memset(_ttcp->payload[id], 0, _ttcp->buflen);

	if ((_ttcp != NULL) && (pcb != NULL) &&
			(buf != NULL) && (len != 0) && (_ttcp->payload[id] != NULL)) {

		if(pcb->state == ESTABLISHED || pcb->state == CLOSE_WAIT ||
				pcb->state == SYN_SENT || pcb->state == SYN_RCVD) {

			memcpy(_ttcp->payload[id], buf, len);
			_ttcp->payload[id][len] = '\0';
			_ttcp->left[id] = len;

			tcp_sent(pcb, tcp_data_sent);
			tcp_send_data_pcb(_ttcp, pcb);
			return WL_SUCCESS;
		}
	}

	return WL_FAILURE;
}

int8_t isDataSent(void* p)
{
	struct ttcp *_ttcp = (struct ttcp *)p;

	uint8_t id = get_currConnId(_ttcp);
	if ((_ttcp) && (!_ttcp->buff_sent[id]))
		return 0;

	return 1;
}

static err_t atcp_recv_cb(void *arg, struct tcp_pcb *pcb, struct pbuf *p,
		err_t err)
{
	struct ttcp* ttcp = arg;

	if (err == ERR_OK && p != NULL) {
		//update recved windows, actual should updated when user readed data.
		//but more than one clients data saved same buf, so updated in here temporary.
		tcp_recved(pcb, p->tot_len);

		insert_pBuf(p, ttcp->sock, (void*) pcb);	//insert data
		pbuf_free(p);
	}

	if (err == ERR_OK && p == NULL) {
		//freeAllTcpData(ttcp->sock);     //Whether the data is clear when an exception occurs
		//all client messages saved same buf, free can cause other client data lost

		close_conn(ttcp, pcb);
	}

	if (err != ERR_OK)
		pr_debug("%s  err=%d p=%p\r\n", __func__, err, p);

	return ERR_OK;
}

#if 0
static void poll_all_pcb_state(struct ttcp *_ttcp)
{
	int8_t i = 0;

	for (; i< MAX_CLIENT_ACCEPTED; ++i) {
		if (_ttcp->tpcb[i] != NULL) {

			pr_debug("sock[%d] _ttcp[%d] state %d\r\n",_ttcp->sock, i, _ttcp->tpcb[i]->state);

			if (_ttcp->tpcb[i]->state == CLOSED)
				close_conn(_ttcp, _ttcp->tpcb[i]);
		}
	}
}
#endif

static err_t atcp_poll(void *arg, struct tcp_pcb *pcb)
{
	struct ttcp* _ttcp = arg;

	uint8_t id = get_client_connId(_ttcp, pcb);
	if (id == NO_VALID_ID) {
		pr_debug("atcp_poll no id\r\n");
		return -1;
	}

	if(pcb->state == CLOSED)
		close_conn(_ttcp, pcb);

	//poll_all_pcb_state(_ttcp);

	/*Task 1: send data ,   if (_ttcp->left[id] > 0)*/
	if(_ttcp->left[id] > 0)
		tcp_send_data_pcb(_ttcp, pcb);

	/*Task 2: close client   if (_ttcp->pending_close[id])*/
	if (_ttcp->pending_close[id]) {
		err_t err = ERR_OK;

		err = tcp_close(pcb);

		if (err != ERR_OK) {
			_ttcp->pending_close[id] = 1;
		} else {
			_ttcp->pending_close[id] = 0;

			FREE_PAYLOAD_ID(_ttcp, id);

			removeNewClientConn(_ttcp, _ttcp->tpcb[id]);

			if (empty_conn_completely(_ttcp))
				clearMapSockMode(_ttcp->sock, _ttcp, TTCP_MODE_TRANSMIT);
			//clear map when last one client remove.
		}
	}
	return ERR_OK;
}

static err_t atcp_poll_conn(void *arg, struct tcp_pcb *pcb)
{
	struct ttcp* _ttcp = arg;

	if(_ttcp == NULL) return ERR_ARG;

	uint8_t id = get_client_connId(_ttcp, pcb);
	if (id == NO_VALID_ID) {
		pr_debug("This is atcp_poll_conn\r\n");
		return -1;
	}

	if ((_ttcp->left[id] > 0) && (_connected[_ttcp->sock]))
		tcp_send_data_pcb(_ttcp, pcb);

	if (pcb->state == CLOSED)
		close_conn(_ttcp, pcb);

	/*Task 2: close client   if (_ttcp->pending_close[id])*/
	if(_ttcp->pending_close[id]){
		err_t err = ERR_OK;

		err = tcp_close(pcb);

		if (err != ERR_OK) {
			_ttcp->pending_close[id] = 1;
		} else {
			_ttcp->pending_close[id] = 0;

			removeNewClientConn(_ttcp, _ttcp->tpcb[id]);

			FREE_PAYLOAD_ID(_ttcp, id);

			clearMapSockMode(_ttcp->sock, _ttcp, TTCP_MODE_TRANSMIT);
		}
	}

	return ERR_OK;
}

static err_t atcp_accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	struct ttcp* _ttcp = arg;

	if(_ttcp == NULL) return ERR_ARG;

	pr_debug("<--new client connected-->\r\n");

	xSemaphoreTake(_ttcp->insert_xMutex, portMAX_DELAY );
	int8_t id = insertNewClientConn(_ttcp, newpcb);		//Indicate new client connected. (for server
	xSemaphoreGive(_ttcp->insert_xMutex);
	if (id == NO_VALID_ID)
		goto fail;

	_ttcp->payload[id] = malloc(_ttcp->buflen);
	if (_ttcp->payload[id] == NULL)
		goto fail;

	tcp_arg(_ttcp->tpcb[id], _ttcp);
	tcp_recv(_ttcp->tpcb[id], atcp_recv_cb);
	tcp_err(_ttcp->tpcb[id], atcp_conn_err_cb);
	tcp_poll(_ttcp->tpcb[id], atcp_poll, 10);
	pr_debug("accept new client ttcp->lpcb[%p] tpcb[%p]\r\n", _ttcp->lpcb, _ttcp->tpcb[id]);

	//pr_debug("accept ttcp->lpcb->state %d\r\n", _ttcp->lpcb->state);
	//Copy the pointer to ttcp also to TRANSMIT mode for the clients connected to the server
	int _sock = getSock(_ttcp);
	if((_sock != -1) && (IS_VALID_SOCK(_sock)))
		setMapSockMode(_sock, _ttcp, TTCP_MODE_TRANSMIT);
	//whenever new client connected. re-set mapSockTCP[sock][client] save ttcp

	return ERR_OK;

fail:
	tcp_abort(newpcb);
	close_conn_pcb(newpcb);
	return ERR_ABRT;
}

static err_t tcp_connect_cb(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	struct ttcp* _ttcp = arg;

	if(_ttcp == NULL) return ERR_ARG;
	_connected[_ttcp->sock] = ( tpcb->state == ESTABLISHED) ? 1 : 0;
	pr_debug("client linking to server. state %d\r\n", tpcb->state);

	return ERR_OK;
}
static int atcp_start(struct ttcp* ttcp)
{
	err_t err = ERR_OK;

	struct tcp_pcb *p = tcp_new();
	if (p == NULL) {
		pr_debug("could not allocate pcb\r\n");
		return ERR_MEM;
	}

	ttcp->insert_xMutex = xSemaphoreCreateMutex();
	if (ttcp->insert_xMutex == NULL)
		return ERR_MEM;

	tcp_arg(p, ttcp);				//set tcp_pcb *arg

	if (ttcp->mode == TTCP_MODE_TRANSMIT) {		//for client

		int8_t id = insertNewClientConn(ttcp, p);	//always insert 0.
		if (id == NO_VALID_ID)
			return ERR_MEM;

		//Global val, Indicate client whether connect server success.
		_connected[ttcp->sock] = 0;

		ttcp->payload[id] = malloc(ttcp->buflen);	//malloc receive buf
		if (ttcp->payload[id] == NULL) {
			removeNewClientConn(ttcp, p);

			pr_debug("TTCP [%p]: could not allocate payload\r\n", ttcp);
			return -1;
		}

		struct tcp_pcb *pcb = p;

		tcp_err(pcb,  atcp_conn_cli_err_cb);
		tcp_recv(pcb, atcp_recv_cb);
		tcp_sent(pcb, tcp_data_sent);
		tcp_poll(pcb, atcp_poll_conn, 10);
		if ((err = tcp_connect(pcb, &ttcp->addr, ttcp->port, tcp_connect_cb))
				!= ERR_OK) {
			removeNewClientConn(ttcp, p);
			FREE_PAYLOAD_ID(ttcp, id);

			pr_debug("TTCP [%p]: tcp connect failed\r\n", ttcp);
			return err;
		}
	} else {					//for server
		// begin() : wait client connect
		err = tcp_bind(p, IP_ADDR_ANY, ttcp->port);
		if (err != ERR_OK) {
			pr_debug("bind failed err Port already used, err %d\r\n", err);
			return -1;
		}

		ttcp->lpcb = tcp_listen(p);		//save listen tcp_pcb
		if (ttcp->lpcb == NULL) {
			pr_debug("listen failed\r\n");
			return -1;
		}

		tcp_accept(ttcp->lpcb, atcp_accept_cb); //start to wait client connect.
	}
	return 0;
}

//Udp RemoteIp and remote Port
static tRemoteClient remoteClients[MAX_SOCK_NUM] = {{0,0}};

void setRemoteClient(uint16_t sock, uint32_t _ipaddr, uint16_t _port)
{
	if (sock < MAX_SOCK_NUM) {
		remoteClients[sock].ipaddr = _ipaddr;
		remoteClients[sock].port = _port;
	}
}

tRemoteClient* getRemoteClient(uint16_t sock)
{
	if(sock < MAX_SOCK_NUM){
		return &remoteClients[sock];
	}
	return NULL;
}

void getRemoteData(uint8_t sock, uint8_t mode, tRemoteClient* remoteData)
{
	if((sock>=0) && (sock<MAX_SOCK_NUM)){
		void* p = getTTCP(sock, mode);
		if(p){
			ttcp_t* _ttcp = (ttcp_t* )p;
			if((_ttcp->udp == UDP_MODE))
			{
				if (_ttcp->mode == TTCP_MODE_RECEIVE) {
					remoteData->ipaddr = getRemoteClient(sock)->ipaddr;
					remoteData->port = getRemoteClient(sock)->port;
				} else {
					remoteData->ipaddr = (_ttcp->upcb) ? _ttcp->upcb->remote_ip.addr : 0;
					remoteData->port = (_ttcp->upcb) ? _ttcp->upcb->remote_port : 0;
				}
			}
		}
	}
}

int8_t sendUdpData(void* ttcp, uint8_t* buf, uint16_t len)
{
	struct ttcp* _ttcp = (struct ttcp*) ttcp;
	if((_ttcp != NULL) && (buf != NULL) && (len != 0)){
		//pr_debug("buf:%s len:%d\r\n", buf, len);
	}else{
		return -1;
	}

	struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
	if(p == NULL){
		pr_debug("TTCP [%p]: could not allocate pbuf\r\n", ttcp);
		return -1;
	}
	memcpy(p->payload, buf, len);
	if(udp_send(_ttcp->upcb, p) != ERR_OK){
		pr_debug("TTCP [%p]: udp_send() failed\r\n", _ttcp);
		pbuf_free(p);
		return -1;
	}

	pbuf_free(p);
	return 1;
}

static void audp_recv_cb(void *arg, struct udp_pcb *upcb, struct pbuf *p,
		ip_addr_t *addr, u16_t port) {
	struct ttcp* ttcp = arg;

	insert_pBuf(p, ttcp->sock, (void*) upcb);
	setRemoteClient(ttcp->sock, addr->addr, port);
	pbuf_free(p);
}

static int udp_start(struct ttcp* ttcp)
{
	err_t err = ERR_OK;
	ttcp->upcb = udp_new();

	if (ttcp->upcb == NULL) {
		pr_debug("TTCP [%p]: could not allocate pcb\r\n", ttcp);
		return -1;
	}

	if (ttcp->mode == TTCP_MODE_TRANSMIT) {
		if (udp_connect(ttcp->upcb, &(ttcp->addr), ttcp->port) != ERR_OK) {
			pr_debug("TTCP [%p]: udp connect failed\r\n", ttcp);
			return -1;
		}
		udp_recv(ttcp->upcb, audp_recv_cb, ttcp);
	} else {
		//udp server
		err = udp_bind(ttcp->upcb, IP_ADDR_ANY, ttcp->port);
		if (err!= ERR_OK) {
			pr_debug("TTCP [%p]: bind failed err=%d Port already used\r\n", ttcp, err);
			return -1;
		}
		// clear remote client data
		setRemoteClient(ttcp->sock, 0, 0);
		udp_recv(ttcp->upcb, audp_recv_cb, ttcp);
	}
	return 0;
}

int ard_tcp_start(ip_addr_t addr, struct ttcp* ttcp, uint16_t port,
		int mode, uint16_t buflen, int udp, uint8_t sock)
{
	unsigned char status = 0;

	if (mode != TTCP_MODE_TRANSMIT && mode != TTCP_MODE_RECEIVE) {
		pr_debug("TTCP [-]: invalid mode\r\n");
		return -1;
	}

	if (buflen == 0) {
		pr_debug("TTCP [-]: invalid buflen\r\n");
		return -1;
	}

	ttcp->addr = addr;
	ttcp->port = port;
	ttcp->sock = sock;
	ttcp->udp = udp;
	ttcp->buflen = buflen;
	ttcp->mode = mode;
	ttcp->sock = sock;

	init_clean_conn(ttcp);			//init client parameter
	atcp_init_pend_flags(ttcp);		//init pending close

	if (ttcp->udp) {
		status = udp_start(ttcp);	//init udp
	} else {
		status = atcp_start(ttcp);	//init tcp
	}

	if (status < 0) {
		pr_debug("Start server FAILED!\r\n");//Failed destroy ttcp.
		ard_tcp_destroy(ttcp);
		return -1;
	}
	return 0;
}
