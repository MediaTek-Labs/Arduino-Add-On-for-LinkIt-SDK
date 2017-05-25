/*
  WiFiUdp.cpp - Library for LinkIt 7697 HDK.
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

#include "LWiFi.h"
#include "WiFiUdp.h"
#include <string.h>

extern "C"
{
#include "log_dump.h"
#include "constants.h"
#include "delay.h"
#include "lwip/sockets.h"
#include "utility/wl_definitions.h"
}

/* Constructor */
WiFiUDP::WiFiUDP() : 
	m_socket(-1),
	m_recvCursor(0)
{
	memset(&m_sendAddr, 0, sizeof(m_sendAddr));
	memset(&m_recvAddr, 0, sizeof(m_recvAddr));
	m_sendBuffer.clear();
	m_recvBuffer.clear();	
}

int WiFiUDP::createSocket()
{
	m_socket = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(-1 == m_socket)
	{
		pr_debug("lwip_socket failed");
		return 0;
	}
	return 1;
}

/* Start WiFiUDP socket, listening at local port PORT */
uint8_t WiFiUDP::begin(uint16_t port) {
	// close previous socket
	if(-1 != m_socket)
	{
		stop();
	}

	// allocate new socket
	if(!createSocket())
	{
		return 0;
	}

	// listen on port
	sockaddr_in si_local;
	memset(&si_local, 0, sizeof(si_local));
	si_local.sin_family = AF_INET;
	si_local.sin_port = lwip_htons(port);
	si_local.sin_addr.s_addr = lwip_htonl(INADDR_ANY);

	const int bindResult = lwip_bind(m_socket, (struct sockaddr*)&si_local, sizeof(si_local));
	if(-1 == bindResult)
	{
		pr_debug("lwip_bind failed with %d", bindResult);
		return 0;
	}
	return 1;
}

/* Release any resources being used by this WiFiUDP instance */
void WiFiUDP::stop()
{
	if (m_socket == -1)
		return;

	lwip_close(m_socket);
	m_socket = -1;
}

int WiFiUDP::beginPacket(const char *host, uint16_t port)
{
	// Look up the host first
	int ret = 0;
	IPAddress remote_addr;
	if (WiFi.hostByName(host, remote_addr))
	{
		return beginPacket(remote_addr, port);
	}
	return ret;
}

int WiFiUDP::beginPacket(IPAddress ip, uint16_t port)
{
	if (m_socket == -1)
	{
		createSocket();
	}

	if(-1 == m_socket)
	{
		return 0;
	}

	// clear remote buffer and port
	m_sendBuffer.clear();
	memset(&m_sendAddr, 0, sizeof(m_sendAddr));
	m_sendAddr.sin_addr.s_addr = (uint32_t)ip;
    m_sendAddr.sin_family = AF_INET;
    m_sendAddr.sin_port = lwip_htons( port );

	return 1;
}

int WiFiUDP::endPacket()
{
	if(m_sendBuffer.empty())
	{
		return 0;
	}
	
	// actually send the data
	uint8_t* pData = &m_sendBuffer[0];
	size_t sentBytes = 0;
	const size_t totalBytes = m_sendBuffer.size();
	int ret = -1;

	// sendto() may not write out the entire buffer,
	// so we use a loop to ensure it does.
	do
	{
		ret = lwip_sendto(m_socket, 
						  pData, 
						  (totalBytes - sentBytes), 
						  0, 
						  (struct sockaddr*)&m_sendAddr, 
						  sizeof(m_sendAddr));
		if(-1 == ret)
		{
			pr_debug("lwip_sendto failed, sent %d bytes", sentBytes);
			return 0;
		}
		// increase cursors
		sentBytes += ret;
		pData += ret;
	}while(sentBytes < totalBytes);

	return 1;
}

size_t WiFiUDP::write(uint8_t byte)
{
	m_sendBuffer.push_back(byte);
	return 1;
}

size_t WiFiUDP::write(const uint8_t *buffer, size_t size)
{
	size_t endPos = m_sendBuffer.size();
	m_sendBuffer.resize(m_sendBuffer.size() + size);
	if(m_sendBuffer.size() < (endPos + size))
	{
		pr_debug("m_sendBuffer resize() failed");
		return 0;
	}
	memcpy(&m_sendBuffer[endPos], buffer, size);
	return size;
}

int WiFiUDP::parsePacket()
{
	if(-1 == m_socket)
	{
		createSocket();
	}

	if(-1 == m_socket)
	{
		pr_debug("parsePacket failed due to socket allocation failure");
		return 0;
	}

	flush();
	m_recvBuffer.resize(MAX_DATAGRAM_SIZE);
	socklen_t addrLen = sizeof(m_recvAddr);
	int ret = lwip_recvfrom(m_socket, &m_recvBuffer[0], m_recvBuffer.size(), MSG_DONTWAIT, (struct sockaddr*)&m_recvAddr, &addrLen);
	if(ret <= 0)
	{
		pr_debug("lwip_recvfrom failed with %d", ret);
		return 0;
	}

	pr_debug("lwip_recvfrom recevied %d bytes", ret);
	// resize to actual received datagram size.
	m_recvBuffer.resize(ret);

	return ret;
}

/* return number of bytes available in the current packet,
    will return zero if parsePacket hasn't been called yet */
int WiFiUDP::available() {
	return m_recvBuffer.size() - m_recvCursor;
}

int WiFiUDP::read()
{
	if(m_recvCursor >= m_recvBuffer.size())
	{
		return 0;
	}

	return m_recvBuffer[m_recvCursor++];
}

int WiFiUDP::read(unsigned char* buffer, size_t len)
{
	const size_t copySize = min(len, (m_recvBuffer.size() - m_recvCursor));
	if(copySize)
	{
		memcpy(buffer, &m_recvBuffer[m_recvCursor], copySize);
	}

	m_recvCursor += copySize;

	return (int) copySize;
}

int WiFiUDP::peek()
{
	if(m_recvCursor >= m_recvBuffer.size())
	{
		return 0;
	}

	return m_recvBuffer[m_recvCursor];
}

void WiFiUDP::flush()
{
	m_recvBuffer.clear();
	m_recvCursor = 0;
}

IPAddress  WiFiUDP::remoteIP()
{
	const IPAddress ip((uint32_t)m_recvAddr.sin_addr.s_addr);
	return ip;
}

uint16_t  WiFiUDP::remotePort()
{
	return m_recvAddr.sin_port;
}
