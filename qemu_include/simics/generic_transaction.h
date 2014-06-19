/*
 * generic_transaction.h: 
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
/**
 * @file generic_transaction.h
 * @brief generic transaction 
 * @author alloc, alloc.young@gmail.com
 * @version 1.0
 * @date 2013-05-24
 */

#ifndef __GENERIC_TRANSACTION_H__ 
#define __GENERIC_TRANSACTION_H__  
#include "simics/base_types.h"
typedef uint16 stall_id_t;

typedef enum {
	INIT_TYPE_CPU,
	INIT_TYPE_DEVICE
}ini_type_t;

typedef enum {
	Sim_Endian_Target,
	SIM_Endian_Host_From_BE,
	SIM_Endian_Host_From_LE,
	SIM_Endian_Qemu_Specific
} endianness_t;

typedef enum {
	Sim_RW_Read = 0,
	Sim_RW_Write = 1
}read_or_write_t;

typedef enum {
	Sim_PE_No_Exception
} exception_t;

/*borrowed from simics memory.h*/
enum {
	SIM_Trn_Instr = 1,
	SIM_Trn_Write = 2,
	SIM_Trn_Control = 4,
	SIM_Trn_Prefetch = 8,
	Sim_Trans_Store = SIM_Trn_Write
};

typedef enum {
	SIM_Trans_Load        = 0,
	SIM_Trans_Store       = SIM_Trn_Write,
	SIM_Trans_Instr_Fetch = SIM_Trn_Instr,
	SIM_Trans_Prefech     = SIM_Trn_Prefetch | SIM_Trn_Control,
	SIM_Trans_Cache       = SIM_Trn_Control
} mem_op_type_t;

typedef struct generic_transaction {
	logical_address_t logical_address;
	physical_address_t physical_address; 
	unsigned int size;
	uint64_t value;
	mem_op_type_t op;
	unsigned int atomic:1;

	unsigned int inquiry:1;

	unsigned int speculative:1;

	unsigned int ignore:1;

	unsigned int may_stall:1;

	unsigned int ma_no_reissue:1;

	unsigned int reissue:1;

	unsigned int block_STC:1;

	unsigned int use_page_cache:1;
	
	unsigned int inverse_endian:1;

	unsigned int page_cross:2;


	unsigned int use_iostc:1;
	
	unsigned int iostc_blocked:1;

	unsigned int inhibit_iostc_lookup:1;

	unsigned int mem_hier_fetch:1;

	unsigned int block_flags;

	ini_type_t ini_type;
	
	conf_object_t *ini_ptr;
	
	int id;

	exception_type_t exception;
	
	void *user_ptr;

	uint8 *real_address;

	int space_count;

	stall_id_t stall_id;

} generic_transaction_t;

#define SIM_get_mem_op_value_le SIM_get_mem_op_value
#define SIM_get_mem_op_value_be SIM_get_mem_op_value

#define SIM_set_mem_op_value_le SIM_set_mem_op_value
#define SIM_set_mem_op_value_be SIM_set_mem_op_value
#define SIM_set_mem_op_type SIM_set_mem_op

static inline bool SIM_access_type_is_read(read_or_write_t rw) {
	return rw == Sim_RW_Read;
}

static inline bool SIM_access_type_is_write(read_or_write_t rw) {
	return rw == Sim_RW_Write;
}

static inline void SIM_set_mem_op(generic_transaction_t *memop, mem_op_type_t op) {
	memop->op = op;	
}

static inline bool SIM_mem_op_is_read(generic_transaction_t *memop) {
	return !(memop->op & SIM_Trn_Write);
}

static inline bool SIM_mem_op_is_write(generic_transaction_t *memop) {
	return memop->op & SIM_Trn_Write;
}

static inline bool SIM_get_mem_op_inquiry(generic_transaction_t *memop) {
	return memop->inquiry;
}

static inline uint64_t SIM_mem_op_get_value(generic_transaction_t *memop) {
	return memop->value;
}

static inline unsigned SIM_get_mem_op_size(generic_transaction_t *mop) {
	return mop->size;
}

static inline uint8 *SIM_get_mem_op_buf(generic_transaction_t *mop) {
	return mop->real_address;
}

static inline void SIM_mem_op_set_value(generic_transaction_t *memop, uint64_t value) {
	memop->value = value;
}

static inline physical_address_t SIM_mem_op_physical_address(generic_transaction_t *memop) {
	return memop->physical_address;
}

#endif /* __GENERIC_TRANSACTION_H__ */
