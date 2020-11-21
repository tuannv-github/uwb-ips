#include <rbuf/rbuf.h>

void rbuf_init(rbuf_t *rbuf, uint8_t *buf, size_t size)
{
    dpl_sem_init(&rbuf->sem, 0x01);

    dpl_sem_pend(&rbuf->sem, DPL_TIMEOUT_NEVER);
	rbuf->buffer = buf;
	rbuf->size = size;
    rbuf->head = 0;
    rbuf->tail = 0;
    rbuf->full = false;
    dpl_sem_release(&rbuf->sem);
}

void rbuf_reset(rbuf_t *rbuf)
{
    dpl_sem_pend(&rbuf->sem, DPL_TIMEOUT_NEVER);
    rbuf->head = 0;
    rbuf->tail = 0;
    rbuf->full = false;
    dpl_sem_release(&rbuf->sem);
}

int rbuf_put(rbuf_t* rbuf, char data)
{
    if(!rbuf_full(rbuf))
    {   
        dpl_sem_pend(&rbuf->sem, DPL_TIMEOUT_NEVER);
        rbuf->buffer[rbuf->head] = data;
        rbuf->head = (rbuf->head + 1) % rbuf->size;
        rbuf->full = (rbuf->head == rbuf->tail);
        dpl_sem_release(&rbuf->sem);
        return 0;
    }
    return -1;
}

int rbuf_get(rbuf_t *rbuf, char *data)
{
    if(!rbuf_empty(rbuf))
    {
        dpl_sem_pend(&rbuf->sem, DPL_TIMEOUT_NEVER);
        *data = rbuf->buffer[rbuf->tail];
    	rbuf->tail = (rbuf->tail + 1) % rbuf->size;
    	rbuf->full = false;
        dpl_sem_release(&rbuf->sem);
        return 0;
    }
    return -1;
}

bool rbuf_empty(rbuf_t* rbuf)
{
    return (!rbuf->full && (rbuf->head == rbuf->tail));
}

bool rbuf_full(rbuf_t *rbuf)
{
    return rbuf->full;
}

size_t rbuf_size(rbuf_t* rbuf)
{
	size_t size = rbuf->size;

	if(!rbuf->full)
	{
		if(rbuf->head >= rbuf->tail)
		{
			size = (rbuf->head - rbuf->tail);
		}
		else
		{
			size = (rbuf->size + rbuf->head - rbuf->tail);
		}
	}

	return size;
}