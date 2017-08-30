#include <HttpLib.h>
#include <string.h>
#include <stdio.h>
#include <VCSLib.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////
// Function for internal usage.
// Sets passes pointer to method string name.
static void _GetHttpMethodString(HttpMethod method, const char** methodText) {
	switch (method) {
	case GET:
		*methodText = "GET";
		break;
	case HEAD:
		*methodText = "HEAD";
		break;
	case POST:
		*methodText = "POST";
		break;
	case PUT:
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
	case HTTP_10:
		*versionText = "1.0";
		break;
	case HTTP_11:
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
	// Request ContentLength.
	int length = 0;

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
int _HttpSend(const unsigned char* request, int requestSize, const HttpContext* httpContext) {
	return VCS_TransmitRawData(httpContext->VCSSessionHandle, request, requestSize, httpContext->Timeout);
}

///////////////////////////////////////////////////////////////////////////////
// Code copied from old Http library.
// Checks if buffer contains complete Http response header (\r\n\r\n).
// If yes, then it reads following values: Content-Length, Transfer-Encoding.
// Returns header length (if buffer does not contain complete header, then returns 0).
//static int _ReadHttpHeader(HttpContext* handle, unsigned char* buffer, unsigned short size) {
static int _ReadHttpHeader(HttpContext* httpCtx, char* buffer, int bufferSize) {
	// Pointers used for data extraction.
	unsigned char * data_start = NULL;
	//unsigned char * tmp0_p = NULL;
	//unsigned char * tmp1_p = NULL;
	int header_size = 0;
	char textBuffer[32] = { 0 };

	// Check if we deal with complete header.
	data_start = strstr(buffer, "\r\n\r\n");
	//if ((data_start = (unsigned char *)strstr((const char *)buffer, "\r\n\r\n")) != NULL)
	//{
	if (data_start != NULL) {
		// Konieczne zostawienie 2 aby odnaleŸæ chunk.
		data_start += 2;
		// Calculate header size.
		header_size = (int)(data_start - buffer);
		// Wype³nijmy strukture.
		//tmp0_p = strstr(buffer, "Content-Length: ");
		//if ((tmp0_p = (unsigned char *)strstr((const char *)buffer, "Content-Length: ")) != NULL)
		//{
		//if (tmp0_p != NULL) {
			// Wyci¹gamy content-length.
			//tmp1_p = (unsigned char *)strstr((const char *)tmp0_p, " ");
			//handle->ContentLength = atoi((const char *)tmp1_p);
		//}
		_HttpGetProperty("Content-Length", textBuffer, sizeof(textBuffer), buffer);
		httpCtx->ContentLength = atoi(textBuffer);
		//if ((tmp0_p = (unsigned char *)strstr((const char *)buffer, "Transfer-Encoding: chunked")) != NULL)
		//{
			//handle->chunked = 1;
		//}
		//else
		//{
			//handle->chunked = 0;
			//header_size += 2; // ok nie ma chunka wiêc usuñmy
		//}
		_HttpGetProperty("Transfer-Encoding", textBuffer, sizeof(textBuffer), buffer);
		// If we have chunked transfer.
		if (strstr(textBuffer, "chunked") != NULL)
			//httpCtx->chunked = 1;
			httpCtx->Flags |= HTTP_CHUNKED;
		else {
			//httpCtx->chunked = 0;
			httpCtx->Flags &= ~HTTP_CHUNKED;
			// Ok nie ma chunka wiêc usuñmy.
			header_size += 2;
		}
		// przesuñmy dane do pocz¹tku !!! i zwróæmy rozmiar bez nag³ówka
		memmove(buffer, (buffer + header_size), (bufferSize - header_size - 8));
		// Return header length.
		return header_size;
	}
	// Http header is not completed. Return length = 0.
	else
		return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Code copied from old Http library.
// Converts hex value (chunk length) to int.
static int _HexToInt(HttpContext* handle, int* chunk_len) {
	unsigned short chunk_val = 0;
	char *p = NULL;
	unsigned char * possition_n = NULL;
	unsigned char * possition_m = NULL;
	int i = 0;

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
}

///////////////////////////////////////////////////////////////////////////////
int _HttpRecv(char* buffer, int bufferSize, HttpContext* httpCtx) {
	unsigned short rc_size = 0;
	int rc_total_size = 0;
	int header_size = 0;
	int chunk_len = 0;
	unsigned char trash_buffer[4] = { 0 };
	unsigned short i = 0;
	unsigned char header_buffer[HTTP_MAX_HEADER] = { 0 };
	int result = 0;

	// pocz¹tek !!! nie mamy jeszcze nic wiêc  najpierw nag³ówek
	if (httpCtx->rc_parser == 0) {
		// zbierajmy nag³ówek !!! (powinniœmy zacz¹æ od rozmiaru minimalnego np 100b
		// i przerwaæ jeœli za du¿o powtórzeñ
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
		// Jeœli dzielone - na pocz¹tku zawsze bêdzie chunk.
		while (rc_total_size < bufferSize) {

			if (httpCtx->chunk_tmp_size > bufferSize) {
				memcpy(buffer + rc_total_size, httpCtx->chunk_tmp_buffer, bufferSize);
				httpCtx->rc_parser += bufferSize;
				httpCtx->chunk_size -= bufferSize;
				rc_total_size += bufferSize;
				httpCtx->chunk_tmp_size -= bufferSize;
			}

			// Jesteœmy w trakcie chunk'a i w nim jest wystarczaj¹co du¿o danych.
			if (httpCtx->chunk_size >= bufferSize - rc_total_size) {
				result = VCS_RecieveRawData(httpCtx->VCSSessionHandle, (unsigned char*)buffer + rc_total_size, bufferSize - rc_total_size, &rc_size, httpCtx->Timeout);
				if (rc_size <= 0)
					break;
				httpCtx->chunk_size -= rc_size;
				httpCtx->rc_parser += rc_size;
				rc_total_size += rc_size;

				// jeœli zakoñczyliœmy chunk ... sprawdŸmy jaki jest nastêpny!!
			}
			// Jesteœmy na etapie chunka'a ale jest w nim ma³o danych.
			else if (httpCtx->chunk_size > 0) {
				result = VCS_RecieveRawData(httpCtx->VCSSessionHandle, (unsigned char*)buffer + rc_total_size, httpCtx->chunk_size, &rc_size, httpCtx->Timeout);
				if (rc_size <= 0)
					break;
				httpCtx->chunk_size -= rc_size;
				httpCtx->rc_parser += rc_size;
				rc_total_size += rc_size;
			}
			// Jesteœmy przed chunk'iem.
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

		return rc_total_size;
	}
	// Poczatek transmisji z nag³ówkiem ale bez chunków.
	else {
		VCS_RecieveRawData(httpCtx->VCSSessionHandle, (unsigned char*)buffer, bufferSize, &rc_size, httpCtx->Timeout);
		if (rc_size > 0)
			httpCtx->rc_parser += rc_size;
		return rc_size;
	}
}
