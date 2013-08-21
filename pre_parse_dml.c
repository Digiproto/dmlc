/*
 * pre_dml.c:
 *
 * Copyright (C) 2013 Oubang Shen <shenoubang@gmail.com>
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
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "symbol-common.h"
#include "types.h"
#include "pre_parse_dml.h"
#include "symbol.h"
#include "ast.h"

standard_param_t object_common_param[] = {
	{"this",			OBJECT_TYPE,		AUTO_VALUE},
	{"parent",			OBJECT_TYPE,		AUTO_VALUE},
	{"desc",			CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"documentation",	CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"limitations",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"name",			CONST_STRING_TYPE,	AUTO_VALUE},
	{"qname",			CONST_STRING_TYPE,  AUTO_VALUE},
	{"objtype",			CONST_STRING_TYPE,  ASSIGNED_VALUE},
	{"index",			INT_TYPE,			UNDEFINED_VALUE},
	{"indexvar",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"dev",				OBJECT_TYPE,		ASSIGNED_VALUE},
	{"bank",			OBJECT_TYPE,		ASSIGNED_VALUE},
	{NULL, 0, 0},
};

standard_param_t device_param[] = {
	{"this",			OBJECT_TYPE,		AUTO_VALUE},
	{"parent",			OBJECT_TYPE,		AUTO_VALUE},
	{"desc",			CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"documentation",	CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"limitations",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"name",			CONST_STRING_TYPE,	AUTO_VALUE},
	{"qname",			CONST_STRING_TYPE,  AUTO_VALUE},
	{"objtype",			CONST_STRING_TYPE,  ASSIGNED_VALUE},
	{"index",			INT_TYPE,			UNDEFINED_VALUE},
	{"indexvar",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"dev",				OBJECT_TYPE,		ASSIGNED_VALUE},
	//{"bank",			OBJECT_TYPE,		ASSIGNED_VALUE},

	{"classname",		CONST_STRING_TYPE,	DEFAULT_VALUE},
	{"banks",			OBJECT_TYPE,		AUTO_VALUE},
	{"register_size",	INT_TYPE,			UNDEFINED_VALUE},
	{"byte_order",		CONST_STRING_TYPE,  DEFAULT_VALUE},
	{"log_group",		INT_TYPE,			UNDEFINED_VALUE},
	{"obj",				OBJECT_TYPE,		AUTO_VALUE},
	{"logobj",			OBJECT_TYPE,		AUTO_VALUE},
	{NULL, 0, 0},
};

standard_param_t attribute_param[] = {
	{"this",			OBJECT_TYPE,		AUTO_VALUE},
	{"parent",			OBJECT_TYPE,		AUTO_VALUE},
	{"desc",			CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"documentation",	CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"limitations",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"name",			CONST_STRING_TYPE,	AUTO_VALUE},
	{"qname",			CONST_STRING_TYPE,  AUTO_VALUE},
	{"objtype",			CONST_STRING_TYPE,  ASSIGNED_VALUE},
	{"index",			INT_TYPE,			UNDEFINED_VALUE},
	{"indexvar",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"dev",				OBJECT_TYPE,		ASSIGNED_VALUE},
	{"bank",			OBJECT_TYPE,		ASSIGNED_VALUE},

	{"allocate_type",	CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"type",			CONST_STRING_TYPE,	DEFAULT_VALUE},
	{"configuration",	CONST_STRING_TYPE,  DEFAULT_VALUE},
	{"persistent",		BOOL_TYPE,			DEFAULT_VALUE},
	{"internal",		BOOL_TYPE,			DEFAULT_VALUE},
	{"attr_type",		CONST_STRING_TYPE,	AUTO_VALUE},
	{NULL, 0, 0},
};

standard_param_t bank_param[] = {
	{"this",			OBJECT_TYPE,		AUTO_VALUE},
	{"parent",			OBJECT_TYPE,		AUTO_VALUE},
	{"desc",			CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"documentation",	CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"limitations",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"name",			CONST_STRING_TYPE,	AUTO_VALUE},
	{"qname",			CONST_STRING_TYPE,  AUTO_VALUE},
	{"objtype",			CONST_STRING_TYPE,  ASSIGNED_VALUE},
	{"index",			INT_TYPE,			UNDEFINED_VALUE},
	{"indexvar",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"dev",				OBJECT_TYPE,		ASSIGNED_VALUE},
	{"bank",			OBJECT_TYPE,		ASSIGNED_VALUE},

	{"mapped_registers",	OBJECT_TYPE,	AUTO_VALUE},
	{"unmapped_registers",	OBJECT_TYPE,	AUTO_VALUE},
	{"numbered_registers",	OBJECT_TYPE,	UNDEFINED_VALUE},
	{"mappable",			BOOL_TYPE,		DEFAULT_VALUE},
	{"function",			INT_TYPE,		UNDEFINED_VALUE},
	{"overlapping",			BOOL_TYPE,		DEFAULT_VALUE},
	{"partial",				BOOL_TYPE,		DEFAULT_VALUE},
	{"signed",				BOOL_TYPE,		DEFAULT_VALUE},
	{"allocate",			BOOL_TYPE,		DEFAULT_VALUE},
	{"register_size",		INT_TYPE,		DEFAULT_VALUE},
	{"byte_order",			CONST_STRING_TYPE,	DEFAULT_VALUE},
	{"log_group",			INT_TYPE,		DEFAULT_VALUE},
	{"miss_bank",			OBJECT_TYPE,	UNDEFINED_VALUE},
	{"miss_bank_offset",	INT_TYPE,		DEFAULT_VALUE},
	{NULL, 0, 0},
};

standard_param_t register_param[] = {
	{"this",			OBJECT_TYPE,		AUTO_VALUE},
	{"parent",			OBJECT_TYPE,		AUTO_VALUE},
	{"desc",			CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"documentation",	CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"limitations",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"name",			CONST_STRING_TYPE,	AUTO_VALUE},
	{"qname",			CONST_STRING_TYPE,  AUTO_VALUE},
	{"objtype",			CONST_STRING_TYPE,  ASSIGNED_VALUE},
	{"index",			INT_TYPE,			UNDEFINED_VALUE},
	{"indexvar",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"dev",				OBJECT_TYPE,		ASSIGNED_VALUE},
	{"bank",			OBJECT_TYPE,		ASSIGNED_VALUE},

	{"szie",			INT_TYPE,			DEFAULT_VALUE},
	{"bitsize",			INT_TYPE,			ASSIGNED_VALUE},
	{"offset",			INT_TYPE,			UNDEFINED_VALUE},
	{"regnum",			INT_TYPE,			UNDEFINED_VALUE},
	{"signed",			BOOL_TYPE,			DEFAULT_VALUE},
	{"allocate",		BOOL_TYPE,			DEFAULT_VALUE},
	{"fields",			OBJECT_TYPE,		AUTO_VALUE},
	{"hard_reset_value",INT_TYPE,			DEFAULT_VALUE},
	{"soft_reset_value", INT_TYPE,			DEFAULT_VALUE},
	{"logging",			BOOL_TYPE,			DEFAULT_VALUE},
	{"log_group",		INT_TYPE,			DEFAULT_VALUE},
	{"read_logging",	BOOL_TYPE,			DEFAULT_VALUE},
	{"write_logging",	BOOL_TYPE,			DEFAULT_VALUE},
	{"configuration",	CONST_STRING_TYPE,	DEFAULT_VALUE},
	{"persistent",		BOOL_TYPE,			DEFAULT_VALUE},
	{"internal",		BOOL_TYPE,			DEFAULT_VALUE},
	{"attr_type",		CONST_STRING_TYPE,	ASSIGNED_VALUE},
	{"notinregister ",	INT_TYPE,			ASSIGNED_VALUE},
	{NULL, 0, 0},
};

standard_param_t field_param[] = {
	{"this",			OBJECT_TYPE,		AUTO_VALUE},
	{"parent",			OBJECT_TYPE,		AUTO_VALUE},
	{"desc",			CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"documentation",	CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"limitations",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"name",			CONST_STRING_TYPE,	AUTO_VALUE},
	{"qname",			CONST_STRING_TYPE,  AUTO_VALUE},
	{"objtype",			CONST_STRING_TYPE,  ASSIGNED_VALUE},
	{"index",			INT_TYPE,			UNDEFINED_VALUE},
	{"indexvar",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"dev",				OBJECT_TYPE,		ASSIGNED_VALUE},
	{"bank",			OBJECT_TYPE,		ASSIGNED_VALUE},

	{"reg",				OBJECT_TYPE,	ASSIGNED_VALUE},
	{"explicit",		BOOL_TYPE,		AUTO_VALUE},
	{"lsb",				INT_TYPE,		UNDEFINED_VALUE},
	{"msb",				INT_TYPE,		UNDEFINED_VALUE},
	{"bitsize",			INT_TYPE,		ASSIGNED_VALUE},
	{"signed",			BOOL_TYPE,		DEFAULT_VALUE},
	{"allocate",		BOOL_TYPE,		DEFAULT_VALUE},
	{"hard_reset_value",INT_TYPE,		DEFAULT_VALUE},
	{"soft_reset_value",INT_TYPE,		DEFAULT_VALUE},
	{"notinfield",		INT_TYPE,		ASSIGNED_VALUE},
	{NULL, 0, 0},
};

standard_param_t connect_param[] = {
	{"this",			OBJECT_TYPE,		AUTO_VALUE},
	{"parent",			OBJECT_TYPE,		AUTO_VALUE},
	{"desc",			CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"documentation",	CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"limitations",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"name",			CONST_STRING_TYPE,	AUTO_VALUE},
	{"qname",			CONST_STRING_TYPE,  AUTO_VALUE},
	{"objtype",			CONST_STRING_TYPE,  ASSIGNED_VALUE},
	{"index",			INT_TYPE,			UNDEFINED_VALUE},
	{"indexvar",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"dev",				OBJECT_TYPE,		ASSIGNED_VALUE},
	{"bank",			OBJECT_TYPE,		ASSIGNED_VALUE},

	{"interfaces",		OBJECT_TYPE,		AUTO_VALUE},
	{"configuration",	CONST_STRING_TYPE,	DEFAULT_VALUE},
	{"persistent",		BOOL_TYPE,			DEFAULT_VALUE},
	{"internal",		BOOL_TYPE,			DEFAULT_VALUE},
	{"attr_type",		CONST_STRING_TYPE,	ASSIGNED_VALUE},
	{"required",		BOOL_TYPE,			DEFAULT_VALUE},
	{"c_type",			CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"evclass",			POINTER_TYPE,		UNDEFINED_VALUE},
	{NULL, 0, 0},
};

standard_param_t interface_param[] = {
	{"this",			OBJECT_TYPE,		AUTO_VALUE},
	{"parent",			OBJECT_TYPE,		AUTO_VALUE},
	{"desc",			CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"documentation",	CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"limitations",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"name",			CONST_STRING_TYPE,	AUTO_VALUE},
	{"qname",			CONST_STRING_TYPE,  AUTO_VALUE},
	{"objtype",			CONST_STRING_TYPE,  ASSIGNED_VALUE},
	{"index",			INT_TYPE,			UNDEFINED_VALUE},
	{"indexvar",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"dev",				OBJECT_TYPE,		ASSIGNED_VALUE},
	{"bank",			OBJECT_TYPE,		ASSIGNED_VALUE},

	{"required",		BOOL_TYPE,			DEFAULT_VALUE},
	{"c_type",			CONST_STRING_TYPE,	DEFAULT_VALUE},
	{"c_name",			CONST_STRING_TYPE,	DEFAULT_VALUE},
	{"evclass",			POINTER_TYPE,		UNDEFINED_VALUE},
	{NULL, 0, 0},
};

standard_param_t event_param[] = {
	{"this",			OBJECT_TYPE,		AUTO_VALUE},
	{"parent",			OBJECT_TYPE,		AUTO_VALUE},
	{"desc",			CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"documentation",	CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"limitations",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"name",			CONST_STRING_TYPE,	AUTO_VALUE},
	{"qname",			CONST_STRING_TYPE,  AUTO_VALUE},
	{"objtype",			CONST_STRING_TYPE,  ASSIGNED_VALUE},
	{"index",			INT_TYPE,			UNDEFINED_VALUE},
	{"indexvar",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"dev",				OBJECT_TYPE,		ASSIGNED_VALUE},
	{"bank",			OBJECT_TYPE,		ASSIGNED_VALUE},

	{"timebase",		CONST_STRING_TYPE, DEFAULT_VALUE},
	{NULL, 0, 0},
};

standard_param_t implement_param[] = {
	{"this",			OBJECT_TYPE,		AUTO_VALUE},
	{"parent",			OBJECT_TYPE,		AUTO_VALUE},
	{"desc",			CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"documentation",	CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"limitations",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"name",			CONST_STRING_TYPE,	AUTO_VALUE},
	{"qname",			CONST_STRING_TYPE,  AUTO_VALUE},
	{"objtype",			CONST_STRING_TYPE,  ASSIGNED_VALUE},
	{"index",			INT_TYPE,			UNDEFINED_VALUE},
	{"indexvar",		CONST_STRING_TYPE,	UNDEFINED_VALUE},
	{"dev",				OBJECT_TYPE,		ASSIGNED_VALUE},
	{"bank",			OBJECT_TYPE,		ASSIGNED_VALUE},

	{"c_type",			CONST_STRING_TYPE, UNDEFINED_VALUE},
	{NULL, 0, 0},
};

char* byte_order_value[] = {
	"\"little-endian\"",
	"\"big-endian\"",
	NULL,
};

char* configuration_value[] = {
	"\"none\"",
	"\"pseudo\"",
	"\"optional\"",
	"\"required\"",
	NULL,
};

char* type_value[] = {
	"\"f\"",
	"\"i\"",
	"\"s\"",
	"\"b\"",
	"\"d\"",
	"\"D\"",
	"\"o\"",
	"\"n\"",
	"\"a\"",
	NULL,
};

#define ALLOCATE_VALUE_NUM 70
char* allocate_type_value[ALLOCATE_VALUE_NUM] = {
	"\"double\"",
	"\"string\"",
	"\"bool\"",
};

char* timebase_vale[] = {
	"\"steps\"",
	"\"cycles\"",
	"\"seconds\"",
	"\"stacked\"",
	NULL,
};

special_param_t device_special[] = {
	{"byte_order",	byte_order_value},
	{NULL, NULL},
};

special_param_t attribute_special[] = {
	{"allocate_type", allocate_type_value},
	{"type",		type_value},
	{"configuration", configuration_value},
	{NULL, NULL},
};

special_param_t bank_special[] = {
	{"byte_order", byte_order_value},
	{NULL, NULL},
};

special_param_t register_special[] = {
	{"configuration", configuration_value},
	{NULL, NULL},
};

special_param_t connect_special[] = {
	{"configuration", configuration_value},
	{NULL, NULL},
};

special_param_t event_special[] = {
	{"timebase", timebase_vale},
	{NULL, NULL},
};

pre_dml_t pre_dml_struct[] = {
	/* ------------typedef--------- */
	/* int */
	{"attr_attr_t", TYPEDEF_TYPE},
	{"hap_type_t", TYPEDEF_TYPE},
	{"register_id_t", TYPEDEF_TYPE},
	{"sparc_access_type_t", TYPEDEF_TYPE},
	{"ppc_mc_exc_t", TYPEDEF_TYPE},
	{"usb_type_t", TYPEDEF_TYPE},
	{"firewire_response_code_t", TYPEDEF_TYPE},
	{"ieee_802_3_link_status_t", TYPEDEF_TYPE},
	{"read_or_write_t", TYPEDEF_TYPE},
	{"x86_access_type_t", TYPEDEF_TYPE},
	{"breakpoint_id_t", TYPEDEF_TYPE},
	{"sim_exception_t", TYPEDEF_TYPE},
	{"ppc_sleep_state_t", TYPEDEF_TYPE},
	{"gen_spr_ret_t", TYPEDEF_TYPE},
	{"endianness_t", TYPEDEF_TYPE},
	{"processor_mode_t", TYPEDEF_TYPE},
	{"sapic_trigger_mode_t", TYPEDEF_TYPE},
	{"sapic_bus_status_t", TYPEDEF_TYPE},
	{"hap_handle_t", TYPEDEF_TYPE},
	{"ppc_decoration_type_t", TYPEDEF_TYPE},
	{"serial_peripheral_interface_flags_t", TYPEDEF_TYPE},
	{"event_class_flag_t", TYPEDEF_TYPE},
	{"ms1553_dir_t", TYPEDEF_TYPE},
	{"i2c_device_state_t", TYPEDEF_TYPE},
	{"trackee_unix_type_t", TYPEDEF_TYPE},
	{"notify_mode_t", TYPEDEF_TYPE},
	{"addr_space_t", TYPEDEF_TYPE},
	{"usb_direction_t", TYPEDEF_TYPE},
	{"instruction_fetch_mode_t", TYPEDEF_TYPE},
	{"cpu_endian_t", TYPEDEF_TYPE},
	{"ms1553_phase_t", TYPEDEF_TYPE},
	{"exception_type_t", TYPEDEF_TYPE},
	{"h8_transition_direction_t", TYPEDEF_TYPE},
	{"usb_speed_t", TYPEDEF_TYPE},
	{"rapidio_operation_t", TYPEDEF_TYPE},
	{"local_apic_interrupt_t", TYPEDEF_TYPE},
	{"map_type_t", TYPEDEF_TYPE},
	{"ms1553_mode_code_t", TYPEDEF_TYPE},
	{"ms1553_error_t", TYPEDEF_TYPE},
	{"stop_bits_t", TYPEDEF_TYPE},
	{"ppc_mem_instr_origin_t", TYPEDEF_TYPE},
	{"hap_flags_t", TYPEDEF_TYPE},
	{"set_error_t", TYPEDEF_TYPE},
	{"sync_t", TYPEDEF_TYPE},
	{"palcode_memop_flags_t", TYPEDEF_TYPE},
	{"ini_type_t", TYPEDEF_TYPE},
	{"data_or_instr_t", TYPEDEF_TYPE},
	{"usb_transfer_completion_t", TYPEDEF_TYPE},
	{"h8_sleep_mode_t", TYPEDEF_TYPE},
	{"attr_kind_t", TYPEDEF_TYPE},
	{"ireg_info_t", TYPEDEF_TYPE},
	{"parity_mode_t", TYPEDEF_TYPE},
	{"firewire_ack_code_t", TYPEDEF_TYPE},
	{"mod_def_t", TYPEDEF_TYPE},
	{"pci_express_hotplug_interface_t", TYPEDEF_TYPE},
	{"apic_trigger_mode_t", TYPEDEF_TYPE},
	{"i2c_device_flag_t", TYPEDEF_TYPE},
	{"branch_recorder_direction_t", TYPEDEF_TYPE},
	{"access_t", TYPEDEF_TYPE},
	{"pseudo_exceptions_t", TYPEDEF_TYPE},
	{"apic_bus_status_t", TYPEDEF_TYPE},
	{"phy_speed_t", TYPEDEF_TYPE},
	{"firewire_transaction_code_t", TYPEDEF_TYPE},
	{"pcie_message_type_t", TYPEDEF_TYPE},
	{"i2c_status_t", TYPEDEF_TYPE},
	{"swap_mode_t", TYPEDEF_TYPE},
	{"ms1553_shadow_word_t", TYPEDEF_TYPE},
	{"apic_destination_mode_t", TYPEDEF_TYPE},
	{"x86_memory_type_t", TYPEDEF_TYPE},
	{"gen_spr_access_type_t", TYPEDEF_TYPE},
	{"h8_interrupt_type_t", TYPEDEF_TYPE},
	{"x86_pin_t", TYPEDEF_TYPE},
	{"log_type_t", TYPEDEF_TYPE},
	{"usb_status_t", TYPEDEF_TYPE},
	{"breakpoint_kind_t", TYPEDEF_TYPE},
	{"branch_arc_type_t", TYPEDEF_TYPE},
	{"mem_op_type_t", TYPEDEF_TYPE},
	{"sapic_delivery_mode_t", TYPEDEF_TYPE},
	{"apic_delivery_mode_t", TYPEDEF_TYPE},
	{"class_kind_t", TYPEDEF_TYPE},
	{"register_type_t", TYPEDEF_TYPE},
	/* uint16 */
	{"stall_id_num_t", TYPEDEF_TYPE},
	/* uint32 */
	{"size_t", TYPEDEF_TYPE},
	/* uint64 */
	{"integer_t", TYPEDEF_TYPE},
	{"instr_type_t", TYPEDEF_TYPE},
	{"linear_address_t", TYPEDEF_TYPE},
	{"vdisk_addr_t", TYPEDEF_TYPE},
	{"generic_address_t", TYPEDEF_TYPE},
	{"physical_address_t", TYPEDEF_TYPE},
	{"uinteger_t", TYPEDEF_TYPE},
	{"logical_address_t", TYPEDEF_TYPE},
	/* void */
	{"interface_t", TYPEDEF_TYPE},
	{"class_interface_t", TYPEDEF_TYPE},
	/* simtime_t */
	{"pc_step_t", TYPEDEF_TYPE},
	{"cycles_t", TYPEDEF_TYPE},
	/* integer_t */
	{"nano_secs_t", TYPEDEF_TYPE},
	{"simtime_t", TYPEDEF_TYPE},
	/* gen_spr_ret_t */
	{"ppc_spr_ret_t", TYPEDEF_TYPE},
	/* gen_spr_access_type_t */
	{"ppc_spr_access_type_t", TYPEDEF_TYPE},
	/* gen_spr_user_getter_func_t */
	{"ppc_spr_user_getter_func_t", TYPEDEF_TYPE},
	/* gen_spr_user_setter_func_t */
	{"ppc_spr_user_setter_func_t", TYPEDEF_TYPE},

	/* --------------- struct------------- */
	{"conf_class_t", STRUCT_TYPE},
	{"strbuf_t", STRUCT_TYPE},
	{"class_data_t", STRUCT_TYPE},
	{"ethernet_common_link_interface_t", STRUCT_TYPE},
	{"alpha_interface_t", STRUCT_TYPE},
	{"arinc429_bus_interface_t", STRUCT_TYPE},
	{"generic_message_device_interface_t", STRUCT_TYPE},
	{"arm_avic_t", STRUCT_TYPE},
	{"ppc_interface_t", STRUCT_TYPE},
	{"h8_interrupt_interface_t", STRUCT_TYPE},
	{"interrupt_query_interface_t", STRUCT_TYPE},
	{"scale_factor_listener_interface_t", STRUCT_TYPE},
	{"ms1553_words_t", STRUCT_TYPE},
	{"sapic_interface_t", STRUCT_TYPE},
	{"extended_serial_interface_t", STRUCT_TYPE},
	{"cell_interrupt_bus_interface_t", STRUCT_TYPE},
	{"x86_cpuid_interface_t", STRUCT_TYPE},
	{"usb_device_interface_t", STRUCT_TYPE},
	{"x86_interface_t", STRUCT_TYPE},
	{"firewire_bus_interface_t", STRUCT_TYPE},
	{"mips_memory_transaction_t", STRUCT_TYPE},
	{"parse_object_t", STRUCT_TYPE},
	{"stall_interface_t", STRUCT_TYPE},
	{"frags_frag_t", STRUCT_TYPE},
	{"checkpoint_interface_t", STRUCT_TYPE},
	{"branch_arc_interface_t", STRUCT_TYPE},
	{"step_cycle_ratio_t", STRUCT_TYPE},
	{"ram_interface_t", STRUCT_TYPE},
	{"snoop_memory_interface_t", STRUCT_TYPE},
	{"translate_interface_t", STRUCT_TYPE},
	{"io_memory_interface_t", STRUCT_TYPE},
	{"attr_dict_pair_t", STRUCT_TYPE},
	{"image_interface_t", STRUCT_TYPE},
	{"i2c_bridge_interface_t", STRUCT_TYPE},
	{"step_info_interface_t", STRUCT_TYPE},
	{"step_cycle_ratio_interface_t", STRUCT_TYPE},
	{"tracker_settings_interface_t", STRUCT_TYPE},
	{"ppc_memory_transaction_t", STRUCT_TYPE},
	{"map_func_t", STRUCT_TYPE},
	{"set_class_attr_t", STRUCT_TYPE},
	{"disassemble_interface_t", STRUCT_TYPE},
	{"sparc_irq_bus_interface_t", STRUCT_TYPE},
	{"fmn_station_control_interface_t", STRUCT_TYPE},
	{"sparc_v8_interface_t", STRUCT_TYPE},
	{"serial_peripheral_interface_master_interface_t", STRUCT_TYPE},
	{"log_object_t", STRUCT_TYPE},
	{"ms1553_terminal_interface_t", STRUCT_TYPE},
	{"simulator_cache_interface_t", STRUCT_TYPE},
	{"attr_value_t", STRUCT_TYPE},
	{"sparc_u3_interface_t", STRUCT_TYPE},
	{"ethernet_link_interface_t", STRUCT_TYPE},
	{"FILE", STRUCT_TYPE},
	{"utopia_ii_egress_interface_t", STRUCT_TYPE},
	{"instr_info_dep_t", STRUCT_TYPE},
	{"ia64_memory_transaction_t", STRUCT_TYPE},
	{"arm_interface_t", STRUCT_TYPE},
	{"event_poster_interface_t", STRUCT_TYPE},
	{"sh_interrupt_interface_t", STRUCT_TYPE},
	{"context_handler_interface_t", STRUCT_TYPE},
	{"ethernet_cable_link_interface_t", STRUCT_TYPE},
	{"h8_power_interface_t", STRUCT_TYPE},
	{"i2c_slave_interface_t", STRUCT_TYPE},
	{"utopia_ii_ingress_interface_t", STRUCT_TYPE},
	{"pci_express_interface_t", STRUCT_TYPE},
	{"vt_event_poster_interface_t", STRUCT_TYPE},
	{"i2c_link_interface_t", STRUCT_TYPE},
	{"arm_coprocessor_interface_t", STRUCT_TYPE},
	{"generic_message_link_interface_t", STRUCT_TYPE},
	{"linear_image_interface_t", STRUCT_TYPE},
	{"uint32_array_t", STRUCT_TYPE},
	{"reset_interface_t", STRUCT_TYPE},
	{"recorder_t", STRUCT_TYPE},
	{"timing_model_interface_t", STRUCT_TYPE},
	{"ms1553_bridge_terminal_interface_t", STRUCT_TYPE},
	{"mouse_interface_t", STRUCT_TYPE},
	{"serial_peripheral_interface_slave_interface_t", STRUCT_TYPE},
	{"interrupt_ack_interface_t", STRUCT_TYPE},
	{"kbd_interface_t", STRUCT_TYPE},
	{"breakpoint_vector_t", STRUCT_TYPE},
	{"cache_miss_interface_t", STRUCT_TYPE},
	{"usb_device_request_t", STRUCT_TYPE},
	{"cdrom_media_interface_t", STRUCT_TYPE},
	{"nand_flash_interface_t", STRUCT_TYPE},
	{"ethernet_device_interface_t", STRUCT_TYPE},
	{"cpu_list_t", STRUCT_TYPE},
	{"ieee_802_3_mac_interface_t", STRUCT_TYPE},
	{"ms1553_link_interface_t", STRUCT_TYPE},
	{"stall_id_t", STRUCT_TYPE},
	{"ppc_decoration_t", STRUCT_TYPE},
	{"serial_device_interface_t", STRUCT_TYPE},
	{"ia64_interface_t", STRUCT_TYPE},
	{"ieee_802_3_phy_v2_interface_t", STRUCT_TYPE},
	{"ethernet_cable_device_interface_t", STRUCT_TYPE},
	{"multi_level_signal_interface_t", STRUCT_TYPE},
	{"firewire_device_interface_t", STRUCT_TYPE},
	{"bridge_interface_t", STRUCT_TYPE},
	{"cpuid_ret_t", STRUCT_TYPE},
	{"symtable_interface_t", STRUCT_TYPE},
	{"mii_management_interface_t", STRUCT_TYPE},
	{"tuple_int_string_t", STRUCT_TYPE},
	{"pci_device_interface_t", STRUCT_TYPE},
	{"dbuffer_t", STRUCT_TYPE},
	{"coreint_interface_t", STRUCT_TYPE},
	{"page_t", STRUCT_TYPE},
	{"iosapic_interface_t", STRUCT_TYPE},
	{"bytes_t", STRUCT_TYPE},
	{"abs_pointer_activate_interface_t", STRUCT_TYPE},
	{"get_page_t", STRUCT_TYPE},
	{"hpi_interface_t", STRUCT_TYPE},
	{"trigger_interface_t", STRUCT_TYPE},
	{"step_interface_t", STRUCT_TYPE},
	{"abs_pointer_interface_t", STRUCT_TYPE},
	{"apic_bus_interface_t", STRUCT_TYPE},
	{"usb_interface_t", STRUCT_TYPE},
	{"sample_memory_transaction_t", STRUCT_TYPE},
	{"frags_t", STRUCT_TYPE},
	{"log_object_interface_t", STRUCT_TYPE},
	{"gbic_transceiver_interface_t", STRUCT_TYPE},
	{"memory_page_update_interface_t", STRUCT_TYPE},
	{"apic_cpu_interface_t", STRUCT_TYPE},
	{"alpha_memory_transaction_t", STRUCT_TYPE},
	{"interrupt_cpu_interface_t", STRUCT_TYPE},
	{"ieee_802_3_phy_interface_t", STRUCT_TYPE},
	{"arm_avic_interface_t", STRUCT_TYPE},
	{"rs232_console_interface_t", STRUCT_TYPE},
	{"pci_bus_interface_t", STRUCT_TYPE},
	{"cell_interrupt_receiver_interface_t", STRUCT_TYPE},
	{"mips_coprocessor_interface_t", STRUCT_TYPE},
	{"abs_pointer_state_t", STRUCT_TYPE},
	{"pre_conf_object_set_t", STRUCT_TYPE},
	{"instruction_fetch_interface_t", STRUCT_TYPE},
	{"i2c_device_interface_t", STRUCT_TYPE},
	{"map_info_t", STRUCT_TYPE},
	{"conf_object_t", STRUCT_TYPE},
	{"map_demap_interface_t", STRUCT_TYPE},
	{"atm_msg_t", STRUCT_TYPE},
	{"breakpoint_interface_t", STRUCT_TYPE},
	{"int_register_interface_t", STRUCT_TYPE},
	{"i2c_bus_interface_t", STRUCT_TYPE},
	{"rs232_device_interface_t", STRUCT_TYPE},
	{"ia64_fp_register_t", STRUCT_TYPE},
	{"pci_bridge_interface_t", STRUCT_TYPE},
	{"recorder_interface_t", STRUCT_TYPE},
	{"cell_interrupt_provider_interface_t", STRUCT_TYPE},
	{"branch_arc_iter_t", STRUCT_TYPE},
	{"reg_info_t", STRUCT_TYPE},
	{"execute_interface_t", STRUCT_TYPE},
	{"tracker_interface_t", STRUCT_TYPE},
	{"breakpoint_range_t", STRUCT_TYPE},
	{"map_list_t", STRUCT_TYPE},
	{"v9_memory_transaction_t", STRUCT_TYPE},
	{"memory_space_interface_t", STRUCT_TYPE},
	{"conf_object_interface_t", STRUCT_TYPE},
	{"frags_it_t", STRUCT_TYPE},
	{"image_snoop_interface_t", STRUCT_TYPE},
	{"rapidio_v3_interface_t", STRUCT_TYPE},
	{"usb_transfer_t", STRUCT_TYPE},
	{"microwire_interface_t", STRUCT_TYPE},
	{"generic_transaction_t", STRUCT_TYPE},
	{"link_status_interface_t", STRUCT_TYPE},
	{"simple_dispatcher_interface_t", STRUCT_TYPE},
	{"mii_interface_t", STRUCT_TYPE},
	{"h8_ic_interface_t", STRUCT_TYPE},
	{"pin_interface_t", STRUCT_TYPE},
	{"event_class_t", STRUCT_TYPE},
	{"simple_interrupt_interface_t", STRUCT_TYPE},
	{"common_dma_interface_t", STRUCT_TYPE},
	{"instr_info_t", STRUCT_TYPE},
	{"port_space_interface_t", STRUCT_TYPE},
	{"ms1553_bridge_bus_interface_t", STRUCT_TYPE},
	{"cpu_group_interface_t", STRUCT_TYPE},
	{"component_interface_t", STRUCT_TYPE},
	{"a20_interface_t", STRUCT_TYPE},
	{"sapic_bus_interface_t", STRUCT_TYPE},
	{"branch_arc_t", STRUCT_TYPE},
	{"serial_link_interface_t", STRUCT_TYPE},
	{"physical_block_t", STRUCT_TYPE},
	{"i2c_master_interface_t", STRUCT_TYPE},
	{"buffer_t", STRUCT_TYPE},
	{"sparc_interrupt_interface_t", STRUCT_TYPE},
	{"exception_interface_t", STRUCT_TYPE},
	{"pci_interrupt_interface_t", STRUCT_TYPE},
	{"cycle_interface_t", STRUCT_TYPE},
	{"processor_info_interface_t", STRUCT_TYPE},
	{"mips_interface_t", STRUCT_TYPE},
	{"p2_memory_transaction_t", STRUCT_TYPE},
	{"kbd_console_interface_t", STRUCT_TYPE},
	{"tracker_unix_interface_t", STRUCT_TYPE},
	{"image_spage_t", STRUCT_TYPE},
	{"ethernet_common_device_interface_t", STRUCT_TYPE},
	{"temporal_state_interface_t", STRUCT_TYPE},
	{"frequency_listener_interface_t", STRUCT_TYPE},
	{"signal_interface_t", STRUCT_TYPE},
	{"disasm_instr_t", STRUCT_TYPE},
	{"byte_string_t", STRUCT_TYPE},
	{"arinc429_receiver_interface_t", STRUCT_TYPE},
	{"arm_memory_transaction_t", STRUCT_TYPE},
	{"breakpoint_t", STRUCT_TYPE},
	{"pci_memory_transaction_t", STRUCT_TYPE},
	{"ppc4xx_mal_interface_t", STRUCT_TYPE},
	{"spr_interface_t", STRUCT_TYPE},

	/* ----------------function point type---------- */
	{"dma_set_interrupt_t", FUNCTION_POINTER_TYPE},
	{"interrupt_cpu_t", FUNCTION_POINTER_TYPE},
	{"dma_enable_call_t", FUNCTION_POINTER_TYPE},
	{"dma_write_data_t", FUNCTION_POINTER_TYPE},
	{"retrieve_packet_t", FUNCTION_POINTER_TYPE},
	{"operate_func_t", FUNCTION_POINTER_TYPE},
	{"sparc_u3_error_t", FUNCTION_POINTER_TYPE},
	{"get_bp_ranges_func_t", FUNCTION_POINTER_TYPE},
	{"interrupt_query_enabled_t", FUNCTION_POINTER_TYPE},
	{"dma_clr_interrupt_t", FUNCTION_POINTER_TYPE},
	{"gen_spr_user_getter_func_t", FUNCTION_POINTER_TYPE},
	{"reset_all_t", FUNCTION_POINTER_TYPE},
	{"output_handler_t", FUNCTION_POINTER_TYPE},
	{"dma_read_data_t", FUNCTION_POINTER_TYPE},
	{"recorder_input_handler_t", FUNCTION_POINTER_TYPE},
	{"remove_bp_func_t", FUNCTION_POINTER_TYPE},
	{"set_bus_id_t", FUNCTION_POINTER_TYPE},
	{"interrupt_query_register_t", FUNCTION_POINTER_TYPE},
	{"device_interrupt_t", FUNCTION_POINTER_TYPE},
	{"trigger_t", FUNCTION_POINTER_TYPE},
	{"get_object_by_id_t", FUNCTION_POINTER_TYPE},
	{"set_attr_t", FUNCTION_POINTER_TYPE},
	{"store_packet_t", FUNCTION_POINTER_TYPE},
	{"operation_func_t", FUNCTION_POINTER_TYPE},
	{"interrupt_ack_fn_t", FUNCTION_POINTER_TYPE},
	{"get_class_attr_t", FUNCTION_POINTER_TYPE},
	{"insert_bp_func_t", FUNCTION_POINTER_TYPE},
	{"ethernet_link_snoop_t", FUNCTION_POINTER_TYPE},
	{"dma_write_data_addr_t", FUNCTION_POINTER_TYPE},
	{"remove_bp_range_func_t", FUNCTION_POINTER_TYPE},
	{"utopia_ii_send_func", FUNCTION_POINTER_TYPE},
	{"get_bp_func_t", FUNCTION_POINTER_TYPE},
	{"gen_spr_user_setter_func_t", FUNCTION_POINTER_TYPE},
	{"obj_hap_func_t", FUNCTION_POINTER_TYPE},
	{"get_attr_t", FUNCTION_POINTER_TYPE},
	{"device_interrupt_clear_t", FUNCTION_POINTER_TYPE},
	{"interrupt_ack_t", FUNCTION_POINTER_TYPE},
	{"trap_cpu_t", FUNCTION_POINTER_TYPE},
	{"dma_transfer_time_t", FUNCTION_POINTER_TYPE},
	{"interrupt_changed_state_callback_t", FUNCTION_POINTER_TYPE},
	{"interrupt_by_id_t", FUNCTION_POINTER_TYPE},
	{"dma_read_data_addr_t", FUNCTION_POINTER_TYPE},

	/* -----------------defined---------------------- */
	{"Sim_Attr_Required", CONSTANT_TYPE},
	{"Sim_Attr_Optional", CONSTANT_TYPE},
	{"Sim_Attr_Session", CONSTANT_TYPE},
	{"Sim_Attr_Pseudo", CONSTANT_TYPE},
	{"Sim_Attr_Flag_Mask", CONSTANT_TYPE},
	{"Sim_Init_Phase_Shift", CONSTANT_TYPE},
	{"V9_Access_Normal", CONSTANT_TYPE},
	{"V9_Access_Normal_FP", CONSTANT_TYPE},
	{"V9_Access_Double_FP", CONSTANT_TYPE},
	{"V9_Access_Short_FP", CONSTANT_TYPE},
	{"V9_Access_FSR", CONSTANT_TYPE},
	{"V9_Access_Atomic", CONSTANT_TYPE},
	{"V9_Access_Atomic_Load", CONSTANT_TYPE},
	{"V9_Access_Prefetch", CONSTANT_TYPE},
	{"V9_Access_Partial_Store", CONSTANT_TYPE},
	{"V9_Access_Ldd_Std_1", CONSTANT_TYPE},
	{"V9_Access_Ldd_Std_2", CONSTANT_TYPE},
	{"V9_Access_Block", CONSTANT_TYPE},
	{"V9_Access_Internal1", CONSTANT_TYPE},
	{"Sim_PPC_Generic_MC", CONSTANT_TYPE},
	{"Sim_PPC_MC_TEA", CONSTANT_TYPE},
	{"Sim_PPC_MC_MCP", CONSTANT_TYPE},
	{"Sim_PPC_Bus_Address_Parity", CONSTANT_TYPE},
	{"Sim_PPC_Bus_Data_Parity", CONSTANT_TYPE},
	{"Sim_PPC_Instruction_Cache_Parity", CONSTANT_TYPE},
	{"Sim_PPC_Data_Cache_Parity", CONSTANT_TYPE},
	{"Sim_PPC_L2_Data_Parity", CONSTANT_TYPE},
	{"Sim_PPC_L3_Data_Parity", CONSTANT_TYPE},
	{"Sim_PPC_L3_Address_Parity", CONSTANT_TYPE},
	{"Sim_PPC_L3_Address_Parity", CONSTANT_TYPE},
	{"Sim_PPC970_Data_Cache_Parity", CONSTANT_TYPE},
	{"Sim_PPC970_Data_Cache_Tag_Parity", CONSTANT_TYPE},
	{"Sim_PPC970_D_ERAT_Parity", CONSTANT_TYPE},
	{"Sim_PPC970_TLB_Parity", CONSTANT_TYPE},
	{"Sim_PPC970_SLB_Parity", CONSTANT_TYPE},
	{"Sim_PPC970_L2_Load_ECC_Parity", CONSTANT_TYPE},
	{"Sim_PPC970_L2_Page_Table_ECC_Parity", CONSTANT_TYPE},
	{"Sim_PPC970_Uncacheable_Load_Parity", CONSTANT_TYPE},
	{"Sim_PPC970_MC_External", CONSTANT_TYPE},
	{"USB_Type_Control", CONSTANT_TYPE},
	{"USB_Type_Interrupt", CONSTANT_TYPE},
	{"USB_Type_Isochronous", CONSTANT_TYPE},
	{"USB_Type_Bulk", CONSTANT_TYPE},
	{"Firewire_Response_Complete", CONSTANT_TYPE},
	{"Firewire_Response_Data_Error", CONSTANT_TYPE},
	{"Firewire_Response_Type_Error", CONSTANT_TYPE},
	{"Firewire_Response_Address_Error", CONSTANT_TYPE},
	{"IEEE_link_unconnected", CONSTANT_TYPE},
	{"IEEE_link_down", CONSTANT_TYPE},
	{"IEEE_link_up", CONSTANT_TYPE},
	{"Sim_RW_Read", CONSTANT_TYPE},
	{"Sim_RW_Write", CONSTANT_TYPE},
	{"X86_Other", CONSTANT_TYPE},
	{"X86_Vanilla", CONSTANT_TYPE},
	{"X86_Instruction", CONSTANT_TYPE},
	{"X86_Clflush", CONSTANT_TYPE},
	{"X86_Fpu_Env", CONSTANT_TYPE},
	{"X86_Fpu_State", CONSTANT_TYPE},
	{"X86_Idt", CONSTANT_TYPE},
	{"X86_Gdt", CONSTANT_TYPE},
	{"X86_Ldt", CONSTANT_TYPE},
	{"X86_Task_Segment", CONSTANT_TYPE},
	{"X86_Task_Switch", CONSTANT_TYPE},
	{"X86_Far_Call_Parameter", CONSTANT_TYPE},
	{"X86_Stack", CONSTANT_TYPE},
	{"X86_Pml4", CONSTANT_TYPE},
	{"X86_Pdp", CONSTANT_TYPE},
	{"X86_Pd", CONSTANT_TYPE},
	{"X86_Pt", CONSTANT_TYPE},
	{"X86_Sse", CONSTANT_TYPE},
	{"X86_Fpu", CONSTANT_TYPE},
	{"X86_Access_Simple", CONSTANT_TYPE},
	{"X86_Microcode_Update", CONSTANT_TYPE},
	{"X86_Non_Temporal", CONSTANT_TYPE},
	{"X86_Prefetch_3DNow", CONSTANT_TYPE},
	{"X86_Prefetchw_3DNow", CONSTANT_TYPE},
	{"X86_Prefetch_T0", CONSTANT_TYPE},
	{"X86_Prefetch_T1", CONSTANT_TYPE},
	{"X86_Prefetch_T2", CONSTANT_TYPE},
	{"X86_Prefetch_NTA", CONSTANT_TYPE},
	{"X86_Loadall", CONSTANT_TYPE},
	{"X86_Atomic_Info", CONSTANT_TYPE},
	{"X86_Cmpxchg16b", CONSTANT_TYPE},
	{"X86_Smm_State", CONSTANT_TYPE},
	{"X86_Vmcs", CONSTANT_TYPE},
	{"X86_Vmx_IO_Bitmap", CONSTANT_TYPE},
	{"X86_Vmx_Vapic", CONSTANT_TYPE},
	{"X86_Vmx_Msr", CONSTANT_TYPE},
	{"SimExc_No_Exception", CONSTANT_TYPE},
	{"SimExc_General", CONSTANT_TYPE},
	{"SimExc_Arithmetic", CONSTANT_TYPE},
	{"SimExc_Lookup", CONSTANT_TYPE},
	{"SimExc_Attribute", CONSTANT_TYPE},
	{"SimExc_IOError", CONSTANT_TYPE},
	{"SimExc_Index", CONSTANT_TYPE},
	{"SimExc_Memory", CONSTANT_TYPE},
	{"SimExc_Inquiry", CONSTANT_TYPE},
	{"SimExc_InquiryOutsideMemory", CONSTANT_TYPE},
	{"SimExc_InquiryUnhandled", CONSTANT_TYPE},
	{"SimExc_Overflow", CONSTANT_TYPE},
	{"SimExc_Syntax", CONSTANT_TYPE},
	{"SimExc_Type", CONSTANT_TYPE},
	{"SimExc_ZeroDivision", CONSTANT_TYPE},
	{"SimExc_RuntimeError", CONSTANT_TYPE},
	{"SimExc_Break", CONSTANT_TYPE},
	{"SimExc_PythonTranslation", CONSTANT_TYPE},
	{"SimExc_License", CONSTANT_TYPE},
	{"SimExc_IllegalValue", CONSTANT_TYPE},
	{"SimExc_ObjectNotFound", CONSTANT_TYPE},
	{"SimExc_InterfaceNotFound", CONSTANT_TYPE},
	{"SimExc_AttrNotFound", CONSTANT_TYPE},
	{"SimExc_AttrNotReadable", CONSTANT_TYPE},
	{"SimExc_AttrNotWritable", CONSTANT_TYPE},
	{"SimExc_NeedInteger", CONSTANT_TYPE},
	{"SimExc_NeedFloating", CONSTANT_TYPE},
	{"SimExc_NeedBoolean", CONSTANT_TYPE},
	{"SimExc_NeedString", CONSTANT_TYPE},
	{"SimExc_NeedList", CONSTANT_TYPE},
	{"SimExc_NeedDict", CONSTANT_TYPE},
	{"SimExc_NeedData", CONSTANT_TYPE},
	{"SimExc_NeedObject", CONSTANT_TYPE},
	{"Sim_Exceptions", CONSTANT_TYPE},
	{"PPC_Sleep_Awake", CONSTANT_TYPE},
	{"PPC_Sleep_MSR", CONSTANT_TYPE},
	{"PPC_Sleep_Doze", CONSTANT_TYPE},
	{"PPC_Sleep_Nap", CONSTANT_TYPE},
	{"PPC_Sleep_Sleep", CONSTANT_TYPE},
	{"PPC_Sleep_Rvwinkle", CONSTANT_TYPE},
	{"PPC_Sleep_Wait", CONSTANT_TYPE},
	{"PPC_Sleep_Mdors", CONSTANT_TYPE},
	{"Sim_Gen_Spr_Ok", CONSTANT_TYPE},
	{"Sim_Gen_Spr_Illegal", CONSTANT_TYPE},
	{"Sim_Gen_Spr_Privilege", CONSTANT_TYPE},
	{"Sim_Gen_Spr_Processor_Sleeps", CONSTANT_TYPE},
	{"Sim_Endian_Target", CONSTANT_TYPE},
	{"Sim_Endian_Host_From_BE", CONSTANT_TYPE},
	{"Sim_Endian_Host_From_LE", CONSTANT_TYPE},
	{"Sim_CPU_Mode_User", CONSTANT_TYPE},
	{"Sim_CPU_Mode_Supervisor", CONSTANT_TYPE},
	{"Sim_CPU_Mode_Hypervisor", CONSTANT_TYPE},
	{"SAPIC_Edge", CONSTANT_TYPE},
	{"SAPIC_Level", CONSTANT_TYPE},
	{"SAPIC_Bus_Accepted", CONSTANT_TYPE},
	{"SAPIC_Bus_Retry", CONSTANT_TYPE},
	{"SAPIC_Bus_No_Target", CONSTANT_TYPE},
	{"SAPIC_Bus_Invalid", CONSTANT_TYPE},
	{"Decoration_None", CONSTANT_TYPE},
	{"Decoration_Notify", CONSTANT_TYPE},
	{"Decoration_Load", CONSTANT_TYPE},
	{"Decoration_Store", CONSTANT_TYPE},
	{"SPI_Flags_CPHA", CONSTANT_TYPE},
	{"SPI_Flags_CPOL", CONSTANT_TYPE},
	{"Sim_EC_No_Flags", CONSTANT_TYPE},
	{"Sim_EC_Notsaved", CONSTANT_TYPE},
	{"Sim_EC_Slot_Early", CONSTANT_TYPE},
	{"Sim_EC_Slot_Late", CONSTANT_TYPE},
	{"Sim_EC_Machine_Sync", CONSTANT_TYPE},
	{"MS1553_Dir_Transmit", CONSTANT_TYPE},
	{"MS1553_Dir_Receive", CONSTANT_TYPE},
	{"I2C_idle", CONSTANT_TYPE},
	{"I2C_master_transmit", CONSTANT_TYPE},
	{"I2C_master_receive", CONSTANT_TYPE},
	{"I2C_slave_transmit", CONSTANT_TYPE},
	{"I2C_slave_receive", CONSTANT_TYPE},
	{"I2C_max_states", CONSTANT_TYPE},
	{"Sim_Trackee_Invalid", CONSTANT_TYPE},
	{"Sim_Trackee_Kernel", CONSTANT_TYPE},
	{"Sim_Trackee_Process", CONSTANT_TYPE},
	{"Sim_NM_Read", CONSTANT_TYPE},
	{"Sim_NM_Write", CONSTANT_TYPE},
	{"Sim_Addr_Space_Conf", CONSTANT_TYPE},
	{"Sim_Addr_Space_IO", CONSTANT_TYPE},
	{"Sim_Addr_Space_Memory", CONSTANT_TYPE},
	{"USB_Direction_None", CONSTANT_TYPE},
	{"USB_Direction_In", CONSTANT_TYPE},
	{"USB_Direction_Out", CONSTANT_TYPE},
	{"Instruction_Fetch_None", CONSTANT_TYPE},
	{"Instruction_Fetch_Cache_Access_Trace", CONSTANT_TYPE},
	{"Instruction_Fetch_Trace", CONSTANT_TYPE},
	{"Sim_Endian_Little", CONSTANT_TYPE},
	{"Sim_Endian_Big", CONSTANT_TYPE},
	{"MS1553_Phase_Idle", CONSTANT_TYPE},
	{"MS1553_Phase_T_Command", CONSTANT_TYPE},
	{"MS1553_Phase_R_Command", CONSTANT_TYPE},
	{"MS1553_Phase_M_T_Command", CONSTANT_TYPE},
	{"MS1553_Phase_M_R_Command", CONSTANT_TYPE},
	{"MS1553_Phase_M_N_Command", CONSTANT_TYPE},
	{"MS1553_Phase_Data", CONSTANT_TYPE},
	{"MS1553_Phase_Status", CONSTANT_TYPE},
	{"MS1553_Num_Phases", CONSTANT_TYPE},
	{"H8_Transition_From_Active", CONSTANT_TYPE},
	{"H8_Transition_From_Sleep", CONSTANT_TYPE},
	{"USB_Speed_Low", CONSTANT_TYPE},
	{"USB_Speed_Full", CONSTANT_TYPE},
	{"USB_Speed_High", CONSTANT_TYPE},
	{"RapidIO_Read", CONSTANT_TYPE},
	{"RapidIO_Write", CONSTANT_TYPE},
	{"RapidIO_Increment", CONSTANT_TYPE},
	{"RapidIO_Decrement", CONSTANT_TYPE},
	{"RapidIO_Test_and_Swap", CONSTANT_TYPE},
	{"RapidIO_Set", CONSTANT_TYPE},
	{"RapidIO_Clear", CONSTANT_TYPE},
	{"RapidIO_Compare_and_Swap", CONSTANT_TYPE},
	{"Apic_Performance_Counter", CONSTANT_TYPE},
	{"Apic_Thermal_Sensor", CONSTANT_TYPE},
	{"Sim_Map_Ram", CONSTANT_TYPE},
	{"Sim_Map_Rom", CONSTANT_TYPE},
	{"Sim_Map_IO", CONSTANT_TYPE},
	{"Sim_Map_Port", CONSTANT_TYPE},
	{"Sim_Map_Translate", CONSTANT_TYPE},
	{"Sim_Map_Translate_To_Space", CONSTANT_TYPE},
	{"Sim_Map_Translate_To_Ram", CONSTANT_TYPE},
	{"Sim_Map_Translate_To_Rom", CONSTANT_TYPE},
	{"MS1553_Dynamic_Bus_Control", CONSTANT_TYPE},
	{"MS1553_Synchronize_T", CONSTANT_TYPE},
	{"MS1553_Transmit_Status_word", CONSTANT_TYPE},
	{"MS1553_Initiate_Self_Test", CONSTANT_TYPE},
	{"MS1553_Transmitter_Shutdown", CONSTANT_TYPE},
	{"MS1553_Override_Transmitter_Shutdown", CONSTANT_TYPE},
	{"MS1553_Inhibit_Terminal_Flag_Bit", CONSTANT_TYPE},
	{"MS1553_Override_Inhibit_Terminal_Flag_Bit", CONSTANT_TYPE},
	{"MS1553_Reset_Remote_Terminal", CONSTANT_TYPE},
	{"MS1553_Transmit_Vector_Word", CONSTANT_TYPE},
	{"MS1553_Synchronize_R", CONSTANT_TYPE},
	{"MS1553_Transmit_Last_Command_Word", CONSTANT_TYPE},
	{"MS1553_Transmit_BIT_Vector", CONSTANT_TYPE},
	{"MS1553_Selected_Transmitter_Shutdown", CONSTANT_TYPE},
	{"MS1553_Override_Selected_Transmitter_Shutdown", CONSTANT_TYPE},
	{"MS1553_Err_Manchester", CONSTANT_TYPE},
	{"MS1553_Err_Sync_Field", CONSTANT_TYPE},
	{"MS1553_Err_Word_Count", CONSTANT_TYPE},
	{"MS1553_Err_Parity", CONSTANT_TYPE},
	{"MS1553_Err_Protocol", CONSTANT_TYPE},
	{"Stop_Bits_1", CONSTANT_TYPE},
	{"Stop_Bits_1p5", CONSTANT_TYPE},
	{"Stop_Bits_2", CONSTANT_TYPE},
	{"Normal_Load_Store", CONSTANT_TYPE},
	{"Caching_Inhibited", CONSTANT_TYPE},
	{"Instr_Multiple", CONSTANT_TYPE},
	{"Instr_String", CONSTANT_TYPE},
	{"Instr_Altivec_Element", CONSTANT_TYPE},
	{"Instr_dcbt", CONSTANT_TYPE},
	{"Instr_dcbst", CONSTANT_TYPE},
	{"Instr_dcbtst", CONSTANT_TYPE},
	{"Instr_dcbi", CONSTANT_TYPE},
	{"Instr_dcbf", CONSTANT_TYPE},
	{"Instr_dcbfl", CONSTANT_TYPE},
	{"Instr_dcba", CONSTANT_TYPE},
	{"Instr_dcbz", CONSTANT_TYPE},
	{"Instr_icbi", CONSTANT_TYPE},
	{"Instr_dst", CONSTANT_TYPE},
	{"Instr_dstt", CONSTANT_TYPE},
	{"Instr_dstst", CONSTANT_TYPE},
	{"Instr_dststt", CONSTANT_TYPE},
	{"Instr_dcblc_l1", CONSTANT_TYPE},
	{"Instr_dcblc_l2", CONSTANT_TYPE},
	{"Instr_dcbtls_l1", CONSTANT_TYPE},
	{"Instr_dcbtls_l2", CONSTANT_TYPE},
	{"Instr_dcbtstls_l1", CONSTANT_TYPE},
	{"Instr_dcbtstls_l2", CONSTANT_TYPE},
	{"Instr_icblc_l1", CONSTANT_TYPE},
	{"Instr_icblc_l2", CONSTANT_TYPE},
	{"Instr_icbtls_l1", CONSTANT_TYPE},
	{"Instr_icbtls_l2", CONSTANT_TYPE},
	{"Instr_lwarx", CONSTANT_TYPE},
	{"Instr_stwcx", CONSTANT_TYPE},
	{"Instr_ldarx", CONSTANT_TYPE},
	{"Instr_stdcx", CONSTANT_TYPE},
	{"Instr_lq", CONSTANT_TYPE},
	{"Instr_stq", CONSTANT_TYPE},
	{"Instr_sync", CONSTANT_TYPE},
	{"Instr_eieio", CONSTANT_TYPE},
	{"Instr_ecowx", CONSTANT_TYPE},
	{"Instr_eciwx", CONSTANT_TYPE},
	{"Instr_tlbie", CONSTANT_TYPE},
	{"Instr_tlbsync", CONSTANT_TYPE},
	{"Instr_isync", CONSTANT_TYPE},
	{"Instr_lfdp", CONSTANT_TYPE},
	{"Instr_stfdp", CONSTANT_TYPE},
	{"Instr_spe", CONSTANT_TYPE},
	{"Instr_dcbal", CONSTANT_TYPE},
	{"Sim_Hap_Simulation", CONSTANT_TYPE},
	{"Sim_Hap_Attr", CONSTANT_TYPE},
	{"Sim_Set_Ok", CONSTANT_TYPE},
	{"Sim_Set_Need_Integer", CONSTANT_TYPE},
	{"Sim_Set_Need_Floating", CONSTANT_TYPE},
	{"Sim_Set_Need_String", CONSTANT_TYPE},
	{"Sim_Set_Need_List", CONSTANT_TYPE},
	{"Sim_Set_Need_Dict", CONSTANT_TYPE},
	{"Sim_Set_Need_Boolean", CONSTANT_TYPE},
	{"Sim_Set_Need_Data", CONSTANT_TYPE},
	{"Sim_Set_Need_Object", CONSTANT_TYPE},
	{"Sim_Set_Object_Not_Found", CONSTANT_TYPE},
	{"Sim_Set_Interface_Not_Found", CONSTANT_TYPE},
	{"Sim_Set_Illegal_Value", CONSTANT_TYPE},
	{"Sim_Set_Illegal_Type", CONSTANT_TYPE},
	{"Sim_Set_Illegal_Index", CONSTANT_TYPE},
	{"Sim_Set_Attribute_Not_Found", CONSTANT_TYPE},
	{"Sim_Set_Not_Writable", CONSTANT_TYPE},
	{"Sim_Set_Ignored", CONSTANT_TYPE},
	{"Sim_Set_Error_Types", CONSTANT_TYPE},
	{"Sim_Sync_Processor", CONSTANT_TYPE},
	{"Sim_Sync_Machine", CONSTANT_TYPE},
	{"Pal_memop_normal", CONSTANT_TYPE},
	{"Pal_memop_physical", CONSTANT_TYPE},
	{"Pal_memop_altmode", CONSTANT_TYPE},
	{"Pal_memop_wrtck", CONSTANT_TYPE},
	{"Pal_memop_quadword", CONSTANT_TYPE},
	{"Pal_memop_vpte", CONSTANT_TYPE},
	{"Pal_memop_lock", CONSTANT_TYPE},
	{"Pal_memop_cond", CONSTANT_TYPE},
	{"Sim_Initiator_Illegal", CONSTANT_TYPE},
	{"Sim_Initiator_CPU", CONSTANT_TYPE},
	{"Sim_Initiator_CPU_V9", CONSTANT_TYPE},
	{"Sim_Initiator_CPU_UII", CONSTANT_TYPE},
	{"Sim_Initiator_CPU_UIII", CONSTANT_TYPE},
	{"Sim_Initiator_CPU_UIV", CONSTANT_TYPE},
	{"Sim_Initiator_CPU_UT1", CONSTANT_TYPE},
	{"Sim_Initiator_CPU_X86", CONSTANT_TYPE},
	{"Sim_Initiator_CPU_PPC", CONSTANT_TYPE},
	{"Sim_Initiator_CPU_Alpha", CONSTANT_TYPE},
	{"Sim_Initiator_CPU_IA64", CONSTANT_TYPE},
	{"Sim_Initiator_CPU_MIPS", CONSTANT_TYPE},
	{"Sim_Initiator_CPU_MIPS_RM7000", CONSTANT_TYPE},
	{"Sim_Initiator_CPU_MIPS_E9000", CONSTANT_TYPE},
	{"Sim_Initiator_CPU_ARM", CONSTANT_TYPE},
	{"Sim_Initiator_Device", CONSTANT_TYPE},
	{"Sim_Initiator_PCI_Device", CONSTANT_TYPE},
	{"Sim_Initiator_Cache", CONSTANT_TYPE},
	{"Sim_Initiator_Other", CONSTANT_TYPE},
	{"Sim_DI_Instruction", CONSTANT_TYPE},
	{"Sim_DI_Data", CONSTANT_TYPE},
	{"USB_Transfer_Completed", CONSTANT_TYPE},
	{"USB_Transfer_Not_Ready", CONSTANT_TYPE},
	{"H8_Sleep_Awake", CONSTANT_TYPE},
	{"H8_Sleep_Sleep", CONSTANT_TYPE},
	{"H8_Sleep_Soft_Standby", CONSTANT_TYPE},
	{"H8_Sleep_Hard_Standby", CONSTANT_TYPE},
	{"H8_Sleep_Watch", CONSTANT_TYPE},
	{"H8_Sleep_Subsleep", CONSTANT_TYPE},
	{"H8_Sleep_Subactive", CONSTANT_TYPE},
	{"Sim_Val_Invalid", CONSTANT_TYPE},
	{"Sim_Val_String", CONSTANT_TYPE},
	{"Sim_Val_Integer", CONSTANT_TYPE},
	{"Sim_Val_Floating", CONSTANT_TYPE},
	{"Sim_Val_List", CONSTANT_TYPE},
	{"Sim_Val_Data", CONSTANT_TYPE},
	{"Sim_Val_Nil", CONSTANT_TYPE},
	{"Sim_Val_Object", CONSTANT_TYPE},
	{"Sim_Val_Dict", CONSTANT_TYPE},
	{"Sim_Val_Boolean", CONSTANT_TYPE},
	{"Sim_Val_Unresolved_Data", CONSTANT_TYPE},
	{"Sim_Val_Unresolved_Object", CONSTANT_TYPE},
	{"Sim_RegInfo_Catchable", CONSTANT_TYPE},
	{"Parity_None", CONSTANT_TYPE},
	{"Parity_Even", CONSTANT_TYPE},
	{"Parity_Odd", CONSTANT_TYPE},
	{"Firewire_Ack_Complete", CONSTANT_TYPE},
	{"Firewire_Ack_Pending", CONSTANT_TYPE},
	{"Firewire_Ack_Busy_X", CONSTANT_TYPE},
	{"Firewire_Ack_Busy_A", CONSTANT_TYPE},
	{"Firewire_Ack_Busy_B", CONSTANT_TYPE},
	{"Firewire_Ack_Tardy", CONSTANT_TYPE},
	{"Firewire_Ack_Conflict_Error", CONSTANT_TYPE},
	{"Firewire_Ack_Data_Error", CONSTANT_TYPE},
	{"Firewire_Ack_Type_Error", CONSTANT_TYPE},
	{"Firewire_Ack_Address_Error", CONSTANT_TYPE},
	{"Firewire_Ack_No_Destination", CONSTANT_TYPE},
	{"Firewire_Ack_No_Ack", CONSTANT_TYPE},
	{"Apic_Trigger_Mode_Edge", CONSTANT_TYPE},
	{"Apic_Trigger_Mode_Level", CONSTANT_TYPE},
	{"I2C_flag_exclusive", CONSTANT_TYPE},
	{"I2C_flag_shared", CONSTANT_TYPE},
	{"BR_Direction_From", CONSTANT_TYPE},
	{"BR_Direction_To", CONSTANT_TYPE},
	{"Sim_Access_Read", CONSTANT_TYPE},
	{"Sim_Access_Write", CONSTANT_TYPE},
	{"Sim_Access_Execute", CONSTANT_TYPE},
	{"Sim_PE_No_Exception", CONSTANT_TYPE},
	{"Sim_PE_Silent_Break", CONSTANT_TYPE},
	{"Sim_PE_Stop_Request", CONSTANT_TYPE},
	{"Sim_PE_Inquiry_Outside_Memory", CONSTANT_TYPE},
	{"Sim_PE_Inquiry_Unhandled", CONSTANT_TYPE},
	{"Sim_PE_Execute_Outside_Memory", CONSTANT_TYPE},
	{"Sim_PE_IO_Not_Taken", CONSTANT_TYPE},
	{"Sim_PE_IO_Error", CONSTANT_TYPE},
	{"Sim_PE_Interrupt_Break", CONSTANT_TYPE},
	{"Sim_PE_Interrupt_Break_Take_Now", CONSTANT_TYPE},
	{"Sim_PE_Stall_Cpu", CONSTANT_TYPE},
	{"Sim_PE_Locked_Memory", CONSTANT_TYPE},
	{"Sim_PE_Return_Break", CONSTANT_TYPE},
	{"Sim_PE_Instruction_Finished", CONSTANT_TYPE},
	{"Sim_PE_Default_Semantics", CONSTANT_TYPE},
	{"Sim_PE_Ignore_Semantics", CONSTANT_TYPE},
	{"Sim_PE_Speculation_Failed", CONSTANT_TYPE},
	{"Sim_PE_Invalid_Address", CONSTANT_TYPE},
	{"Sim_PE_MAI_Return", CONSTANT_TYPE},
	{"Sim_PE_Last", CONSTANT_TYPE},
	{"Apic_Bus_Accepted", CONSTANT_TYPE},
	{"Apic_Bus_Retry", CONSTANT_TYPE},
	{"Apic_Bus_No_Target", CONSTANT_TYPE},
	{"Apic_Bus_Invalid", CONSTANT_TYPE},
	{"Phy_Speed_10BASE_T", CONSTANT_TYPE},
	{"Phy_Speed_10BASE_T_FD", CONSTANT_TYPE},
	{"Phy_Speed_100BASE_TX", CONSTANT_TYPE},
	{"Phy_Speed_100BASE_T4", CONSTANT_TYPE},
	{"Phy_Speed_100BASE_T2", CONSTANT_TYPE},
	{"Phy_Speed_100BASE_TX_FD", CONSTANT_TYPE},
	{"Phy_Speed_100BASE_T2_FD", CONSTANT_TYPE},
	{"Phy_Speed_1000BASE_T", CONSTANT_TYPE},
	{"Phy_Speed_1000BASE_T_FD", CONSTANT_TYPE},
	{"Firewire_Write_Quadlet", CONSTANT_TYPE},
	{"Firewire_Write_Block", CONSTANT_TYPE},
	{"Firewire_Write_Response", CONSTANT_TYPE},
	{"Firewire_Read_Quadlet", CONSTANT_TYPE},
	{"Firewire_Read_Block", CONSTANT_TYPE},
	{"Firewire_Read_Quadlet_Response", CONSTANT_TYPE},
	{"Firewire_Read_Block_Response", CONSTANT_TYPE},
	{"Firewire_Cycle_Start", CONSTANT_TYPE},
	{"Firewire_Lock", CONSTANT_TYPE},
	{"Firewire_Streaming", CONSTANT_TYPE},
	{"Firewire_Lock_Response", CONSTANT_TYPE},
	{"PCIE_Msg_Assert_INTA", CONSTANT_TYPE},
	{"PCIE_Msg_Assert_INTB", CONSTANT_TYPE},
	{"PCIE_Msg_Assert_INTC", CONSTANT_TYPE},
	{"PCIE_Msg_Assert_INTD", CONSTANT_TYPE},
	{"PCIE_Msg_Deassert_INTA", CONSTANT_TYPE},
	{"PCIE_Msg_Deassert_INTB", CONSTANT_TYPE},
	{"PCIE_Msg_Deassert_INTC", CONSTANT_TYPE},
	{"PCIE_Msg_Deassert_INTD", CONSTANT_TYPE},
	{"PCIE_PM_Active_State_Nak", CONSTANT_TYPE},
	{"PCIE_PM_PME", CONSTANT_TYPE},
	{"PCIE_PM_Turn_Off", CONSTANT_TYPE},
	{"PCIE_PM_PME_TO_Ack", CONSTANT_TYPE},
	{"PCIE_ERR_COR", CONSTANT_TYPE},
	{"PCIE_ERR_NONFATAL", CONSTANT_TYPE},
	{"PCIE_ERR_FATAL", CONSTANT_TYPE},
	{"PCIE_Locked_Transaction", CONSTANT_TYPE},
	{"PCIE_Set_Slot_Power_Limit", CONSTANT_TYPE},
	{"PCIE_HP_Power_Indicator_On", CONSTANT_TYPE},
	{"PCIE_HP_Power_Indicator_Blink", CONSTANT_TYPE},
	{"PCIE_HP_Power_Indicator_Off", CONSTANT_TYPE},
	{"PCIE_HP_Attention_Button_Pressed", CONSTANT_TYPE},
	{"PCIE_HP_Attention_Indicator_On", CONSTANT_TYPE},
	{"PCIE_HP_Attention_Indicator_Blink", CONSTANT_TYPE},
	{"PCIE_HP_Attention_Indicator_Off", CONSTANT_TYPE},
	{"I2C_status_success", CONSTANT_TYPE},
	{"I2C_status_noack", CONSTANT_TYPE},
	{"I2C_status_bus_busy", CONSTANT_TYPE},
	{"Sim_Swap_None", CONSTANT_TYPE},
	{"Sim_Swap_Bus", CONSTANT_TYPE},
	{"Sim_Swap_Bus_Trans", CONSTANT_TYPE},
	{"Sim_Swap_Trans", CONSTANT_TYPE},
	{"MS1553_Shadow_Word_Command", CONSTANT_TYPE},
	{"MS1553_Shadow_Word_Status", CONSTANT_TYPE},
	{"MS1553_Shadow_Word_BIT", CONSTANT_TYPE},
	{"MS1553_Shadow_Word_Vector", CONSTANT_TYPE},
	{"Apic_Destination_Mode_Physical", CONSTANT_TYPE},
	{"Apic_Destination_Mode_Logical", CONSTANT_TYPE},
	{"X86_None", CONSTANT_TYPE},
	{"X86_Strong_Uncacheable", CONSTANT_TYPE},
	{"X86_Uncacheable", CONSTANT_TYPE},
	{"X86_Write_Combining", CONSTANT_TYPE},
	{"X86_Write_Through", CONSTANT_TYPE},
	{"X86_Write_Back", CONSTANT_TYPE},
	{"X86_Write_Protected", CONSTANT_TYPE},
	{"Sim_Gen_Spr_Instruction_Access", CONSTANT_TYPE},
	{"Sim_Gen_Spr_Attribute_Access", CONSTANT_TYPE},
	{"Sim_Gen_Spr_Int_Register_Access", CONSTANT_TYPE},
	{"H8_Int_Exception", CONSTANT_TYPE},
	{"H8_Int_Trap", CONSTANT_TYPE},
	{"H8_Int_Interrupt", CONSTANT_TYPE},
	{"H8_Int_Nmi", CONSTANT_TYPE},
	{"Pin_Init", CONSTANT_TYPE},
	{"Pin_Smi", CONSTANT_TYPE},
	{"Pin_Nmi", CONSTANT_TYPE},
	{"Pin_Ignne", CONSTANT_TYPE},
	{"Sim_Log_Info", CONSTANT_TYPE},
	{"Sim_Log_Error", CONSTANT_TYPE},
	{"Sim_Log_Undefined", CONSTANT_TYPE},
	{"Sim_Log_Spec_Violation", CONSTANT_TYPE},
	{"Sim_Log_Unimplemented", CONSTANT_TYPE},
	{"Sim_Log_Num_Types", CONSTANT_TYPE},
	{"USB_Status_Undef", CONSTANT_TYPE},
	{"USB_Status_Ack", CONSTANT_TYPE},
	{"USB_Status_Nak", CONSTANT_TYPE},
	{"USB_Status_Stall", CONSTANT_TYPE},
	{"Sim_Break_Physical", CONSTANT_TYPE},
	{"Sim_Break_Virtual", CONSTANT_TYPE},
	{"Sim_Break_Linear", CONSTANT_TYPE},
	{"Branch_Arc_Branch", CONSTANT_TYPE},
	{"Branch_Arc_Exception", CONSTANT_TYPE},
	{"Branch_Arc_Exception_Return", CONSTANT_TYPE},
	{"Branch_Arc_Max", CONSTANT_TYPE},
	{"Sim_Trans_Load", CONSTANT_TYPE},
	{"SAPIC_Fixed0", CONSTANT_TYPE},
	{"SAPIC_Fixed1", CONSTANT_TYPE},
	{"SAPIC_PMI", CONSTANT_TYPE},
	{"SAPIC_NMI", CONSTANT_TYPE},
	{"SAPIC_INIT", CONSTANT_TYPE},
	{"SAPIC_Ext_INT", CONSTANT_TYPE},
	{"Sim_Class_Kind_Vanilla", CONSTANT_TYPE},
	{"Sim_Class_Kind_Session", CONSTANT_TYPE},
	{"Sim_Class_Kind_Pseudo", CONSTANT_TYPE},
	{"Apic_Delivery_Mode_Fixed", CONSTANT_TYPE},
	{"Apic_Delivery_Mode_Lowest_Priority", CONSTANT_TYPE},
	{"Apic_Delivery_Mode_SMI", CONSTANT_TYPE},
	{"Apic_Delivery_Mode_Remote_Read", CONSTANT_TYPE},
	{"Apic_Delivery_Mode_NMI", CONSTANT_TYPE},
	{"Apic_Delivery_Mode_INIT", CONSTANT_TYPE},
	{"Apic_Delivery_Mode_Start_Up", CONSTANT_TYPE},
	{"Apic_Delivery_Mode_Ext_INT", CONSTANT_TYPE},
	{"Sim_Reg_Type_Invalid", CONSTANT_TYPE},
	{"Sim_Reg_Type_Integer", CONSTANT_TYPE},
	{"Sim_Reg_Type_Floating", CONSTANT_TYPE},
	{"Sim_Reg_Type_Control", CONSTANT_TYPE},

#if 0
	/*---------------------- auto type ---------------*/
	{"parent", AUTO_TYPE},
	{"name", AUTO_TYPE},
	{"qname", AUTO_TYPE},
	{"banks", AUTO_TYPE},
	{"obj", AUTO_TYPE},
	{"logobj", AUTO_TYPE},
	{"mapped_registers", AUTO_TYPE},
	{"unmapped_registers", AUTO_TYPE},
	{"numbered_registers", AUTO_TYPE},
	{"fields", AUTO_TYPE},
	{"explicit", AUTO_TYPE},
	{"interfaces", AUTO_TYPE},
	{"_regname", AUTO_TYPE},
	{"explicit", AUTO_TYPE},
	{"evclass", AUTO_TYPE},
#endif
};

int insert_pre_dml_struct(void) {
	int i = 0;
	int num = (sizeof(pre_dml_struct)) / (sizeof(pre_dml_t));

	for (i = 0; i < num; i++) {
		pre_symbol_insert(pre_dml_struct[i]);
	}

	//find_all_pre_dml_struct();

	return 0;
}

int find_all_pre_dml_struct(void) {
	int i = 0;
	int num = (sizeof(pre_dml_struct)) / (sizeof(pre_dml_t));
	printf("pre_dml_struct num: %d\n", num);

	pre_parse_symbol_t* symbol = NULL;
	for (i = 0; i < num; i++) {
		symbol = pre_symbol_find(pre_dml_struct[i].name);
		printf("finded: %d, type: %d : %s\n", i, symbol->type, symbol->name);
	}

	return 0;
}

standard_param_t* get_standard_param(int type) {
	standard_param_t* standard_param = NULL;

	switch (type) {
		case DEVICE_TYPE:
			standard_param = device_param;
			break;
		case BANK_TYPE:
			standard_param = bank_param;
			break;
		case REGISTER_TYPE:
			standard_param = register_param;
			break;
		case FIELD_TYPE:
			standard_param = field_param;
			break;
		case CONNECT_TYPE:
			standard_param = connect_param;
			break;
		case INTERFACE_TYPE:
			standard_param = interface_param;
			break;
		case ATTRIBUTE_TYPE:
			standard_param = attribute_param;
			break;
		case EVENT_TYPE:
			standard_param = event_param;
			break;
		case GROUP_TYPE:
			standard_param = object_common_param;
			break;
		case IMPLEMENT_TYPE:
			standard_param = implement_param;
			break;
		default:
			standard_param = NULL;
			break;
	}

	return standard_param;
}

int standard_param_type(const char* name, standard_param_t* standard_param) {
	assert(name != NULL);

	if (standard_param == NULL) {
		return 0;
	}

	int i = 0;
	while (standard_param[i].name != NULL) {
		if (strcmp(standard_param[i].name, name) == 0) {
			return standard_param[i].value_type;
		}
		i++;
	}

	return 0;
}

static int array_paramer(const char* name, symtab_t table) {
	assert(name != NULL);
	assert(table != NULL);

	symbol_t symbol = NULL;
	/* if the parameter is array parameter */
	if ((strcmp(name, "i") == 0) ||
			(strcmp(name, "index") == 0) ||
			(strcmp(name, "indexvar") == 0)) {
		/* if object is array, the i, index, indexvar parameter insert it's
		 * table when the object created, and their values are fixed automatic,
		 * can not assign the again */
		symbol = symbol_find(table, name, PARAMETER_TYPE);
		if (symbol != NULL) {
			return 1;
		}
	}

	return 0;
}

int get_standard_param_type(const char* name, standard_param_t* standard_param) {
	assert(name != NULL);
	assert(standard_param != NULL);

	int i = 0;
	while(standard_param[i].name != NULL) {
		if (strcmp(standard_param[i].name, name) == 0) {
			return standard_param[i].type;
		}
		i++;
	}

	return 0;
}

int charge_standard_param_type(int standard_type, int param_type) {
	int diff_type = 0;
	switch (standard_type) {
		case CONST_STRING_TYPE:
			if (param_type == CONST_STRING_TYPE || param_type == POINTER_TYPE ||
					param_type == UNDEFINED_TYPE) {
				diff_type = 0;
			}
			else {
				diff_type = 1;
			}
			break;
		case INT_TYPE:
			if (param_type == INT_TYPE || param_type == INTEGER_TYPE ||
					param_type == UNDEFINED_TYPE) {
				diff_type = 0;
			}
			else {
				diff_type = 1;
			}
			break;
		case BOOL_TYPE:
			if (param_type == INT_TYPE || param_type == INTEGER_TYPE ||
					param_type == BOOL_TYPE || param_type == UNDEFINED_TYPE) {
				diff_type = 0;
			}
			else {
				diff_type = 1;
			}
			break;
		case POINTER_TYPE:
			if (param_type == POINTER_TYPE || param_type == CONST_STRING_TYPE
					|| param_type == UNDEFINED_TYPE) {
				diff_type = 0;
			}
			else {
				diff_type = 1;
			}
			break;
		case OBJECT_TYPE:
			diff_type = 0;
			break;
	}

	return diff_type;
}

static int bad_value_info(const char* name) {
	fprintf(stderr, "error: bad value for parameter '%s'\n", name);

	/* TODO: handle  the error*/
	return 2;
}

static int duplicate_assign_info(const char* name) {
	fprintf(stderr, "duplicate assignment to parameter '%s'\n", name);

	/* TODO: handle  the error*/
	return 2;
}

static int mismatch_type(const char* name) {
	fprintf(stderr, "error: illegal comparison; mismatching types to parameter '%s'", name);

	/* TODO: handle  the error*/
	return 2;
}

static int special_param_value(const char* param_name, parameter_attr_t* attr, char* special_value[]) {
	if (attr->spec == NULL) {
		return 1;
	}

	if ((attr->spec->is_auto) || (attr->spec->is_default)) {
		return duplicate_assign_info(param_name);
	}

	if (attr->spec->type == CONST_STRING_TYPE) {
		int i = 0;
		while (special_value[i] != NULL) {
			if (strcmp(attr->spec->str, special_value[i]) == 0) {
				return 1;
			}
			i++;
		}

		return bad_value_info(param_name);
	}
	else {
		return mismatch_type(param_name);
	}

	return 0;
}

static int obj_special_param(const char* name, parameter_attr_t* attr, special_param_t* special_param) {
	assert(name != NULL);

	int i = 0;
	while (special_param[i].name != NULL) {
		if (strcmp(name, special_param[i].name) == 0) {
			return special_param_value(name, attr, special_param[i].value);
		}
		i++;
	}

	return 0;
}

void init_allocate_type_arr() {
	/* we need to init it only once,
	 * take this swich to make sure
	 * it is inited or not */
	static int inited = 0;
	if (inited) {
		return;
	}
	inited++;
	/* allocate_type value can be the types
	 * bool double sting int int1..32 uint, uint1..32*/
	const char* int_value = "\"int";
	const char* uint_value = "\"uint";
	const char* marks = "\"";

	/* we parse the type is "uint4" "uint32" */
	int marks_len = strlen(marks);
	int int_len = strlen(int_value) + marks_len + 3;
	int uint_len = strlen(uint_value) + marks_len + 3;

	int i = 0;

	allocate_type_value[3] = "\"int\"";
	/* bool, double, string: 3
	 * int : 1 */
	i = 4;
	for (; i < ALLOCATE_VALUE_NUM - 34; i++) {
		allocate_type_value[i] = (char*)gdml_zmalloc(int_len);
		sprintf(allocate_type_value[i], "%s%d%s", int_value, i - 3, marks);
	}

	allocate_type_value[i] = "\"uint\"";
	/* bool, double, string: 3 */
	i = i + 1;
	for (; i < ALLOCATE_VALUE_NUM - 1; i++) {
		allocate_type_value[i] = (char*)gdml_zmalloc(uint_len);
		sprintf(allocate_type_value[i], "%s%d%s", uint_value, i - 36, marks);
	}

	allocate_type_value[ALLOCATE_VALUE_NUM] = NULL;

	/* this is only for debug */
	/*
	i = 0;
	while (allocate_type_value[i] != NULL) {
		printf("allocate_type_value: %s\n", allocate_type_value[i]);
		i++;
	}
	*/
}

static int special_standard_param(int table_type, const char* name, parameter_attr_t* attr) {
	assert(name != NULL);
	int is_special = 0;
	/* the attribute parameter allocate_type has many values
	 * and most of the value are int1..32, uint1..32*/
	init_allocate_type_arr();
	switch (table_type) {
		case DEVICE_TYPE:
			is_special = obj_special_param(name, attr, device_special);
			break;
		case BANK_TYPE:
			is_special = obj_special_param(name, attr, bank_special);
			break;
		case REGISTER_TYPE:
			is_special = obj_special_param(name, attr, register_special);
			break;
		case CONNECT_TYPE:
			is_special = obj_special_param(name, attr, connect_special);
			break;
		case EVENT_TYPE:
			is_special = obj_special_param(name, attr, event_special);
			break;
		case ATTRIBUTE_TYPE:
			is_special = obj_special_param(name, attr, attribute_special);
			break;
		default:
			is_special = 0;
	}

	return is_special;

}

int charge_standard_parameter(symtab_t table, parameter_attr_t* attr) {
	int obj_type = 0;
	int param_type = 0;
	const char* name = attr->name;

	/* when object is array, three paramers(i, index, indexvar) are insert the table
	 * when the object is created, and after that we can declare them, but
	 * fobiden assigning value to them */
	if (array_paramer(name, table)) {
		if ((attr->spec) != NULL) {
			fprintf(stderr, "error: duplicate assignment to parameter '%s'\n", name);
			/* TODO: handle the error */
			return 1;
		}
		return 1;
	}

	/* some standard parameters have special value
	 * such as: parameter byte_order can noly have
	 * two value: little-endian and bit-endian */
	int is_special = special_standard_param(table->type, name, attr);
	if (is_special == 1) {
		symbol_insert(table, name, PARAMETER_TYPE, attr);
		return 1;
	}
	else if (is_special == 2) {
		return 1;
	}


	standard_param_t* obj_standard_param = NULL;

	/* the standard parameter is contained by object itself
	 * and we insert the auto, assigned, default value parameter
	 * into table when the object is created
	 * the auto and assigned parameter can not be re-assigned
	 * but parameter with default value can be assigned again
	 */
	obj_standard_param = get_standard_param(table->type);
	param_type = standard_param_type(name, obj_standard_param);
	if ((param_type == AUTO_VALUE) || (param_type == ASSIGNED_VALUE)) {
		if (attr->spec != NULL) {
			fprintf(stderr, "error: duplicate assignment to parameter '%s'\n", name);
			/* FIXME: only for debug, delete the exit */
			exit(-1);
			/* TODO: handle the error */
			return 1;
		}
	}
	else if ((param_type == DEFAULT_VALUE) || (param_type == UNDEFINED_VALUE)) {
		symbol_t symbol = symbol_find(table, name, PARAMETER_TYPE);
		obj_type = get_standard_param_type(name, obj_standard_param);
		if (charge_standard_param_type(obj_type, attr->spec->type) == 0) {
			/* the standard parameter with default value in one object is
			 * inserted when object is created, but it can be assigned again */
			if (param_type == DEFAULT_VALUE) {
				symbol->attr = attr;
			}
			else {
				/* the standard parameter that is undefined, and they are not
				 * inserted when the object create, but it has their type,
				 * so should charge the type*/
				symbol_insert(table, name, PARAMETER_TYPE, attr);
			}
			return 1;
		}
		else {
			fprintf(stderr, "error: wrong type assign\n");
			/* FIXME: only for debug, delete the exit */
			exit(-1);
			/* TODO: handle the error */
			return 1;
		}
	}

	return 0;
}

void insert_auto_defaut_param(symtab_t table) {
	assert(table != NULL);

	standard_param_t* obj_standard_param = NULL;

	/* diffrent object has diffrent standard parameters */
	obj_standard_param = get_standard_param(table->type);

	/* an object contains many standard parameters,
	 * when the parameter is auto, assigned, or has
	 * default value, insert them automatic
	 */
	int i = 0;
	parameter_attr_t* param_attr = NULL;
	while (obj_standard_param[i].name != NULL) {
		if(obj_standard_param[i].value_type != UNDEFINED_VALUE) {
			param_attr = (parameter_attr_t*)gdml_zmalloc(sizeof(parameter_attr_t));
			param_attr->name = strdup(obj_standard_param[i].name);
			symbol_insert(table, obj_standard_param[i].name, PARAMETER_TYPE, param_attr);
		}
		i++;
	}

	return;
}
