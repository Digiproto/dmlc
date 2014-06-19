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
#include "simics/generic_transaction.h"
#include "simics/attribute.h"
typedef struct signal {
	void (*signal_raise)(conf_object_t *obj);
	void (*signal_lower)(conf_object_t *obj);
	void (*signal_level)(conf_object_t *obj, int level);
	void (*deprecated_signal_level)(conf_object_t *obj, int level);
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
typedef struct { void (*set_cs)(conf_object_t *obj, int cs); void (*set_sk)(conf_object_t *obj, int sk); void (*set_di)(conf_object_t *obj, int di); int (*get_do)(conf_object_t *obj); uint16 (*read_word)(conf_object_t *obj, int offset); void (*write_word)(conf_object_t *obj, int offset, uint16 value); } microwire_interface_t;
typedef int i2c_device_state_t;
typedef struct i2c_device {
	int (*set_state) (conf_object_t *obj, i2c_device_state_t state, uint8_t address);
	uint8_t (*read_data) (conf_object_t *obj);
	void (*write_data) (conf_object_t *obj, uint8_t value);
} i2c_device_interface_t;
typedef conf_object_t log_object_t;
static uint32 SIM_log_level(const log_object_t *obj){return 0;}
typedef struct { } image_spage_t;
typedef struct { 
	void (*read)(conf_object_t *img, void *to_buf, uint64 start, size_t length); 
	void (*write)(conf_object_t *img, const void *from_buf, uint64 start, size_t length); 
	int (*for_all_spages)(conf_object_t *img, int (*f)(image_spage_t *p, uint64 ofs, void *arg), void *arg); 
	void (*set_persistent)(conf_object_t *obj); 
	void (*save_to_file)(conf_object_t *obj, const char *file, uint64 start, uint64 length);
	void (*save_diff)(conf_object_t *obj, const char *file); 
	void (*clear_range)(conf_object_t *obj, uint64 start, uint64 length); 
	void (*fill)(conf_object_t *obj, uint64 start, uint64 length, uint8 value); 
	void (*dealloc_icode_page)(conf_object_t *obj, image_spage_t *spage);
	uint64 (*size)(conf_object_t *obj); void (*set)(conf_object_t *obj, uint64 ofs, bytes_t b); 
	bytes_t (*get)(conf_object_t *obj, uint64 ofs, size_t size); 
} image_interface_t;

static void *SIM_get_interface(conf_object_t *obj, const char *name){return NULL;}
typedef struct { tuple_int_string_t (*disassemble)(conf_object_t *obj, generic_address_t address, attr_value_t instruction_data, int sub_operation); void (*set_program_counter)(conf_object_t *obj, logical_address_t pc); logical_address_t (*get_program_counter)(conf_object_t *obj); physical_block_t (*logical_to_physical)(conf_object_t *obj, logical_address_t address, access_t access_type); int (*enable_processor)(conf_object_t *obj); int (*disable_processor)(conf_object_t *obj); int (*get_enabled)(conf_object_t *obj); cpu_endian_t (*get_endian)(conf_object_t *obj); conf_object_t *(*get_physical_memory)(conf_object_t *obj); int (*get_logical_address_width)(conf_object_t *obj); int (*get_physical_address_width)(conf_object_t *obj); const char *(*architecture)(conf_object_t *obj); } processor_info_interface_t;

typedef struct { generic_transaction_t s; uint32 original_size; int bus_address; } pci_memory_transaction_t;
typedef struct {  } page_t;
typedef struct { page_t *page; physical_address_t page_base; physical_address_t page_size; page_t *extra_page; uint32 extra_page_offset; } get_page_t;
typedef struct { get_page_t (*get_page)(conf_object_t *obj, physical_address_t addr); } ram_interface_t;
typedef struct { get_page_t (*get_page)(conf_object_t *obj, physical_address_t addr); } rom_interface_t;

typedef struct { void (*bus_reset)(conf_object_t *obj); int (*interrupt_acknowledge)(conf_object_t *obj); void (*special_cycle)(conf_object_t *obj, uint32 value); void (*system_error)(conf_object_t *obj); void (*interrupt_raised)(conf_object_t *obj, int pin); void (*interrupt_lowered)(conf_object_t *obj, int pin); } pci_device_interface_t;
typedef struct { physical_address_t base; physical_address_t start; physical_address_t length; int function; uint8 priority; int align_size; swap_mode_t reverse_endian; } map_info_t;
typedef int (*map_func_t)(conf_object_t *obj, addr_space_t memory_or_io, map_info_t map_info);
typedef exception_type_t (*operation_func_t)(conf_object_t *obj, generic_transaction_t *mem_op, map_info_t map_info);
typedef struct { map_func_t map; operation_func_t operation; } io_memory_interface_t;
typedef struct { void (*system_error)(conf_object_t *obj); void (*raise_interrupt)(conf_object_t *obj, conf_object_t *irq_obj, int device, int pin); void (*lower_interrupt)(conf_object_t *obj, conf_object_t *irq_obj, int device, int pin); } pci_bridge_interface_t;
typedef struct { int (*send_message)(conf_object_t *dst, conf_object_t *src, pcie_message_type_t type, byte_string_t payload); } pci_express_interface_t;
typedef struct { exception_type_t (*memory_access)(conf_object_t *obj, generic_transaction_t *mem_op); void (*raise_interrupt)(conf_object_t *obj, conf_object_t *dev, int pin); void (*lower_interrupt)(conf_object_t *obj, conf_object_t *dev, int pin); int (*interrupt_acknowledge)(conf_object_t *obj); int (*add_map)(conf_object_t *obj, conf_object_t *dev, addr_space_t space, conf_object_t *target, map_info_t info); int (*remove_map)(conf_object_t *obj, conf_object_t *dev, addr_space_t space, int function); void (*set_bus_number)(conf_object_t *obj, int bus_id); void (*set_sub_bus_number)(conf_object_t *obj, int bus_id); void (*add_default)(conf_object_t *obj, conf_object_t *dev, addr_space_t space, conf_object_t *target, map_info_t info); void (*remove_default)(conf_object_t *obj, addr_space_t space); void (*bus_reset)(conf_object_t *obj); void (*special_cycle)(conf_object_t *obj, uint32 value); void (*system_error)(conf_object_t *obj); int (*get_bus_address)(conf_object_t *obj, conf_object_t *dev); void (*set_device_timing_model)(conf_object_t *obj, conf_object_t *dev, conf_object_t *timing_model); void (*set_device_status)(conf_object_t *obj, int device, int function, int enabled); } pci_bus_interface_t;
typedef int ieee_802_3_link_status_t;
typedef struct { conf_object_t *(*translate)(conf_object_t *obj, generic_transaction_t *mem_op, map_info_t mapinfo); } translate_interface_t;
typedef struct { exception_type_t (*not_taken)(conf_object_t *obj, conf_object_t *src_space, conf_object_t *dst_space, exception_type_t ex, generic_transaction_t *mem_op, map_info_t mapinfo); } bridge_interface_t;
typedef struct { const uint8 *start; size_t len; } frags_frag_t;
typedef struct { size_t len; uint32 nfrags; frags_frag_t fraglist[8]; } frags_t;
typedef struct { const frags_frag_t *frag; size_t skip; size_t left; } frags_it_t;
void *frags_extract_alloc(const frags_t *buf);
void *frags_extract_slice_alloc(const frags_t *buf, size_t offset, size_t len);
void frags_extract(const frags_t *buf, void *vdst);
void frags_extract_slice(const frags_t *buf, void *vdst, size_t offset, size_t len);
int frags_it_end(frags_it_t it);
frags_it_t frags_it_next(frags_it_t it);
frags_it_t frags_it(const frags_t *buf, size_t offset, size_t len);
frags_t frags_suffix(const frags_t *body, void *header, size_t header_len);
frags_t frags_prefix(const void *header, size_t header_len, const frags_t *body);
const uint8 *frags_it_data(frags_it_t it);
size_t frags_it_len(frags_it_t it);
size_t frags_len(const frags_t *buf);
uint16 frags_extract_be16(const frags_t *buf, size_t offset);
uint16 frags_extract_le16(const frags_t *buf, size_t offset);
uint32 frags_extract_be32(const frags_t *buf, size_t offset);
uint32 frags_extract_le32(const frags_t *buf, size_t offset);
uint64 frags_extract_be64(const frags_t *buf, size_t offset);
uint64 frags_extract_le64(const frags_t *buf, size_t offset);
uint8 frags_extract_8(const frags_t *buf, size_t offset);
void frags_init(frags_t *buf);
void frags_add(frags_t *buf, const void *data, size_t len);
void frags_init_add(frags_t *buf, const void *data, size_t len);
void frags_add_from_frags(frags_t *dst, const frags_t *src, size_t offset, size_t len);
void frags_init_add_from_frags(frags_t *dst, const frags_t *src, size_t offset, size_t len);

typedef struct { char *buf; int len;} dbuffer_t;
typedef struct { char *s; uint32 size; uint32 len; } strbuf_t;
typedef struct { int (*send_frame)(conf_object_t *obj, const frags_t *frame, int replace_crc); int (*check_tx_bandwidth)(conf_object_t *obj); } ieee_802_3_phy_v3_interface_t;

typedef struct { void (*receive_frame)(conf_object_t *obj, int phy, const frags_t *frame, int crc_ok); void (*tx_bandwidth_available)(conf_object_t *obj, int phy); void (*link_status_changed)(conf_object_t *obj, int phy, ieee_802_3_link_status_t status); } ieee_802_3_mac_v3_interface_t;

typedef struct { int (*receive_frame)(conf_object_t *obj, int phy, dbuffer_t *buf, int crc_ok); void (*tx_bandwidth_available)(conf_object_t *obj, int phy); void (*link_status_changed)(conf_object_t *obj, int phy, ieee_802_3_link_status_t status); } ieee_802_3_mac_interface_t;

typedef struct { int (*send_frame)(conf_object_t *obj, dbuffer_t *buf, int replace_crc); int (*check_tx_bandwidth)(conf_object_t *obj); void (*add_mac)(conf_object_t *obj, byte_string_t mac); void (*del_mac)(conf_object_t *obj, byte_string_t mac); void (*add_mac_mask)(conf_object_t *obj, byte_string_t mac, byte_string_t mask); void (*del_mac_mask)(conf_object_t *obj, byte_string_t mac, byte_string_t mask); void (*set_promiscous_mode)(conf_object_t *obj, int enable); } ieee_802_3_phy_v2_interface_t;

typedef struct { int (*send_frame)(conf_object_t *obj, dbuffer_t *buf, int replace_crc); int (*check_tx_bandwidth)(conf_object_t *obj); void (*add_mac)(conf_object_t *obj, const uint8 *mac); void (*del_mac)(conf_object_t *obj, const uint8 *mac); void (*add_mac_mask)(conf_object_t *obj, const uint8 *mac, const uint8 *mask); void (*del_mac_mask)(conf_object_t *obj, const uint8 *mac, const uint8 *mask); void (*set_promiscous_mode)(conf_object_t *obj, int enable); } ieee_802_3_phy_interface_t;

typedef struct { void (*changed)(conf_object_t *obj, ieee_802_3_link_status_t status); } link_status_interface_t;

typedef struct { uint16 (*read_register)(conf_object_t *obj, int mmd, int reg); void (*write_register)(conf_object_t *obj, int mmd, int reg, uint16 value); } mdio45_phy_interface_t;
typedef struct { int (*serial_access)(conf_object_t *obj, int data_in, int clock); uint16 (*read_register)(conf_object_t *obj, int phy, int reg); void (*write_register)(conf_object_t *obj, int phy, int reg, uint16 value); } mii_management_interface_t;
typedef struct { uint16 (*read_register)(conf_object_t *obj, int phy, int mmd, int reg); void (*write_register)(conf_object_t *obj, int phy, int mdd, int reg, uint16 value); } mdio45_bus_interface_t;
typedef struct { int (*serial_access)(conf_object_t *obj, int data_in, int clock); uint16 (*read_register)(conf_object_t *obj, int index); void (*write_register)(conf_object_t *obj, int index, uint16 value); } mii_interface_t;

typedef struct { int (*connect_device)(conf_object_t *_obj, conf_object_t *dev, int *new_connection); void (*disconnect_device)(conf_object_t *_obj, conf_object_t *dev); void (*send_frame)(conf_object_t *_obj, int id, dbuffer_t *frame, int crc_calculated, nano_secs_t delay); void (*auto_negotiate)(conf_object_t *_obj, int id, phy_speed_t speed); void (*add_mac)(conf_object_t *_obj, int id, byte_string_t addr); void (*add_mac_mask)(conf_object_t *_obj, int id, byte_string_t addr, byte_string_t mask); void (*delete_mac)(conf_object_t *_obj, int id, byte_string_t addr); void (*delete_mac_mask)(conf_object_t *_obj, int id, byte_string_t addr, byte_string_t mask); void (*clear_macs)(conf_object_t *_obj, int id); void (*promiscuous_mode)(conf_object_t *_obj, int id, int enable); } ethernet_link_interface_t;

typedef struct { void (*receive_frame)(conf_object_t *dev, conf_object_t *link, dbuffer_t *frame, int crc_calculated); phy_speed_t (*auto_neg_request)(conf_object_t *dev, phy_speed_t speed); void (*auto_neg_reply)(conf_object_t *dev, phy_speed_t speed); } ethernet_device_interface_t;
typedef struct { void (*receive_frame)(conf_object_t *dev, conf_object_t *link, dbuffer_t *frame); } generic_message_device_interface_t;
static dbuffer_t *new_dbuffer(void){return (dbuffer_t *)g_malloc0(sizeof(dbuffer_t));}
static uint8 *dbuffer_append(dbuffer_t *dbuffer, size_t len) {return NULL;}
static size_t dbuffer_len(const dbuffer_t *dbuffer){return 0;}
static uint8 *dbuffer_append_value(dbuffer_t *dbuffer, int value, size_t len){return NULL;}
static uint8 *dbuffer_update_all(dbuffer_t *dbuffer){return NULL;}
static  uint8 *dbuffer_update(dbuffer_t *dbuffer, size_t offset, size_t len){return NULL;}
static uint8 *dbuffer_update_some(dbuffer_t *dbuffer, size_t offset, size_t len, size_t *actual_len){return NULL;}
static const uint8 *dbuffer_read_all(dbuffer_t *dbuffer){return dbuffer->buf;}
static void dbuffer_remove_head(dbuffer_t *dbuffer, size_t remove_len){}
static void dbuffer_free(dbuffer_t *dbuffer){g_free(dbuffer);}
static void dbuffer_remove_tail(dbuffer_t *dbuffer, size_t remove_len){}
static uint8 *dbuffer_insert(dbuffer_t *dbuffer, size_t offset, size_t len) {return NULL;}
static void dbuffer_remove(dbuffer_t *dbuffer, size_t offset, size_t remove_len){} 
static uint8 *dbuffer_prepend_value(dbuffer_t *dbuffer, int value, size_t len){return NULL;}
static uint8 *dbuffer_prepend(dbuffer_t *dbuffer, size_t len){return NULL;}
static dbuffer_t *dbuffer_clone(dbuffer_t *dbuffer){return NULL;}
static void sb_init(strbuf_t *sb) {}
static void sb_addfmt(strbuf_t *sb, const char *format, ...) {}
static void sb_free(strbuf_t *sb){}
static uint8 *dbuffer_append_external_data(dbuffer_t *dbuffer, void *data, size_t len, int adopt){dbuffer->buf = data; dbuffer->len = len; return data;}

typedef struct { int (*get_number)(conf_object_t *obj, const char *name); const char *(*get_name)(conf_object_t *obj, int reg); uinteger_t (*read)(conf_object_t *obj, int reg); void (*write)(conf_object_t *obj, int reg, uinteger_t val); attr_value_t (*all_registers)(conf_object_t *obj); int (*register_info)(conf_object_t *obj, int reg, ireg_info_t info); } int_register_interface_t;
typedef struct {  } recorder_t;
typedef void (*recorder_input_handler_t)(conf_object_t *obj, dbuffer_t *data, uint32 uint_data);
typedef struct { recorder_t *(*attach)(conf_object_t *rec, conf_object_t *obj, recorder_input_handler_t input_from_recorder); void (*detach)(conf_object_t *rec, recorder_t *r); void (*input)(conf_object_t *rec, recorder_t *r, dbuffer_t *data, uint32 uint_data); } recorder_interface_t;

typedef struct { int (*connect_device)(conf_object_t *obj, conf_object_t *dev); void (*disconnect_device)(conf_object_t *obj, conf_object_t *dev); int (*send_char)(conf_object_t *obj, int id, int ch); void (*receive_ready)(conf_object_t *obj, int id); } serial_link_interface_t;
static uint32 LOAD_BE32(void *src){return 0;}
static uint16 LOAD_BE16(void *src){return 0;}
static uint64 LOAD_BE64(void *src){return 0;}
static uint32 UNALIGNED_LOAD_BE32(void *src){return 0;}
static uint16 UNALIGNED_LOAD_BE16(void *src){return 0;}
static uint64 UNALIGNED_LOAD_BE64(void *src) {return 0;}
static void UNALIGNED_STORE_BE32(void *dst, uint32 val){}
static void UNALIGNED_STORE_BE64(void *dst, uint64 val){}
static void UNALIGNED_STORE_BE16(void *dst, uint64 val){}
static void STORE_BE32(void *dst, uint32 val){}
static void STORE_BE64(void *dst, uint64 val){}
static void STORE_BE16(void *dst, uint64 val){}

static uint32 LOAD_LE32(void *src){return 0;}
static uint16 LOAD_LE16(void *src){return 0;}
static uint64 LOAD_LE64(void *src){return 0;}
static uint32 UNALIGNED_LOAD_LE32(void *src){return *((int *)src);}
static uint16 UNALIGNED_LOAD_LE16(void *src){return 0;}
static uint64 UNALIGNED_LOAD_LE64(void *src) {return *((uint64_t *)src);}
static void UNALIGNED_STORE_LE32(void *dst, uint32 val){*((int *)dst) = val;}
static void UNALIGNED_STORE_LE64(void *dst, uint64 val){*((uint64_t *)dst) = val;}
static void UNALIGNED_STORE_LE16(void *dst, uint64 val){}
static void STORE_LE32(void *dst, uint32 val){}
static void STORE_LE64(void *dst, uint64 val){}
static void STORE_LE16(void *dst, uint64 val){}

static inline attr_value_t SIM_get_attribute(conf_object_t *obj, const char *name)
{
        return SIM_make_attr_boolean(0);
}

static set_error_t SIM_set_attribute(conf_object_t *obj, const char *name, attr_value_t *value) {
	return 0;
}

static double SIM_time(conf_object_t *obj) {
	return 0.0;
}

static uint32 SIM_attr_data_size(attr_value_t attr) {
	return 0;
} 

static void *SIM_attr_data(attr_value_t attr) {
	return NULL;
}

static conf_object_t *SIM_get_mem_op_initiator(generic_transaction_t *mop) {
	/* TODO: implement it as about cpu*/
	return NULL;
}
static cycles_t VT_old_time_next_occurrence(conf_object_t *obj, void (*func)(conf_object_t *obj, void *param), void *user_data){return 0;}
static pc_step_t VT_old_step_next_occurrence(conf_object_t *obj, void (*func)(conf_object_t *obj, void *param), void *user_data){return 0;}
static conf_object_t *SIM_object_clock(conf_object_t *obj){return NULL;}
static void SIM_frontend_exception(sim_exception_t exc_type, const char *str){}
static  void SIM_set_mem_op_inquiry(generic_transaction_t *mop, int inquiry){}
static uint32 LOG2_64(uint64 x){return 0;}
static void dbuffer_clear(dbuffer_t *dbuffer){}
static uint32 dml_load_be_s32(char *addr) {return 0;};
static void dml_store_be_s32(char *addr, uint32 val) {}
static uint32 dml_load_le_s32(char *addr) {return 0;};
static void dml_store_le_s32(char *addr, uint32 val) {}
#endif /*__DEV_IFACE_H__ */
