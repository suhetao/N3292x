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

#include "buffer.h"

static int32_t buffer_grow(buffer* buf, size_t newsize)
{
	size_t fixsize = buf->capacity;
	if(!fixsize){
		fixsize = DEFAULT_BUFFER_SIZE;
	}

	do{
		fixsize <<= 1;
	}
	while(fixsize < newsize);
	buf->data = (uint8_t *)realloc(buf->data, fixsize);
	if(!buf->data){
		return -1;
	}
	else{
		buf->capacity = fixsize;
		return 0;
	}
}

buffer* buffer_create()
{
	buffer* buf = (buffer*)malloc(sizeof(buffer));
	if(!buf){
		return NULL;
	}
	buf->capacity = 0;
	buf->written = 0;
	buf->data = NULL;
	return buf;
}

int32_t buffer_destroy(buffer* buf)
{
	if(buf){
		if(!buf->data){
			free(buf->data);
			buf->data = NULL;
		}
		buf->capacity = 0;
		buf->written = 0;
		free(buf);
		buf = NULL;
	}
	return 0;
}

int32_t buffer_append(buffer* buf, uint8_t byte)
{
	if (buf->capacity - buf->written == 0){
		if (buffer_grow(buf, buf->capacity + 1) < 0){
			return -1;
		}
	}
	buf->data[buf->written++] = byte;
	return 0;
}

int32_t buffer_write(buffer* buf, uint8_t *data, size_t len)
{
	if (buf->capacity - buf->written < len){
		if (buffer_grow(buf, buf->capacity + len) < 0){
			return -1;
		}
	}
	memcpy(buf->data + buf->written, data, len);
	buf->written += len;
	return 0;
}

int32_t buffer_overwrite(buffer* buf, uint8_t *data, size_t len)
{
	if (buf->capacity < len){
		if (buffer_grow(buf, len) < 0){
			return -1;
		}
	}
	memcpy(buf->data, data, len);
	buf->written = len;
	return 0;
}
