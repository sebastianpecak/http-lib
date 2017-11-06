#include <HttpLib.h>
#include <string.h>
#include <stdio.h>
#include <VCSLib.h>
#include <stdlib.h>
#include <logsys.h>

///////////////////////////////////////////////////////////////////////////////
// Allocator used in code (default).
static Allocator_t MemAlloc = malloc;
// Deallocator used in code (default).
static Deallocator_t MemFree = free;

///////////////////////////////////////////////////////////////////////////////
static const char* MethodsText[] = {
	"GET",
	"HEAD",
	"POST",
	"PUT"
};

///////////////////////////////////////////////////////////////////////////////
static const char* VersionsText[] = {
	"1.0",
	"1.1"
};

///////////////////////////////////////////////////////////////////////////////
// Function for internal usage.
// Sets passes pointer to method string name.
static void _GetHttpMethodString(HttpMethod method, const char** methodText) {
	LOG_PRINTF(("_GetHttpMethodString() -> Start."));

	*methodText = MethodsText[method];
}

///////////////////////////////////////////////////////////////////////////////
static void _GetHttpVersionString(HttpVersion version, const char** versionText) {
	LOG_PRINTF(("_GetHttpVersionString() -> Start."));
	
	*versionText = VersionsText[version];
}

///////////////////////////////////////////////////////////////////////////////
int _HttpInitRequest(HttpMethod method, const char* site, HttpVersion version, char* request, int requestSize) {
	// Selected method.
	const char* methodString = NULL;
	// Selected version.
	const char* versionString = NULL;

	LOG_PRINTF(("_HttpInitRequest() -> Start."));

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
int _HttpCompleteRequest(char* request, int requestBufferSize) {
	size_t length = 0;

	LOG_PRINTF(("_HttpCompleteRequest() -> Start."));

	if (request && requestBufferSize > 0) {
		// Search for \r\n\r\n.
		if (strstr(request, "\r\n\r\n") == NULL) {
			// Check if last 2 characters are \r\n. If yes append 1 more pair.
			length = strlen(request);
			if (request[length - 2] == '\r' && request[length - 1] == '\n') {
				// Check if we can append.
				if (length + 2 <= requestBufferSize) {
					strcpy((request + length), "\r\n");
					return length + 2;
				}
			}
			// If not check if we can append 2x\r\n.
			else {
				if ((length + 4) <= requestBufferSize) {
					strcpy((request + length), "\r\n\r\n");
					return length + 4;
				}
			}
		}
		// Found. Probably header is terminated by 2xCRLF.
		return 0;
	}
	else
		return 0;



	/*if (request && requestBufferSize > 0) {
		// Seek for header terminator.
		if (strstr(request, HTTP_HEADER_TERMINATOR) == NULL)
			// Append terminator.
			strcat(request, HTTP_HEADER_TERMINATOR);
		// Return request length.
		return strlen(request);
	}
	// Invalid parameters, return 0 as request length.
	return 0;*/
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

	LOG_PRINTF(("_HttpSetProperty() -> Start."));

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
int _HttpGetProperty(const char* key, char* buffer, int bufferSize, const char* http) {
	const char* propertyFound = NULL;
	const char* propertyTerm = NULL;
	int valueSize = 0;
	int result = -2;

	// Seek for key.
	propertyFound = strstr(http, key);
	if (propertyFound != NULL) {
		// Find terminator.
		propertyTerm = strstr(propertyFound, "\r\n");
		// Check if we found property terminator.
		if (propertyTerm == NULL)
			// Property is not correct.
			result = -1;
		else {
			// Check if buffer is big enough to store property's value (1 is :).
			valueSize = (int)(propertyTerm - propertyFound - strlen(key) - 1);
			strncpy(buffer, (propertyFound + strlen(key) + 1), valueSize);
			// Set string terminator.
			buffer[valueSize] = '\0';
			// Set resuklt success.
			result = 0;
		}
	}

	return result;
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
			// Return request lenght.
			return strlen(request);
		}
		else
			return -2;
	}
	else
		return -1;
}

///////////////////////////////////////////////////////////////////////////////
int _HttpSetRequestBodyRaw(const void* bodyRaw, int bodySize, char* request, int requestSize) {
	// Generic text buffer.
	char buffer[16] = { 0 };
	// Request header length.
	int headerLength = 0;

	sprintf(buffer, "%d", bodySize);
	_HttpSetProperty("Content-Length", buffer, request, requestSize);
	_HttpCompleteRequest(request, requestSize);
	headerLength = strlen(request);
	memcpy(request + headerLength, bodyRaw, bodySize);
	// Return complete request length.
	return (headerLength + bodySize);
}

///////////////////////////////////////////////////////////////////////////////
int _HttpConnect(const char* url, unsigned short port, unsigned char ssl, HttpContext* httpContext) {
	// Result buffer.
	int result = 0;

	LOG_PRINTF(("_HttpConnect() -> Start."));

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
// This function is used internally for cleaning up connection context state.
// By state we understand flags and other request-response specific data.
static void _ResetConnectionContext(HttpContext* ctx) {
	ctx->ContentLength = 0;
	ctx->Flags = 0;
	ctx->DataInBuffer = 0;
	ctx->ChunkRead = 0;
}

///////////////////////////////////////////////////////////////////////////////
int _HttpDisconnect(HttpContext* httpContext, unsigned char force) {
	// Result buffer.
	int result = 0;

	LOG_PRINTF(("_HttpDisconnect() -> Start."));

	// Drop data buffer.
	if (httpContext->DataBuffer && httpContext->DataBufferSize > 0) {
		//free(httpContext->DataBuffer);
		MemFree(httpContext->DataBuffer);
		httpContext->DataBuffer = NULL;
		httpContext->DataBufferSize = 0;
	}
	_ResetConnectionContext(httpContext);
	// Disconnect from remote host.
	result = VCS_Disconnect(httpContext->VCSSessionHandle, httpContext->Timeout);
	if (!force && result != 0)
		return result;
	// End session with VCS.
	result = VCS_DropSession(&httpContext->VCSSessionHandle, httpContext->Timeout);
	if (!force && result != 0)
		return result;
	// Return success.
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
int _HttpSend(const void* request, int requestSize, HttpContext* httpContext) {
	char buffer[64] = { 0 };
	unsigned short dataRecv = 0;

	LOG_PRINTF(("_HttpSend() -> Start."));

	// If we had chunked transfer we have to check if we recieved ending chunk.
	if (httpContext->Flags & ENDING_CHUNK_REQUIRED) {
		do {
			_ReceiveChunkedTransfer(buffer, sizeof(buffer), httpContext, &dataRecv);
		} while (httpContext->Flags & ENDING_CHUNK_REQUIRED);
	}
	// We have to reset connection context to get rid of trash data.
	_ResetConnectionContext(httpContext);
	return VCS_TransmitRawData(httpContext->VCSSessionHandle, request, requestSize, httpContext->Timeout);
}

///////////////////////////////////////////////////////////////////////////////
// This function converts hex string to integer value.
// As parameters it takes: string pointer and its length (path of big buffer).
// Returns converted value.
static int _HexToInt(const char* hexString, unsigned int hexStringLength) {
	// Null-terminated copy of hexString.
	char* hexZeroString = NULL;
	// Result buffer.
	int result = 0;

	// Allocate buffer for copy of hexString (null-terminated).
	//hexZeroString = malloc(hexStringLength + 1);
	hexZeroString = MemAlloc(hexStringLength + 1);
	// Check for errors.
	if (hexZeroString == NULL)
		// Return error.
		return -1;
	// Set string terminator.
	hexZeroString[hexStringLength] = '\0';
	// Copy string.
	strncpy(hexZeroString, hexString, hexStringLength);
	// Convert value.
	result = strtol(hexZeroString, NULL, 16);
	// Free buffer.
	//free(hexZeroString);
	MemFree(hexZeroString);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
// This function extracts properties values from response's header.
// If any additional property's value is needed, code should be added here.
static void _ExtractResponseProperties(const char* buffer, HttpContext* ctx) {
	// Buffer for property's value.
	char propertyValue[128] = { 0 };
	// Result buffer.
	int result = 0;

	// Get Content-Length.
	result = _HttpGetProperty("Content-Length", propertyValue, sizeof(propertyValue), buffer);
	if (result == 0)
		ctx->ContentLength = (size_t)atoi(propertyValue);
	// Get Transfer-Encoding.
	result = _HttpGetProperty("Transfer-Encoding", propertyValue, sizeof(propertyValue), buffer);
	// If we have chunked transfer, set flag.
	if (strstr(propertyValue, "chunked") != NULL)
		ctx->Flags |= TRANSFER_CHUNKED;
}

///////////////////////////////////////////////////////////////////////////////
// This function finds last occurence of given value.
static const char* _FindLast(const char* source, const char* value) {
	const char* last = NULL;
	const char* current = NULL;

	for (;;) {
		current = strstr(source, value);
		if (current == NULL)
			return last;
		else {
			source = current + 2;
			last = current;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// This function is responsible for receiving complete response header.
// It parses properties and modifies HttpContext configuration.
// Returns:
// >= 0 : Data length left in buffer (part of response body).
// < 0 : Error value.
static int _ReadHeader(HttpContext* ctx) {
	// Size of data received from VCS.
	unsigned short dataReceived = 0;
	// Status buffer.
	int result = 0;
	// Header terminator.
	char* headerEnd = NULL;
	// Pointer to last full property that has been read.
	// Data following that point will be moved to buffer's beginning.
	char* lastFullProperty = NULL;
	// Data left in buffer.
	size_t dataLeft = 0;
	// Buffer offset (used when some data left in buffer).
	size_t bufferOffset = 0;

	LOG_PRINTF(("_ReadHeader() ->"));

	// Start receiving data from VCS.
	do {
		result = VCS_RecieveRawData(ctx->VCSSessionHandle, (unsigned char*)(ctx->DataBuffer + bufferOffset), ctx->DataBufferSize - bufferOffset, &dataReceived, ctx->RecvTimeout);
		// Check for errors.
		if (result != 0) {
			LOG_PRINTF(("_ReadHeader() -> VCS_RecieveRawData() call returned: %d.", result));
			break;
		}
		// Try to extract all required response's properties.
		_ExtractResponseProperties(ctx->DataBuffer, ctx);
		// Check if we recieved complete header (\r\n\r\n).
		headerEnd = strstr(ctx->DataBuffer, HTTP_HEADER_TERMINATOR);
		// If we received complete http header.
		if (headerEnd != NULL) {
			LOG_PRINTF(("_ReadHeader() -> Found header terminator."));
			// We set complete header flag.
			ctx->Flags |= HEADER_RECEIVED;
			// We calculate how much data is body.
			bufferOffset = (dataReceived + bufferOffset - (unsigned short)(headerEnd - ctx->DataBuffer)) - 4;
			// Now we shift data that belongs to body at buffer's beginning.
			memmove(ctx->DataBuffer, (headerEnd + 4), bufferOffset);
		}
		// We have just another header part.
		else {
			// Try to locate last complete property in this part.
			// Next property (which is not complete) will be moved to buffer's beginning.
			lastFullProperty = (char*)_FindLast(ctx->DataBuffer, "\r\n");
			// If we could not locate last complete property that means out buffer is too small.
			if (lastFullProperty == NULL) {
				LOG_PRINTF(("_ReadHeader() -> Buffer too small to receive response header."));
				// Buffer to small.
				result = -1;
				break;
			}
			// If lastFullProperty is last 2 charatcres of buffer, we have to keep it.
			if ((ctx->DataBuffer + dataReceived + bufferOffset - lastFullProperty) == 2) {
				// 2 as we keep 2 charactres.
				bufferOffset = 2;
				ctx->DataBuffer[0] = '\r';
				ctx->DataBuffer[1] = '\n';
			}
			// Otherwise we omit \r\n as it indicates end of property.
			// We move what left in buffer.
			else {
				// Calculate size of data left in buffer.
				bufferOffset = ctx->DataBufferSize - (lastFullProperty - ctx->DataBuffer) - 2;
				memmove(ctx->DataBuffer, lastFullProperty + 2, bufferOffset);
			}
		}
	} while (!(ctx->Flags & HEADER_RECEIVED));

	// Set how much data left in buffer.
	ctx->DataInBuffer = bufferOffset;
	return result;
}

///////////////////////////////////////////////////////////////////////////////
static int _ReceiveChunkedTransfer(char* buffer, int bufferSize, HttpContext* ctx, unsigned short* dataReceived) {
	// Result buffer.
	int result = 0;
	// Chunk size terminator pointer.
	char* chunkTerminator = NULL;
	unsigned int toRecv = 0;
	// Number of bytes recieved in current call.
	unsigned int dataRecvTotal = 0;
	// Size of data received by VCS_RecieveRawData.
	*dataReceived = 0;

	// Check if we are reading chunk right now.
	// If not we have to find chunk size first.
	if (!(ctx->Flags & READING_CHUNK)) {
		// If we have no data in DataBuffer we have to fill it up.
		// [Value]\r\n.
		if (ctx->DataInBuffer < 3) {
			result = VCS_RecieveRawData(
				ctx->VCSSessionHandle,
				(unsigned char*)(ctx->DataBuffer + ctx->DataInBuffer),
				(ctx->DataBufferSize - ctx->DataInBuffer),
				dataReceived,
				ctx->RecvTimeout
			);
			// Check for error.
			if (result != 0) {
				LOG_PRINTF(("_ReceiveChunkedTransfer() -> VCS_RecieveRawData call returned: %d.", result));
				return result;
			}
			// Increase DataInBuffer by dataReceived.
			ctx->DataInBuffer += *dataReceived;
		}
		// It could happen that first 2 characters will be \r\n, so we have to omit them.
		if (ctx->DataBuffer[0] == '\r' && ctx->DataBuffer[1] == '\n') {
			memmove(ctx->DataBuffer, (ctx->DataBuffer + 2), (ctx->DataInBuffer - 2));
			ctx->DataInBuffer -= 2;
		}
		// Here we have buffer filled up with data.
		// Find chunk size terminator (\r\n).
		chunkTerminator = strstr(ctx->DataBuffer, "\r\n");
		// Check if we found terminator.
		if (chunkTerminator == NULL) {
			LOG_PRINTF(("_ReceiveChunkedTransfer() -> Did not find chunk terminator in buffer."));
			return -1;
		}
		// Save new chunk size.
		ctx->ChunkSize = _HexToInt(ctx->DataBuffer, (chunkTerminator - ctx->DataBuffer));
		// Throw out chunk size from DataBuffer.
		ctx->DataInBuffer = (ctx->DataInBuffer - (chunkTerminator - ctx->DataBuffer) - 2);
		memmove(ctx->DataBuffer, (chunkTerminator + 2), ctx->DataInBuffer);

		// If chunk size is 0, then we received ending chunk.
		if (ctx->ChunkSize == 0) {
			// Reset flag that ending chunk is required.
			ctx->Flags &= ~ENDING_CHUNK_REQUIRED;
			LOG_PRINTF(("_ReceiveChunkedTransfer() -> Received ending chunk."));
			*dataReceived = 0;
			return 0;
		}
		else {
			LOG_PRINTF(("_ReceiveChunkedTransfer() -> Start chunk: %d.", ctx->ChunkSize));
			// Set flags.
			ctx->Flags |= ENDING_CHUNK_REQUIRED | READING_CHUNK;
		}
	}

	// Calculate how much data we should receive in this call (what left for current chunk).
	toRecv = (ctx->ChunkSize - ctx->ChunkRead);
	// If it is more than we can store in output buffer then we limit use bufferSize as limit.
	toRecv = (toRecv > bufferSize ? bufferSize : toRecv);

	LOG_PRINTF(("ToRecv: %d", toRecv));

	// If we have anything in buffer we have to receive it first.
	if (ctx->DataInBuffer > toRecv) {
		// We receive toRecv.
		memcpy(buffer, ctx->DataBuffer, toRecv);
		// Shift data in buffer.
		memmove(ctx->DataBuffer, (ctx->DataBuffer + toRecv), (ctx->DataInBuffer - toRecv));
		// Decrease DataInBuffer size.
		ctx->DataInBuffer -= toRecv;
		// Set dataReceived.
		dataRecvTotal = toRecv;
		// Increase ChunkRead.
		ctx->ChunkRead += toRecv;
		// Reset toRecv as we already received all allowed bytes.
		toRecv = 0;
	}
	else if (ctx->DataInBuffer > 0 && ctx->DataInBuffer < toRecv) {
		// Get everything from DataBuffer.
		memcpy(buffer, ctx->DataBuffer, ctx->DataInBuffer);
		// Decrease toRecv by DataInBuffer value.
		toRecv -= ctx->DataInBuffer;
		// Set dataReceived.
		dataRecvTotal = ctx->DataInBuffer;
		// Increase ChunkRead.
		ctx->ChunkRead += ctx->DataInBuffer;
		// Reset data in buffer.
		ctx->DataInBuffer = 0;
	}

	// Use VCS_Recv to receive all left toRecv bytes.
	result = VCS_RecieveRawData(
		ctx->VCSSessionHandle,
		(unsigned char*)(buffer + dataRecvTotal),
		toRecv,
		dataReceived,
		ctx->RecvTimeout
	);
	// Increase dataRecvTotal by dataReceived (in current call).
	dataRecvTotal += *dataReceived;
	// Increase ChunkRead as global state variable.
	ctx->ChunkRead += *dataReceived;

	// Check if we completly read current chunk.
	if (ctx->ChunkRead == ctx->ChunkSize) {
		LOG_PRINTF(("_ReceiveChunkedTransfer() -> Chunk of size: %d received completely.", ctx->ChunkSize));
		// Reset receiving flag.
		ctx->Flags &= ~READING_CHUNK;
		// Reset chunk read.
		ctx->ChunkRead = 0;
	}

	// Save datReceived.
	*dataReceived = dataRecvTotal;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// This function receives http data in plain encoding.
// Returns:
// 0 : On success.
// < 0 : On error.
static int _ReceivePlainTransfer(char* buffer, int bufferSize, HttpContext* ctx, unsigned short* dataRecieved) {
	int result = 0;
	int dataToBeCopied = 0;

	// If we have data in DataBuffer, we receive it first.
	if (ctx->DataInBuffer > 0) {
		// Calculate how much data we can recieve at once.
		dataToBeCopied = (ctx->DataInBuffer > bufferSize ? bufferSize : ctx->DataInBuffer);
		// Copy to output buffer.
		memcpy(buffer, ctx->DataBuffer, dataToBeCopied);
		// Decrease DataInBuffer value by dataToBeCopied, as it is already received.
		ctx->DataInBuffer -= dataToBeCopied;
		// Shift data in buffer by those we received.
		memmove(ctx->DataBuffer, (ctx->DataBuffer + dataToBeCopied), ctx->DataInBuffer);
		// Set how much data we already copied.
		*dataRecieved = dataToBeCopied;
		// If dataToBeCopied is less than buffer size, we get additional data from VCS.
		if (dataToBeCopied < bufferSize) {
			result = VCS_RecieveRawData(
				ctx->VCSSessionHandle,
				(unsigned char*)(buffer + dataToBeCopied),
				(bufferSize - dataToBeCopied),
				dataRecieved,
				ctx->RecvTimeout
			);
			// We have to add dataToBeCopied value, as we are completing buffer.
			*dataRecieved += dataToBeCopied;
		}
	}
	// There is no data in DataBuffer, so we simply receive new data from VCS.
	else
		result = VCS_RecieveRawData(ctx->VCSSessionHandle, (unsigned char*)buffer, bufferSize, dataRecieved, ctx->RecvTimeout);

	// Return result.
	return result;
}

///////////////////////////////////////////////////////////////////////////////
// Here we specify how much data we can recieve.
int _HttpRecv(char* buffer, int bufferSize, HttpContext* ctx) {
	// Result buffer.
	int result = 0;
	// How much data has been received.
	unsigned short dataReceived = 0;

	// Check if we have data buffer already created.
	if (ctx->DataBuffer == NULL) {
		// Create buffer.
		//ctx->DataBuffer = malloc(HTTP_BUFFER_SIZE);
		ctx->DataBuffer = MemAlloc(HTTP_BUFFER_SIZE);
		// Check for error.
		if (ctx->DataBuffer == NULL) {
			LOG_PRINTF(("_HttpRecv() -> Could not create DataBuffer of size: %d", HTTP_BUFFER_SIZE));
			return 0;
		}
		// Save buffer size.
		ctx->DataBufferSize = HTTP_BUFFER_SIZE;
	}

	// Check if we already received response header.
	if (!(ctx->Flags & HEADER_RECEIVED)) {
		// We receive header.
		result = _ReadHeader(ctx);
		// Check for error.
		if (result < 0) {
			LOG_PRINTF(("_HttpRecv() -> _ReadHeader() call returned: %d.", result));
			// For safety we return 0, as no data were received.
			return 0;
		}
		LOG_PRINTF(("_HttpRecv() -> After receiving header we have %d bytes left in buffer.", ctx->DataInBuffer));
	}

	// Here we are sure that response header has been received.
	// Depending on transfer type (chunked or not) we use specific function.
	if (ctx->Flags & TRANSFER_CHUNKED)
		result = _ReceiveChunkedTransfer(buffer, bufferSize, ctx, &dataReceived);
	else
		result = _ReceivePlainTransfer(buffer, bufferSize, ctx, &dataReceived);

	// Check for errors.
	if (result != 0) {
		LOG_PRINTF(("_HttpRecv() -> _Receive*Transfer() call returned: %d.", result));
		return 0;
	}
	// There was no error.
	else {
		LOG_PRINTF(("_HttpRecv() -> Return: %d", dataReceived));
		// And we return dataRecieved.
		return (int)dataReceived;
	}
}

///////////////////////////////////////////////////////////////////////////////
int _HttpIsConnected(const HttpContext* ctx) {
	unsigned short socketStatus = 0;
	int result = 0;

	// Check if session handle is correct.
	if (ctx->VCSSessionHandle >= 0 && ctx->VCSSessionHandle < 15) {
		result = VCS_GetSocketStatus(ctx->VCSSessionHandle, &socketStatus, ctx->Timeout);
		// Check for error.
		if (result != 0)
			return 0;
		// If there was no error, we return actual socket status.
		return (int)socketStatus;
	}
	// Invalid handle.
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
int _HttpSetMemoryInterface(Allocator_t alloc, Deallocator_t free) {
	// Both arguments must be valid pointers.
	if (alloc && free) {
		MemAlloc = alloc;
		MemFree = free;
		return 0;
	}
	// Invalid arguments, return error.
	return -1;
}
