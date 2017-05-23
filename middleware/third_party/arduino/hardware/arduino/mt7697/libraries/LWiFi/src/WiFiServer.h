/*
  WiFiServer.h - Library for LinkIt 7697 HDK.
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

#ifndef wifiserver_h
#define wifiserver_h

#include "Server.h"
#include "WiFiClient.h"
#include <set>

class WiFiClient;

class WiFiServer : public Server {
private:
    static const unsigned int MAX_INCOMING_CLIENT = 5;
    uint16_t m_port;
    int m_socket;
    std::set<int> m_clientSockets;
public:
    // allocate a TCP listening socket
    WiFiServer(uint16_t port);

    // start listening on the port
    void begin();

    // Gets a client that is newly connected to the server **or**
    // a connected client that has some data available for reading. 
    // The connection **persists** when the returned client object goes out of scope; 
    // you can close it by calling client.stop().
    WiFiClient available(uint8_t* status = NULL);

    // broadcast to all connected clients.
    virtual size_t write(uint8_t);

    // boardcaste to all connected clients.
    virtual size_t write(const uint8_t *buf, size_t size);

    // This is internal API for WiFiClient/WiFiSever.
    uint8_t status();

    using Print::write;
};

#endif
