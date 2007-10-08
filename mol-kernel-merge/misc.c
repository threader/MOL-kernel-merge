/* 
 *   Copyright (C) 1997-2004 Samuel Rydh (samuel@ibrium.se)
 *   Kernel interface
 *   
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation
 *   
 */

#include "archinclude.h"
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/prom.h>
#include "kernel_vars.h"
#include "misc.h"
#include "performance.h"
#include "map.h"

#define MMU	kv->mmu

kernel_vars_t *alloc_kvar_pages(void)
{
	kernel_vars_t *kv;
	int i, order;
	char *ptr;

	for (i = 1, order = 0; i < NUM_KVARS_PAGES; i = i << 1, order++) ;
	if (!(kv = (kernel_vars_t *) __get_free_pages(GFP_KERNEL, order)))
		return NULL;

	/* To be able to export the kernel variables to user space, we
	 * must set the PG_reserved bit. This is due to a check
	 * in remap_pte_range() in kernel/memory.c (is this bug or a feature?).
	 */
	for (ptr = (char *)kv, i = 0; i < NUM_KVARS_PAGES; i++, ptr += 0x1000)
		SetPageReserved(virt_to_page(ptr));

	return kv;
}

void free_kvar_pages(kernel_vars_t * kv)
{
	char *ptr = (char *)kv;
	int i, order;

	for (i = 0; i < NUM_KVARS_PAGES; i++, ptr += 0x1000)
		ClearPageReserved(virt_to_page(ptr));

	for (i = 1, order = 0; i < NUM_KVARS_PAGES; i = i << 1, order++) ;
	free_pages((ulong) kv, order);
}

/************************************************************************/
/*	hash access							*/
/************************************************************************/

ulong *map_emulated_hash(kernel_vars_t * kv, ulong mbase, ulong size)
{
	return (ulong *) (MMU.userspace_ram_base + mbase);
}

void unmap_emulated_hash(kernel_vars_t * kv)
{
	/* nothing */
}

/************************************************************************/
/*	kernel lowmem asm <-> kernel C-code switching			*/
/************************************************************************/

typedef int (*kernelfunc_t) (kernel_vars_t *, ulong, ulong, ulong);
typedef void (*trampoline_t) (struct pt_regs * regs);
static trampoline_t old_trampoline;

static void mol_trampoline_vector(struct pt_regs *r)
{
	kernel_vars_t *kv = (kernel_vars_t *) r->gpr[8];

	TICK_CNTR_PUSH(kv);
	r->gpr[3] =
	    (*(kernelfunc_t) r->gpr[3]) (kv, r->gpr[4], r->gpr[5], r->gpr[6]);
	TICK_CNTR_POP(kv, in_kernel);
}

static trampoline_t set_trampoline(trampoline_t tramp)
{
	trampoline_t old;
	extern trampoline_t mol_trampoline;
	old = mol_trampoline;
	mol_trampoline = tramp;
	return old;
}

int arch_common_init(void)
{
	old_trampoline = set_trampoline(mol_trampoline_vector);
	return !old_trampoline;
}

void arch_common_cleanup(void)
{
	set_trampoline(old_trampoline);
}
