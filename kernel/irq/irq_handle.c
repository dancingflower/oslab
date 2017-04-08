#include "../include/common.h"
#include "irq.h"

#define NR_IRQ_HANDLE 32
#define NR_HARD_INTR 16

struct IRQ_t
{
	void (*routine)(void);
	struct IRQ_t *next;
};

static struct IRQ_t handle_pool[NR_IRQ_HANDLE];
static struct IRQ_t *handles[NR_HARD_INTR];
static int handle_count = 0;

void do_syscall(TrapFrame *);

void add_irq_handle(int irq, void (*func)(void))
{
	assert(irq < NR_HARD_INTR);
	assert(handle_count <= NR_IRQ_HANDLE);

	struct IRQ_t *ptr;
	ptr = &handle_pool[handle_count ++];
	ptr->routine = func;
	ptr->next = handles[irq];
	handles[irq] = ptr;
}

void irq_handle(TrapFrame *tf)
{

	int irq = tf->irq;

	if(irq == 0x80) do_syscall(tf);
	else 
	if(irq < 1000) {printk("Unhandled irq = %d!\n",irq);assert(0);}
	else
	{
		int irq_id = irq - 1000;
		assert(irq_id < NR_HARD_INTR);
		struct IRQ_t *f = handles[irq_id];

		while(f != NULL)
		{
			f->routine();
			f = f->next;
		}
	}
}

