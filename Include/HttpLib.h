#ifndef HTTPLIB_H
#define HTTPLIB_H

#include <HttpStreamIface.h>
#include <stdbool.h>

///////////////////////////////////////////////////////////////////////////////
// HttpLib API return values.
#define HTTP_ERROR       (-1)
#define HTTP_SUCCESS     (0)

///////////////////////////////////////////////////////////////////////////////
// Library memory allocator and deallocator type.
typedef void*(*Allocator_t)(size_t);
typedef void(*Deallocator_t)(void*);

///////////////////////////////////////////////////////////////////////////////
// Enumeration of HTTP versions.
typedef enum HttpVersion {
    HTTP_10 = 0,
    HTTP_11
} HttpVersion;

///////////////////////////////////////////////////////////////////////////////
// Enumeration of HTTP methods.
typedef enum HttpMethod {
    GET = 0,
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
struct _HttpContext_t {
    // Stream handle used for communication.
    HttpStream_t Socket;
    // Global timeout.
    Timeout_ms Timeout;
    // Receive timeout.
    Timeout_ms RecvTimeout;
    // Socket connect timeout.
    Timeout_ms ConnectTimeout;
    // Flags.
    uint32_t Flags;
    // Response content length.
    uint32_t ContentLength;
    // Buffer for data.
    char* DataBuffer;
    size_t DataBufferSize;
    // Size of data currently stored in buffer.
    size_t DataInBuffer;
    // Current chunk size.
    size_t ChunkSize;
    // Bytes of chunk already read.
    size_t ChunkRead;
};
typedef struct _HttpContext_t HttpContext;

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

	///////////////////////////////////////////////////////////////////////////////
	// This method intializes request's header.
	// Arguments:
	// 1) Request method.
	// 2) Requested remote site.
	// 3) HTTP protocol version.
	// 4) Request buffer.
	// 5) Request buffer size.
	// Returns: Non-zero value on error.
	int32_t _HttpInitRequest(
        HttpMethod      method,
        const char*     site,
        HttpVersion     version,
        char*           buffer,
        size_t          bufferSize
    );

	///////////////////////////////////////////////////////////////////////////////
	// This function checks if request header is correctly terminated.
	// If not it completes header.
	// If request body is defined, then no completion is needed.
	// Arguments:
	// 1) Request buffer.
	// 2) Request buffer size.
	// Returns: request length.
	int32_t _HttpCompleteRequest(
        char*   buffer,
        size_t  bufferSize
    );

	///////////////////////////////////////////////////////////////////////////////
	// This method adds property to HTTP request.
	// If property is already, then old is replaced with new one.
	// Arguments:
	// 1) Property name (i.e. Host, Content-Type).
	// 2) Property value.
	// 3) Request buffer.
	// 4) Request buffer size.
	// Returns: Non-zero value on error.
	int32_t _HttpSetProperty(
        const char*     key,
        const char*     value,
        char*           buffer,
        size_t          bufferSize
    );
	// This function gets property value from request/response.
	// Returns non-zero value on error.
	// -1 : Property is not terminated correctly.
	// -2 : Property not found.
	int32_t _HttpGetProperty(
        const char*     key,
        char*           buffer,
        size_t          bufferSize,
        const char*     request
    );

	///////////////////////////////////////////////////////////////////////////////
	// This function completes request header and sets its body.
	// Arguments:
	// 1) Body content (null-terminated string).
	// 2) Request buffer.
	// 3) Request buffer size.
	// Returns: 
	// >= 0 : Request size.
	// < 0 : On error.
	int32_t _HttpSetRequestBody(
        const char*     body,
        char*           buffer,
        size_t          bufferSize
    );
	// This function sets body to raw data (can cantain zeroes).
	// Returns request size (in bytes). Important when sending binary data.
	int32_t _HttpSetRequestBodyRaw(
        const void*     body,
        size_t          bodySize,
        char*           buffer,
        size_t          bufferSize
    );

	///////////////////////////////////////////////////////////////////////////////
	// This function establishes connection with remote host, using given: url, port and SSL setting.
	// Requires valid HttpContext object passes as argument.
	// Arguments:
	// 1) URL.
	// 2) Remote host port number.
	// 3) SSL usage flag (0 - do not use, other - use ssl).
	// 4) Valid pointer to HttpContext.
	// Returns: Non-zero value on error.
	int32_t _HttpConnect(
        const char*     url,
        uint16_t        port,
        bool            ssl,
        HttpContext*    ctx
    );

	///////////////////////////////////////////////////////////////////////////////
	// This function disconnects from remote host.
	// Requires valid HttpContext pointer.
	// Arguments:
	// Valid pointer to HttpContext object.
	// Force disconnect flag.
	// Returns: Non-zero value on error.
	int32_t _HttpDisconnect(
        HttpContext*    ctx,
        bool            force
    );

	///////////////////////////////////////////////////////////////////////////////
	// This function sends HTTP request over established connection.
	// Requires valid HttpContext pointer.
	// Arguments:
	// 1) Request (null-terminated string).
	// 2) Valid HttpContext pointer.
	// Returns: Non-zero value on error.
	int32_t _HttpSend(
        const void*     data,
        size_t          dataSize,
        HttpContext*    ctx
    );

	///////////////////////////////////////////////////////////////////////////////
	int32_t _HttpRecv(
        void*           buffer,
        size_t          bufferSize,
        HttpContext*    ctx
    );

	///////////////////////////////////////////////////////////////////////////////
	// This function checks underlying socket status and returns:
	// 0 : Not connected.
	// Anything other: Connected.
	bool _HttpIsConnected(const HttpContext*);

	///////////////////////////////////////////////////////////////////////////////
	// This function is used to set library memory managment functions.
	// Memory allocation and free.
	// Arguments:
	// Allocator_t - valid pointer to function allocating memory.
	// Deallocator_t - valid pointer to function freeing allocated memory.
	// Returns: Non-zero value on error.
	int32_t _HttpSetMemoryInterface(Allocator_t, Deallocator_t);

    ///////////////////////////////////////////////////////////////////////////////
    int32_t _HttpSetStream(const HttpStreamIface_t*);

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
