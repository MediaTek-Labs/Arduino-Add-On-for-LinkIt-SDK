/*
  WiFiServer.cpp - Library for LinkIt 7697 HDK.
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
extern "C"{
#include "lwipopts.h"
}
#include <Arduino.h>
#include "LWiFi.h"
#include "WiFiClient.h"
#include "WiFiServer.h"

extern "C" {
#include "log_dump.h"
#include "delay.h"
#include "lwip/sockets.h"
#include "constants.h"
}


WiFiServer::WiFiServer(uint16_t port):
	m_port(port)
	,m_socket(-1)
{
	
}

void WiFiServer::begin()
{
	pr_debug("calling WiFiServer::begin\r\n");

	m_socket = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(-1 == m_socket)
	{
		pr_debug("lwip_socket fails");
		return;
	}

	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = lwip_htonl(INADDR_ANY);
	serverAddr.sin_port = lwip_htons(m_port);

	pr_debug("calling lwip_bind");

	int ret = lwip_bind(m_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	pr_debug("socket %d bind return %d", m_socket, ret);

	// we need accept() to be non-blocking
	const int flags = lwip_fcntl(m_socket, F_GETFL, 0);
	ret = lwip_fcntl(m_socket, F_SETFL, flags | O_NONBLOCK);

	// start listening on port
	ret = lwip_listen(m_socket, MAX_INCOMING_CLIENT);
	if(ret < 0)
	{
		pr_debug("lwip_listen fails with %d", ret)
		return;
	}
}

WiFiClient WiFiServer::available(uint8_t* status)
{
 	// Gets a client that is newly connected to the server **or**
	// a connected client that has some data available for reading. 
    // The connection persists when the returned client object goes out of scope; 
    // you can close it by calling client.stop().
	if(-1 == m_socket)
	{
		return WiFiClient(-1);
	}

	// check for new connections first.
	sockaddr_in remoteAddr;
	memset(&remoteAddr, 0, sizeof(remoteAddr));
	socklen_t addrSize = sizeof(remoteAddr);
	memset(&remoteAddr, 0, sizeof(remoteAddr));
	int clientSocket = lwip_accept(m_socket, (struct sockaddr*)&remoteAddr, &addrSize);
	if(-1 != clientSocket)
	{
		pr_debug("lwip_accept returns valid socket %d", clientSocket);
		// register into our "clients" list.
		m_clientSockets.insert(clientSocket);

		// immediately return our new found socket
		return WiFiClient(clientSocket);
	}

	// if there are no new connections,
	// check if we have some socket that are available to read
	for(auto i = m_clientSockets.begin(); i != m_clientSockets.end(); ++i)
	{
		// simply return the 1st one that has something to read
		WiFiClient c(*i);	
		if(c.available())
		{
			return c;
		}
	}

	// found nothing
	return WiFiClient(-1);
}

size_t WiFiServer::write(uint8_t c)
{
	size_t succeedOnce = 0;
	for(auto i = m_clientSockets.begin(); i != m_clientSockets.end(); ++i)
	{
		int ret = lwip_send(*i, &c, 1, 0);
		pr_debug("send to sock %d returns %d", *i, ret);
		if(-1 != ret)
		{
			succeedOnce = 1;
		}
	}
	return succeedOnce;
}

size_t WiFiServer::write(const uint8_t *buf, size_t size)
{
	size_t succeedOnce = 0;
	for(auto i = m_clientSockets.begin(); i != m_clientSockets.end(); ++i)
	{
		int ret = lwip_send(*i, buf, size, 0);
		pr_debug("send to sock %d returns %d", *i, ret);
		if(-1 != ret)
		{
			succeedOnce = ret;
		}
	}
	return succeedOnce;
}

