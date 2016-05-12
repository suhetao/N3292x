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

#ifndef _queue_h
#define _queue_h

#include <stdlib.h>
#include <memory.h>  //memset
#include <inttypes.h> //
#include <linux/stddef.h> //NULL
#include <stdbool.h>

#include "buffer.h"

typedef struct queue_t
{
	buffer** volatile buf;
	volatile uint32_t* volatile tag;
	volatile int32_t capacity;
	volatile int32_t size;
	volatile int32_t size2;
	volatile uint32_t mask;
	volatile uint32_t head;
	volatile uint32_t tail;
} queue;

#define DEFAULT_QUEUE_SIZE (16)

int32_t queue_init(queue *q, uint32_t size);
queue *queue_create();
int32_t queue_push(queue *q, uint8_t* data, int32_t size);
int32_t queue_pop(queue *q, buffer **out);
int32_t queue_destroy(queue *q);
__inline int32_t queue_is_empty(queue *q) { return (q->size <= 0);}

#endif  //_queue_h
