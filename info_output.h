/* 
 * © Copyright 2013 eJim Lee. All Rights Reserved.
 * File:    info_output.h
 * Email:   lyjforjob@gmail.com
 * Date:    2013-08-22
 */

#ifndef __INFO_OUTPUT_H__
#define __INFO_OUTPUT_H__

#define PERROR(FMT, LOC, ...) \
	do{ \
		fprintf(stderr, \
				"[error] file: %s\n" \
				"        (line: %d, column: %d) to (line: %d, column: %d)\n" \
				"        reason: " FMT "\n", ((LOC).file->name != NULL ? (LOC).file->name : "NotKnown"), \
										(LOC).first_line, (LOC).first_column, \
										(LOC).last_line, (LOC).last_column, ## __VA_ARGS__); \
		exit(-1); \
	}while(0)

#define PWARN(FMT, LOC, ...) \
	do{ \
		fprintf(stderr, \
				"[warning] file: %s\n" \
				"          (line: %d, column: %d) to (line: %d, column: %d)\n" \
				"          reason: " FMT "\n", ((LOC).file->name != NULL ? (LOC).file->name : "NotKnown"), \
										(LOC).first_line, (LOC).first_column, \
										(LOC).last_line, (LOC).last_column, ## __VA_ARGS__); \
	}while(0)

#define warning(FMT, ...) \
	do{ \
		fprintf(stderr, \
				"[warning] " FMT "\n", ## __VA_ARGS__); \
	}while(0)

#define error(FMT, ...) \
	do{ \
		fprintf(stderr, \
				"[error] " FMT "\n", ## __VA_ARGS__); \
		exit(-1); \
	}while(0)

#endif
