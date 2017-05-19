#include <Arduino.h>
#include "TLSClient.h"
#include <stdio.h>

extern "C"
{
#include "log_dump.h"
#include "mbedtls/debug.h"
}

static void debugOutput(void *ctx, int level, const char *file, int line, const char *str)
{
    pr_debug("%s:%d - %s\n", file, line, str);
}

int TLSContext::init(const unsigned char* host)
{
    // debug log level. 0 means no log.
    mbedtls_debug_set_threshold(0);

    // context initialization
    mbedtls_net_init(&net_ctx);
    mbedtls_ssl_init(&ssl_ctx);
    mbedtls_ssl_config_init(&ssl_conf);
    mbedtls_entropy_init(&entropy);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    
    // entropy source init.
    // we pass the host URL/address as a "customization".
    const char *pers = "https";
    
    int seedResult = mbedtls_ctr_drbg_seed(&ctr_drbg,
                                            mbedtls_entropy_func, 
                                            &entropy,
                                            (const unsigned char*)pers,
                                            strlen(pers));
    if(0 != seedResult)
    {
        pr_debug("mbedtls_ctr_drbg_seed failed with %d", seedResult);
    }
    return seedResult;
}

void TLSContext::free()
{
    mbedtls_ssl_close_notify(&ssl_ctx);
    mbedtls_net_free(&net_ctx);
    mbedtls_x509_crt_free(&cacert);
    mbedtls_ssl_free(&ssl_ctx);    
    mbedtls_ssl_config_free(&ssl_conf);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);         
}

TLSClient::TLSClient():
    m_connected(false)
    ,m_rootCA(NULL)
    ,m_rootCALen(0)
    ,m_peekByte(-1)
{
}

void TLSClient::setRootCA(const char* rootCA, size_t length)
{
    m_rootCA = (uint8_t*)rootCA;
    m_rootCALen = length;
}

int TLSClient::connect(IPAddress ip, uint16_t port)
{
    // convert IP to string
    String ipString;
	for (int i =0; i < 3; i++)
	{
		ipString += ip[i];
		ipString += '.';
	}
	ipString += ip[3];

    return connectImpl(ipString.c_str(), port, true);
}

int TLSClient::connect(const char *host, uint16_t port)
{
    return connectImpl(host, port, false);
}

int TLSClient::connectImpl(const char* host, uint16_t port, bool isIPAddress)
{
    int ret = 0;

    // initialize mbedTLS
    m_cntx.init((const unsigned char*)host);

    // initilaize root CA
    if(m_rootCA)
    {
        ret = mbedtls_x509_crt_parse(&m_cntx.cacert,
                            (const unsigned char *)m_rootCA,
                            m_rootCALen);
        if(0 != ret)
        {
            pr_debug("mbedtls_x509_crt_parse failed with %d", ret);
        }
    }

    // start connection. We'll set SSL/TLS later.
    do
    {
        // "port" ranges from 0 to 65535, so a buffer of 10 characters
        // would be sufficient.
        char portStr[10] = {0};
        sprintf(portStr, "%d", port) ;
        pr_debug("connect to %s:%s", host, portStr);

        ret = mbedtls_net_connect(&m_cntx.net_ctx, host, portStr, MBEDTLS_NET_PROTO_TCP);
        if(0 != ret)
        {
            pr_debug("mbedtls_net_connect failed with %d", ret);
            switch(ret)
            {
            case MBEDTLS_ERR_NET_SOCKET_FAILED:
                pr_debug("MBEDTLS_ERR_NET_SOCKET_FAILED");
                break;
            case MBEDTLS_ERR_NET_UNKNOWN_HOST:
                pr_debug("MBEDTLS_ERR_NET_UNKNOWN_HOST");
                break;
            case MBEDTLS_ERR_NET_CONNECT_FAILED:
                pr_debug("MBEDTLS_ERR_NET_CONNECT_FAILED");
                break;
            }
            break;
        }

        ret = mbedtls_ssl_config_defaults(&m_cntx.ssl_conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT);
        if(0 != ret)
        {
            pr_debug("mbedtls_ssl_config_defaults failed with %d", ret);   
            break;
        }

        // modify connection profile
        static mbedtls_x509_crt_profile profile;
        memcpy(&profile, m_cntx.ssl_conf.cert_profile, sizeof(mbedtls_x509_crt_profile));
        profile.allowed_mds |= MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_MD5);
        mbedtls_ssl_conf_cert_profile(&m_cntx.ssl_conf, &profile);
        
        
        // configure root CA if present
        mbedtls_ssl_conf_authmode(&m_cntx.ssl_conf, m_rootCA ? MBEDTLS_SSL_VERIFY_OPTIONAL : MBEDTLS_SSL_VERIFY_NONE);
        if(m_rootCA){
            mbedtls_ssl_conf_ca_chain(&m_cntx.ssl_conf, &m_cntx.cacert, NULL);
        }
        mbedtls_ssl_conf_rng(&m_cntx.ssl_conf, mbedtls_ctr_drbg_random, &m_cntx.ctr_drbg);
        mbedtls_ssl_conf_dbg(&m_cntx.ssl_conf, debugOutput, NULL);

        ret = mbedtls_ssl_setup(&m_cntx.ssl_ctx, &m_cntx.ssl_conf);
        if(0 != ret)
        {
            pr_debug("mbedtls_ssl_setup failed with %d", ret);
            break;
        }

        // only set hostname if we're connecting to a URL
        if(!isIPAddress)
        {
            if( ( ret = mbedtls_ssl_set_hostname( &m_cntx.ssl_ctx, host) ) != 0 )
            {
                pr_debug("mbedtls_ssl_set_hostname returned %d\r\n", ret );
                break;
            }
        }

        mbedtls_ssl_set_bio(&m_cntx.ssl_ctx, &m_cntx.net_ctx, mbedtls_net_send, NULL, mbedtls_net_recv_timeout);

        // waiting for handshaking
        while ((ret = mbedtls_ssl_handshake(&m_cntx.ssl_ctx)) != 0) {
            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {      
                pr_debug("mbedtls_ssl_handshake() failed, ret:-0x%x., verify_result=", -ret);
                char verificationError[512] = {0};
                mbedtls_x509_crt_verify_info(verificationError, sizeof(verificationError), "  ! ", m_cntx.ssl_ctx.session_negotiate->verify_result);
                pr_debug("%s", verificationError);
                return 0;
            }
        }
            
        /*
        * Verify the server certificate
        */
        /* In real life, we would have used MBEDTLS_SSL_VERIFY_REQUIRED so that the
            * handshake would not succeed if the peer's cert is bad.  Even if we used
            * MBEDTLS_SSL_VERIFY_OPTIONAL, we would bail out here if ret != 0 */
        int flags = 0;
        if ((flags = mbedtls_ssl_get_verify_result(&m_cntx.ssl_ctx)) != 0) {
            char verificationError[512] = {0};
            pr_debug("svr_cert varification failed:");
            mbedtls_x509_crt_verify_info(verificationError, sizeof(verificationError), "  ! ", flags);
            pr_debug("%s", verificationError);
        }
        
        ret = 0;
        m_connected = true;

        pr_debug("TLS socket connected and verified\r\n");

    }while(false);
    
    return (ret==0);
}

size_t TLSClient::write(uint8_t c)
{
    pr_debug("%c", c);
    return write(&c, 1);
}

size_t TLSClient::write(const uint8_t *data, size_t length)
{
    size_t written_len = 0;

    while (written_len < length) {
        int ret = mbedtls_ssl_write(&m_cntx.ssl_ctx, (unsigned char *)(data + written_len), (length - written_len));
        if (ret > 0) {
            // loop until we've sent everything
            written_len += ret;
            continue;
        } else if (ret == 0) {
            // cannot write anymore
            return written_len;
        } else {
            // something goes wrong...
            pr_debug("mbedtls_ssl_write returns error 0x%x", ret);
            m_connected = false;
            return 0; /* Connnection error */
        }
    }

    return written_len;
}

int TLSClient::available()
{
    // if there's a cached peek byte, we have something available.
    if(-1 != m_peekByte)
    {
        return 1;
    }

    // check if the read buffer has something.
    int ret = mbedtls_ssl_read( &m_cntx.ssl_ctx, NULL, 0 );
    
    if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
    {    
        pr_debug("want read/write\r\n");
        // this is "normal" - we just need to wait for the data.
        return 0;
    }

    if( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
    {
        // host closes the connection, so we disconnect too.
        pr_debug("peer closed");
        stop();
        return 0;
    }   

    if( ret < 0 )
    {
        // something goes wrong, disconnect.
        pr_debug("failed\n  ! mbedtls_ssl_read returned %d\n\n", ret );
        stop();
        return 0;
    }

    int len = mbedtls_ssl_get_bytes_avail(&m_cntx.ssl_ctx);
    return len;
}

int TLSClient::read()
{
    // is there a cached peek?
    if(-1 != m_peekByte)
    {
        int ret = m_peekByte;
        m_peekByte = -1;
        return ret;
    }

    // read from read buffer
    uint8_t c = 0;
    if(1 == read(&c, 1))
    {
        return c;
    }
    else
    {
        return -1;
    }
}

int TLSClient::read(uint8_t *buf, size_t size)
{
    if(NULL == buf || size == 0)
    {
        return 0;
    }

    // is there a cached peek?
    if(-1 != m_peekByte)
    {
        // pop the cached peek byte and early return first.
        *buf = (uint8_t)m_peekByte;
        m_peekByte = -1;

        // we "read()" exactly 1 byte
        return 1;
    }

    // no cached peek byte, read from the read buffer.
    int ret = mbedtls_ssl_read(&m_cntx.ssl_ctx, buf, size);
    
    if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
    {    
        // this is "normal" - we just need to wait for the data. 
        pr_debug("want read/write\r\n");
        return -1;
    }

    if( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
    {
        // host closes the connection.
        pr_debug("peer closed");
        stop();
        return -1;
    }   

    if( ret < 0 )
    {
        // mbedTLS error condition
        pr_debug("failed\n  ! mbedtls_ssl_read returned %d\n\n", ret );
        stop();
        return -1;
    }

    return ret;
}

int TLSClient::peek()
{
    // check if we already cached.
    if(-1 != m_peekByte)
    {
        return m_peekByte;
    }

    // check if there's something in the read buffer
    if(!available())
    {
        // not available, return -1
        return -1;
    }

    // read 1 byte and cache it
    m_peekByte = read();
    return m_peekByte;
}

void TLSClient::flush()
{
    uint8_t buf[64] = {0};
    while(available())
    {
        if(0 >= read(buf, sizeof(buf)))
            return;
    }
}

void TLSClient::stop()
{
    // disconnect
    m_cntx.free();
    m_connected = false;
    m_peekByte = -1;
}

uint8_t TLSClient::connected()
{
    return m_connected;
}

TLSClient::operator bool()
{
    return m_connected;
}