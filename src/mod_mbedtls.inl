#if defined(USE_MBEDTLS) // USE_MBEDTLS used with NO_SSL

#include "mbedtls/certs.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/debug.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/net.h"
#include "mbedtls/pk.h"
#include "mbedtls/platform.h"
#include "mbedtls/ssl.h"
#include "mbedtls/x509.h"
#include "mbedtls/x509_crt.h"
#include <string.h>

typedef mbedtls_ssl_context SSL;

typedef struct {
	mbedtls_ssl_config conf;         /* SSL configuration */
	mbedtls_x509_crt cert;           /* Certificate */
	mbedtls_ctr_drbg_context ctr;    /* Counter random generator state */
	mbedtls_entropy_context entropy; /* Entropy context */
	mbedtls_pk_context pkey;         /* Private key */
} SSL_CTX;


/* public api */
int mbed_sslctx_init(SSL_CTX *ctx, const char *crt);
void mbed_sslctx_uninit(SSL_CTX *ctx);
void mbed_ssl_close(mbedtls_ssl_context *ssl);
int mbed_ssl_accept(mbedtls_ssl_context **ssl,
                    SSL_CTX *ssl_ctx,
                    int *sock,
                    struct mg_context *phys_ctx);
int mbed_ssl_read(mbedtls_ssl_context *ssl, unsigned char *buf, int len);
int mbed_ssl_write(mbedtls_ssl_context *ssl, const unsigned char *buf, int len);

static void mbed_debug(void *context,
                       int level,
                       const char *file,
                       int line,
                       const char *str);
static int mbed_ssl_handshake(mbedtls_ssl_context *ssl);


int
mbed_sslctx_init(SSL_CTX *ctx, const char *crt)
{
	mbedtls_ssl_config *conf;
	int rc;

	if (ctx == NULL || crt == NULL) {
		return -1;
	}

	DEBUG_TRACE("%s", "Initializing MbedTLS SSL");
	mbedtls_entropy_init(&ctx->entropy);

	conf = &ctx->conf;
	mbedtls_ssl_config_init(conf);

	/* Set mbedTLS debug level by defining MG_CONFIG_MBEDTLS_DEBUG:
	 *   0 No debug = mbedTLS DEFAULT
	 *   1 Error (default if "DEBUG" is set for CivetWeb)
	 *	2 State change
	 *	3 Informational
	 *	4 Verbose
	 */
#if defined(DEBUG) || defined(MG_CONFIG_MBEDTLS_DEBUG)
#if defined(MG_CONFIG_MBEDTLS_DEBUG)
	mbedtls_debug_set_threshold(MG_CONFIG_MBEDTLS_DEBUG);
#else
	mbedtls_debug_set_threshold(1);
#endif
	mbedtls_ssl_conf_dbg(conf, mbed_debug, (void *)ctx);
#endif

	/* Initialize TLS key and cert */
	mbedtls_pk_init(&ctx->pkey);
	mbedtls_ctr_drbg_init(&ctx->ctr);
	mbedtls_x509_crt_init(&ctx->cert);

	rc = mbedtls_ctr_drbg_seed(&ctx->ctr,
	                           mbedtls_entropy_func,
	                           &ctx->entropy,
	                           (unsigned char *)"CivetWeb",
	                           strlen("CivetWeb"));
	if (rc != 0) {
		DEBUG_TRACE("TLS random seed failed (%i)", rc);
		return -1;
	}

	rc = mbedtls_pk_parse_keyfile(&ctx->pkey, crt, NULL);
	if (rc != 0) {
		DEBUG_TRACE("TLS parse key file failed (%i)", rc);
		return -1;
	}

	rc = mbedtls_x509_crt_parse_file(&ctx->cert, crt);
	if (rc != 0) {
		DEBUG_TRACE("TLS parse crt file failed (%i)", rc);
		return -1;
	}

	rc = mbedtls_ssl_config_defaults(conf,
	                                 MBEDTLS_SSL_IS_SERVER,
	                                 MBEDTLS_SSL_TRANSPORT_STREAM,
	                                 MBEDTLS_SSL_PRESET_DEFAULT);
	if (rc != 0) {
		DEBUG_TRACE("TLS set defaults failed (%i)", rc);
		return -1;
	}

	mbedtls_ssl_conf_rng(conf, mbedtls_ctr_drbg_random, &ctx->ctr);

	/* Set auth mode if peer cert should be verified */
	mbedtls_ssl_conf_authmode(conf, MBEDTLS_SSL_VERIFY_NONE);
	mbedtls_ssl_conf_ca_chain(conf, NULL, NULL);

	/* Configure server cert and key */
	rc = mbedtls_ssl_conf_own_cert(conf, &ctx->cert, &ctx->pkey);
	if (rc != 0) {
		DEBUG_TRACE("TLS cannot set certificate and private key (%i)", rc);
		return -1;
	}
	return 0;
}


void
mbed_sslctx_uninit(SSL_CTX *ctx)
{
	mbedtls_ctr_drbg_free(&ctx->ctr);
	mbedtls_pk_free(&ctx->pkey);
	mbedtls_x509_crt_free(&ctx->cert);
	mbedtls_entropy_free(&ctx->entropy);
	mbedtls_ssl_config_free(&ctx->conf);
}


int
mbed_ssl_accept(mbedtls_ssl_context **ssl,
                SSL_CTX *ssl_ctx,
                int *sock,
                struct mg_context *phys_ctx)
{
	int rc;
	*ssl = mg_calloc_ctx(1, sizeof(**ssl), phys_ctx);
	if (*ssl == NULL) {
		DEBUG_TRACE("TLS accept: malloc ssl failed (%i)", (int)sizeof(**ssl));
		return -1;
	}

	mbedtls_ssl_init(*ssl);
	mbedtls_ssl_setup(*ssl, &ssl_ctx->conf);
	mbedtls_ssl_set_bio(*ssl, sock, mbedtls_net_send, mbedtls_net_recv, NULL);
	rc = mbed_ssl_handshake(*ssl);
	if (rc != 0) {
		DEBUG_TRACE("TLS handshake failed (%i)", rc);
		mg_free(*ssl);
		*ssl = NULL;
		return -1;
	}

	DEBUG_TRACE("TLS connection %p accepted, state: %d", ssl, (*ssl)->state);
	return 0;
}


void
mbed_ssl_close(mbedtls_ssl_context *ssl)
{
	DEBUG_TRACE("TLS connection %p closed", ssl);
	mbedtls_ssl_close_notify(ssl);
	mbedtls_ssl_free(ssl);
	mg_free(ssl); /* mg_free for mg_calloc in mbed_ssl_accept */
}


static int
mbed_ssl_handshake(mbedtls_ssl_context *ssl)
{
	int rc;
	while ((rc = mbedtls_ssl_handshake(ssl)) != 0) {
		if (rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE
		    && rc != MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS) {
			break;
		}
	}

	DEBUG_TRACE("TLS handshake rc: %d, state: %d", rc, ssl->state);
	return rc;
}


int
mbed_ssl_read(mbedtls_ssl_context *ssl, unsigned char *buf, int len)
{
	int rc = mbedtls_ssl_read(ssl, buf, len);
	/* DEBUG_TRACE("mbedtls_ssl_read: %d", rc); */
	return rc;
}


int
mbed_ssl_write(mbedtls_ssl_context *ssl, const unsigned char *buf, int len)
{
	int rc = mbedtls_ssl_write(ssl, buf, len);
	/* DEBUG_TRACE("mbedtls_ssl_write: %d", rc); */
	return rc;
}


static void
mbed_debug(void *user_param,
           int level,
           const char *file,
           int line,
           const char *str)
{
	(void)level; /* Ignored. Limit is set using mbedtls_debug_set_threshold */
	(void)user_param; /* Ignored. User parameter (context) is set using
	                  mbedtls_ssl_conf_dbg */

	DEBUG_TRACE("mbedTLS DEBUG: file: [%s] line: [%d] str: [%s]",
	            file,
	            line,
	            str);
}

#endif /* USE_MBEDTLS */
