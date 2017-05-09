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

	uint8_t status();

	// Set the Root Certificate of the remote host.
	// Note that length should contain the NULL-terminator
	// if the CA is in PEM string format.
	// 
	// the rootCA buffer must be valid during
	// connect() - stop().
	void setRootCA(const char* rootCA, size_t length);

	virtual int connect(IPAddress ip, uint16_t port);
	virtual int connect(const char *host, uint16_t port);
	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t *buf, size_t size);
	virtual int available();
	virtual int read();
	virtual int read(uint8_t *buf, size_t size);
	virtual int peek();
	virtual void flush();
	virtual void stop();
	virtual uint8_t connected();
	virtual operator bool();

	friend class WiFiServer;

	using Print::write;

private:
	int connectImpl(const char* host, uint16_t port, bool isIPAddress);

private:
	TLSContext m_cntx;
	bool m_connected;
	uint8_t *m_rootCA;
	size_t m_rootCALen;
};

#endif
