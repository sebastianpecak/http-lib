#ifndef HTTPLIB_H
#define HTTPLIB_H

// Comstants copied from old HTTP library.
#define HTTP_MAX_HEADER		300
#define HTTP_MIN_HEADER		140
#define HTTP_MAX_CHUNK_LEN	8
#define PARSER_TMP_BUFFER	32
#define HTTP_MAX_CHUNK_SIZE	4096
// Http flags values.
#define HTTP_CHUNKED		(1)
#define HTTP_ENDING_CHUNK	(1 << 1)

#define HTTP_BUFFER_SIZE 256
#define HTTP_HEADER_TERMINATOR "\r\n\r\n"

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

	/*
		HttpLib delivers tools for HTTP communication.
	*/

	///////////////////////////////////////////////////////////////////////////////
	// Enumeration of HTTP versions.
	typedef enum HttpVersion {
		HTTP_10,
		HTTP_11
	} HttpVersion;

	///////////////////////////////////////////////////////////////////////////////
	// Enumeration of HTTP methods.
	typedef enum HttpMethod {
		GET,
		HEAD,
		POST,
		PUT
	} HttpMethod;

	///////////////////////////////////////////////////////////////////////////////
	typedef enum HttpFlags {
		TRANSFER_CHUNKED = 1,
		ENDING_CHUNK = 2,
		HEADER_RECEIVED = 4
	} HttpFlags;

	///////////////////////////////////////////////////////////////////////////////
	typedef struct HttpContextNew {
		// Session handle used for VCS communication.
		unsigned short VCSSessionHandle;
		// Global timeout setting.
		unsigned long Timeout;
		// Flags.
		unsigned int Flags;
		// Response content length.
		size_t ContentLength;
	} HttpContextNew;

	///////////////////////////////////////////////////////////////////////////////
	// This struct contains data required for HTTP communication.
	// Is used as Http interface argument.
	typedef struct HttpContext {
		// Session handle used for VCS communication.
		unsigned short VCSSessionHandle;
		// Global timeout setting.
		long Timeout;

		// This part is copied from old _csh struct.
		// Corrected int fileds to unsigned int not to get -1 value on output.
		unsigned short rc_data;				/**< przechowuje iloœæ danych w buforze roboczym */
		unsigned short rc_pos;				/**< przechowuje pozycje w buforze roboczym */
		unsigned char index;				/**< indek w tablicy po³azeñ ComService */
		unsigned short chunk_size; 			/**< rozmiar bie¿¹cego chunka (post-parser)*/
		unsigned short init_chunk_size; 	/**< rozmiar chunka inicjalizujacego (post-parser)*/
		// Http context flags.
		unsigned char Flags;
		// Response body length.
		unsigned long ContentLength;
		long rc_size;						/**< iloœc danych odebranych */
		long tr_size;						/**< iloœæ danych wys³anych */
		long rc_parser;						/**< rozmiar danych odebranych i przetworzonych przez zewnêtrzny parser protoko³u warstwy aplikacji np http (post-parser)*/
		unsigned char * parser_buffer; 		/**< wskaŸnik na bufor dodatkowy roboczy (post-parser)*/
		unsigned short parser_buffer_size; 	/**<  bierz¹cy rozmiar danych w dodatkowym buforze roboczym (post-parser)*/
		unsigned char chunk_tmp_buffer[PARSER_TMP_BUFFER];		/**< tymczasowy bufor roboczy (post-parser)*/
		unsigned short chunk_tmp_size;		/** rozmiar bie¿¹cego chunka (post-parser)*/
	} HttpContext;

	///////////////////////////////////////////////////////////////////////////////
	// This method intializes request's header.
	// Arguments:
	// 1) Request method.
	// 2) Requested remote site.
	// 3) HTTP protocol version.
	// 4) Request buffer.
	// 5) Request buffer size.
	// Returns: Non-zero value on error.
	extern int _HttpInitRequest(HttpMethod, const char*, HttpVersion, char*, int);

	///////////////////////////////////////////////////////////////////////////////
	// This function checks if request header is correctly terminated.
	// If not it completes header.
	// If request body is defined, then no completion is needed.
	// Arguments:
	// 1) Request buffer.
	// 2) Request buffer size.
	// Returns: Non-zero value on error.
	extern int _HttpCompleteRequest(char*, int);

	///////////////////////////////////////////////////////////////////////////////
	// This method adds property to HTTP request.
	// If property is already, then old is replaced with new one.
	// Arguments:
	// 1) Property name (i.e. Host, Content-Type).
	// 2) Property value.
	// 3) Request buffer.
	// 4) Request buffer size.
	// Returns: Non-zero value on error.
	extern int _HttpSetProperty(const char*, const char*, char*, int);
	// This function gets property value from request/response.
	// Returns non-zero value on error.
	// -1 : Property is not terminated correctly.
	// -2 : Property not found.
	extern int _HttpGetProperty(const char*, char*, int, const char*);

	///////////////////////////////////////////////////////////////////////////////
	// This function completes request header and sets its body.
	// Arguments:
	// 1) Body content (null-terminated string).
	// 2) Request buffer.
	// 3) Request buffer size.
	// Returns: Non-zero value on error.
	extern int _HttpSetRequestBody(const char*, char*, int);
	// This function sets body to raw data (can cantain zeroes).
	// Returns request size (in bytes). Important when sending binary data.
	extern int _HttpSetRequestBodyRaw(const void*, int, char*, int);

	///////////////////////////////////////////////////////////////////////////////
	// This function establishes connection with remote host, using given: url, port and SSL setting.
	// Requires valid HttpContext object passes as argument.
	// Arguments:
	// 1) URL.
	// 2) Remote host port number.
	// 3) SSL usage flag (0 - do not use, other - use ssl).
	// 4) Valid pointer to HttpContext.
	// Returns: Non-zero value on error.
	extern int _HttpConnect(const char*, unsigned short, unsigned char, HttpContext*);

	///////////////////////////////////////////////////////////////////////////////
	// This function disconnects from remote host.
	// Requires valid HttpContext pointer.
	// Arguments:
	// Valid pointer to HttpContext object.
	// Force disconnect flag.
	// Returns: Non-zero value on error.
	extern int _HttpDisconnect(HttpContext*, unsigned char);

	///////////////////////////////////////////////////////////////////////////////
	// This function sends HTTP request over established connection.
	// Requires valid HttpContext pointer.
	// Arguments:
	// 1) Request (null-terminated string).
	// 2) Valid HttpContext pointer.
	// Returns: Non-zero value on error.
	extern int _HttpSend(const void*, int, const HttpContext*);

	///////////////////////////////////////////////////////////////////////////////
	extern int _HttpRecv(char*, int, HttpContext*);

#ifdef __cplusplus
}
#endif	// __cplusplus

///////////////////////////////////////////////////////////////////////////////
// Set global names for library interface functions.
#ifdef HttpInitRequest
#undef HttpInitRequest
#endif
#define HttpInitRequest _HttpInitRequest

///////////////////////////////////////////////////////////////////////////////
#ifdef HttpSetProperty
#undef HttpSetProperty
#endif
#define HttpSetProperty _HttpSetProperty

///////////////////////////////////////////////////////////////////////////////
#ifdef HttpGetProperty
#undef HttpGetProperty
#endif
#define HttpGetProperty _HttpGetProperty

///////////////////////////////////////////////////////////////////////////////
#ifdef HttpSetRequestBody
#undef HttpSetRequestBody
#endif	// HttpSetRequestBody
#ifndef HTTP_3DES_ENCRYPTION
#define HttpSetRequestBody _HttpSetRequestBody
#else
#define HttpSetRequestBody
#endif	// HTTP_3DES_ENCRYPTION

///////////////////////////////////////////////////////////////////////////////
#ifdef HttpSend
#undef HttpSend
#endif
#define HttpSend _HttpSend

///////////////////////////////////////////////////////////////////////////////
#ifdef HttpRecv
#undef HttpRecv
#endif
#define HttpRecv _HttpRecv

///////////////////////////////////////////////////////////////////////////////
#ifdef HttpConnect
#undef HttpConnect
#endif
#define HttpConnect _HttpConnect

///////////////////////////////////////////////////////////////////////////////
#ifdef HttpDisconnect
#undef HttpDisconnect
#endif
#define HttpDisconnect(httpCtx) _HttpDisconnect(httpCtx, 0)

///////////////////////////////////////////////////////////////////////////////
#ifdef HttpDisconnectForce
#undef HttpDisconnectForce
#endif
#define HttpDisconnectForce(httpCtx) _HttpDisconnect(httpCtx, 1)

///////////////////////////////////////////////////////////////////////////////
#ifdef HttpSetRequestBodyRaw
#undef HttpSetRequestBodyRaw
#endif
#define HttpSetRequestBodyRaw _HttpSetRequestBodyRaw

///////////////////////////////////////////////////////////////////////////////
#ifdef HttpCompleteRequest
#undef HttpCompleteRequest
#endif
#define HttpCompleteRequest _HttpCompleteRequest

#endif	// HTTPLIB_H
