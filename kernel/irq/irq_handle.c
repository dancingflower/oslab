#include "../include/common.h"
#include "../include/irq.h"
#include "../include/assert.h"
#include "../include/mmu.h"
#include "../include/x86.h"

#define NR_IRQ_HANDLE 32
#define NR_HARD_INTR 16

void keyboard_event(int);
void timer_event(void);
void do_syscall(TrapFrame *);
uint32_t Get_gdt_off(uint32_t);

int offset;
void irq_handle(TrapFrame *tf)
{
	int seg_tmp;
	asm volatile("movl %%es, %0" : "=a"(seg_tmp) :);
	asm volatile("movl %0, %%es\n\t"
				 "movl %0, %%ds\n\t"
				 "movl %0, %%fs\n\t"
				 "movl %0, %%gs\n\t" 
				 : 
				 : "a"(SELECTOR_KERNEL(SEG_KERNEL_DATA)));
	offset = Get_gdt_off(seg_tmp >> 3);
	uint32_t code, val;
	if ((tf->irq < 1000))
	{
		if(tf->irq == -1)
			printk("%s, %d: Unhandled exception!\n", __FUNCTION__, __LINE__);
		else if(tf->irq == 0x80)
			do_syscall(tf);
		else
			printk("%s, %d: Unexpected exception #%d %d!\n", __FUNCTION__, __LINE__, tf->irq, tf->error_code);
	}
	else 
		switch (tf->irq)
		{
			case 1000:
				timer_event();
				break;
			case 1001: 
				code = inb(0x60);
				val = inb(0x61);
				outb(0x61, val | 0x80);
				outb(0x61, val);
				//printk("%s, %d: key code = %x\n", __FUNCTION__, __LINE__, code);
				keyboard_event(code);
			case 13:
				break;
			case 1014:
				break;
			default : printk("Error in irq_handle.c : %d\n", tf->irq);
		}
	asm volatile("movl %0, %%es\n\t"
				 "movl %0, %%ds\n\t"
				 "movl %0, %%fs\n\t"
				"movl %0, %%gs" : : "a"(seg_tmp));
}

uint32_t Get_seg_off()
{
	return offset;
}
