/* 
 * © Copyright 2013 eJim Lee. All Rights Reserved.
 * File:    dmlc.h
 * Email:   lyjforjob@gmail.com
 * Date:    2013-07-19
 */

#ifndef __DMLC_H__
#define __DMLC_H__

#include "tree.h"

extern tree_t* get_ast(const char* filename);
extern const char *import_file_list[];

#endif /* __DMLC_H__ */
