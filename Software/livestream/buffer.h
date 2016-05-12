/*
The MIT License (MIT)
Copyright (c) 2015-? suhetao
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef BUFFER_H
#define BUFFER_H

#include <inttypes.h>
#include <stdlib.h>
#include <memory.h>

typedef struct buffer_t{
	size_t capacity;
	size_t written;
	uint8_t *data;
} buffer;

#define DEFAULT_BUFFER_SIZE (1024)

#define BUFFER_RESET(b) ((b)->written = 0)
#define BUFFER_APPEND(b, byte) ((b)->data[(b)->written++] = (byte))
#define BUFFER_GET(b) ((b)->data)
#define BUFFER_CAPACITY(b) ((b)->capacity)
#define BUFFER_SIZE(b) ((b)->written)
#define BUFFER_REMAIN(b) ((b)->capacity - (b)->written)
#define BUFFER_IS_EMPTY(b) ((b)->written == 0)
#define BUFFER_IS_FULL(b) ((b)->written == (b)->capacity)

buffer* buffer_create();
int32_t buffer_destroy(buffer* buf);
int32_t buffer_append(buffer* buf, uint8_t byte);
int32_t buffer_write(buffer* buf, uint8_t *data, size_t len);
int32_t buffer_overwrite(buffer* buf, uint8_t *data, size_t len);

#endif
