#ifndef HTTPSTREAMIFACE_H
#define HTTPSTREAMIFACE_H

#include <stdint.h>
#include <string.h>

////////////////////////////////////////////////////////////////////
typedef int HttpStream_t;
// Timeout in seconds.
typedef unsigned long Timeout_s;
// Timeout in miliseconds.
typedef unsigned long Timeout_ms;
// Timeout in microseconds.
typedef unsigned long Timeout_us;

////////////////////////////////////////////////////////////////////
struct _HttpStreamIface_t {
    int32_t(*Open)(HttpStream_t*, const char* url, uint16_t port, Timeout_ms);
    int32_t(*Close)(HttpStream_t*, Timeout_ms);
    int32_t(*Read)(HttpStream_t, void* buffer, size_t bufferSize, size_t* dataRead, Timeout_ms);
    int32_t(*Write)(HttpStream_t, const void* data, size_t dataSize, size_t* dataWritten, Timeout_ms);
};
typedef struct _HttpStreamIface_t HttpStreamIface_t;

#endif  // HTTPSTREAMIFACE_H
