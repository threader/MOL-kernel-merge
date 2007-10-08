/* 
 * Copyright (C) 1998-2004 Samuel Rydh (samuel@ibrium.se)
 * MMU context allocation
 *   
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation
 *   
 */

#include "archinclude.h"
#include "alloc.h"
#include "mmu.h"
#include "mmu_contexts.h"
#include "misc.h"
#include "asmfuncs.h"
#include "emu.h"
#include "mtable.h"
#include "performance.h"
#include "context.h"
#include "hash.h"

static int flush_all_PTEs(kernel_vars_t * kv)
{
	int i, count = 0, npte = (ptehash.pte_mask + 8) / 8;
	ulong *pte, ea, v;

	for (pte = ptehash.base, i = 0; i < npte; i++, pte += 2) {
		v = *pte;
		if (!(v & BIT(0)))	/* test V-bit */
			continue;
		v = (v & ~BIT(0)) >> 7;
		v = (v -
		     ((v & 0xf) * MOL_MUNGE_ESID_ADD)) * MOL_MUNGE_MUL_INVERSE;
		v = (v >> 4) & MOL_CTX_MASK;

		if (v >= (kb->mmu).first_mol_context
		    && v <= (kb->mmu).last_mol_context) {
			*pte = 0;
			count++;
		}
	}

	/* perform a tlbia */
	for (ea = 0; ea <= (0x3f << 12); ea += 0x1000)
		__tlbie(ea);

	if (count)
		printk(KERN_ERR "%d stale PTEs flushed (something is wrong)\n",
		       count);
	return count;
}

int init_contexts(kernel_vars_t * kv)
{
	(kb->mmu).first_mol_context = FIRST_MOL_CONTEXT(kv->session_index);
	(kb->mmu).last_mol_context = LAST_MOL_CONTEXT(kv->session_index);
	(kb->mmu).next_mol_context = (kb->mmu).first_mol_context;

	(kb->mmu).illegal_sr = alloc_context(kv) | VSID_Kp | VSID_N;

	flush_all_PTEs(kv);
	return 0;
}

void cleanup_contexts(kernel_vars_t * kv)
{
	flush_all_PTEs(kv);
}

void handle_context_wrap(kernel_vars_t * kv, int n)
{
	if ((kb->mmu).next_mol_context + n > (kb->mmu).last_mol_context) {
		printk(KERN_NOTICE "MOL context wrap\n");

		clear_all_vsids(kv);
		init_contexts(kv);
	}
}

int alloc_context(kernel_vars_t * kv)
{
	int mol_context = (kb->mmu).next_mol_context++;
	vsid = MOL_CTX_TO_VSID(mol_context >> 4);
	return (vsid & VSID_MASK);
}
