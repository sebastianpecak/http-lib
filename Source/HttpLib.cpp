#include <HttpLib.h>
#include <string.h>
#include <stdio.h>
#include <VCSLib.h>

///////////////////////////////////////////////////////////////////////////////
// Function for internal usage.
// Sets passes pointer to method string name.
static void _GetHttpMethodString(HttpMethod method, const char** methodText) {
	switch (method) {
	case HttpMethod::GET:
		*methodText = "GET";
		break;
	case HttpMethod::HEAD:
		*methodText = "HEAD";
		break;
	case HttpMethod::POST:
		*methodText = "POST";
		break;
	case HttpMethod::PUT:
		*methodText = "PUT";
		break;
	default:
		*methodText = "";
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
static void _GetHttpVersionString(HttpVersion version, const char** versionText) {
	switch (version) {
	case HttpVersion::HTTP_10:
		*versionText = "1.0";
		break;
	case HttpVersion::HTTP_11:
		*versionText = "1.1";
		break;
	default:
		*versionText = "";
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
int _HttpInitRequest(HttpMethod method, const char* site, HttpVersion version, char* request, int requestSize) {
	// Selected method.
	const char* methodString = NULL;
	// Selected version.
	const char* versionString = NULL;

	// Check request buffer validity.
	if (request && requestSize > 0) {
		// Reset buffer.
		memset(request, 0, requestSize);
		// Resolve method.
		_GetHttpMethodString(method, &methodString);
		// Resolve version.
		_GetHttpVersionString(version, &versionString);
		// Format request.
		sprintf(
			request,
			"%s %s HTTP/%s\r\n",
			methodString,
			site,
			versionString
		);
		// Return success.
		return 0;
	}
	// Return invalid buffer.
	else
		return -1;
}

///////////////////////////////////////////////////////////////////////////////
int _HttpCompleteRequest(char* request, int requestSize) {

}

///////////////////////////////////////////////////////////////////////////////
int _HttpSetProperty(const char* key, const char* value, char* request, int requestSize) {
	// Current request length.
	int requestLength = 0;
	// Length of new property.
	int propertyLength = 0;
	// Pointer to substring found in request.
	char* oldProperty = NULL;
	// Pointer to substring tail (part that comes after old property).
	char* propertyTail = NULL;

	if (request && requestSize > 0) {
		// Check if in request there is no such as property already set.
		oldProperty = strstr(request, key);
		if (oldProperty != NULL) {
			// We have replace old property with new one.
			// Search for tail (properties that will be kept).
			propertyTail = strstr(oldProperty, "\r\n") + 2;
			// Move tail.
			strcpy(oldProperty, propertyTail);
		}
		// Get request length.
		requestLength = strlen(request);
		// 'PropertyName: Value\r\n'.
		propertyLength = strlen(key) + 2 + strlen(value) + 2;
		// Check if there is enough free space in buffer to add new property.
		// We have to consider that request ends with 2x\r\n (4 bytes).
		if ((requestSize - requestLength - propertyLength) >= 2) {
			// Add new property.
			sprintf(
				(request + requestLength),
				"%s: %s\r\n",
				key,
				value
			);
			// Return success.
			return 0;
		}
		else
			// There is no free space to add property.
			return -2;
	}
	else
		return -1;
}

///////////////////////////////////////////////////////////////////////////////
int _HttpSetRequestBody(const char* body, char* request, int requestSize) {
	int requestLength = 0;
	int bodyLength = strlen(body);
	// Result buffer.
	int result = 0;
	// Text buffer.
	char textBuffer[16] = { 0 };

	// Check parameter validity.
	if (request && requestSize > 0) {
		// Before adding body to request, we have to check if Content-Length property is present.
		if (strstr(request, "Content-Length") == NULL) {
			// Add that field.
			sprintf(textBuffer, "%d", bodyLength);
			result = _HttpSetProperty("Content-Length", textBuffer, request, requestSize);
			if (result != 0)
				return result;
		}
		// Request length.
		requestLength = strlen(request);
		// Cehck if buffer is big enough to store body (2 bytes for header termination).
		if ((requestSize - requestLength - bodyLength - 2) >= 0) {
			// Set request body.
			sprintf(
				(request + requestLength),
				"\r\n%s",
				body
			);
			// Return success.
			return 0;
		}
		else
			return -2;
	}
	else
		return -1;
}

///////////////////////////////////////////////////////////////////////////////
int _HttpConnect(const char* url, unsigned short port, unsigned char ssl, HttpContext* httpContext) {
	// Result buffer.
	int result = 0;

	// Intialize session with VCS.
	result = VCS_InitializeSession(&httpContext->VCSSessionHandle, httpContext->Timeout);
	// Check for error.
	if (result != 0)
		return result;
	// Connect to remote host.
	result = VCS_Connect(httpContext->VCSSessionHandle, url, port, ssl, httpContext->Timeout);
	// Check for error.
	if (result != 0)
		return result;
	// Return success.
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
int _HttpDisconnect(HttpContext* httpContext) {
	// Result buffer.
	int result = 0;

	// Disconnect from remote host.
	result = VCS_Disconnect(httpContext->VCSSessionHandle, httpContext->Timeout);
	if (result != 0)
		return result;
	// End session with VCS.
	result = VCS_DropSession(&httpContext->VCSSessionHandle, httpContext->Timeout);
	if (result != 0)
		return result;
	// Return success.
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
int _HttpSend(const char* request, const HttpContext* httpContext) {
	return VCS_TransmitRawData(httpContext->VCSSessionHandle, (unsigned char*)request, strlen(request), httpContext->Timeout);
}

///////////////////////////////////////////////////////////////////////////////
int _HttpRecv(char* buffer, int bufferSize, HttpContext* httpContext) {

}

int main() {
	char httpRequest[1024];
	HttpInitRequest(HttpMethod::POST, "/lisener3des.php", HttpVersion::HTTP_11, httpRequest, sizeof(httpRequest));
	HttpSetProperty("Host", "195.78.239.101", httpRequest, sizeof(httpRequest));
	HttpSetProperty("Content-Type", "data/binary", httpRequest, sizeof(httpRequest));
	// Open connection.
	HttpContext context;
	context.Timeout = 3000;
	HttpConnect("195.78.239.101", 2080, 0, &context);
	HttpSend(httpRequest, &context);
	// Cleanup buffer.
	memset(httpRequest, 0, sizeof(httpRequest));
	HttpRecv(httpRequest, sizeof(httpRequest), &context);
	HttpDisconnect(&context);
}
