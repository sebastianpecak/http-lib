#ifndef HTTPLIB_H
#define HTTPLIB_H

/*
	HttpLib delivers tools for HTTP communication.
*/

///////////////////////////////////////////////////////////////////////////////
// Enumeration of HTTP versions.
enum HttpVersion {
	HTTP_10,
	HTTP_11
};

///////////////////////////////////////////////////////////////////////////////
// Enumeration of HTTP methods.
enum HttpMethod {
	GET,
	HEAD,
	POST,
	PUT
};

///////////////////////////////////////////////////////////////////////////////
// This struct contains data required for HTTP communication.
// Is used as Http interface argument.
struct HttpContext {
	// Session handle used for VCS communication.
	unsigned short VCSSessionHandle;
	// Global timeout setting.
	long Timeout;
};

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

///////////////////////////////////////////////////////////////////////////////
// This function completes request header and sets its body.
// Arguments:
// 1) Body content (null-terminated string).
// 2) Request buffer.
// 3) Request buffer size.
extern int _HttpSetRequestBody(const char*, char*, int);

///////////////////////////////////////////////////////////////////////////////
// This function establishes connection with remote host, using given: url, port and SSL setting.
// Requires valid HttpContext object passes as argument.
// Arguments:
// 1) URL.
// 2) Remote host port number.
// 3) SSL usage flag (0 - do not use, other - use ssl).
// 4) Valid pointer to HttpContext.
extern int _HttpConnect(const char*, unsigned short, unsigned char, HttpContext*);

///////////////////////////////////////////////////////////////////////////////
extern int _HttpDisconnect(HttpContext*);

///////////////////////////////////////////////////////////////////////////////
extern int _HttpSend(const char*, const HttpContext*);

///////////////////////////////////////////////////////////////////////////////
extern int _HttpRecv(char*, int, HttpContext*);

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
#define HttpDisconnect _HttpDisconnect

#endif	// HTTPLIB_H