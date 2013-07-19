/* 
 * © Copyright 2013 eJim Lee. All Rights Reserved.
 * File:    dmlc.h
 * Email:   lyjforjob@gmail.com
 * Date:    2013-07-19
 */

#ifndef __DMLC_H__
#define __DMLC_H__

#ifndef DIR_MAX_LEN
#define DIR_MAX_LEN 1024
#endif

#ifndef GDML_LIBRARY_DIR
#ifdef _WIN32
#define GDML_LIBRARY_DIR "..\\include\\gdml\\1.0\\"
#else
#define GDML_LIBRARY_DIR "../include/gdml/1.0/"
#endif
#endif

char *gdml_library_dir;

#endif /* __DMLC_H__ */
