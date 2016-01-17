/* -*- linux-c -*- */

/*
 *  ussp.h
 *
 *  Copyright (C) 2000 R.E.Wolff@BitWizard.nl, patrick@BitWizard.nl
 *
 *  Version 1.0 July 2000 .
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 */

#ifdef __KERNEL__

#define		MCU_MAX_BUF_SIZE	256
#define		MCU_MAX_FIFO_SIZE	128*1024
struct mcu_device {
	struct tty_struct      *tty;
	struct tty_port tty_port;
	struct device *tty_dev;
	struct serio *serio;
	int count;
	struct work_struct  rx_work;
	struct delayed_work  tx_work;
	struct kfifo rx_fifo;
	struct kfifo tx_fifo;
    	struct input_dev *input_dev;
	int rx_frame_len;
	int tx_frame_len;
	unsigned char rx_buf[MCU_MAX_BUF_SIZE];
	unsigned char tx_buf[MCU_MAX_BUF_SIZE];
	unsigned long flags;
	spinlock_t read_lock;
	spinlock_t write_lock;
};

#endif
