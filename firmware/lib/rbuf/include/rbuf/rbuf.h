#ifndef _RBUF_H_
#define _RBUF_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <dpl/dpl.h>

typedef struct{
	uint8_t *buffer;
	size_t  size;
	size_t  head;
	size_t  tail;
	bool    full;
	struct dpl_sem sem;
}rbuf_t;

void rbuf_init(rbuf_t *rbuf, uint8_t *buf, size_t size);
void rbuf_reset(rbuf_t  *rbuf);

int rbuf_put(rbuf_t *rbuf, char data);
int rbuf_get(rbuf_t *rbuf, char *data);

bool rbuf_empty(rbuf_t *rbuf);
bool rbuf_full(rbuf_t *rbuf);
size_t rbuf_size(rbuf_t *rbuf);

#endif //_RBUF_H_