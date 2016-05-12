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

#include "queue.h"

int32_t queue_init(queue *q, uint32_t size)
{
	uint32_t i;
	if(!size){
		return -1;
	}
	if(size & (size - 1)){
		// Round up to the next highest power of 2
		for(i = 1; i < (sizeof size) << 3; i <<= 1){
			size |= size >> i;
		}
		++size;
	}
	q->buf = NULL;
	q->tag = NULL;
	q->capacity = size;
	q->size = 0;
	q->size2 = 0;
	q->mask = size - 1;
	q->head = 0;
	q->tail = 0;

	q->buf = (buffer **)calloc(size, sizeof(buffer*));
	if(NULL == q->buf){
		return -1;
	}
	for(i = 0; i < size; i++){
		q->buf[i] = buffer_create();
		if(NULL == q->buf[i]){

			return -1;
		}
	}
	q->tag = (uint32_t *)calloc((size >> 5) + 1, sizeof(uint32_t));
	if(NULL == q->tag){
		for(i = 0; i < size; i++){
			buffer_destroy(q->buf[i]);
		}
		free(q->buf);
		q->buf = NULL;
		return -1;
	}
	//clear tag
	for(i = 0; i < (size >> 5) + 1; ++i){
		q->tag[i] = 0;
	}
	return 0;
}

queue *queue_create()
{
	queue *q = NULL;
	q = (queue *)malloc(sizeof(queue));
	if(NULL == q){
		return NULL;
	}
	if(queue_init(q, DEFAULT_QUEUE_SIZE) < 0){
		free(q);
		q = NULL;
	}
	return q;
}

int32_t queue_push(queue *q, uint8_t* in, int32_t size)
{
	int32_t tail;
	volatile uint32_t tag, *ptag;
	uint32_t tmp;

	if(__sync_fetch_and_add(&q->size2, 1) >= q->capacity){
		__sync_sub_and_fetch(&q->size2, 1);
		return -1;
	}
	tail = __sync_fetch_and_add(&q->tail, 1);
	tail &= q->mask;
	ptag = &q->tag[tail >> 5];
	tag = *ptag;
	tmp = 1U << (tail & 31);
	while(tag & tmp);
	//
	buffer_overwrite(q->buf[tail], in, size);
	//
	__sync_or_and_fetch(ptag, tmp);
	__sync_add_and_fetch(&q->size, 1);
	return 0;
}

int32_t queue_pop(queue *q, buffer **out)
{
	int32_t head;
	volatile uint32_t tag, *ptag;
	uint32_t tmp;

	if(__sync_fetch_and_sub(&q->size, 1) <= 0){
		__sync_add_and_fetch(&q->size, 1);
		return -1;
	}
	head = __sync_fetch_and_add(&q->head, 1);
	head &= q->mask;
	ptag = &q->tag[head >> 5];
	tag = *ptag;
	tmp = 1U << (head & 31);
	while(tmp & ~tag);
	//
	*out = q->buf[head];
	//
	__sync_and_and_fetch(ptag, ~(1U<<(head & 31)));
	__sync_sub_and_fetch(&q->size2, 1);

	return 0;
}

int32_t queue_clear(queue *q)
{
	int32_t i;

	if(q->buf){
		for(i = 0; i < q->size; i++){
			if(q->buf[i]){
				buffer_destroy(q->buf[i]);
				q->buf[i] = NULL;
			}
		}
		free(q->buf);
		q->buf = NULL;
	}
	if(q->tag){
		free((void*)q->tag);
		q->tag = NULL;
	}
	q->capacity = 0;
	q->size = 0;
	q->size2 = 0;
	q->mask = 0;
	q->head = 0;
	q->tail = 0;

	return 0;
}

int32_t queue_destroy(queue *q)
{
	int32_t i;

	if(q){
		queue_clear(q);
		free(q);
	}
	return 0;
}
