/*
 * s3c6410x_timer.h: 
 *
 * Copyright (C) 2013 alloc <alloc.young@gmail.com>
 * Skyeye Develop Group, for help please send mail to
 * <skyeye-developer@lists.gro.clinux.org>
 *
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __S3C6410X_TIMER_H__ 
#define __S3C6410X_TIMER_H__  

#include <simics/dev_iface.h>
#include "schedule_tmp.h"

#define TCFG0       (0x0)  /*Timer Configuration Register 0 that configures the two 8-bit Prescaler and DeadZone Length*/
#define TCFG1       (0x4)  /*Timer Configuration Register 1 that controls 5 MUX and DMA Mode Select Bit*/
#define TCON        (0x8)  /*Timer Control Register*/
#define TCNTB0      (0xc)  /*Timer 0 Count Buffer Register*/
#define TCMPB0      (0x10) /*Timer 0 Compare Buffer Register*/
#define TCNTO0      (0x14) /*Timer 0 Count Observation Register*/
#define TCNTB1      (0x18) /*Timer 1 Count Buffer Register*/
#define TCMPB1      (0x1c) /*Timer 1 Cpmpare Buffer Register*/
#define TCNTO1      (0x20) /*Timer 1 Count Observation Register*/
#define TCNTB2      (0x24) /*Timer 2 Count Buffer Register*/
#define TCNTO2      (0x2c) /*Timer 2 Count Observation Register*/
#define TCNTB3      (0x30) /*Timer 3 Count Buffer Register*/
#define TCNTO3      (0x38) /*Timer 3 Count Observation Register*/
#define TCNTB4      (0x3c) /*Timer 4 Count Buffer Register*/
#define TCNTO4      (0x40) /*Timer 4 Count Observatin Register*/
#define TINT_CSTAT  (0x44) /*Timer interrupt Control and Status Register*/
#define S3C6410_TIMER_NUM 5

typedef struct timer_device {
	conf_object_t obj;
	struct {
		uint32  tcfg0;
		uint32  tcfg1;
		uint32  tcon;
		int tcnt[S3C6410_TIMER_NUM];
		uint32 tcmp[S3C6410_TIMER_NUM];
		int tcntb[S3C6410_TIMER_NUM];
		uint32 tcmpb[S3C6410_TIMER_NUM];
		uint32 tcnto[S3C6410_TIMER_NUM];
		/* shenoubang 2012-4-19 */
		uint32 tint_cstat;
	} regs;
	struct {
		conf_object_t *obj;
		const char *port;
		const signal_interface_t *iface;
	} irq;
	sched_group_t *group;
	schedule_t    *sched;
} s3c6410x_timer_t;

#endif /* __S3C6410X_TIMER_H__ */
