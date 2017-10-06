#include <HttpLib.h>
#include <string.h>
#include <stdio.h>
#include <VCSLib.h>
#include <stdlib.h>
#include <logsys.h>

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
int _HttpCompleteRequest(char* request, int requestSize) {
	// Request ContentLength.
	int length = 0;

	LOG_PRINTF(("_HttpCompleteRequest() -> Start."));

	if (request && requestSize > 0) {
		// Search for \r\n\r\n.
		if (strstr(request, "\r\n\r\n") == NULL) {
			// Check if last 2 characters are \r\n. If yes append 1 more pair.
			length = strlen(request);
			if (request[length - 2] == '\r' && request[length - 1] == '\n') {
				// Check if we can append.
				if (length + 2 <= requestSize)
					strcpy((request + length), "\r\n");
				else
					return -2;
			}
		}
		// Found. Probably header is terminated by 2xCRLF.
		return 0;
	}
	else
		return -1;
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
/*int _HttpGetProperty(const char* key, char* buffer, int bufferSize, const char* http) {
	const char* propertyFound = NULL;
	const char* propertyTerm = NULL;
	int valueSize = 0;

	LOG_PRINTF(("_HttpGetProperty() -> Start."));

	// Seek for key.
	propertyFound = strstr(http, key);
	if (propertyFound != NULL) {
		// Find terminator.
		propertyTerm = strstr(propertyFound, "\r\n");
		// Check if buffer is big enough to store property's value (1 is :).
		valueSize = (int)(propertyTerm - propertyFound - strlen(key) - 1);
		strncpy(buffer, (propertyFound + strlen(key) + 1), valueSize);
		// Set string terminator.
		buffer[valueSize] = '\0';
	}

	return 0;
}*/

///////////////////////////////////////////////////////////////////////////////
int _HttpGetProperty(const char* key, char* buffer, int bufferSize, const char* http) {
	const char* propertyFound = NULL;
	const char* propertyTerm = NULL;
	int valueSize = 0;
	int result = -2;

	LOG_PRINTF(("_HttpGetProperty() -> Start."));

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

	LOG_PRINTF(("_HttpSetRequestBody() -> Start."));

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
int _HttpSetRequestBodyRaw(const void* bodyRaw, int bodySize, char* request, int requestSize) {
	// Generic text buffer.
	char buffer[16] = { 0 };
	// Request header length.
	int headerLength = 0;

	LOG_PRINTF(("_HttpSetRequestBodyRaw() -> Start."));

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
int _HttpDisconnect(HttpContext* httpContext, unsigned char force) {
	// Result buffer.
	int result = 0;

	LOG_PRINTF(("_HttpDisconnect() -> Start."));

	// Drop data buffer.
	if (httpContext->DataBuffer && httpContext->DataBufferSize > 0) {
		free(httpContext->DataBuffer);
		httpContext->DataBuffer = NULL;
		httpContext->DataBufferSize = 0;
	}
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
// This function is used internally for cleaning up connection context state.
// By state we understand flags and other request-response specific data.
static void _ResetConnectionContext(HttpContext* ctx) {
	ctx->ContentLength = 0;
	ctx->Flags = 0;
	ctx->DataInBuffer = 0;
}

///////////////////////////////////////////////////////////////////////////////
int _HttpSend(const void* request, int requestSize, HttpContext* httpContext) {
	LOG_PRINTF(("_HttpSend() -> Start."));

	// We have to reset connection context to get rid of trash data.
	_ResetConnectionContext(httpContext);
	return VCS_TransmitRawData(httpContext->VCSSessionHandle, request, requestSize, httpContext->Timeout);
}

///////////////////////////////////////////////////////////////////////////////
// Code copied from old Http library.
// Converts hex value (chunk length) to int.
/*static int _HexToInt(HttpContext* handle, int* chunk_len) {
	unsigned short chunk_val = 0;
	char *p = NULL;
	unsigned char * possition_n = NULL;
	unsigned char * possition_m = NULL;
	int i = 0;

	LOG_PRINTF(("_HexToInt() -> Start."));

	chunk_val = (unsigned short)strtol((const char *)handle->chunk_tmp_buffer, (char**)&p, 16);

	if (chunk_val <= 0)
		return 0;
	// This limitaion comes from old Http library (for omni terminals especially).
	// Probaly will be removed in the future.
	if (chunk_val > HTTP_MAX_CHUNK_SIZE)
		return -1;

	while ((handle->chunk_tmp_buffer[i] != '\r') && (i < HTTP_MAX_CHUNK_LEN))
		++i;

	possition_n = handle->chunk_tmp_buffer + i;
	i = 1;

	while ((possition_n[i] != '\r') && (i < HTTP_MAX_CHUNK_LEN))
		++i;

	if (i <= HTTP_MAX_CHUNK_LEN)
		possition_m = possition_n + i + 2;
	else
		return -1;

	*chunk_len = (int)(possition_m - possition_n);

	return chunk_val;
}

///////////////////////////////////////////////////////////////////////////////
static int _ReadNextChunkSize(HttpContext* handle, unsigned char* buffer, int size, int* chunk_len) {
	unsigned short rc_size = 0;
	int rc_size_total = 0;
	unsigned short size_orig = size;
	unsigned short pattern_count = 0;
	int init_offset = 0;
	char pattern[] = "\r\n";
	handle->parser_buffer_size = 0L;

	LOG_PRINTF(("_ReadNextChunkSize() -> Start."));

	// Buffer for text hex value.
	memset(handle->chunk_tmp_buffer, 0, HTTP_MAX_CHUNK_LEN);

	if (handle->rc_parser == 0) {
		init_offset = 2;
		handle->chunk_tmp_buffer[0] = '\r';
		handle->chunk_tmp_buffer[1] = '\n';
	}

	while ((rc_size_total + init_offset) < HTTP_MAX_CHUNK_LEN) {
		int i = 0;
		VCS_RecieveRawData(handle->VCSSessionHandle, (handle->chunk_tmp_buffer + init_offset + rc_size_total), 1, &rc_size, handle->Timeout);
		if (rc_size <= 0)
			return 0;

		rc_size_total += rc_size;
		pattern_count = 0;

		for (i = 0; i < (rc_size_total + init_offset) - 1; ++i) {
			if (memcmp(handle->chunk_tmp_buffer + i, pattern, 2) == 0)
				pattern_count++;
		}

		if (pattern_count == 2)
			break;
	}

	if (pattern_count != 2)
		return 0;

	// ok mamy chunka w buforze...
	handle->chunk_size = _HexToInt(handle, chunk_len);
	if (handle->chunk_size <= 0)
		return 0;
	//chunk startowy
	if (handle->rc_parser == 0)
		handle->init_chunk_size = handle->chunk_size;
	// Return chunk size.
	return handle->chunk_size;
}*/

///////////////////////////////////////////////////////////////////////////////
// Code copied from old Http library.
// Checks if buffer contains complete Http response header (\r\n\r\n).
// If yes, then it reads following values: Content-Length, Transfer-Encoding.
// Returns header length (if buffer does not contain complete header, then returns 0).
//static int _ReadHttpHeader(HttpContext* handle, unsigned char* buffer, unsigned short size) {
/*static int _ReadHttpHeader(HttpContext* httpCtx, char* buffer, int bufferSize) {
	// Pointers used for data extraction.
	unsigned char * data_start = NULL;
	int header_size = 0;
	char textBuffer[32] = { 0 };

	LOG_PRINTF(("_ReadHttpHeader() -> Start."));

	// Check if we deal with complete header.
	data_start = strstr(buffer, "\r\n\r\n");
	if (data_start != NULL) {
		// Konieczne zostawienie 2 aby odnale�� chunk.
		data_start += 2;
		// Calculate header size.
		header_size = (int)(data_start - buffer);
		// Wype�nijmy strukture.
		_HttpGetProperty("Content-Length", textBuffer, sizeof(textBuffer), buffer);
		httpCtx->ContentLength = atoi(textBuffer);
		_HttpGetProperty("Transfer-Encoding", textBuffer, sizeof(textBuffer), buffer);
		// If we have chunked transfer.
		if (strstr(textBuffer, "chunked") != NULL)
			httpCtx->Flags |= HTTP_CHUNKED;
		else {
			httpCtx->Flags &= ~HTTP_CHUNKED;
			// Ok nie ma chunka wi�c usu�my.
			header_size += 2;
		}
		// przesu�my dane do pocz�tku !!! i zwr��my rozmiar bez nag��wka
		memmove(buffer, (buffer + header_size), (bufferSize - header_size - 8));
		// Return header length.
		return header_size;
	}
	// Http header is not completed. Return length = 0.
	else
		return 0;
}*/

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
		LOG_PRINTF(("'%s'", ctx->DataBuffer));
		headerEnd = strstr(ctx->DataBuffer, HTTP_HEADER_TERMINATOR);
		// If we received complete http header.
		if (headerEnd != NULL) {
			LOG_PRINTF(("Found terminator."));
			// We set complete header flag.
			ctx->Flags |= HEADER_RECEIVED;
			// We calculate how much data is body.
			bufferOffset = (dataReceived - (unsigned short)(headerEnd - ctx->DataBuffer)) - 4;
			// Now we shift data that belongs to body at buffer's beginning.
			memmove(ctx->DataBuffer, (headerEnd + 4), bufferOffset);
			// Here we gonna leave loop.
			LOG_PRINTF(("Leaving loop."));
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
				// Clean rest of buffer.
				//memset(buffer + 2, 0, bufferSize - 2);
			}
			// Otherwise we omit \r\n as it indicates end of property.
			// We move what left in buffer.
			else {
				// Calculate size of data left in buffer.
				bufferOffset = ctx->DataBufferSize - (lastFullProperty - ctx->DataBuffer) - 2;
				//memset(buffer + bufferOffset, 0, bufferSize - bufferOffset);
				memmove(ctx->DataBuffer, lastFullProperty + 2, bufferOffset);
			}
		}
	} while (!(ctx->Flags & HEADER_RECEIVED));

	// Set how much data left in buffer.
	ctx->DataInBuffer = bufferOffset;
	return result;
}

///////////////////////////////////////////////////////////////////////////////
static int _ReceiveChunkedTransfer(char* buffer, int bufferSize, HttpContext* ctx) {
	LOG_PRINTF(("_ReceiveChunkedTransfer() ->"));
}

///////////////////////////////////////////////////////////////////////////////
// Returns >= 0 for number of bytes recieved.
// < 0 for error.
static int _ReceivePlainTransfer(char* buffer, int bufferSize, const HttpContext* ctx) {
	int result = 0;
	unsigned short dataRecieved = 0;

	result = VCS_RecieveRawData(ctx->VCSSessionHandle, (unsigned char*)buffer, bufferSize, &dataRecieved, ctx->RecvTimeout);
	// Check for errors.
	if (result != 0) {
		LOG_PRINTF(("_ReceivePlainTransfer() -> VCS_RecieveRawData() call returned: %d.", result));
		return 0;
	}
	// No error, return number of bytes recieved.
	return (int)dataRecieved;
}

///////////////////////////////////////////////////////////////////////////////
// Here we specify how much data we can recieve.
int _HttpRecv(char* buffer, int bufferSize, HttpContext* ctx) {
	// Result buffer.
	int result = 0;
	// How much data has been received.
	int dataReceived = 0;
	// How much data will be copied to output buffer.
	int dataToBeCopied = 0;

	// Check if we have data buffer already created.
	if (ctx->DataBuffer == NULL) {
		// Create buffer.
		ctx->DataBuffer = malloc(HTTP_BUFFER_SIZE);
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
		LOG_PRINTF(("Data left in buffer: %d", ctx->DataInBuffer));
	}

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
		// If dataToBeCopied is less than buffer size, we get additional data from VCS.
		if (dataToBeCopied < bufferSize) {
			if (ctx->Flags & TRANSFER_CHUNKED)
				dataReceived = _ReceiveChunkedTransfer((buffer + dataToBeCopied), (bufferSize - dataToBeCopied), ctx);
			else
				dataReceived = _ReceivePlainTransfer((buffer + dataToBeCopied), (bufferSize - dataToBeCopied), ctx);
		}
	}
	// There is no data in DataBuffer, so we simply receive new data from VCS.
	else {
		if (ctx->Flags & TRANSFER_CHUNKED)
			dataReceived = _ReceiveChunkedTransfer(buffer, bufferSize, ctx);
		else
			dataReceived = _ReceivePlainTransfer(buffer, bufferSize, ctx);
	}

	LOG_PRINTF(("_HttpRecv() -> Return: %d", (dataToBeCopied + dataReceived)));
	// And we return dataRecieved plus that that might be in buffer before (result).
	return (dataToBeCopied + dataReceived);
}

///////////////////////////////////////////////////////////////////////////////
/*int __HttpRecv(char* buffer, int bufferSize, HttpContext* httpCtx) {
	unsigned short rc_size = 0;
	int rc_total_size = 0;
	int header_size = 0;
	int chunk_len = 0;
	unsigned char trash_buffer[4] = { 0 };
	unsigned short i = 0;
	unsigned char header_buffer[HTTP_MAX_HEADER] = { 0 };
	int result = 0;

	LOG_PRINTF(("_HttpRecv() -> Start."));

	// pocz�tek !!! nie mamy jeszcze nic wi�c  najpierw nag��wek
	if (httpCtx->rc_parser == 0) {
		// zbierajmy nag��wek !!! (powinni�my zacz�� od rozmiaru minimalnego np 100b
		// i przerwa� je�li za du�o powt�rze�
		result = VCS_RecieveRawData(httpCtx->VCSSessionHandle, header_buffer, HTTP_MIN_HEADER, &rc_size, httpCtx->Timeout);

		i += rc_size;

		if (rc_size <= 0)
			return rc_size;

		while (((header_size = _ReadHttpHeader(httpCtx, header_buffer, HTTP_MAX_HEADER)) == 0) && (i < HTTP_MAX_HEADER)) {
			result = VCS_RecieveRawData(httpCtx->VCSSessionHandle, header_buffer + i, 1, &rc_size, httpCtx->Timeout);
			if (rc_size <= 0)
				return rc_size;
			++i;
		}
	}
	// Zero rc.
	rc_size = 0;

	//if (httpCtx->chunked == 1) {
	if (httpCtx->Flags & HTTP_CHUNKED) {
		// Je�li dzielone - na pocz�tku zawsze b�dzie chunk.
		while (rc_total_size < bufferSize) {

			if (httpCtx->chunk_tmp_size > bufferSize) {
				memcpy(buffer + rc_total_size, httpCtx->chunk_tmp_buffer, bufferSize);
				httpCtx->rc_parser += bufferSize;
				httpCtx->chunk_size -= bufferSize;
				rc_total_size += bufferSize;
				httpCtx->chunk_tmp_size -= bufferSize;
			}

			// Jeste�my w trakcie chunk'a i w nim jest wystarczaj�co du�o danych.
			if (httpCtx->chunk_size >= bufferSize - rc_total_size) {
				result = VCS_RecieveRawData(httpCtx->VCSSessionHandle, (unsigned char*)buffer + rc_total_size, bufferSize - rc_total_size, &rc_size, httpCtx->Timeout);
				if (rc_size <= 0)
					break;
				httpCtx->chunk_size -= rc_size;
				httpCtx->rc_parser += rc_size;
				rc_total_size += rc_size;

				// je�li zako�czyli�my chunk ... sprawd�my jaki jest nast�pny!!
			}
			// Jeste�my na etapie chunka'a ale jest w nim ma�o danych.
			else if (httpCtx->chunk_size > 0) {
				result = VCS_RecieveRawData(httpCtx->VCSSessionHandle, (unsigned char*)buffer + rc_total_size, httpCtx->chunk_size, &rc_size, httpCtx->Timeout);
				if (rc_size <= 0)
					break;
				httpCtx->chunk_size -= rc_size;
				httpCtx->rc_parser += rc_size;
				rc_total_size += rc_size;
			}
			// Jeste�my przed chunk'iem.
			if (httpCtx->chunk_size == 0) {
				if (_ReadNextChunkSize(httpCtx, (unsigned char*)buffer, bufferSize - rc_total_size, (int*)&chunk_len) <= 0)
					break;
				if (httpCtx->chunk_size == 0) {
					httpCtx->Flags |= HTTP_ENDING_CHUNK;
					// Read trash from vsocket.
					VCS_RecieveRawData(httpCtx->VCSSessionHandle, trash_buffer, sizeof(trash_buffer), &rc_size, httpCtx->Timeout);
				}
			}
			rc_size = 0;
		}

		LOG_PRINTF((buffer));

		return rc_total_size;
	}
	// Poczatek transmisji z nag��wkiem ale bez chunk�w.
	else {
		VCS_RecieveRawData(httpCtx->VCSSessionHandle, (unsigned char*)buffer, bufferSize, &rc_size, httpCtx->Timeout);
		if (rc_size > 0)
			httpCtx->rc_parser += rc_size;

		LOG_PRINTF((buffer));
		return rc_size;
	}
}*/
