/*
 *	saa71x6.h  -- Definition of the saa7116 registers, and hopefully
 *      someday something about the saa7196 also.
 *
 *	Copyright (C) 1996  Jim Bray (http://as220.org/jb).
 *
 *      This work was supported by the AI Lab at Brown University,
 *      and RWI (Real World Interface) Inc.
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
 */

/*
 * The registers of the saa7116 video chip.
 */

/*
 * Bitfield words are each defined as a separate struct. This unclutters
 * the actual register struct definitions, and also enables use of gcc
 * constructor expressions for assignment, which is readable and results
 * in better object code than individual assignments to bitfields.
 *
 * Note bitfields build from LSB up.
 *
 * Slight variances from actual field width are made to use byte boundaries
 * when possible. Actual widths are noted in comments.
 */

/* Bitfields of the 7116 */

struct rt_mode {
  unsigned mode:8;		/* 0:7  Pixel format for even fields */
  unsigned route:24;		/* 8:31 Pixel router mode for even fields */
};

struct ff_trig {
  unsigned packed:7;		/* 0:7(0:6) Fifo trigger Packed Mode */
  unsigned :9;			/* 8:15(7:15) Unused */
  unsigned planar:7;		/* 16:31(16:22) Fifo trigger Planar Mode */
  unsigned :9;			/* 23:31 Unused */
};

struct i2c_stat {
  unsigned :9;			/* 0:8 Not needed */
  unsigned d_abort:1;		/* 9:9 Status of i2c write: 0==success */
  unsigned :14;			/* 10:23 Not needed */
  unsigned r_data:8;		/* 24:31 Data returned by i2c read */
};

struct i2c_cmd {
  unsigned w_data:8;		/* 0:7 Data to be written out */
  unsigned reg_addr:8;		/* 8:15 Register address in device space */
  unsigned dev_addr:8;		/* 16:23 16: 0==write; 17:23: Device address */
  unsigned xfer:1;		/* 24: Set to start write. Clears when done */
  unsigned :7;			/* 25:31 Unused */
};
#define v_struct volatile struct


				/* (pointer-addition-values)hex-offset(s) */
struct saa7116 {
  u_long dma_e[3];		/* (0-2)00-08: Base address, even fields */
  u_long dma_o[3];		/* (3-5)0c-14: Base address, odd fields */
  u_long stride_e[3];		/* (6-8)18-20: Address stride, even fields */
  u_long stride_o[3];		/* (9-11)24-2c: Address stride, odd fields */
  struct rt_mode rt_mode_e;	/* (12)30: Route and Mode for even fields */
  struct rt_mode rt_mode_o;	/* (13)34: Route and Mode for odd fields */
  struct ff_trig fifo_trigger;	/* (14)38 Fifo triggers: Planar and Packed */
  u_long field_toggle;		/* (15)3c */
  volatile u_long capt_ctl;	/* (16)40: Capture control, etc. */
  u_long retry_wait;		/* (17)44 */
  volatile u_long ints_ctl;	/* (18)48: Interrupt control and status */
  u_long field_mask_e;		/* (19)4c */
  u_long field_mask_o;		/* (20)50 */
  u_long mask_lengths;		/* (21)54 */
  u_long fifo_limits;		/* (22)58 */
  u_long i2c_clocks;		/* (23)5c */
  v_struct i2c_stat i2c_stat;	/* (24)60 i2c mode control and status */
  v_struct i2c_cmd i2c_cmd;	/* (25)64 Direct mode command and data */
  u_long i2c_auto_0_e;		/* (26)68 */
  u_long i2c_auto_2_e;		/* (27)6c */
  u_long i2c_auto_4_e;		/* (28)70 */
  u_long i2c_auto_6_e;		/* (29)74 */
  u_long i2c_auto_0_o;		/* (30)78 */
  u_long i2c_auto_2_o;		/* (31)7c */
  u_long i2c_auto_4_o;		/* (32)80 */
  u_long i2c_auto_6_o;		/* (33)84 */
  u_long i2c_regs_enable;	/* (34)88 */
  u_long dma_end_e;		/* (35)8c */
  u_long dma_end_o;		/* (36)90 */
};

				/* Define saa7196 if we get specs */

#undef v_struct



