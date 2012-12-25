/* Copyright (C) 
* 2012 - Michael.Kang blackfin.kang@gmail.com
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/
/**
* @file code_gen.c
* @brief The c source code generation
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2012-12-21
*/

#include <stdio.h>
#include <time.h>
#include "types.h"
#include "ast.h"
#include "symbol.h"
/**
* @brief generate the source file module_id.c
*
* @param file the file written to
*/
void generate_module_id_source(node_t* root, FILE* file){
	/* get the device name */
	symbol_t* symbol = symbol_find("DEVICE", DEVICE_TYPE);
	if(symbol == NULL){
		printf("can not find device\n");
		exit(-1);
	}
		
	device_attr_t* attr = (device_attr_t*)symbol->attr.device;	
	char* device_name = attr->name;
	time_t timep;
	time (&timep);

	printf("In %s\n", __FUNCTION__);
	fprintf(file, "\
/*						\
\n* module_id.c - automatically generated, do not edit\
\n*/						\
\n						\
\n#include <simics/global.h>			\
\n#include <simics/build-id.h>			\
\n#include <simics/core/types.h>		\
\n#include <simics/utils.h>			\
\n						\
\n#define EXTRA \"                                           \"\
\n						\
\nextern const char _module_capabilities_[];	\
\nconst char _module_capabilities_[] =		\
\n\t\"VER:\" SYMBOL_TO_STRING(SIM_VERSION_COMPAT) \";\"\
\n\t\"BLD:\" SYMBOL_TO_STRING(SIM_VERSION) \";\"\
\n\t\"BUILDDATE:\" \"1287200153\" \";\"\
\n\t\"MOD:\" \"%s\" \";\"\
\n\t\"CLS:%s\" \";\"\
\n\t\"HOSTTYPE:\" \"x86-linux\" \";\"\
\n\t\"THREADSAFE;\"				\
\n\tEXTRA \";\";				\
\nconst char _module_date[] = \"%s\";\
", device_name, device_name, asctime(gmtime(&timep)));

}

/**
* @brief Generate the device source file
*
* @param file
*/
void generate_device_source(node_t* root, FILE* file){
}

void generate_device_header(node_t* root, FILE* file){
}

void generate_device_struct_header(node_t* root, FILE* file){
}

void generate_device_protos_source(node_t* root, FILE* file){
}
typedef enum{
	MODULE_ID = 0,
	DEVICE_DML,
	DEVICE_HEADER,
	DEVICE_STRUCT_HEADER,
	DEVICE_PROTOS,
	GEN_FILE_NUM
}filename_list_t;
char* gen_filename[GEN_FILE_NUM];
typedef void(*gen_code_func_t)(node_t* root, FILE* output);
gen_code_func_t gen_code[GEN_FILE_NUM] = {
	generate_module_id_source,
	generate_device_source,
	generate_device_header,
	generate_device_struct_header,
	generate_device_protos_source
};

const char* module_id_filename = "module_id.c";

char* suffix_filename[GEN_FILE_NUM] = {
	"",
	"-dml.c",
	"-dml.h",
	"-dml-struct.h",
	"-dml-protos.c"
	};

char* concat_filename(char* device_name, char* suffix){
	char* filename = (char *)malloc(strlen(device_name) + strlen(suffix) + 1);
	strncpy(filename, device_name, strlen(device_name));
	filename[strlen(device_name)] = '\0';
	strcat(filename, suffix);
	return filename;
}

#define DEBUG_FILELIST 1
void complete_filename(char* device_name, char* path_name){
	gen_filename[MODULE_ID] = module_id_filename;
	int i = DEVICE_DML;
	for(; i < GEN_FILE_NUM; i++)
		gen_filename[i] = concat_filename(device_name, suffix_filename[i]);

	i = MODULE_ID;
	for(; i < GEN_FILE_NUM; i++){
		/* we need to free the previous memory space and set the full name */
		char* full_filename = concat_filename(path_name, gen_filename[i]);
		if(i != MODULE_ID)
			free(gen_filename[i]);
		gen_filename[i] = NULL;
		gen_filename[i] = full_filename;
	}
	#if DEBUG_FILELIST
	i = MODULE_ID;
	for(; i < GEN_FILE_NUM; i++)
		printf("filename[%d] = %s\n", i, gen_filename[i]);
	#endif
}
void generate_code(node_t* root, char* path_name){
	/* get the device name */
	symbol_t* symbol = symbol_find("DEVICE", DEVICE_TYPE);
	if(symbol == NULL){
		printf("can not find device\n");
		exit(-1);
	}
		
	device_attr_t* attr = (device_attr_t*)symbol->attr.device;	
	char* device_name = attr->name;

	/* complete the full filename generated */
	complete_filename(device_name, path_name);

	/* begin to generate the c source file */
	FILE* file = NULL;
	int i = 0;
	for(; i < GEN_FILE_NUM; i++){
		file = fopen(gen_filename[i], "w");
		if(file != NULL){
			gen_code[i](root, file);
			fclose(file);
		}
		else{
			fprintf(stderr, "can not open file %s\n", gen_filename[i]);
			exit(-1);
		}
	}
}
