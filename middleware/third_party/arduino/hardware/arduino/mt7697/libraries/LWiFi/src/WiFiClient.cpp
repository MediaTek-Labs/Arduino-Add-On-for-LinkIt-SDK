/*
  WiFiClient.cpp - Library for LinkIt 7697 HDK.
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

#include "LWiFi.h"
#include "WiFiClient.h"

extern "C" {
#include "log_dump.h"
#include "delay.h"
#include "lwip/sockets.h"
#include "utility/wl_definitions.h"
}


WiFiClient::WiFiClient(): 
	m_socket(-1),
	m_externalSocket(false)
{
	
}

WiFiClient::WiFiClient(int sock): 
	m_socket(sock),
	m_externalSocket(true)
{
	// accepting an externally
	// allocated socket
}

WiFiClient::~WiFiClient()
{
	// only auto-close sockets that are created
	// by the connect() method.
	// This is designed for WiFiServer::available().
	// The WiFiClient object returned from WiFiServer::available() should
	// persist its connection even when the returned client object goes out of scope; 
    // The user should close it by calling client.stop().
	if(!m_externalSocket)
	{
		stop();
	}
}

int WiFiClient::connect(const char* host, uint16_t port) {
	IPAddress remote_addr;
	if (WiFi.hostByName(host, remote_addr))
	{
		return connect(remote_addr, port);
	}
	return 0;
}

int WiFiClient::connect(IPAddress ip, uint16_t port) {
	if(m_socket)
	{
		stop();
	}

	m_socket = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	m_externalSocket = true;

	if( -1 == m_socket)
	{
		pr_debug("socket allocation failed!");
		return 0;
	}

	struct sockaddr_in server;
	server.sin_addr.s_addr = (uint32_t)ip;
    server.sin_family = AF_INET;
    server.sin_port = lwip_htons( port );

	if(0 > lwip_connect(m_socket, (struct sockaddr*)&server, sizeof(server)))
	{
		lwip_close(m_socket);
		m_socket = -1;
		pr_debug("connect failed");
		return 0;
	}

	pr_debug("connected! with socket = %d", m_socket);
	return 1;
}

size_t WiFiClient::write(uint8_t b) {
	if(m_socket == -1)
	{
		return 0;
	}

	int ret =  lwip_send(m_socket, &b, 1, 0);

	pr_debug("lwip_send returns %d", ret);
	return ret;
}

size_t WiFiClient::write(const uint8_t *buf, size_t size) {

	if(m_socket == -1)
	{
		return 0;
	}

	int ret = lwip_send(m_socket, buf, size, 0);
	if(ret < 0)
	{
		pr_debug("buffered lwip_send fails with ret= %d", ret);
	}
	return ret;
}

int WiFiClient::available() {
	if(m_socket == -1)
	{
		return 0;
	}

#if 0
	// Note: to enable FIONREAD, we need to enable lwIP option `LWIP_SO_RCVBUF	1`.
	// Currently, it is not enabled.
	int count = 0;
	int ret = lwip_ioctl(m_socket, FIONREAD, &count);
	pr_debug("lwip_ioctl FIONREAD returns %d with count = %d", ret, count);
	return count;
#endif

	if(-1 == peek())
	{
		return 0;
	}

	return 1;
}

int WiFiClient::read() {
	if(m_socket == -1)
	{
		return -1;
	}

	uint8_t b;
	if(1 == lwip_recv(m_socket, &b, 1, MSG_DONTWAIT))
	{
		return b;
	}
	else
	{
		return -1;
	}
}


int WiFiClient::read(uint8_t* buf, size_t size) {
	if(m_socket == -1)
	{
		return 0;
	}
	
	int ret = lwip_recv(m_socket, buf, size, MSG_DONTWAIT);
	pr_debug("lwip_recv returns %d", ret);
	return ret;
}

int WiFiClient::peek() {
	if(m_socket == -1)
	{
		return -1;
	}

	uint8_t b;
	int ret = lwip_recv(m_socket, &b, 1, MSG_PEEK | MSG_DONTWAIT);
	if(ret == 1)
	{
		return b;
	}

	return -1;
}

void WiFiClient::flush() {
	return;
}

void WiFiClient::stop() {
	if(m_socket == -1)
	{
		return;
	}

	pr_debug("lwip_close on socket %d", m_socket);
	lwip_close(m_socket);
	m_socket = -1;
}

uint8_t WiFiClient::connected() {
	if(-1 == m_socket)
	{
		return 0;
	}

	int error_code = 0;
	socklen_t error_code_size = sizeof(error_code);
	lwip_getsockopt(m_socket, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);

	if((error_code == ENOTCONN) ||
		(error_code == ENOTSOCK) ||
		(error_code == ECONNRESET) ||
		(error_code == ETIMEDOUT))
	{
		return 0;	
	}
	else
	{
		return 1;
	}
}

uint8_t WiFiClient::status() {
	return 0;
}

WiFiClient::operator bool() {
	return connected();
}
