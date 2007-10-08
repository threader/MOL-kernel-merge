/* 
 *   Copyright (C) 2002, 2003, 2004 Samuel Rydh (samuel@ibrium.se)
 *   
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation
 *   
 */

#ifndef _H_ARCHINCLUDE
#define _H_ARCHINCLUDE

/* FIXME - Turn into options */
//#define PERF_MONITOR
//#define PERFORMANCE_INFO              /* collect performance statistics */
//#define PERFORMANCE_INFO_LIGHT        /* sample only the most important counters */

#include "mol_config.h"
#include "kconfig.h"
#include <linux/utsrelease.h>
#include <linux/autoconf.h>

#ifndef __ASSEMBLY__
#include <linux/kernel.h>
#include <asm/atomic.h>
#include <linux/sched.h>	/* needed by <asm/mmu_context.h> */
#include <asm/mmu_context.h>
#include <asm/time.h>
#include "dbg.h"
#endif				/* __ASSEMBLY__ */

#endif				/* _H_ARCHINCLUDE */
