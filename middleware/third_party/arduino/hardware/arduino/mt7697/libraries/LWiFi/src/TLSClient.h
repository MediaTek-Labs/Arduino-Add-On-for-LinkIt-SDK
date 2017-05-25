/*
  TLSClient.h - Library for LinkIt 7697
*/

#ifndef wifitlsclient_h
#define wifitlsclient_h

#include <stdio.h>
#include "Print.h"
#include "Client.h"
#include "IPAddress.h"

extern "C"
{
#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/certs.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
}

// This class stores context data required for underlying mbedTLS library
struct TLSContext
{
	mbedtls_ssl_context ssl_ctx;        /* mbedtls ssl context */
	mbedtls_net_context net_ctx;        /* Fill in socket id */
	mbedtls_ssl_config ssl_conf;        /* SSL configuration */
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_x509_crt cacert;
	// -1 is returned when error happens
	int init(const unsigned char* host);

	void free();
};

class TLSClient : public Client {
public:
	TLSClient();

	// Set the Root Certificate of the remote host.
	// Note that `length` parameter should contain the NULL-terminator
	// if the CA is in PEM string format.
	// 
	// the rootCA buffer must be valid during
	// connect() - stop().
	//
	// connect() without calling setRootCA() first suppresses
	// root CA verification - which can be a potential
	// security risk.
	void setRootCA(const char* rootCA, size_t length);

	// connects to a remote TLS host on a given port
	virtual int connect(IPAddress ip, uint16_t port);

	// connects to a remote TLS host on a given port.
	// host can be a domain name such as "www.mediatek.com"
	virtual int connect(const char *host, uint16_t port);

	// write a single byte to remote server.
	// returns actual bytes written.
	virtual size_t write(uint8_t);

	// write a buffer of size bytes to remote host.
	// returns actual bytes written.
	virtual size_t write(const uint8_t *buf, size_t size);

	// returns > 1 when there is something in the read buffer.
	// returns 0 when there is nothing to read.
	virtual int available();
	
	// read a single byte. -1(0xFFFFFFFF) is returned when there is nothing to read.
	virtual int read();

	// read at most `size` bytes of content into `buf`. 
	// returns actual bytes read.
	virtual int read(uint8_t *buf, size_t size);

	// return the 1st byte from the read buffer - if availalbe.
	// -1 is returned when there is nothing to read.
	virtual int peek();

	// discards content currently in the read buffer
	virtual void flush();

	// disconnects from the remote host
	virtual void stop();

	// returns true when the TLS socket connection is alive; false otherwise.
	virtual uint8_t connected();

	// This TLSClient object is evaluated to true when connected().
	virtual operator bool();

	friend class WiFiServer;

	using Print::write;

private:
	int connectImpl(const char* host, uint16_t port, bool isIPAddress);

private:
	TLSContext m_cntx;
	bool m_connected;		// connection state
	uint8_t *m_rootCA;		// pointer to the CA buffer. The CA Buffer must be valid for the entire life cycle of TLSClient.
	size_t m_rootCALen;		// length of the CA buffer content. NULL terminators included.
	int32_t m_peekByte;		// cache for the peek() API, which returns a single byte(0x0 ~ 0xFF). -1(0xFFFFFFFF) means the cache is empty.
};

#endif
