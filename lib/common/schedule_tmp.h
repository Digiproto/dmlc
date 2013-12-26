#ifndef __SCHEDULE_TMP_H__
#define __SCHEDULE_TMP_H__

#include <stdint.h>
#include "thread_tmp.h"

typedef enum schedule_mode {
	SchedTmpOnce,
	SchedTmpLoop,
	SchedTmpNull,
}schedule_mode_t;

typedef enum schedule_status {
	SchedTmpRun,
	SchedTmpStop,
}schedule_status_t;

typedef void* (*schedule_callback_t) (void*);
typedef struct schedule schedule_t;
typedef struct sched_group sched_group_t;

sched_group_t* schedule_tmp_create();
schedule_t* schedule_tmp_insert(sched_group_t *group, schedule_callback_t cb,
		void* cb_args, schedule_mode_t mode, int timeout);
void schedule_tmp_update(schedule_t *sched, schedule_callback_t cb,
		void* cb_args, schedule_mode_t mode, int timeout);
int schedule_tmp_delete(schedule_t *sched);
void* schedule_tmp_get_return(schedule_t *sched);
int schedule_tmp_sched_count(schedule_t *sched);
schedule_status_t schedule_tmp_status_sched(schedule_t *sched);
void schedule_tmp_destroy_sched(schedule_t *sched);
void schedule_tmp_destroy_group(sched_group_t *group);

#endif
