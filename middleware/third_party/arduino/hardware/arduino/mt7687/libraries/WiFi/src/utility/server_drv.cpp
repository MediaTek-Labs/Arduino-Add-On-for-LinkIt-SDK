/*
  server_drv.cpp - Library for Arduino Wifi shield.
  Copyright (c) 2011-2014 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

//#define _DEBUG_

#include "utility/server_drv.h"

extern "C" {
#include "delay.h"
#include "utility/ard_mtk.h"
#include "utility/wl_definitions.h"
}

// Start server TCP on port specified
void ServerDrv::startServer(uint16_t port, uint8_t sock, uint8_t protMode)
{
	start_server_tcp(port, sock, protMode);
}

// Start server TCP on port specified
void ServerDrv::startClient(uint32_t ipAddress, uint16_t port, uint8_t sock, uint8_t protMode)
{
	start_client_tcp(ipAddress, port, sock, protMode);
}

// Start server TCP on port specified
void ServerDrv::stopClient(uint8_t sock)
{
	stop_client_tcp(sock);
}


uint8_t ServerDrv::getServerState(uint8_t sock)
{
	return get_state_tcp(sock);
}

uint8_t ServerDrv::getClientState(uint8_t sock)
{
	return get_client_state_tcp(sock);
}

uint16_t ServerDrv::availData(uint8_t sock)
{
	uint16_t len = 0;
	len = avail_data_tcp(sock);

	return len;
}

bool ServerDrv::getData(uint8_t sock, uint8_t *data, uint8_t peek)
{
	uint8_t	_data;
	if (get_data_tcp(sock, &_data, peek)) {
		*data = _data;
		return true;
	}
	return false;
}

bool ServerDrv::getDataBuf(uint8_t sock, uint8_t *_data, uint16_t *_dataLen)
{
	if(get_databuf_tcp(sock, _data, _dataLen) != 0){
		if (*_dataLen != 0){
			return true;
		}
	}
	return false;
}

bool ServerDrv::insertDataBuf(uint8_t sock, const uint8_t *data, uint16_t _len)
{
	if(insert_data(sock, data, _len) == 0)
		return true;

	return false;
}

bool ServerDrv::sendUdpData(uint8_t sock)
{
	if(send_data_udp(sock))
		return true;

	return false;
}


bool ServerDrv::sendData(uint8_t sock, const uint8_t *data, uint16_t len)
{
	int8_t ret =  send_data_tcp(sock, data, len);
	return (ret == WL_SUCCESS)? true : false;
}


uint8_t ServerDrv::checkDataSent(uint8_t sock)
{
	const uint16_t TIMEOUT_DATA_SENT = 25;
	uint8_t timeout = 0;
	int8_t ret  = 0;
	do {
		ret = data_sent_tcp(sock);
		if (ret) timeout = 0;
		else{
			++timeout;
			delay(100);
		}
	} while((ret == 0) && (timeout < TIMEOUT_DATA_SENT));

	return (timeout == TIMEOUT_DATA_SENT) ? 0:1;
}

ServerDrv serverDrv;
