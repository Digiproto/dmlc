/*
 * dev_iface.h: 
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

#ifndef __DEV_IFACE_H__ 
#define __DEV_IFACE_H__  
#include <simics/generic_transaction.h>
typedef struct signal {
	void (*signal_raise)(conf_object_t *obj);
	void (*signal_lower)(conf_object_t *obj);
} signal_interface_t;

typedef struct memory_space {
	exception_type_t (*access)(conf_object_t *obj,
								generic_transaction_t *mop);
	exception_type_t (*access_simple)(conf_object_t *obj,
									  conf_object_t *initiator,
									  physical_address_t addr,
									  uint8 *buf,
									  physical_address_t len,
									  read_or_write_t rw,
									  endianness_t endian);
} memory_space_interface_t;

typedef struct serial_device {
	int (*write) (conf_object_t *obj, int value);
	void (*receive_ready) (conf_object_t *obj);
} serial_device_interface_t;

typedef struct simple_interrupt {
	void (*interrupt) (conf_object_t *obj, int irq);
	void (*interrupt_clear) (conf_object_t *obj, int irq);
} simple_interrupt_interface_t;

typedef enum {
	I2C_flag_exclusive,
	I2C_flag_shared
} i2c_device_flag_t;

typedef struct i2c_bus {
	int (*start) (conf_object_t *obj, uint8_t address);
	int (*stop) (conf_object_t *obj);
	uint8_t (*read_data) (conf_object_t *obj);
	void (*write_data) (conf_object_t *obj, uint8_t data);
	int (*register_device) (conf_object_t *bus, conf_object_t *device, uint8_t address, uint8_t mask, i2c_device_flag_t flags);
	void (*unregister_device) (conf_object_t *bus, conf_object_t *device, uint8_t address, uint8_t mask);
} i2c_bus_interface_t;

typedef int i2c_device_state_t;
typedef struct i2c_device {
	int (*set_state) (conf_object_t *obj, i2c_device_state_t state, uint8_t address);
	uint8_t (*read_data) (conf_object_t *obj);
	void (*write_data) (conf_object_t *obj, uint8_t value);
} i2c_device_interface_t;

#endif /* __DEV_IFACE_H__ */
