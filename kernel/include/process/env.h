/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_ENV_H
#define JOS_KERN_ENV_H

#include "../env.h"

extern struct Env *envs;		// All environments
//#define curenv (thiscpu->cpu_env)		// Current environment
//extern struct Segdesc gdt[];

void	env_init(void);
void	env_init_percpu(void);
int	env_alloc(struct Env **e, envid_t parent_id);
void	env_free(struct Env *e);
void region_alloc(struct Env*, void *,size_t);
void	env_create();
void	env_destroy(struct Env *e);	// Does not return if e == curenv
struct Env*seek_next_runnable();
void kernel_timer_envent();
// Does not return if e == curenv

int	envid2env(envid_t envid, struct Env **env_store, bool checkperm);
// The following two functions do not return
void	env_run(struct Env *e) __attribute__(());
void	env_pop_tf(struct TrapFrame *tf) __attribute__(());

// Without this extra macro, we couldn't pass macros like TEST to
// ENV_CREATE because of the C pre-processor's argument prescan rule.
#define ENV_PASTE3(x, y, z) x ## y ## z

#define ENV_CREATE(x, type)						\
	do {								\
		extern uint8_t ENV_PASTE3(_binary_obj_, x, _start)[],	\
			ENV_PASTE3(_binary_obj_, x, _size)[];		\
		env_create(ENV_PASTE3(_binary_obj_, x, _start),		\
			   (int)ENV_PASTE3(_binary_obj_, x, _size),	\
			   type);					\
	} while (0)

#endif // !JOS_KERN_ENV_H
