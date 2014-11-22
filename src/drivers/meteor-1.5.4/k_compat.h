/*
 * k_compat.h -- A header file containing definitions to allow the driver
 * that links with it to be backward and forward compatable with a number
 * of linux kernels.
 *
 * Copyright (c) 1999 The Laitram Corporation
 *
 * This file allows the Matrox Meteor driver to compile and run
 * without modification across a range of Linux kernel versions.
 *
 * It is a derivitave work of a similar file contained in the
 * Linux pcmcia-cs distribution by David Hinds <dhinds@hyper.stanford.edu>.
 * Copyright (c) David A. Hinds
 * 
 * The Linux pcmcia-cs package is covered under the Mozilla Public
 * License, the text of which can be found at:
 * http://www.mozilla.org/MPL/
 *
 * Since this file is so heavily derived from that file, it is also
 * covered under the Mozilla Public License.
 *
 * The remainder of the meteor driver is covered under the
 * GNU General Public License, as outlined in other files
 * in the meteor distribution.
 */

#include "kernelversioninfo.h"

#define VERSION(v,p,s)		(((v)<<16)+(p<<8)+s)

#if (LINUX_VERSION_CODE < VERSION(2,1,4)) && !defined(__alpha__)
#define FS_SIZE_T		int
#define U_FS_SIZE_T		int
#else
#if (LINUX_VERSION_CODE < VERSION(2,1,60))
#define FS_SIZE_T		long
#define U_FS_SIZE_T		unsigned long
#else
#define FS_SIZE_T		ssize_t
#define U_FS_SIZE_T		size_t
#endif
#endif
#if (LINUX_VERSION_CODE < VERSION(2,1,31))
#define FS_RELEASE_T		void
#else
#define FS_RELEASE_T		int
#endif

#if (LINUX_VERSION_CODE < VERSION(2,1,0))
#define copy_from_user		memcpy_fromfs
#define copy_to_user		memcpy_tofs

#define ioremap(a,b) \
    (((a) < 0x100000) ? (void *)((u_long)(a)) : vremap(a,b))
#define iounmap(v) \
    do { if ((u_long)(v) > 0x100000) vfree(v); } while (0)

/* This is evil... throw away the built-in get_user in 2.0 */
#include <asm/segment.h>
#undef get_user

#define get_user(x, ptr) \
		((sizeof(*ptr) == 4) ? (x = get_fs_long(ptr)) : \
		 (sizeof(*ptr) == 2) ? (x = get_fs_word(ptr)) : \
		 (x = get_fs_byte(ptr)))
#else /* 2.1.X */
#include <asm/uaccess.h>
#endif

#if (LINUX_VERSION_CODE < VERSION(2,1,45))
#define F_INODE(file)		((file)->f_inode)
#else
#define F_INODE(file)		((file)->f_dentry->d_inode)
#endif

#if (LINUX_VERSION_CODE < VERSION(2,1,60))
#define IRQ_MAP(irq, dev)	irq2dev_map[irq] = dev
#define FOPS(i,f,b,c,p)		(i,f,b,c)
#define FPOS			(file->f_pos)
#else
#define IRQ_MAP(irq, dev)	while (0)
#define FOPS(i,f,b,c,p)		(f,b,c,p)
#define FPOS			(*ppos)
#endif

#if (LINUX_VERSION_CODE < VERSION(2,1,68))
#define signal_pending(cur)	((cur)->signal & ~(cur)->blocked)
#endif

#if (LINUX_VERSION_CODE < VERSION(2,1,93))
#include <linux/bios32.h>
#endif
#include <linux/pci.h>
#ifndef PCI_FUNC
#define PCI_FUNC(devfn)		((devfn)&7)
#define PCI_SLOT(devfn)		((devfn)>>3)
#define PCI_DEVFN(dev,fn)	(((dev)<<3)|((fn)&7))
#endif

#if (LINUX_VERSION_CODE < VERSION(2,1,127))
#define interruptible_sleep_on_timeout(w,t) \
    ({(current->timeout=jiffies+(t));wsleep(w);current->timeout;})
#define schedule_timeout(t) \
    do { current->timeout = jiffies+(t); schedule(); } while (0)
#endif

#include <asm/io.h>

#if (LINUX_VERSION_CODE > VERSION(2,1,117))
#define NULL_FLUSH              NULL,
#else
#define NULL_FLUSH
#endif
