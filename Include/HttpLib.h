#ifndef HTTPLIB_H
#define HTTPLIB_H

// For size_t.
#include <string.h>

///////////////////////////////////////////////////////////////////////////////
// Library defines.
// This value is used for buffering response header data.
#define HTTP_BUFFER_SIZE				256
#define HTTP_HEADER_TERMINATOR			"\r\n\r\n"
#define HTTP_PROPERTY_DELIMITER         "\r\n"

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

	/*
		HttpLib delivers tools for HTTP communication.
	*/

	///////////////////////////////////////////////////////////////////////////////
	// Library memory allocator and deallocator type.
	typedef void*(*Allocator_t)(size_t);
	typedef void(*Deallocator_t)(void*);

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
		// This flag says if we are reading chunk and we know its size.
		// Or if we have to first find chunk size.
		READING_CHUNK = 2,
		HEADER_RECEIVED = 4,
		ENDING_CHUNK_REQUIRED = 8
	} HttpFlags;

	///////////////////////////////////////////////////////////////////////////////
	typedef struct HttpContext {
		// Session handle used for VCS communication.
		unsigned short VCSSessionHandle;
		// Global timeout setting (in miliseconds).
		unsigned short Timeout;
		// Receive timeout (in seconds).
		unsigned short RecvTimeout;
		// Flags.
		unsigned int Flags;
		// Response content length.
		unsigned int ContentLength;
		// Buffer for data.
		char* DataBuffer;
		unsigned int DataBufferSize;
		// Size of data currently stored in buffer.
		unsigned int DataInBuffer;
		// Current chunk size.
		unsigned int ChunkSize;
		// Bytes of chunk already read.
		unsigned int ChunkRead;
		// Socket connect timeout (in miliseconds).
		unsigned short ConnectTimeout;
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
	// Returns: request length.
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
	// Returns: 
	// >= 0 : Request size.
	// < 0 : On error.
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
	extern int _HttpSend(const void*, int, HttpContext*);

	///////////////////////////////////////////////////////////////////////////////
	extern int _HttpRecv(char*, int, HttpContext*);

	///////////////////////////////////////////////////////////////////////////////
	// This function checks underlying socket status and returns:
	// 0 : Not connected.
	// Anything other: Connected.
	extern int _HttpIsConnected(const HttpContext*);

	///////////////////////////////////////////////////////////////////////////////
	// This function is used to set library memory managment functions.
	// Memory allocation and free.
	// Arguments:
	// Allocator_t - valid pointer to function allocating memory.
	// Deallocator_t - valid pointer to function freeing allocated memory.
	// Returns: Non-zero value on error.
	extern int _HttpSetMemoryInterface(Allocator_t, Deallocator_t);

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
#define HttpSetRequestBody _HttpSetRequestBody

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

///////////////////////////////////////////////////////////////////////////////
#ifdef HttpIsConnected
#undef HttpIsConnected
#endif
#define HttpIsConnected _HttpIsConnected

#endif	// HTTPLIB_H
