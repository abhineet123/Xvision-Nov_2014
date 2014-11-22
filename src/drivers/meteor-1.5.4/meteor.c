/* #define DEBUG_METEOR  */
/*
 *	meteor.c  -- Matrox Meteor FrameGrabber Driver.
 *
 *	Copyright (C) 1996  Jim Bray (http://as220.org/jb).
 *      [ send comments and bug reports to mark.sutton@laitram.com]
 *
 *      This work was supported by RWI (Real World Interface) Inc.,
 *      and the AI Lab at Brown University.
 *
 *      This driver is a port and partial rewrite of the Meteor driver
 *      written by Mark Tinguely and Jim Lowe. Their copyright notice is
 *      above the beginning of the ported code.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *      Comments and bug reports should now be sent to Mark Sutton
 *      <mark.sutton@laitram.com> who has taken over maintenance of
 *      the driver (Version 1.5.0 and later).
 *
 * HISTORY
 * =======
 *
 * Version 1.0: 5/1/96
 *  Made checkPCI() inline, #ifdeffed it on DEBUG_METEOR so it should
 * disappear unless debugging is set.
 *  Changed Makefile to compile -Wall -Wstrict-prototypes. This revealed
 * a typo in ioctl METEORSBT254. Fixed. Added parens, prototypes etc
 * until no warnings.
 *
 * Version 1.1: 5/6/96
 *  Implemented METEORSTATUS ioctl fix posted by Jim Lowe.
 *
 * Version 1.1.1: 5/7/96
 *  Minor tweaks to meteor_init.
 *
 * Version 1.2: 14/May/96 (IDR)
 *  -- Fixed frames per second so that it actually implements
 *     the call in the saa7116.
 *  -- Fixed bug in SVIDEO input which ignored chrominance by just
 *     reverting to the bsd bit of this code.
 *  -- Fixed #define METEOR_INPUT_DEV_RCA
 *  -- Fixed #define METEOR_DEV_MASK
 *  -- Fixed spelling of PLANAR (maintains backward compatibility)
 *  -- Added a selectable system default (SYSTEM=PAL/SECAM/NTSC)
 *
 * Version 1.3: 22/May/96 (IDR)
 *  -- Fixed problem with PAL RGB mode (from jonas@mcs.com)
 *  -- Fixed problem with frames per second code
 *  -- Fixed problem with PCF8574_xxxx_WRITE
 *  -- Added METEOR_GEO_ODD_EVEN to enable separate capture of odd/even
 *     fields
 *  -- Added METEOR_SIG_[FRAME|FIELD] to toggle between interrupts after
 *     each field or frame (frame still the default)
 *  -- Added METEOR_FIELD_MODE to capture fields as if they were frames
 *     *** this has not been thoroughly tested on all image types and
 *         on synchronous capture ***
 *
 * Version 1.4: 17/Jul/96
 *  -- Fixed prob inadvertently introduced to set_fps in the last fix
 *     (thanks Pedro Felzenszwalb pff@cs.cornell.edu)
 *  -- Tested and fixed FIELD_MODE stuff
 *  -- Added support for YUV in synchronous mode
 *  -- Allow lowat==hiwat==0 in synchronous mode to mean capture
 *     in round-robin fashion without user intervention.  Also
 *     added cur_frame to struct meteor_mem so that this info is
 *     available in shared memory.
 *  -- It appears that not only is there a problem with the number of bits
 *     in the dma_end registers (only 22), but that the dma regs themselves don't
 *     increment over 4Mb boundaries.  Instead they wrap around to the start of the 
 *     4Mb block.  Furthermore, since they only increment in 4 byte words, if the
 *     image width is not divisible by 8 then YUV images can overflow (since the U
 *     and V parts are saved separately using dma regs 1 and 2).
 *     The changes to fix these problems are due to anuj@fwi.uva.nl and include a
 *     complete rewrite of alloc_frame_buffer which may not be completely backwardly
 *     compatibile, but is much cleaner and more general.  You can now grab a 
 *     sequence of images once (using a new capture flag METEOR_CAP_CONT_ONCE added
 *     by anuj@fwi.uva.nl) and use buffers of >4Mb.  In order to allow this, a new
 *     ioctl (METEORGFROFF) returns the offsets into the frame buffer of each frame
 *     and the meteor_mem structure (see ioctl_meteor.h for the meteor_frame_offset
 *     structure).
 *
 * Version 1.4a 1/9/96
 *  -- Very minor changes to tidy v1.4 and update some info in README
 *  -- Bug fixes from Mark Wolski (mwolski@rvssun3.gmr.com)
 *  -- Fix to FIELD_MODE so it resets properly
 *
 * Version 1.4b 5/11/96
 *  -- Minor bug fixes
 *  -- Added equivalence between YUV_422 and YUV_PLANAR
 *
 * Version 1.4c 20/2/97
 *  -- Bug fixes (from burschka@mail.lpr.e-technik.tu-muenchen.de)
 *
 * Version 1.5.0 27 July 1998
 *  -- Modified driver and all examples except fgrab to compile under 
 *     glibc2 (aka. libc6) based Linux distributions. 
 *     (Mark Sutton <mark.sutton@laitram.com>)
 *     
 *
 * Version 1.5.1 28 July 1998
 * -- fgrab also fixed to work on glibc distributions.
 *    (Mark Sutton <mark.sutton@laitram.com>)
 *
 * Version 1.5.2 23 September 1998
 * -- Several minor modifications, see README-1.5.2 for details.
 *    (Mark Sutton <mark.sutton@laitram.com>)
 *
 * Version 1.5.3 January 22, 1999
 * -- Released a package that was exactly the same as 1.5.2, except
 *    that it also contained a seperate meteor.c and meteor.h files,
 *    contributed by Ove Ewerlid <Ove.Ewerlid@syscon.uu.se> for use
 *    on 2.2.x series kernels.
 *
 * Version 1.5.4 April 6, 1999
 * -- Autoconfigures for 2.0.x or 2.2.x (and, fingers crossed, 2.1.x)
 *    kernels automatically.  Fixes bugs reported relating to unloading
 *    the module on kernels 2.2.3 and higher.  Also encorporates
 *    enhancements and bug fixes (applicable to all kernels configurations)
 *    contributed by Tony Hague <tony.hague@bbsrc.ac.uk>.
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <asm/irq.h>
#include <asm/pgtable.h>
#include <linux/time.h>
#include <linux/malloc.h>
#include <linux/bigphysarea.h>

#if 0
#include <linux/serial_reg.h>	/* For the serial port debug signal */
#endif

#include "k_compat.h"
#include "saa71x6.h"
#include "meteor.h"
#include "ioctl_meteor.h"


#if defined(PAL)
#define METEOR_MAXFPS 25
#define METEOR_SYSTEM_DEFAULT METEOR_PAL
#elif defined(SECAM)
#define METEOR_MAXFPS 25
#define METEOR_SYSTEM_DEFAULT METEOR_SECAM
#else
#define METEOR_MAXFPS 30
#define METEOR_SYSTEM_DEFAULT METEOR_NTSC
#endif

#ifdef DEBUG_METEOR
#define CAT(a)\
printk(a"\n")

#define SHOW(a)\
printk(#a": %d 0x%x\n", a, a);

#define SHOW2(a,b)\
printk(#a": %d 0x%x "#b": %d 0x%x\n", a, a, b, b);

#define PRINTK(a) printk a

#else

#define PRINTK(a) /* */

#endif



/*
 * Globals:
 */


static u_short MeteorDevs;
static struct meteor *meteors[MAXMETEORS];
static u_short IrqMap = 0;

/*
 * Prototypes:
 */
static struct meteor *get_mtr_from_inode(struct inode *ip);
static void free_frame_buffer(struct meteor *mtr);
static int alloc_frame_buffer(struct meteor *mtr, u_long wantSize);
static int read_fs(const void *useraddr, void *where, u_int size);
static int write_fs_val(const u_long value, void * useraddr, u_int size);
static int write_fs_ptr(const void * ptr, void * useraddr, u_int size);
static void wait_jiffies(u_long waitJiffies);
static void reset_saa7116(struct meteor *mtr);
static void init_saa7116(struct meteor *mtr);
static int meteor_init(void);
static void meteor_unwind(void);
static void meteor_intr(int irq, void *dev_id, struct pt_regs *fake);
static void stop_capture(struct meteor *);
static void bsd_meteor_init(struct meteor *mtr);
static int bsd_meteor_open(struct meteor *mtr);
static int bsd_meteor_read(struct meteor *mtr, char *user_buf, int user_count);
static int bsd_meteor_ioctl(struct meteor *mtr, unsigned int cmd, void *arg);


static struct meteor *get_mtr_from_inode(struct inode *ip)
{
  int minor = MINOR(ip->i_rdev);
  if (minor >= MeteorDevs)	/* unit out of range */
    return(0);
  return( meteors[minor] );
}

static int read_fs(const void *useraddr, void *where, u_int size)
{
  int err;

  if (useraddr == NULL || !size)
    return -EINVAL;
  if ((err = verify_area(VERIFY_READ, useraddr, size)))
    return err;
  switch (size) {
  case 1:
    get_user(*(u_char *) where, (u_char *) useraddr);
    break;
  case 2:
    get_user(*(u_short *) where, (u_short *) useraddr);
    break;
  case 4:
    get_user(*(u_long *) where, (u_long *) useraddr);
    break;
  default:
    copy_from_user(where, useraddr, size);
  }
  return 0;
}

static int write_fs_val(const u_long value, void * useraddr, u_int size)
{
  int err;

  if (useraddr == NULL)
    return -EINVAL;
  if ((err = verify_area(VERIFY_WRITE, useraddr, size)))
    return err;
  switch (size) {
  case 1:
    put_user((u_char)value, (u_char *) useraddr);
    break;
  case 2:
    put_user((u_short)value, (u_short *) useraddr);
    break;
  case 4:
    put_user((u_long)value, (u_long *) useraddr);
    break;
  default:
    return -EINVAL;
  }
  return 0;
}

static int write_fs_ptr(const void * ptr, void * useraddr, u_int size)
{
  int err;

  if (useraddr == NULL)
    return -EINVAL;
  if ((err = verify_area(VERIFY_WRITE, useraddr, size)))
    return err;
  copy_to_user(useraddr, ptr, size );
  return 0;
}

#define FOURMEG (4 * 1024 * 1024)
static int alloc_frame_buffer(struct meteor *mtr, u_long wantSize)
{
  u_long startAddr, block_size_left, left_over;
  long bufSize;
  int frame_size, i;

  /* Things have changed here.  There may be some backwards incompatibility
   * but I hope not too much.
   *
   * It is important that frames do not cross 4Mb boundaries.
   * The dma_end registers only have 22 significant bits, and even
   * without range checking the dma regs themselves don't seem to
   * increment over 4Mb boundaries (they just wrap around into 
   * the current 4Mb block).
   */

  if ( mtr->frame_buffer ) {
    free_frame_buffer(mtr);
  }
  /* 
   * time struct added with a frame ?? 
   */
  frame_size = mtr->frame_size;
  if(mtr->flags & METEOR_WANT_TS) {
    frame_size += sizeof(struct timeval);
  }

  /* 
   * Iterate until all the frames fit in without crossing a 4Mb boundary 
   */
  mtr->frame_buffer = NULL; left_over = 0;
  while (!mtr->frame_buffer) {
      PRINTK(("Trying to allocate frames\n"));
      bufSize = mtr->frames*frame_size + left_over;
      bufSize = PAGE_ALIGN(bufSize);
      if ( ! (startAddr = (u_long)bigphysarea_alloc(bufSize + PAGE_SIZE)) ) {
	  PRINTK(("meteor%d: frame buffer failed  %ld\n",
		  mtr->unit, bufSize));
	  return -ENOMEM;
      }
      mtr->frame_buffer = (void *)startAddr;
      mtr->mem = mtr->frame_buffer + bufSize;
      mtr->fb_size = bufSize + PAGE_SIZE;
      PRINTK(("frame_buffer=%lx, mem=%lx, fb_size=%x\n", startAddr, (u_long)mtr->mem, mtr->fb_size));

      /* 
       * layout frames in memory 
       */
      i = 0;
      while (i<mtr->frames) {
	  block_size_left = FOURMEG - ((FOURMEG-1) & startAddr);
	  if (block_size_left > frame_size) {
	      mtr->frame_offset[i] = startAddr - (u_long)mtr->frame_buffer;
	      PRINTK(("frame %d: %lx -> %lx\n", i,
		      (u_long)mtr->frame_buffer+mtr->frame_offset[i],
		      (u_long)mtr->frame_buffer+mtr->frame_offset[i]+frame_size));
	      bufSize -= frame_size;
	      startAddr += frame_size;
	      i++;
	  } else {
	      bufSize -= block_size_left;
	      startAddr += block_size_left;
	  }
      }
      if (bufSize < 0) {
	  left_over += -bufSize;
	  bigphysarea_free(mtr->frame_buffer, mtr->fb_size);
	  mtr->frame_buffer = 0;
      }
  }
  /* done */  

  PRINTK(("Done\n"));
  return 0;
}


static void free_frame_buffer(struct meteor *mtr)
{
  if ( mtr->frame_buffer ) {
    PRINTK(("meteor%d: freeing frame buffer address %lx\n",
	    mtr->unit, mtr->frame_buffer));
    bigphysarea_free(mtr->frame_buffer, mtr->fb_size);
  }
  mtr->frame_buffer = NULL;
}

static int meteor_open(struct inode * inode, struct file * file)
{
  int ret;
  struct meteor *mtr;

  if ( !(mtr = get_mtr_from_inode(inode)) )
    return(-ENXIO);
  init_saa7116(mtr);
  if ( ! (ret=bsd_meteor_open(mtr)) ) {
    MOD_INC_USE_COUNT;
    return 0;
  }
  reset_saa7116(mtr);
  free_frame_buffer(mtr);
  return ret;
}

#if (LINUX_VERSION_CODE > VERSION(2,1,117))
static int meteor_flush(struct file * file)
{
  struct meteor *mtr;

  if ( (mtr = get_mtr_from_inode(file->f_dentry->d_inode)) ) {
    PRINTK(("meteor%d: (mtr %x) close.\n", mtr->unit, mtr));
    mtr->flags &= ~METEOR_OPEN;
    stop_capture(mtr);
    reset_saa7116(mtr);
    free_frame_buffer(mtr);
    MOD_DEC_USE_COUNT;
  }
  return 0;
}
#endif

static FS_RELEASE_T meteor_close(struct inode * inode, struct file * file)
{

#if (LINUX_VERSION_CODE > VERSION(2,1,117))
  PRINTK(("meteor: meteor_close()"));  /* only for debugging */
#else
  struct meteor *mtr;
#if (LINUX_VERSION_CODE < VERSION(2,1,31))
  if ( (mtr = get_mtr_from_inode(inode)) ) {
#else
  if ( (mtr = get_mtr_from_inode(file->f_dentry->d_inode)) ) {
#endif
    PRINTK((KERN_INFO "meteor%d: (mtr %lx) close.\n", mtr->unit, (u_long)mtr));
    mtr->flags &= ~METEOR_OPEN;
    stop_capture(mtr);
    reset_saa7116(mtr);
    free_frame_buffer(mtr);
    MOD_DEC_USE_COUNT;
  }
#endif
  return (FS_RELEASE_T)0;
}

static FS_SIZE_T meteor_read FOPS(struct inode * inode, struct file * file, char * buffer, U_FS_SIZE_T count, loff_t * dummy)
{
  struct meteor *mtr;
  int ret;

  if ( !(mtr = get_mtr_from_inode(F_INODE(file))) )
    return(-ENXIO);
  ret = bsd_meteor_read(mtr, buffer, count);
  return ret;
}


/*
 * Writes are meaningless.
 */
static FS_SIZE_T meteor_write FOPS(struct inode * inode, struct file * file,
			const char * buffer, U_FS_SIZE_T count, loff_t * dummy)
{
  return -EPERM;
}

static int meteor_ioctl(struct inode *inode, struct file *file,
			unsigned int cmd, unsigned long arg)
{
  struct meteor *mtr;
  int ret;

  if ( !(mtr = get_mtr_from_inode(inode)) )
    return(-ENXIO);
  ret = bsd_meteor_ioctl(mtr, cmd, (void *)arg);
  return ret;
  
}

#if (LINUX_VERSION_CODE < VERSION(2,1,45))
static int meteor_mmap(struct inode * inode, struct file * file,
		    struct vm_area_struct * vma)
#else
static int meteor_mmap(struct file * file,
		    struct vm_area_struct * vma)
#endif

{
  struct meteor *mtr;

  int err;
  u_long size = vma->vm_end - vma->vm_start;
  u_long offset;

  if ( !(mtr = get_mtr_from_inode(F_INODE(file))) )
    return -ENXIO;
  offset = (u_long)mtr->frame_buffer;
  if ( (offset|size) & ~PAGE_MASK )
    return -EFAULT;
  if ( size > mtr->fb_size ) {
    return -EFBIG;
  }
  /*
   * changed based on /usr/src/linux/kernel/mem.c 
   * (Darius.Burschka@lpr.e-technik.tu-muenchen.de)
   */
#if (LINUX_VERSION_CODE >= VERSION(2,2,0))
  offset = virt_to_phys ((void*)offset);

  vma->vm_offset=offset;
  if ((u32)vma->vm_offset >=(u32)high_memory)
       pgprot_val (vma->vm_page_prot) |= _PAGE_PCD;

  PRINTK ((KERN_INFO "meteor: pre remap:  %08x %08x\n", sd, si));

  if ((err = remap_page_range (vma->vm_start, offset, size, vma->vm_page_prot)))
    return err;
  PRINTK ((KERN_INFO "meteor: post remap: %08x %08x\n", vma->vm_start, offset));

  vma->vm_file = file;
  file->f_count++;
#else
  vma->vm_offset=offset;
  if(vma->vm_offset >=high_memory)
       pgprot_val(vma->vm_page_prot) |= _PAGE_PCD;
  if ( (err=remap_page_range(vma->vm_start, offset,
			     size, vma->vm_page_prot)) )
    return err;
  vma->vm_inode = inode;
  inode->i_count++;
#endif
  return 0;
}

static void meteor_unwind(void)
{
  int i;
  unregister_chrdev(METEOR_MAJOR, "meteor");
  for ( i = 0; i < MAXMETEORS; i++ ) {
    struct meteor *mtr = meteors[i];
    meteors[i] = NULL;
    if ( mtr ) {
      if ( mtr->s7116 ) {
	reset_saa7116(mtr);
	iounmap((void *)mtr->s7116); /* Unwind vremap() */
      }
      if ( mtr->irq )
	free_irq(mtr->irq, mtr);
      free_frame_buffer(mtr);
      kfree(mtr); /* Unwind kalloc() */
    }
  }	    
}

static inline void set_latency(struct meteor *mtr)
{ 
  /* Adjust latency */
  u_short bus = mtr->bus, dev_fn = mtr->dev_fn;
  u_char latency;

  pcibios_read_config_byte(bus, dev_fn, PCI_LATENCY_TIMER, &latency);
  latency = METEOR_LATENCY;
  pcibios_write_config_byte(bus, dev_fn, PCI_LATENCY_TIMER, latency);
#ifdef DEBUG_METEOR
    pcibios_read_config_byte(bus, dev_fn, PCI_LATENCY_TIMER, &latency);
    PRINTK((KERN_INFO "meteor%d: Latency = %d\n", mtr->unit, latency));
#endif
}

/*
 * Check and reset pciStatus errors. 
 */

static inline void checkPCI(struct meteor *mtr)
{
#ifdef DEBUG_METEOR
  u_short status;
  register u_short bus = mtr->bus, devFun = mtr->dev_fn;

  pcibios_read_config_word(bus, devFun, PCI_STATUS, &status);
  pcibios_write_config_word(bus, devFun, PCI_STATUS, status); /* Clear errors */
  if ( status & PCI_STATUS_REC_TARGET_ABORT )
    PRINTK(("meteor%d: pci target abort\n", mtr->unit));
  if ( status & PCI_STATUS_REC_MASTER_ABORT )
    PRINTK(("meteor%d: pci master abort\n", mtr->unit));
  if ( status & PCI_STATUS_DETECTED_PARITY )
    PRINTK(("meteor%d: pci parity error\n", mtr->unit));
#endif 
}

/*
 * Delay for WaitJiffies via timeout and schedule().
 * We need to be technically interruptable for this to work,
 * but we really don't want to be, so we block everything.
 * Don't call from interrupt.
 */

#if (LINUX_VERSION_CODE < VERSION(2,2,0))
static void wait_jiffies(u_long waitJiffies)
{
  u_long saveBlocked = current->blocked;
  u_long endJiffies = jiffies + waitJiffies;

  current->blocked = ~0; /* Block everything we can, ignore the rest */
  while( jiffies < endJiffies ) {
    current->timeout = endJiffies;
    current->state = TASK_INTERRUPTIBLE;
    schedule();
  }
  current->blocked = saveBlocked;
}
#else
static void wait_jiffies(u_long waitJiffies)
{
  sigset_t saveBlocked = current->blocked;
  saveBlocked = current->blocked;
  sigemptyset (&current->blocked); /* Block everything we can, ignore the rest */
  current->state = TASK_INTERRUPTIBLE;
  schedule_timeout (waitJiffies);
  current->blocked = saveBlocked;
}
#endif
/*
 * Reset the saa7116.
 */

static void reset_saa7116(struct meteor *mtr)
{
  struct saa7116 *saa7116 = mtr->s7116;

  PRINTK(("meteor%d: reset saa7116 address %x\n", mtr->unit, (unsigned int) saa7116));
  wait_jiffies(WAIT_JIFFS);		      /* Let things settle down */
  saa7116->capt_ctl = 0x00000f30;	      /* First, put into reset */
  memset(saa7116, 0, sizeof(struct saa7116)); /* Clear and reset everything */
  wait_jiffies(WAIT_JIFFS);			      /* Let the dust settle */
}


/*
 * Initialize the saa7116.
 */

static void init_saa7116(struct meteor *mtr)
{
  volatile struct saa7116 *saa7116 = mtr->s7116;

  reset_saa7116(mtr);
  PRINTK(("meteor%d: init saa7116 address %x\n", mtr->unit, saa7116));
  saa7116->capt_ctl = 0x000080c0; /* Bring out of reset, enable range check */
  wait_jiffies(WAIT_JIFFS);
  saa7116->i2c_clocks = 0x461e1e0f;
  saa7116->rt_mode_e =  saa7116->rt_mode_o = 
    ((struct rt_mode){mode: 0x01, route: 0xeeeeee}); /* RGB 16 */
  saa7116->fifo_trigger = ((struct ff_trig){packed: 0x20, planar:0x20});
  saa7116->field_toggle = 0x00000107;	      /* enable field toggle, etc. */
  saa7116->ints_ctl = 0x00000007;	 /* Clear and disable all interrupts */
  saa7116->field_mask_e =
    saa7116->field_mask_o =  0x00000001;    /* This combination of masks and */
  saa7116->mask_lengths = 0;	     /* mask length means capture all fields */
  saa7116->fifo_limits = 0x0005007c;
  wait_jiffies(WAIT_JIFFS);	/* Let this sink in */
}

static inline void probe_range(struct meteor *mtr)
{
  register struct saa7116 *s7116 = mtr->s7116;

  /*
   * Determine the size of the dma_end registers. It appears there
   * may be different saa7116 versions floating around, and that more
   * bits may be added in future (according to Philips docs).
   */
  s7116->dma_end_e = s7116->dma_end_o = ~0;
  mtr->maxRange = (s7116->dma_end_e & s7116->dma_end_o) | 0xff;
  PRINTK((KERN_INFO "meteor%d: maxRange %lx\n", mtr->unit, mtr->maxRange));
  s7116->dma_end_e = s7116->dma_end_o = 0;
}

/*
 * Talk to a device on the i2c (or iic) bus.
 */

static int talk_i2c(struct meteor *mtr, u_char dev, u_char addr, u_char data)
{
  u_long start_jiffies = jiffies;

  /* execute i2c cycle */
  mtr->s7116->i2c_cmd = 
    ((struct i2c_cmd){w_data:data, reg_addr: addr, dev_addr:dev, xfer:1});


  /* and wait for completion */
  for (;;) {
      wait_jiffies(1);
      if ( ! mtr->s7116->i2c_cmd.xfer ) 
	  break;
      if ( (jiffies - start_jiffies > WAIT_JIFFS) ) {
	  PRINTK((KERN_INFO "meteor%d: i2c timeout error.\n", mtr->unit));
	  goto error;
      }
  }

  if ( mtr->s7116->i2c_stat.d_abort ) {
    mtr->s7116->i2c_stat.d_abort = 1; /* Reset bit to zero */
    PRINTK((KERN_INFO "meteor%d: i2c abort error.\n", mtr->unit));
    goto error;
  }
  return 0;
error:
  return 1;
}

/*
 * Initialize the boards, data structures, etc.
 */

static int meteor_init(void)
{
  struct meteor *mtr;
  u_short pci_index;
  int pci_ret, final_ret = 0;
  
  if ( ! pcibios_present() )
    return -EUNATCH; /* Let's get creative */

  for ( pci_index = 0; pci_index < MAXMETEORS; pci_index++ ) {

    u_char bus, devFun;
    unsigned int s7116_addr;
    unsigned short command;
    u_char irq;

    pci_ret = pcibios_find_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_7116,
				  pci_index, &bus, &devFun);
    if (pci_ret != PCIBIOS_SUCCESSFUL)
      break;
    mtr = (struct meteor *) kmalloc(sizeof(struct meteor),GFP_KERNEL);
    if ( ! mtr ) {
      final_ret = -ENOMEM;
      goto unwind;
    }

    memset(mtr, 0, sizeof(struct meteor)); /* Clear/NULL everything */
    meteors[pci_index]=mtr;

    mtr->bus = bus;
    mtr->dev_fn = devFun;
    mtr->unit = pci_index;

    pcibios_read_config_byte(bus, devFun, PCI_INTERRUPT_LINE, &irq);
    pcibios_read_config_dword(bus, devFun, PCI_BASE_ADDRESS_0, &s7116_addr);
    pcibios_read_config_word(bus, devFun,  PCI_COMMAND, &command);
    command |= (PCI_COMMAND_MEMORY|PCI_COMMAND_MASTER);
    if ( pcibios_write_config_word(bus, devFun, PCI_COMMAND, command) ) {
      final_ret = -EFAULT;
      goto unwind;
    }

    set_latency(mtr);
    if ( ! (IrqMap & (1 << irq )) ) { /* If we don't already have this IRQ */
      /* Request a fast sharable interrupt */
      if ( ! request_irq(irq, meteor_intr, (SA_INTERRUPT|SA_SHIRQ), "meteor", mtr) )
	IrqMap |= (1 << irq);
      else {
	final_ret = -EBUSY;
	goto unwind;
      }
      /* This logic deliberately only makes one request_irq() call for
       * each physical IRQ needed. There is no need for more.
       */
      mtr->irq = irq;
    }
    {  /* Map saa7166 regs into our address space */
      u_long s7116_page = s7116_addr & PAGE_MASK;
      u_long s7116_off = s7116_addr & ~PAGE_MASK;

      s7116_addr = 
	s7116_off | (u_long)ioremap((u_long)s7116_page, sizeof(struct saa7116));
      if ( ! s7116_addr ) {
	final_ret = -EFAULT;
	goto unwind;
      }
    }
    mtr->s7116 = (struct saa7116 *)s7116_addr;
    init_saa7116(mtr);
    probe_range(mtr);
    bsd_meteor_init(mtr);
    printk(KERN_INFO "Meteor driver version " METEOR_VERSION 
	 ": device %d ready\n", pci_index);

  }
  MeteorDevs = pci_index;
  if ( final_ret ) {
  unwind:
    PRINTK((KERN_INFO "meteor: meteor_init failed (errno=%d).\n", -final_ret));
    meteor_unwind();
  }
  return final_ret;
}

static struct file_operations meteor_fops = {
        NULL,		/* seek */
	meteor_read,	/* read */
	meteor_write,	/* write */
	NULL,		/* readdir */
	NULL,		/* select */
	meteor_ioctl,  	/* ioctl */
	meteor_mmap,   	/* mmap */
        meteor_open,    /* open */
#if (LINUX_VERSION_CODE > VERSION(2,1,117))
	meteor_flush,       /* flush */
#endif
        meteor_close,	/* release */
	NULL,		/* fsync */
	NULL,		/* fasync */
	NULL,		/* check_media_change */
	NULL		/* revalidate */
};


/* Designed to be used as a module */
#ifdef MODULE

void cleanup_module(void)
{  
  meteor_unwind();
}


int init_module(void)
{
  int ret = 0;

  if (register_chrdev(METEOR_MAJOR,"meteor",&meteor_fops)) {
    PRINTK((KERN_INFO "meteor: unable to get major %d for meteor devices\n",
	    METEOR_MAJOR));
    return -EIO;
  }
  ret=meteor_init();
  if (ret)
    meteor_unwind();
  return ret;
}
#endif /*MODULE*/

/*
 * Above this point is the stuff I wrote. Below here the code is
 * ported FreeBSD code. --Jim B.
 */

/*
 * Copyright (c) 1995 Mark Tinguely and Jim Lowe
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Mark Tinguely and Jim Lowe
 * 4. The name of the author may not be used to endorse or promote products 
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*		Change History:
	8/21/95		Release
	8/23/95		On advice from Stefan Esser, added volatile to PCI
			memory pointers to remove PCI caching .
	8/29/95		Fixes suggested by Bruce Evans.
			meteor_mmap should return -1 on error rather than 0.
			unit # > NMETEOR should be unit # >= NMETEOR.
	10/24/95	Turn 50 Hz processing for SECAM and 60 Hz processing
			off for AUTOMODE.
	11/11/95	Change UV from always begin signed to ioctl selected
			to either signed or unsigned.
	12/07/95	Changed 7196 startup codes for 50 Hz as recommended
			by Luigi Rizzo (luigi@iet.unipi.it)
	12/08/95	Clear SECAM bit in PAL/NTSC and set input field count
			bits for 50 Hz mode (PAL/SECAM) before I was setting the
			output count bits. by Luigi Rizzo (luigi@iet.unipi.it)
	12/18/95	Correct odd DMA field (never exceed, but good for safety
			Changed 7196 startup codes for 50 Hz as recommended
			by Luigi Rizzo (luigi@iet.unipi.it)
	12/19/95	Changed field toggle mode to enable (offset 0x3c)
			recommended by luigi@iet.unipi.it
			Added in prototyping, include file, staticizing,
			and DEVFS changes from FreeBSD team.
			Changed the default allocated pages from 151 (NTSC)
			to 217 (PAL).
			Cleaned up some old comments in iic_write().
			Added a Field (even or odd) only capture mode to 
			eliminate the high frequency problems with compression
			algorithms.  Recommended by luigi@iet.unipi.it.
			Changed geometry ioctl so if it couldn't allocated a
			large enough contiguous space, it wouldn't free the
			stuff it already had.
			Added new mode called YUV_422 which delivers the
			data in planer Y followed by U followed by V. This
			differs from the standard YUV_PACKED mode in that
			the chrominance (UV) data is in the correct (different)
			order. This is for programs like vic and mpeg_encode
			so they don't have to reorder the chrominance data.
			Added field count to stats.
			Increment frame count stat if capturing continuous on
			even frame grabs.
			Added my email address to these comments
			(james@cs.uwm.edu) suggested by (luigi@iet.unipt.it :-).
			Changed the user mode signal mechanism to allow the
			user program to be interrupted at the end of a frame
			in any one of the modes.  Added SSIGNAL ioctl.
			Added a SFPS/GFPS ioctl so one may set the frames per
			second that the card catpures.  This code needs to be
			completed.
			Changed the interrupt routine so synchronous capture
			will work on fields or frames and the starting frame
			can be either even or odd.
			Added HALT_N_FRAMES and CONT_N_FRAMES so one could
			stop and continue synchronous capture mode.
			Change the tsleep/wakeup function to wait on mtr
			rather than &read_intr_wait.
	1/22/96		Add option (METEOR_FreeBSD_210) for FreeBSD 2.1
			to compile.
			Changed intr so it only printed errors every 50 times.
			Added unit number to error messages.
			Added get_meteor_mem and enabled range checking.
	1/30/96		Added prelim test stuff for direct video dma transfers
			from Amancio Hasty (hasty@rah.star-gate.com).  Until
			we get some stuff sorted out, this will be ifdef'ed
			with METEOR_DIRECT_VIDEO.  This is very dangerous to
			use at present since we don't check the address that
			is passed by the user!!!!!
	2/26/96		Added special SVIDEO input device type.
	2/27/96		Added meteor_reg.h file and associate types Converted
			meteor.c over to using meteor.h file.  Prompted by
			Lars Jonas Olsson <ljo@po.cwru.edu>.
	2/28/96		Added meteor RGB code from Lars Jonas Olsson
			<ljo@po.cwru.edu>.  I make some mods to this code, so
			I hope it still works as I don't have an rgb card to
			test with.
	2/29/96		<ljo@po.cwru.edu> tested the meteor RGB and supplied
			me with diffs.  Thanks, we now have a working RGB
			version of the driver.  Still need to clean up this
			code.
	3/1/96		Fixed a nasty little bug that was clearing the VTR
			mode bit when the 7196 status was requested.
	3/15/96		Fixed bug introduced in previous version that
			stopped the only fields mode from working.
			Added METEOR{GS}TS ioctl, still needs work.
	3/25/96		Added YUV_9 and YUV_12 modes.  Cleaned up some of the
			code and converted variables to use the new register
			types.
			*/



/*---------------------------------------------------------
**
**	Meteor PCI probe and initialization routines
**
**---------------------------------------------------------
*/

static u_char saa7196_i2c_default[NUM_SAA7196_I2C_REGS] = {
			/* SAA7196 I2C bus control			*/
			/* BITS	Function				*/
/* 00 */	0x50,	/* 7:0	Increment Delay				*/
/* 01 */	0x30,	/* 7:0	Horizontal Sync Begin for 50hz		*/
/* 02 */	0x00,	/* 7:0	Horizontal Sync Stop for 50hz		*/
/* 03 */	0xe8,	/* 7:0	Horizontal Sync Clamp Start for 50hz	*/
/* 04 */	0xb6,	/* 7:0	Horizontal Sync Clamp Stop for 50hz 	*/
/* 05 */	0xf4,	/* 7:0	Horizontal Sync Start after PH1 for 50hz */
/* 06 */	0x46,	/*   7	Input mode =0 CVBS, =1 S-Video 
			     6	Pre filter
			   5:4  Aperture Bandpass characteristics
			   3:2	Coring range for high freq
			   1:0	Aperture bandpass filter weights	*/
/* 07 */	0x00,	/* 7:0	Hue					*/
/* 08 */	0x7f,	/* 7:3	Colour-killer threshold QAM (PAL, NTSC) */
/* 09 */	0x7f,	/* 7:3	Colour-killer threshold SECAM		*/
/* 0a */	0x7f,	/* 7:0	PAL switch sensitivity			*/
/* 0b */	0x7f,	/* 7:0	SECAM switch sensitivity		*/
/* 0c */	0x40,	/*   7	Colour-on bit
			   6:5	AGC filter				*/
/* 0d */	0x84,	/*   7	VTR/TV mode bit = 1->VTR mode
			     3	Realtime output mode select bit
			     2	HREF position select
			     1	Status byte select
			     0	SECAM mode bit				*/
/* 0e */	0x38,	/*   7	Horizontal clock PLL
			     5	Select interal/external clock source
			     4	Output enable of Horizontal/Vertical sync
			     3	Data output YUV enable
			     2	S-VHS bit
			     1	GPSW2
			     0	GPSW1					*/
/* 0f */	0x90,	/*   7	Automatic Field detection
			     6	Field Select 0 = 50hz, 1=60hz
			     5	SECAM cross-colour reduction
			     4	Enable sync and clamping pulse
			   3:1	Luminance delay compensation		*/
/* 10 */	0x00,	/*   2	Select HREF Position
			   1:0  Vertical noise reduction		*/
/* 11 */	0x2c,	/* 7:0	Chrominance gain conrtol for QAM	*/
/* 12 */	0x40,	/* 7:0	Chrominance saturation control for VRAM port */
/* 13 */	0x40,	/* 7:0	Luminance contract control for VRAM port */
/* 14 */	0x34,	/* 7:0	Horizontal sync begin for 60hz		*/
#ifdef notdef
/* 15 */	0x0c,	/* 7:0	Horizontal sync stop for 60hz		*/
/* 16 */	0xfb,	/* 7:0	Horizontal clamp begin for 60hz		*/
/* 17 */	0xd4,	/* 7:0	Horizontal clamp stop for 60hz		*/
/* 18 */	0xec,	/* 7:0	Horizontal sync start after PH1 for 60hz */
#else
		0x0a, 0xf4, 0xce, 0xf4,
#endif
/* 19 */	0x80,	/* 7:0	Luminance brightness control for VRAM port */
/* 1a */	0x00,
/* 1b */	0x00,
/* 1c */	0x00,
/* 1d */	0x00,
/* 1e */	0x00,
/* 1f */	0x00,
/* 20 */	0x90,	/*   7	ROM table bypass switch
			   6:5	Set output field mode
			     4	VRAM port outputs enable
			   3:2	First pixel position in VRO data
			   1:0	FIFO output register select		*/
/* 21 */	0x80,	/* 7:0	[7:0] Pixel number per line on output	*/
/* 22 */	0x80,	/* 7:0	[7:0] Pixel number per line on input	*/
/* 23 */	0x03,	/* 7:0	[7:0] Horizontal start position of scaling win*/
/* 24 */	0x8a,	/* 7:5	Horizontal decimation filter
			     4  [8] Horizontal start position of scaling win
			   3:2	[9:8] Pixel number per line on input
			   1:0  [9:8] Pixel number per line on output 	*/
/* 25 */	0xf0,	/* 7:0	[7:0] Line number per output field	*/
/* 26 */	0xf0,	/* 7:0	[7:0] Line number per input field	*/
/* 27 */	0x0f,	/* 7:0	[7:0] Vertical start of scaling window	*/
/* 28 */	0x80,	/*   7	Adaptive filter switch
			   6:5	Vertical luminance data processing
			     4	[8] Vertical start of scaling window 
			   3:2  [9:8] Line number per input field
			   1:0	[9:8] Line number per output field	*/
/* 29 */	0x16,	/* 7:0	[7:0] Vertical bypass start		*/
/* 2a */	0x00,	/* 7:0	[7:0] Vertical bypass count		*/
/* 2b */	0x00,	/*   4  [8] Vertical bypass start
			     2  [8] Vertical bypass count
			     0	Polarity, internally detected odd even flag */
/* 2c */	0x80,	/* 7:0	Set lower limit V for colour-keying	*/
/* 2d */	0x7f,	/* 7:0	Set upper limit V for colour-keying	*/
/* 2e */	0x80,	/* 7:0	Set lower limit U for colour-keying	*/
/* 2f */	0x7f,	/* 7:0	Set upper limit U for colour-keying	*/
/* 30 */	0xbf	/*   7	VRAM bus output format
			     6	Adaptive geometrical filter
			     5	Luminance limiting value
			     4	Monochrome and two's complement output data sel
			     3	Line quailifier flag
			     2	Pixel qualifier flag
			     1	Transparent data transfer
			     0	Extended formats enable bit		*/
};

static u_char bt254_default[NUM_BT254_REGS] = {
	0x00, 	/* 24 bpp */
	0xa0,
	0xa0,
	0xa0,
	0x50,
	0x50,
	0x50,
} ;


static void
bt254_write(struct meteor *mtr, u_char addr, u_char data)
{
	addr &= 0x7;						/* sanity? */
	mtr->bt254_reg[addr] = data;
	PCF8574_DATA_WRITE(mtr, data);				/* set data */
	PCF8574_CTRL_WRITE(mtr, (PCF8574_CTRL_REG(mtr) & ~0x7) | addr);
	PCF8574_CTRL_WRITE(mtr, PCF8574_CTRL_REG(mtr) & ~0x10);	/* WR/ to 0 */
	PCF8574_CTRL_WRITE(mtr, PCF8574_CTRL_REG(mtr) | 0x10);	/* WR to 1 */
	PCF8574_DATA_WRITE(mtr, 0xff);				/* clr data */

}


static void
bt254_init(struct meteor *mtr)
{
int	i;

	PCF8574_CTRL_WRITE(mtr, 0x7f);
	PCF8574_DATA_WRITE(mtr, 0xff);	/* data port must be 0xff */
	PCF8574_CTRL_WRITE(mtr, 0x7f);

	/* init RGB module for 24bpp, composite input */
	for(i=0; i<NUM_BT254_REGS; i++)
		bt254_write(mtr, i, bt254_default[i]);

	bt254_write(mtr, BT254_COMMAND, 0x00);	/* 24 bpp */
}

static void
bt254_ntsc(struct meteor *mtr, int arg)
{
        if (arg){
	  /* Set NTSC bit */
	  PCF8574_CTRL_WRITE(mtr, PCF8574_CTRL_REG(mtr) | 0x20);
	}
	else {
	  /* reset NTSC bit */
	  PCF8574_CTRL_WRITE(mtr, PCF8574_CTRL_REG(mtr) &= ~0x20);
	}
}

static void
select_bt254(struct meteor *mtr)
{

	/* disable saa7196, saaen = 1 */
	PCF8574_CTRL_WRITE(mtr, PCF8574_CTRL_REG(mtr) | 0x80);
	/* enable Bt254, bten = 0 */
	PCF8574_CTRL_WRITE(mtr, PCF8574_CTRL_REG(mtr) & ~0x40);
}

static void
select_saa7196(struct meteor *mtr)
{
	/* disable Bt254, bten = 1 */
	PCF8574_CTRL_WRITE(mtr, PCF8574_CTRL_REG(mtr) | 0x40);
	/* enable saa7196, saaen = 0 */
	PCF8574_CTRL_WRITE(mtr, PCF8574_CTRL_REG(mtr) & ~0x80);
}

static int 
set_fps(struct meteor *mtr, u_short fps)
{
    unsigned status;
    int maxfps, mask, length;
    float b, step;
    volatile struct saa7116 *s7116 = mtr->s7116;


    if (fps<1)
	return (-1);

    SAA7196_WRITE(mtr, SAA7196_STDC, SAA7196_REG(mtr, SAA7196_STDC) | 0x02);
    SAA7196_READ(mtr);
    status = s7116->i2c_stat.r_data;
    if((status & 0x40) == 0) {
	/* got video signal */
	if (status & 0x20) {
	    /* 60Hz system */
	    if (fps>30) 
		return (-1);
	    maxfps = 30;
	} else {
	    /* 50Hz system */
	    if (fps>25)
		return (-1);
	    maxfps = 25;
	}

	if (fps == maxfps) {
	    mask = 0x1;
	    length = 0x0;
	} else if ((float)fps == maxfps/2.0) {
	    mask = 0x1;
	    length = 0x1;
	} else if (fps > maxfps/2) {
	    mask = (1<<maxfps) - 1;
	    length = maxfps - 1;
	    step = (float)(maxfps - 1)/(float)(maxfps - fps);
	    for (b=step; b<maxfps; b+=step) {
		mask &= ~(1<<((int)b));  /* mask out the bth frame */
	    }
	} else {
	    mask = 0x1;
	    length = maxfps - 1;
	    step = (float)(maxfps - 1)/(float)(fps);
	    for (b=step+1; b<maxfps-1; b+=step) {
		mask |= (1<<((int)b));  /* mask in the bth frame */
	    }
	}
    } else {
	/* no signal, just use default */
	mask = 0x1;
	length = 0x0;
	fps = METEOR_MAXFPS;
    }

    mtr->fps = fps;
    s7116->field_mask_e = s7116->field_mask_o = mask; 
    s7116->mask_lengths = (length | (length<<16));

    return 0;
}


/*
 * Initialize the 7116, 7196 and the RGB module.
 */
static void
bsd_meteor_init(struct meteor *mtr)
{
  int 	i;

  /*
   * Check for the Philips SAA7196
   */
  if(talk_i2c(mtr, SAA7196_I2C_ADDR_W, 0, 0xff) == 0) {
    /*
     * Initialize 7196
     */
    for (i = 0; i < NUM_SAA7196_I2C_REGS; i++) 
      SAA7196_WRITE(mtr, i, saa7196_i2c_default[i]);
#ifdef DEBUG_METEOR
    /*
     * Get version number.
     */
    SAA7196_WRITE(mtr, SAA7196_STDC,
		  SAA7196_REG(mtr, SAA7196_STDC) & ~0x02);
    SAA7196_READ(mtr);
    printk(KERN_INFO "meteor%d: <Philips SAA 7196> rev 0x%x\n",
	   METEOR_NUM(mtr), (mtr->s7116->i2c_stat.r_data>>4));
#endif DEBUG_METEOR

  }
  /*
   * Check for RGB module, initialized if found.
   */
  if(talk_i2c(mtr,PCF8574_DATA_I2C_ADDR_W,0,0xff) == 0) {
    PRINTK((KERN_INFO "meteor%d: <Booktree 254 (RGB module)>\n",
	    METEOR_NUM(mtr)));	/* does this have a rev #? */
    bt254_init(mtr);	/* Set up RGB module */
    mtr->flags = METEOR_RGB;
  } else {
    PRINTK((KERN_INFO
	    "meteor%d: i2c abort normal if no RGB module\n", mtr->unit));
    mtr->flags = 0;
  }
  set_fps(mtr, METEOR_MAXFPS);
  mtr->flags |= METEOR_INITALIZED | METEOR_SYSTEM_DEFAULT | METEOR_DEV0 |
    METEOR_RGB16;
}



	/* interrupt handling routine 
	 *  complete meteor_read() if using interrupts
	 */

static void
meteor_intr(int irq, void *dev_id, struct pt_regs *fake)
{
  int dev;
  register struct meteor	*mtr;
  register volatile u_long *cap;
  register u_long status, cap_err, next_base;
  register struct saa7116 *s7116; 

  /*
   * More than one device could be using this interrupt, so ignore
   * irq and dev_id and check all devices.
   */

#if 0
  {
    /* DEBUG - print a timestamp to the system log!
     * A call to gettimeofday takes approx 1 usec on a PII400
     */
    static long long usec1 = 0;
    static long long usec_start = -1;
    static long long usec_old = 0;
    static int frame_cnt = 0;
    struct timeval tv;
    int t1, t2;
    do_gettimeofday(&tv);
    usec1 = tv.tv_sec * 1000000LL + tv.tv_usec;
    if (usec_start < 0) {
      usec_start = usec1;
      usec_old   = usec1;
    }
    t1 = usec1 - usec_start;
    t2 = usec1 - usec_old;
    /*printk (KERN_INFO "%06d %010d %06d\n", frame_cnt, t1, t2);*/
    usec_old = usec1;
    frame_cnt++;

  }
#endif

#if 0
  {
    /* DEBUG - generate a pulse train on DTR/RTS */
    static int flag = 1;
    static int MCR = 0;
    int tty = 0x2F8;	// 0x3F8=ttyS0    0x2F8=ttyS1
    if (flag) {
      flag = 0;
      MCR = inb (tty + UART_MCR);
    }
    MCR ^= (UART_MCR_DTR|UART_MCR_RTS);
    outb(MCR, tty + UART_MCR);
    /*printk (KERN_INFO "%02x %02x\n", MCR0, MCR1);*/
  }
#endif

  for ( dev = 0; dev < MeteorDevs; dev++ ) {

    mtr = meteors[dev];
    checkPCI(mtr);
    s7116 = mtr->s7116;
    status = s7116->ints_ctl;
    if ( ! status & 0x07 )
      continue;			/* This board does not want attention */
    cap = &(s7116->capt_ctl);
    cap_err = *cap & 0x00000f00;

    /*
     * Disable future interrupts if a capture mode is not selected.
     * This can happen when we are in the process of closing or 
     * changing capture modes, otherwise it shouldn't happen.
     */
    if(!(mtr->flags & METEOR_CAP_MASK)) {
      *cap &= 0x8ff0;	/* disable future interrupts */
    }
    /*
     * Check for errors.
     */
    if (cap_err) {
      if (cap_err & 0x300) {
#ifdef SHOW_CAPT_ERRORS
	if(mtr->fifo_errors % 50 == 0) {
	  PRINTK((KERN_INFO "meteor%d: capture error", mtr->unit));
	  PRINTK((KERN_INFO ": %s FIFO overflow.\n",
		  cap_err&0x0100? "even" : "odd"));
	}
#endif
	mtr->fifo_errors++ ;	/* increment fifo capture errors cnt */
      }
      if (cap_err & 0xc00) {
#ifdef SHOW_CAPT_ERRORS
	if(mtr->dma_errors % 50 == 0) {
	  PRINTK((KERN_INFO "meteor%d: capture error", mtr->unit));
	  PRINTK((KERN_INFO ": %s DMA address.\n",
		  cap_err&0x0400? "even" : "odd"));
	}
#endif
	mtr->dma_errors++ ;	/* increment DMA capture errors cnt */
      }
    }
    *cap |= 0x0f30;		/* clear error and field done */
    
    /*
     * In synchronous or round-robin capture mode we need to know what the 
     * address offset for the next field/frame will be.  next_base holds 
     * the value for the even dma buffers (for odd, one must add stride).
     *
     */
    next_base  = (u_long)mtr->frame_buffer + mtr->frame_offset[0];
    if ( mtr->cur_frame < mtr->frames ) {
      if( ((mtr->flags & METEOR_SYNCAP) && !mtr->synch_wait) 
	 || (mtr->flags & METEOR_ONCE) ) {
	next_base  += mtr->frame_offset[mtr->cur_frame] - mtr->frame_offset[0];
      }
    }

    /*
     * Count the field and clear the field flag.
     *
     * In synchronous capture mode, if we have room for another field,
     * adjust DMA buffer pointers.
     * When we are above the hi water mark (hiwat), mtr->synch_wait will
     * be set and we will not bump the DMA buffer pointers.  Thus, once
     * we reach the hi water mark,  the driver acts like a continuous mode
     * capture on the mtr->cur_frame frame until we hit the low water
     * mark (lowat).  The user had the option of stopping or halting
     * the capture if this is not the desired effect.
     */
    if (status & 0x1) {		/* even field */
      mtr->even_fields_captured++;
      mtr->mem->cur_field = 0;
      mtr->flags &= ~METEOR_WANT_EVEN;
    }
    if (status & 0x2) {		/* odd field */
      mtr->odd_fields_captured++;
      mtr->mem->cur_field = 1;
      mtr->flags &= ~METEOR_WANT_ODD;
      if( ((mtr->flags & METEOR_SYNCAP) && !mtr->synch_wait)
	   || (mtr->flags & METEOR_ONCE) ) {
	 s7116->dma_o[0] = mtr->dma_add_o[0] + next_base;
	 s7116->dma_o[1] = mtr->dma_add_o[1] + next_base;
	 s7116->dma_o[2] = mtr->dma_add_o[2] + next_base;
	 s7116->stride_o[0] = mtr->dma_str_o[0];
	 s7116->stride_o[1] = mtr->dma_str_o[1];
	 s7116->stride_o[2] = mtr->dma_str_o[2];
	 s7116->dma_end_o = (next_base + mtr->frame_size - 1 );	 
       }
    }

    /* Complete frame ?? */
    if (!(mtr->flags & METEOR_WANT_MASK) || (mtr->flags & METEOR_FIELD_MODE)) {

      mtr->frames_captured++;
      mtr->mem->frames_captured++;
      /*
       * post the completion time. 
       */
      if(mtr->flags & METEOR_WANT_TS) {
	  struct timeval *ts;
	  ts = (struct timeval *)(mtr->frame_buffer + 
				  mtr->frame_offset[mtr->cur_frame - 1] + 
				  mtr->frame_size);
	  do_gettimeofday(ts);
      }

      /*
       * Reset the want flags if in continuous or
       * synchronous capture mode and got both fields already.
       */
      if ((mtr->flags & (METEOR_CONTIN|METEOR_SYNCAP)) && !(mtr->flags & METEOR_WANT_MASK)) {
	switch(mtr->flags & METEOR_ONLY_FIELDS_MASK) {
	case METEOR_ONLY_ODD_FIELDS:
	  mtr->flags |= METEOR_WANT_ODD;
	  break;
	case METEOR_ONLY_EVEN_FIELDS:
	  mtr->flags |= METEOR_WANT_EVEN;
	  break;
	default:
	  mtr->flags |= METEOR_WANT_MASK;
	  break;
	}
      }
      /*
       * Special handling for synchronous capture mode.
       */
      if(mtr->flags & METEOR_SYNCAP) {
	struct meteor_mem *mm = mtr->mem;
	/*
	 * Mark the current frame as active.  It is up to
	 * the user to clear this, but we will clear it
	 * for the user for the current frame being captured
	 * if we are within the water marks (see below).
	 */
	mm->active |= 1 << (mtr->cur_frame - 1);
	mm->cur_frame = mtr->cur_frame;

	/*
	 * Since the user can muck with these values, we need
	 * to check and see if they are sane. If they don't
	 * pass the sanity check, disable the capture mode.
	 * This is rather rude, but then so was the user.
	 *
	 * Do we really need all of this or should we just
	 * eliminate the possiblity of allowing the
	 * user to change hi and lo water marks while it
	 * is running? XXX
	 */
	if(mm->lowat >= mtr->frames ||
	   mm->hiwat >= mtr->frames ||
	   mm->lowat > mm->hiwat ) {
	    *cap &= 0x8ff0;
	    mtr->flags &= ~(METEOR_SYNCAP|METEOR_WANT_MASK);
	    printk("Stopped capture because of illegitimate meteor_mem struct\n");
	    printk("lowat=%d, hiwat=%d, frames=%d\n", mm->lowat, mm->hiwat, mtr->frames);
	} else {
	  /*
	   * Ok, they are sane, now we want to
	   * check the water marks.
	   */
	  if (mm->lowat==0 && mm->hiwat==0) 
	      mtr->synch_wait = 0;
	  else {
	      if (mm->num_active_bufs <= mm->lowat)
		  mtr->synch_wait = 0;
	      if (mm->num_active_bufs >= mm->hiwat)
		  mtr->synch_wait = 1;
	  }
	  /*
	   * Clear the active frame bit for this frame
	   * and advance the counters if we are within
	   * the banks of the water marks. 
	   */
	  if (!mtr->synch_wait) {
	      mm->active &= ~(1 << (mtr->cur_frame));
	      mtr->cur_frame++;
	      if(mtr->cur_frame > mtr->frames)
		  mtr->cur_frame = 1;
	      mm->num_active_bufs++;
	  }
        } 
      }

      /* 
       * continous ONCE mode: advance current frame, check if we got all
       */
      if ( mtr->flags & METEOR_ONCE ) {
	struct meteor_mem *mm = mtr->mem;
	/* fortran alert: cur_frame starts at 1 */
	if ( mtr->cur_frame < mtr->frames ) {
	  mtr->cur_frame++;
	} else {
	  /* stop capturing since we got all frames */
	  *cap &= 0x8ff0;
	  mtr->flags &= ~(METEOR_ONCE|METEOR_CONTIN|METEOR_WANT_MASK);
	}
	mm->cur_frame = mtr->cur_frame;
      }
      
      /*
       * Wake up the user in single capture mode.
       */
      if(mtr->flags & METEOR_SINGLE)
	wake_up(&mtr->waitq);

      /*
       * If the user requested to be notified via signal,
       * let them know the frame is complete.
       */
      if(mtr->proc && mtr->signal)
	send_sig(mtr->signal, mtr->proc, 1);

    } else {
	/* not complete frame yet, but check if user wants a signal each field anyway */
	if (mtr->sigmode==METEOR_SIG_FIELD && mtr->proc && mtr->signal)
	    send_sig(mtr->signal, mtr->proc, 1);
    }

    mtr->s7116->ints_ctl |=  0x7;		/* clear interrupt status */
  }
  return;
}


/*---------------------------------------------------------
**
**	Meteor character device driver routines
**
**---------------------------------------------------------
*/


static int bsd_meteor_open(struct meteor *mtr)
  {
	int	i;

	if (!(mtr->flags & METEOR_INITALIZED))	/* device not found */
		return(-ENXIO);

	if (mtr->flags & METEOR_OPEN)		/* device is busy */
		return(-EBUSY);

	mtr->flags |= METEOR_OPEN;
	/*
	 * Make sure that the i2c regs are set the same for each open.
	 */
	for(i=0; i< NUM_SAA7196_I2C_REGS; i++) {
		SAA7196_WRITE(mtr, i, saa7196_i2c_default[i]);
	}

	mtr->fifo_errors = 0;
	mtr->dma_errors = 0;
	mtr->frames_captured = 0;
	mtr->even_fields_captured = 0;
	mtr->odd_fields_captured = 0;
	mtr->proc = (struct task_struct *)0;
	set_fps(mtr, METEOR_MAXFPS);

	return(0);
}

static void
start_capture(struct meteor *mtr, unsigned type)
{
  u_long cap, buf;
  struct saa7116 *s7116 = mtr->s7116;

  checkPCI(mtr);
  wait_jiffies(WAIT_JIFFS); /* Let board settle */
  mtr->flags |= type;
  buf = (u_long)(mtr->frame_buffer + mtr->frame_offset[0]);
#if (LINUX_VERSION_CODE >= VERSION(2,2,0))
  buf = virt_to_phys ((void*)buf);
#endif
  s7116->dma_e[0] = mtr->dma_add_e[0] + buf;
  s7116->dma_e[1] = mtr->dma_add_e[1] + buf;
  s7116->dma_e[2] = mtr->dma_add_e[2] + buf;
  s7116->dma_o[0] = mtr->dma_add_o[0] + buf;
  s7116->dma_o[1] = mtr->dma_add_o[1] + buf;
  s7116->dma_o[2] = mtr->dma_add_o[2] + buf;
  s7116->stride_e[0] = mtr->dma_str_e[0];
  s7116->stride_e[1] = mtr->dma_str_e[1];
  s7116->stride_e[2] = mtr->dma_str_e[2];
  s7116->stride_o[0] = mtr->dma_str_o[0];
  s7116->stride_o[1] = mtr->dma_str_o[1];
  s7116->stride_o[2] = mtr->dma_str_o[2];
  s7116->dma_end_e = s7116->dma_end_o =
    (buf + mtr->frame_size - 1);
  mtr->cur_frame = 1;

  switch(mtr->flags & METEOR_ONLY_FIELDS_MASK) {
  case METEOR_ONLY_EVEN_FIELDS:
    mtr->flags |= METEOR_WANT_EVEN;
    if(type == METEOR_SINGLE)
      cap = 0x8ff4;
    else
      cap = 0x8ff1;
    break;
  case METEOR_ONLY_ODD_FIELDS:
    mtr->flags |= METEOR_WANT_ODD;
    if(type == METEOR_SINGLE)
      cap = 0x8ff8;
    else
      cap = 0x8ff2;
    break;
  default:
    mtr->flags |= METEOR_WANT_MASK;
    if(type == METEOR_SINGLE)
      cap = 0x8ffc;
    else
      cap = 0x8ff3;
    break;
  }

  mtr->s7116->capt_ctl = cap;
  mtr->s7116->ints_ctl = 0x307;
}


static void
stop_capture(struct meteor *mtr) /* Stop the capture */
{
  mtr->s7116->capt_ctl = 0x8ff0;
  wait_jiffies(WAIT_JIFFS);
  mtr->flags &= ~(METEOR_CAP_MASK|METEOR_WANT_MASK);
}

static int bsd_meteor_read(struct meteor *mtr, char *user_buf, int user_count)
{
  u_int meteor_count = (mtr->rows * mtr->cols * mtr->depth) >> 3;

  if (mtr->frame_buffer == NULL)/* no frame buffer allocated (ioctl failed) */
    return(-ENOMEM);

  if (mtr->flags & METEOR_CAP_MASK)
    return(-EIO);		/* already capturing */

  if ( user_count != meteor_count )
    return(-EINVAL);

  start_capture(mtr, METEOR_SINGLE);
  interruptible_sleep_on(&mtr->waitq);
  if (signal_pending (current)) {
    stop_capture(mtr);
    return -EINTR;
  }
  /* NOTE: added +mtr->framne_offset[0] 17/7/96 */
  copy_to_user(user_buf,mtr->frame_buffer+mtr->frame_offset[0],meteor_count);
  mtr->flags &= ~(METEOR_SINGLE | METEOR_WANT_MASK);

  return(user_count);
}



static int bsd_meteor_ioctl(struct meteor *mtr, unsigned int cmd, void * arg)
{
  int	error;  
  unsigned int	temp;
  struct saa7116 *s7116 = mtr->s7116;
  u_long arg_val;
  u_short dir = _IOC_DIR(cmd);
  u_short size = _IOC_SIZE(cmd);
  u_short cr_cols, cr_rows;
	
  error = 0;

  if ( (dir == _IOC_WRITE) && size && (size <= sizeof(u_long)) )
    if ( (error = read_fs(arg, &arg_val, size)) ) {
      return error;
    }

  switch (cmd) {

  case METEORSTS:
    if(arg_val)
      mtr->flags |= METEOR_WANT_TS;
    else
      mtr->flags &= ~METEOR_WANT_TS;
    break;
  case METEORGTS:
    if(mtr->flags & METEOR_WANT_TS)
      return write_fs_val(1, arg, size);
    else
      return write_fs_val(0, arg, size);
    break;
  case METEORSFPS:
    if (set_fps(mtr, (u_short)arg_val) < 0)
	return -EINVAL;
    break;
  case METEORGFPS:
    return write_fs_val(mtr->fps, arg, size);
  case METEORSSIGNAL:
    mtr->signal = (int)(arg_val&(~METEOR_SIG_MODE_MASK));
    mtr->sigmode = (int)(arg_val&(METEOR_SIG_MODE_MASK));
    mtr->proc = current;
    mtr->pid = current->pid;
    break;
  case METEORGSIGNAL:
    return write_fs_val(mtr->signal, arg, size);
  case METEORSTATUS:	/* get 7196 status */
    {
      u_short stat;
      SAA7196_WRITE(mtr, SAA7196_STDC,
		    SAA7196_REG(mtr, SAA7196_STDC) | 0x02);
      SAA7196_READ(mtr);
      stat = s7116->i2c_stat.r_data;
      SAA7196_WRITE(mtr, SAA7196_STDC,
		    SAA7196_REG(mtr, SAA7196_STDC) & ~0x02);
      SAA7196_READ(mtr);
      stat |= (s7116->i2c_stat.r_data << 8);
      if ( (error = write_fs_val(stat, arg, size)) )
	return error;
      break;
    }
  case METEORSHUE:	/* set hue */
    SAA7196_WRITE(mtr, SAA7196_HUEC, (char)arg_val);
    break;
  case METEORGHUE:	/* get hue */
    return write_fs_val((char)(SAA7196_REG(mtr, SAA7196_HUEC)), arg, size);
  case METEORSCHCV:	/* set chrominance gain */
    SAA7196_WRITE(mtr, SAA7196_CGAINR, (char)arg_val);
    break;
  case METEORGCHCV:	/* get chrominance gain */
    return write_fs_val((char)(SAA7196_REG(mtr, SAA7196_CGAINR)), arg, size);
  case METEORSBRIG:	/* set brightness */
    SAA7196_WRITE(mtr, SAA7196_BRIG, (char)arg_val);
    break;
  case METEORGBRIG:	/* get brightness */
    return write_fs_val((char)(SAA7196_REG(mtr, SAA7196_BRIG)), arg, size);
    break;
  case METEORSCSAT:	/* set chroma saturation */
    SAA7196_WRITE(mtr, SAA7196_CSAT, (char)arg_val);
    break;
  case METEORGCSAT:	/* get chroma saturation */
    return write_fs_val((char)(SAA7196_REG(mtr, SAA7196_CSAT)), arg, size);
    break;
  case METEORSCONT:	/* set contrast */
    SAA7196_WRITE(mtr, SAA7196_CONT, (char)arg_val);
    break;
  case METEORGCONT:	/* get contrast */
    return write_fs_val((char)(SAA7196_REG(mtr, SAA7196_CONT)), arg, size);
    break;

  case METEORSBT254:
    if((mtr->flags & METEOR_RGB) == 0)
      return EINVAL;
    bt254_write(mtr, arg_val & 0xf, (arg_val & 0x0ff0) >> 4);
    break;
  case METEORGBT254:
    if((mtr->flags & METEOR_RGB) == 0)
      return -EINVAL;
    temp = arg_val & 0x7;
    return write_fs_val((short)(mtr->bt254_reg[temp] << 4 | temp), arg, size);		
  case METEORSHWS:	/* set horizontal window start */
    SAA7196_WRITE(mtr, SAA7196_HWS, arg_val);
    break;
  case METEORGHWS:	/* get horizontal window start */
    return write_fs_val((char)(SAA7196_REG(mtr, SAA7196_HWS)), arg, size);
    break;
  case METEORSVWS:	/* set vertical window start */
    SAA7196_WRITE(mtr, SAA7196_VWS, arg_val);
    break;
  case METEORGVWS:	/* get vertical window start */
    return write_fs_val((char)(SAA7196_REG(mtr, SAA7196_VWS)), arg, size);
    break;
  case METEORSINPUT:	/* set input device */
    switch(arg_val & METEOR_DEV_MASK) {
    case 0:			/* default */
    case METEOR_INPUT_DEV0:
	if(mtr->flags & METEOR_RGB)
	    select_saa7196(mtr);
	mtr->flags = (mtr->flags & ~METEOR_DEV_MASK)
	    | METEOR_DEV0;
	SAA7196_WRITE(mtr, 0x0e,
		      (SAA7196_REG(mtr, 0x0e) & ~0x3) | 0x0);
	SAA7196_WRITE(mtr, 0x06,
		      (SAA7196_REG(mtr, 0x06) & ~0x80));
	break;
    case METEOR_INPUT_DEV1:
	if(mtr->flags & METEOR_RGB)
	    select_saa7196(mtr);
	mtr->flags = (mtr->flags & ~METEOR_DEV_MASK)
	    | METEOR_DEV1;
	SAA7196_WRITE(mtr, 0x0e,
		      (SAA7196_REG(mtr, 0x0e) & ~0x3) | 0x1);
	SAA7196_WRITE(mtr, 0x06,
		      (SAA7196_REG(mtr, 0x06) & ~0x80));
	break;
    case METEOR_INPUT_DEV2:
	if(mtr->flags & METEOR_RGB)
	    select_saa7196(mtr);
	mtr->flags = (mtr->flags & ~METEOR_DEV_MASK)
	    | METEOR_DEV2;
	SAA7196_WRITE(mtr, 0x0e,
		      (SAA7196_REG(mtr, 0x0e) & ~0x3) | 0x2);
	SAA7196_WRITE(mtr, 0x06,
		      (SAA7196_REG(mtr, 0x06) & ~0x80));
	break;
    case METEOR_INPUT_DEV3:
	if(mtr->flags & METEOR_RGB)
	    select_saa7196(mtr);
	mtr->flags = (mtr->flags & ~METEOR_DEV_MASK)
	    | METEOR_DEV3;
	SAA7196_WRITE(mtr, 0x0e,
		      (SAA7196_REG(mtr, 0x0e) & ~0x3) | 0x3);
	SAA7196_WRITE(mtr, 0x06,
		      (SAA7196_REG(mtr, 0x06) & ~0x80) );
	break;
    case METEOR_INPUT_DEV_SVIDEO:
	if(mtr->flags & METEOR_RGB)
	    select_saa7196(mtr);
	mtr->flags = (mtr->flags & ~METEOR_DEV_MASK)
	    | METEOR_DEV_SVIDEO;
	SAA7196_WRITE(mtr, 0x0e,
		      (SAA7196_REG(mtr, 0x0e) & ~0x3) | 0x2);
	SAA7196_WRITE(mtr, 0x06,
		      (SAA7196_REG(mtr, 0x06) & ~0x80) | 0x80);
	break;
    case METEOR_INPUT_DEV_RGB:
	if((mtr->flags & METEOR_RGB) == 0)
	    return (-EINVAL);
	mtr->flags = (mtr->flags & ~METEOR_DEV_MASK)
	    | METEOR_DEV_RGB;
	SAA7196_WRITE(mtr, 0x0e,
		      (SAA7196_REG(mtr, 0x0e) & ~0x3) | 0x3);
	SAA7196_WRITE(mtr, 0x06,
		      (SAA7196_REG(mtr, 0x06) & ~0x80));
	select_bt254(mtr);
	SAA7196_WRITE(mtr, 0x0e,	/* chn 3 for synch */
		      (SAA7196_REG(mtr, 0x0e) & ~0x3) | 0x3);
	break;
    default:
	return (-EINVAL);
    }
    break;
  case METEORGINPUT:	/* get input device */
    return write_fs_val((u_long)(mtr->flags & METEOR_DEV_MASK), arg, size);
  case METEORSFMT:	/* set input format */
    switch(arg_val & METEOR_FORM_MASK ) {

    case 0:			/* default */
    case METEOR_FMT_NTSC:
      mtr->flags = (mtr->flags & ~METEOR_FORM_MASK) |
	METEOR_NTSC;
      SAA7196_WRITE(mtr, SAA7196_STDC, 
		    (SAA7196_REG(mtr, SAA7196_STDC) & ~0x01));
      SAA7196_WRITE(mtr, 0x0f,
		    (SAA7196_REG(mtr, 0x0f) & ~0xe0) | 0x40);
      SAA7196_WRITE(mtr, 0x22, 0x80);
      SAA7196_WRITE(mtr, 0x24, 
		    (SAA7196_REG(mtr, 0x24) & ~0x0c) | 0x08);
      SAA7196_WRITE(mtr, 0x26, 0xf0);
      SAA7196_WRITE(mtr, 0x28, 
		    (SAA7196_REG(mtr, 0x28) & ~0x0c)) ;
      if (mtr->flags & METEOR_RGB) {
	  bt254_ntsc(mtr, 1);
      }
      break;
    case METEOR_FMT_PAL:
      mtr->flags = (mtr->flags & ~METEOR_FORM_MASK) |
	METEOR_PAL;
      SAA7196_WRITE(mtr, SAA7196_STDC, 
		    (SAA7196_REG(mtr, SAA7196_STDC) & ~0x01));
      SAA7196_WRITE(mtr, 0x0f, 
		    (SAA7196_REG(mtr, 0x0f) & ~0xe0));
      SAA7196_WRITE(mtr, 0x22, 0x00);
      SAA7196_WRITE(mtr, 0x24, 
		    (SAA7196_REG(mtr, 0x24) | 0x0c));
      SAA7196_WRITE(mtr, 0x26, 0x20);
      SAA7196_WRITE(mtr, 0x28, 
		    (SAA7196_REG(mtr, 0x28) & ~0x0c) | 0x04) ;
      if(mtr->flags & METEOR_RGB){
	  bt254_ntsc(mtr, 0);			  
      }
      break;
    case METEOR_FMT_SECAM:
      mtr->flags = (mtr->flags & ~METEOR_FORM_MASK) |
	METEOR_SECAM;
      SAA7196_WRITE(mtr, SAA7196_STDC, 
		    (SAA7196_REG(mtr, SAA7196_STDC) & ~0x01) | 0x1);
      SAA7196_WRITE(mtr, 0x0f, 
		    (SAA7196_REG(mtr, 0x0f) & ~0xe0) | 0x20);
      SAA7196_WRITE(mtr, 0x22, 0x00);
      SAA7196_WRITE(mtr, 0x24, 
		    (SAA7196_REG(mtr, 0x24) | 0x0c));
      SAA7196_WRITE(mtr, 0x26, 0x20);
      SAA7196_WRITE(mtr, 0x28, 
		    (SAA7196_REG(mtr, 0x28) & ~0x0c) | 0x04) ;
      if(mtr->flags & METEOR_RGB){
	  bt254_ntsc(mtr, 0);			  
      }
      break;
    case METEOR_FMT_AUTOMODE:
      mtr->flags = (mtr->flags & ~METEOR_FORM_MASK) |
	METEOR_AUTOMODE;
      SAA7196_WRITE(mtr, SAA7196_STDC, 
		    (SAA7196_REG(mtr, SAA7196_STDC) & ~0x01));
      SAA7196_WRITE(mtr, 0x0f, 
		    (SAA7196_REG(mtr, 0x0f) & ~0xe0) | 0x80);
      break;
    default:
      return EINVAL;
    }
    break;
  case METEORGFMT:	/* get input format */
    return write_fs_val((u_long)(mtr->flags & METEOR_FORM_MASK), arg, size);

  case METEORCAPTUR:
    temp = mtr->flags;
    switch ((int)arg_val) {
    case METEOR_CAP_SINGLE: /* XXX: Should merge with meteor_read()*/
      
      if (mtr->frame_buffer==NULL)	/* no frame buffer allocated */
	return(-ENOMEM);

      if (temp & METEOR_CAP_MASK)
	return(-EIO);		/* already capturing */

      start_capture(mtr, METEOR_SINGLE);

      /* wait for capture to complete */
      interruptible_sleep_on(&mtr->waitq);
      if (signal_pending (current)) {
	stop_capture(mtr);
	return -EINTR;
      }
      mtr->flags &= ~(METEOR_SINGLE|METEOR_WANT_MASK);
      break;
    case METEOR_CAP_CONTINOUS:
      if (mtr->frame_buffer==NULL)	/* no frame buffer allocated */
	return(-ENOMEM);
      if (temp & METEOR_CAP_MASK)
	return(-EIO);		/* already capturing */
      start_capture(mtr, METEOR_CONTIN);
      break;
    case METEOR_CAP_CONT_ONCE:
      if (mtr->frame_buffer==NULL)	/* no frame buffer allocated */
	return(-ENOMEM);
      if (temp & METEOR_CAP_MASK)
	return(-EIO);		/* already capturing */
      start_capture(mtr, METEOR_CONTIN|METEOR_ONCE);
      break;
    case METEOR_CAP_STOP_CONT:
      if (mtr->flags & METEOR_CONTIN) {
	/* turn off capture */
	s7116->capt_ctl = 0x8ff0;
	mtr->flags &= ~(METEOR_ONCE|METEOR_CONTIN|METEOR_WANT_MASK);
      }
      break;
    default:
      error = -EINVAL;
    }
    break;
  case METEORCAPFRM:
    {
      struct meteor_capframe loc_frame, *frame=&loc_frame;

      if ( (error=(read_fs(arg, frame, size))) )
	return(error);
      switch (frame->command) {
      case METEOR_CAP_N_FRAMES:
	{
	  struct meteor_mem loc_mem, *mem=&loc_mem;
	  if (mtr->flags & METEOR_CAP_MASK)
	    return(-EIO);
	  if (mtr->frame_buffer == NULL)
	    return(-ENOMEM);
	  if ((mtr->frames < 2) ||
	      (frame->lowat >= mtr->frames) ||
	      (frame->hiwat >= mtr->frames) ||
	      (frame->lowat > frame->hiwat)) 
	    return(-EINVAL);
	  /* meteor_mem structure is on the page after the data */
	  mem = mtr->mem; 
	  mtr->cur_frame = 1;
	  mtr->synch_wait = 0;
	  mem->num_bufs = mtr->frames;
	  mem->frame_size = mtr->frame_size;
	  /* user and kernel change these */ 
	  mem->lowat = frame->lowat;
	  mem->hiwat = frame->hiwat;
	  mem->cur_frame = 1;
	  mem->active = 0;
	  mem->num_active_bufs = 0;
	  /* Start capture */
	  start_capture(mtr, METEOR_SYNCAP);
	  break;
	}
      case METEOR_CAP_STOP_FRAMES:
	if (mtr->flags & METEOR_SYNCAP) {
	  /* turn off capture */
	  s7116->capt_ctl = 0x8ff0;
	  mtr->flags &= ~(METEOR_SYNCAP|METEOR_WANT_MASK);
	}
	break;
      case METEOR_HALT_N_FRAMES:
	if(mtr->flags & METEOR_SYNCAP) {
	  s7116->capt_ctl = 0x8ff0;
	  mtr->flags &= ~(METEOR_WANT_MASK);
	}
	break;
      case METEOR_CONT_N_FRAMES:
	if(!(mtr->flags & METEOR_SYNCAP)) {
	  error = -EINVAL;
	  break;
	}
	start_capture(mtr, METEOR_SYNCAP);
	break;
      default:
	error = -EINVAL;
	break;
      }
      break;
    } 
  case METEORSETGEO:
    {
      struct meteor_geomet loc_geo, *geo=&loc_geo;

      error  = read_fs(arg, geo, size);
      if (error)
	return (error);

      /* set/clear even/odd flags */
      if(geo->oformat & METEOR_GEO_ODD_ONLY)
	mtr->flags |= METEOR_ONLY_ODD_FIELDS;
      else
	mtr->flags &= ~METEOR_ONLY_ODD_FIELDS;
      if(geo->oformat & METEOR_GEO_EVEN_ONLY)
	mtr->flags |= METEOR_ONLY_EVEN_FIELDS;
      else
	mtr->flags &= ~METEOR_ONLY_EVEN_FIELDS;

      /* can't change parameters while capturing */
      if (mtr->flags & METEOR_CAP_MASK)
	return(-EBUSY);

      /* The dma address increments in 4 byte steps, which means dma_end will
	 only be correct if columns%4 == 0.  For YUV we require columns%8 == 0
	 since for each 8 bits of Y there are 4 bits each for U and V.  This bit
	 of code takes the conservative option.  In field mode mode rows don't
	 need to be even, but we're conservative here again.  In field mode the 
	 number of fields specified (geo->frames) must be even since the board 
	 will only stop capture after odd fields.
      */
      if ((geo->columns & 0x3f8) != geo->columns) {
	  PRINTK((KERN_INFO
		  "meteor%d: ioctl: %d: columns too large or diviside by 8.\n",
		  mtr->unit, geo->columns));
	  error = -EINVAL;
      }
      if (((geo->rows & 0x7fe) != geo->rows) ||
	  ((geo->oformat & METEOR_GEO_FIELD_MASK) &&
	   ((geo->rows & 0x3fe) != geo->rows)) ) {
	PRINTK((KERN_INFO
		"meteor%d: ioctl: %d: rows too large or not even.\n",
		mtr->unit, geo->rows));
	error = -EINVAL;
      }
      if ((geo->oformat & METEOR_FIELD_MODE) && 
	  (geo->frames > 1) && 
	  (geo->frames & 0x1) == 0x1) {
	PRINTK((KERN_INFO
		"meteor%d: ioctl: %d: must have an even number of fields in field mode.\n",
		mtr->unit, geo->frames));
	error = -EINVAL;
      }

#if 0
      if (geo->frames > 32) {
	PRINTK((KERN_INFO "meteor%d: ioctl: too many frames.\n", mtr->unit));
	error = -EINVAL;
      }
#endif

      if (error) return error;

      mtr->rows = geo->rows;
      mtr->cols = geo->columns;
      mtr->frames = geo->frames;
      mtr->flags &= (~METEOR_FIELD_MODE);
      mtr->flags |= (geo->oformat & METEOR_FIELD_MODE);

      /* set defaults and end of buffer locations */
      mtr->dma_add_e[0] = 0;	/* DMA 1 even    */
      mtr->dma_add_e[1] = 0;	/* DMA 2 even    */
      mtr->dma_add_e[2] = 0;	/* DMA 3 even    */
      mtr->dma_add_o[0] = 0;	/* DMA 1 odd     */
      mtr->dma_add_o[1] = 0;	/* DMA 2 odd	 */
      mtr->dma_add_o[2] = 0;	/* DMA 3 odd     */
      mtr->dma_str_e[0] = 0;	/* Stride 1 even */
      mtr->dma_str_e[1] = 0;	/* Stride 2 even */
      mtr->dma_str_e[2] = 0;	/* Stride 3 even */
      mtr->dma_str_o[0] = 0;	/* Stride 1 odd  */
      mtr->dma_str_o[1] = 0;	/* Stride 2 odd  */
      mtr->dma_str_o[2] = 0;	/* Stride 3 odd  */

      switch (geo->oformat & METEOR_GEO_OUTPUT_MASK) {

      case 0:			/* default */
      case METEOR_GEO_RGB16:
	mtr->depth = 16;
	mtr->frame_size = (mtr->rows * mtr->cols * mtr->depth) >> 3;
	mtr->flags &= ~METEOR_OUTPUT_FMT_MASK;
	mtr->flags |= METEOR_RGB16;
	temp = (mtr->cols * mtr->depth) >> 3;

	switch(mtr->flags & METEOR_ONLY_FIELDS_MASK) {
	case METEOR_ONLY_ODD_FIELDS:
	  SAA7196_WRITE(mtr, 0x20, 0xd0);
	  break;
	case METEOR_ONLY_EVEN_FIELDS:
	  SAA7196_WRITE(mtr, 0x20, 0xf0);
	  break;
	default: /* interlaced even/odd */
	  if (mtr->flags & METEOR_FIELD_MODE) {
	      SAA7196_WRITE(mtr, 0x20, 0xb0);
	  } else {
	      mtr->dma_add_o[0] = temp;
	      mtr->dma_str_e[0] = mtr->dma_str_o[0] = temp;
	      SAA7196_WRITE(mtr, 0x20, 0x90);
	  }
	  break;
	}
	s7116->rt_mode_e = s7116->rt_mode_o =
	  (struct rt_mode){mode: 0x01, route: 0xeeeeee}; /* RGB 16 */
	  break;

      case METEOR_GEO_RGB24:
	mtr->depth = 32;
	mtr->frame_size = (mtr->rows * mtr->cols * mtr->depth) >> 3;
	mtr->flags &= ~METEOR_OUTPUT_FMT_MASK;
	mtr->flags |= METEOR_RGB24;
	temp = (mtr->cols * mtr->depth) >> 3;

	switch(mtr->flags & METEOR_ONLY_FIELDS_MASK) {
	case METEOR_ONLY_ODD_FIELDS:
	  SAA7196_WRITE(mtr, 0x20, 0xd2);
	  break;
	case METEOR_ONLY_EVEN_FIELDS:
	  SAA7196_WRITE(mtr, 0x20, 0xf2);
	  break;
	default: 
	  if (mtr->flags & METEOR_FIELD_MODE) {
	      /* odd/even separate */
	      SAA7196_WRITE(mtr, 0x20, 0xb2);
	  } else {
	      /* even/odd */
	      mtr->dma_add_o[0] = (mtr->cols * mtr->depth) >> 3;
	      mtr->dma_str_e[0] = mtr->dma_str_o[0] = (mtr->cols * mtr->depth) >> 3;
	      SAA7196_WRITE(mtr, 0x20, 0x92);
	  }
	  break;
	}
	s7116->rt_mode_e = s7116->rt_mode_o =
	  (struct rt_mode){mode: 0x00, route: 0x393939}; /* RGB 24 */
	  break;

      case METEOR_GEO_YUV_PLANAR:
      case METEOR_GEO_YUV_422:
	if ((geo->oformat & METEOR_GEO_YUV_12) && (geo->oformat & METEOR_GEO_YUV_9)) {
	  PRINTK((KERN_INFO "meteor%d: ioctl: YUV_9 -and- YUV_12 ??.\n", mtr->unit));
	  error = -EINVAL;
	}
	if(error) return error;
	
	if ( geo->oformat & METEOR_GEO_YUV_9 ) {
	  mtr->depth = 9;
	  cr_cols = mtr->cols >> 2;
	  cr_rows = mtr->rows >> 2;
	  s7116->rt_mode_e = s7116->rt_mode_o = 
	    (struct rt_mode){mode: 0xc3, route: 0xaaaaff};
	} else {
	  if ( geo->oformat & METEOR_GEO_YUV_12 ) {
	    mtr->depth = 12;
	    cr_cols = mtr->cols >> 1;
	    cr_rows = mtr->rows >> 1;
	    s7116->rt_mode_e = s7116->rt_mode_o = 
	      (struct rt_mode){mode: 0xc2, route: 0xaaaaff};
	  } else { /* YUV16 */
	    mtr->depth = 16;
	    cr_cols = mtr->cols >> 1;
	    cr_rows = mtr->rows;
	    s7116->rt_mode_e = s7116->rt_mode_o = 
	      (struct rt_mode){mode: 0xc1, route: 0xaaaaff};
	  }
	}
	temp = mtr->rows * mtr->cols;	/* compute (Y) frame size */
	mtr->frame_size = (temp * mtr->depth) >> 3;
	mtr->flags &= ~METEOR_OUTPUT_FMT_MASK;
	mtr->flags |= METEOR_YUV_PLANAR;

	switch(mtr->flags & METEOR_ONLY_FIELDS_MASK) {
	case METEOR_ONLY_ODD_FIELDS:
	  mtr->dma_add_o[1] = temp;	                /* U Odd */
	  mtr->dma_add_o[2] = temp + cr_cols * cr_rows; /* V Odd */
	  SAA7196_WRITE(mtr, 0x20, 0xd1);
	  break;
	case METEOR_ONLY_EVEN_FIELDS:
	  mtr->dma_add_e[1] = temp;	                /* U Even */
	  mtr->dma_add_e[2] = temp + cr_cols * cr_rows; /* V Even */
	  SAA7196_WRITE(mtr, 0x20, 0xf1);
	  break;
	default: 
	  if (mtr->flags & METEOR_FIELD_MODE) {
	    /*  even/odd separate*/
	    mtr->dma_add_o[1] = temp;	                  /* U Odd */
	    mtr->dma_add_o[2] = temp + cr_cols * cr_rows; /* V Odd */
	    mtr->dma_add_e[1] = temp;	                  /* U Even */
	    mtr->dma_add_e[2] = temp + cr_cols * cr_rows; /* V Even */
	      SAA7196_WRITE(mtr, 0x20, 0xb1);
	  } else {
	    /* interlaced odd even */
	    mtr->dma_add_o[0] = mtr->cols;                          /* Y Odd */
	    mtr->dma_add_e[1] = temp;		                    /* U Even */
	    mtr->dma_add_o[1] = temp + cr_cols;		            /* U Odd */
	    mtr->dma_add_e[2] = temp + cr_cols * cr_rows;           /* V Even */
	    mtr->dma_add_o[2] = temp + cr_cols * cr_rows + cr_cols; /* V Odd */
	    mtr->dma_str_e[0] = mtr->dma_str_o[0] = mtr->cols;	    /* Y Stride */
	    mtr->dma_str_e[1] = mtr->dma_str_o[1] = cr_cols;	    /* U Stride */
	    mtr->dma_str_e[2] = mtr->dma_str_o[2] = cr_cols;	    /* V Stride */
	    SAA7196_WRITE(mtr, 0x20, 0x91);
	  }
	  break;
	}
	switch (geo->oformat &
		(METEOR_GEO_YUV_12 | METEOR_GEO_YUV_9)) {
	case METEOR_GEO_YUV_9:
	  s7116->rt_mode_e = s7116->rt_mode_o = 
	    (struct rt_mode){mode: 0xc3, route: 0xaaaaff};
	    break;
	case METEOR_GEO_YUV_12:
	  s7116->rt_mode_e = s7116->rt_mode_o = 
	    (struct rt_mode){mode: 0xc2, route: 0xaaaaff};
	    break;
	default:
	  s7116->rt_mode_e = s7116->rt_mode_o = 
	    (struct rt_mode){mode: 0xc1, route: 0xaaaaff};
	    break;
	}
	break;

      case METEOR_GEO_YUV_PACKED:
	mtr->depth = 16;
	mtr->frame_size = (mtr->rows * mtr->cols * mtr->depth) >> 3;
	mtr->flags &= ~METEOR_OUTPUT_FMT_MASK;
	mtr->flags |= METEOR_YUV_PACKED;
	
	switch(mtr->flags & METEOR_ONLY_FIELDS_MASK) {
	case METEOR_ONLY_ODD_FIELDS:
	  SAA7196_WRITE(mtr, 0x20, 0xd1);
	  break;
	case METEOR_ONLY_EVEN_FIELDS:
	  SAA7196_WRITE(mtr, 0x20, 0xf1);
	  break;
	default: /* odd / even */
	  if (mtr->flags & METEOR_FIELD_MODE) {
	      SAA7196_WRITE(mtr, 0x20, 0xb1);
	  } else { /* interlaced even/odd */
	      mtr->dma_add_o[0] = (mtr->cols * mtr->depth) >> 3;
	      mtr->dma_str_e[0] = mtr->dma_str_o[0] = (mtr->cols * mtr->depth) >> 3;
	      SAA7196_WRITE(mtr, 0x20, 0x91);
	  }
	  break;
	}
	s7116->rt_mode_e = s7116->rt_mode_o = 
	  (struct rt_mode){mode: 0x41, route: 0xeeeeee}; /* YUV 16 packed */
	  break;
      default:
	error = -EINVAL;	/* invalid argument */
	PRINTK((KERN_INFO "meteor%d: ioctl: invalid output format\n",mtr->unit));
	break;
      }

      /* only now can we allocate the frame buffer memory */
      /* since now the frame size is known */
      temp= mtr->frame_size * mtr->frames;
      if ( temp ) {
	/* meteor_mem structure for SYNC Capture */
	if (geo->frames > 1) 
	  temp += PAGE_SIZE;
	/* why pass the size (temp) wanted ?? */
	error = alloc_frame_buffer(mtr, temp);
      }
      if(error) return error;

      /* set the saa7116 dma registers with the FIRST frame or field */

      /* set cols */
      SAA7196_WRITE(mtr, 0x21, mtr->cols & 0xff);
      SAA7196_WRITE(mtr, 0x24,
		    ((SAA7196_REG(mtr, 0x24) & ~0x03) |
		     ((mtr->cols >> 8) & 0x03)));
      /* set rows */
      if(mtr->flags & (METEOR_ONLY_FIELDS_MASK|METEOR_FIELD_MODE)) {
	SAA7196_WRITE(mtr, 0x25, ((mtr->rows) & 0xff));
	SAA7196_WRITE(mtr, 0x28,
		      ((SAA7196_REG(mtr, 0x28) & ~0x03) |
		       (((mtr->rows) >> 8) & 0x03)));
      } else {
	SAA7196_WRITE(mtr, 0x25, ((mtr->rows >> 1) & 0xff));
	SAA7196_WRITE(mtr, 0x28,
		      ((SAA7196_REG(mtr, 0x28) & ~0x03) |
		       ((mtr->rows >> 9) & 0x03)));
      }
      /* set signed/unsigned chrominance */
      SAA7196_WRITE(mtr, 0x30, (SAA7196_REG(mtr, 0x30) & ~0x10) |
		    ((geo->oformat&METEOR_GEO_UNSIGNED)?0:0x10));
      break;
    }
  case METEORGETGEO:
    {
      struct meteor_geomet loc_geo, *geo=&loc_geo;
      geo->rows = mtr->rows;
      geo->columns = mtr->cols;
      geo->frames = mtr->frames;
      geo->oformat = ((mtr->flags & (METEOR_FIELD_MODE | METEOR_OUTPUT_FMT_MASK | 
				     METEOR_ONLY_FIELDS_MASK)) |
		      (SAA7196_REG(mtr, 0x30) & 0x10 ? 0
		                                     : METEOR_GEO_UNSIGNED));
      switch(s7116->rt_mode_e.mode) {
      case	0xc3:
	geo->oformat |=  METEOR_GEO_YUV_9;
	break;
      case	0xc2:
	geo->oformat |=  METEOR_GEO_YUV_12;
	break;
      }

      return write_fs_ptr(geo, arg, size);
    }
  case METEORSCOUNT:	/* (re)set error counts */
    {
      struct meteor_counts loc_cnt, *cnt = &loc_cnt;
      if ( (error=(read_fs(arg, cnt, size))) )
	return(error);
      mtr->fifo_errors = cnt->fifo_errors;
      mtr->dma_errors = cnt->dma_errors;
      mtr->frames_captured = cnt->frames_captured;
      mtr->even_fields_captured = cnt->even_fields_captured;
      mtr->odd_fields_captured = cnt->odd_fields_captured;
      break;
    }
  case METEORGCOUNT:	/* get error counts */
    {
      struct meteor_counts loc_cnt, *cnt = &loc_cnt;

      cnt->fifo_errors = mtr->fifo_errors;
      cnt->dma_errors = mtr->dma_errors;
      cnt->frames_captured = mtr->frames_captured;
      cnt->even_fields_captured = mtr->even_fields_captured;
      cnt->odd_fields_captured = mtr->odd_fields_captured;
      return write_fs_ptr(cnt, arg, size);
    }
  case METEORGCAPT:	/* get capt_ctl */
    {
      u_long cap;
      cap = mtr->s7116->capt_ctl;
      return write_fs_ptr(&cap, arg, size);
    }
  case METEORGFROFF:	/* get frame offsets */
    {
      struct meteor_frame_offset off, *poff = &off; 
      memcpy(poff->frame_offset, mtr->frame_offset, sizeof(struct meteor_frame_offset) );
      off.fb_size = mtr->fb_size;
      off.mem_off = (u_long)mtr->mem - (u_long)mtr->frame_buffer;
      return write_fs_ptr(poff, arg, size);
    }
  default:
    PRINTK((KERN_INFO "meteor%d: ioctl: invalid ioctl request\n", mtr->unit));
    error = -ENOTTY;
    break;
  }  
  return(error);

}





