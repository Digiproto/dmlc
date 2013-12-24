#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

#ifdef __MINGW32__
#include <windows.h>
#define sleep(ms) Sleep(1000*(ms))
#endif

#include "schedule_tmp.h"

#define SCHED_DEBUG

#define zalloc(size) memset(malloc(size), 0, size)
#define timecal(tc) (tc).tv_sec * 1000000 + (tc).tv_usec

struct sched_group;
struct schedule {
	struct schedule    *next;
	struct sched_group *belong;
	/* time */
	int                 timeout;
	int                 overtime;
	/* callback info */
	schedule_callback_t cb;
	void*               cb_args;
	void*               ret;
	/* mode */
	schedule_mode_t     mode;
	schedule_status_t   status;
};

struct sched_group {
	thread_tmp_t   *thread;
	pthread_mutex_t mutex;
	long long       oldtime; // ms
	struct schedule list;
};

static void *schedule_main(void *args)
{
	sched_group_t *group = (sched_group_t*) args;
	schedule_t *sched;
	struct timeval newtime;
	int spend;
	long long new;

	pthread_mutex_lock(&group->mutex);

	gettimeofday(&newtime, NULL);
	new = timecal(newtime);
	if(group->oldtime) {
		spend = new - group->oldtime;
		//printf("%d\n", spend);
	}
	group->oldtime = new;

	for(sched = group->list.next; sched; sched = sched->next) {
		sched->overtime -= spend;
	}

	for(sched = &group->list; sched && sched->next; sched = sched->next) {
		schedule_t *tmp = sched->next;
		if(tmp->overtime <= 0) {
			if(tmp->cb)
				tmp->ret = tmp->cb(tmp->cb_args);
			if(tmp->mode == SchedTmpLoop) {
				tmp->overtime = tmp->timeout;
			}else{
				// remove schedule from group if once schedule
				sched->next = tmp->next;
				tmp->status = SchedTmpStop;
			}
		}
	}
	pthread_mutex_unlock(&group->mutex);

	return NULL;
}

sched_group_t* schedule_tmp_create()
{
	sched_group_t *group = zalloc(sizeof(*group));

	//struct timeval newtime;
	//gettimeofday(&newtime, NULL);
	//group->oldtime = timecal(newtime);

	pthread_mutex_init(&group->mutex, NULL);

	group->thread = thread_tmp_init(10, ThreadTmpLoop);
	thread_tmp_create(group->thread, schedule_main, (void*) group);

	return group;
}

static inline void __schedule_tmp_update(sched_group_t *group, schedule_t *sched,
		schedule_callback_t cb, void* cb_args,
		schedule_mode_t mode, int timeout)
{
	schedule_t *node;

	sched->belong = group;
	sched->cb = cb;
	sched->cb_args = cb_args;
	sched->timeout = timeout;
	if(sched->overtime > sched->timeout)
		sched->overtime = sched->timeout;
	sched->mode = mode;
	sched->status = SchedTmpRun;

	for(node = &group->list; node->next; node = node->next) {
		if(node->next == sched)
			break;
	}
	if(!node->next) {
		// schedule is not in the group
		node->next = sched;
		sched->next = NULL;
	}
}

void schedule_tmp_update(schedule_t *sched,
		schedule_callback_t cb, void* cb_args,
		schedule_mode_t mode, int timeout)
{
	assert(sched);
	sched_group_t *group = sched->belong;
	pthread_mutex_lock(&group->mutex);
	__schedule_tmp_update(group, sched, cb, cb_args, mode, timeout);
	pthread_mutex_unlock(&group->mutex);
}

schedule_t* schedule_tmp_insert(sched_group_t *group, schedule_callback_t cb,
		void* cb_args, schedule_mode_t mode, int timeout)
{
	assert(group);

	schedule_t *sched;
	sched = zalloc(sizeof(*sched));

	pthread_mutex_lock(&group->mutex);
	__schedule_tmp_update(group, sched, cb, cb_args, mode, timeout);
	pthread_mutex_unlock(&group->mutex);

	return sched;
}

int schedule_tmp_delete(schedule_t *sched)
{
	if(!sched)
		return 0;

	sched_group_t *group = sched->belong;
	schedule_t *node;

	pthread_mutex_lock(&group->mutex);
	for(node = &group->list; node && node->next; node = node->next) {
		if(node->next == sched) {
			node->next = sched->next;
			sched->next = NULL;
			sched->status = SchedTmpStop;
			break;
		}
	}
	pthread_mutex_unlock(&group->mutex);

	return 0;
}

void* schedule_tmp_get_return(schedule_t *sched)
{
	assert(sched);
	pthread_mutex_lock(&sched->belong->mutex);
	void *ret = sched->ret;
	pthread_mutex_unlock(&sched->belong->mutex);
	return ret;
}

schedule_status_t schedule_tmp_status_sched(schedule_t *sched)
{
	if(!sched)
		return SchedTmpNull;
	pthread_mutex_lock(&sched->belong->mutex);
	schedule_status_t status = sched->status;
	pthread_mutex_unlock(&sched->belong->mutex);
	return status;
}

void schedule_tmp_destroy_sched(schedule_t *sched)
{
	schedule_tmp_delete(sched);
	free(sched);
}

void schedule_tmp_destroy_group(sched_group_t *group)
{
	schedule_t *node, *tmp;

	pthread_mutex_lock(&group->mutex);
	node = &group->list;
	while(node->next) {
		tmp = node->next;
		node->next = tmp->next;
		schedule_tmp_destroy_sched(tmp);
	}
	thread_tmp_stop(group->thread);
	thread_tmp_destroy(group->thread);
	pthread_mutex_unlock(&group->mutex);
	pthread_mutex_destroy(&group->mutex);
	free(group);
}

#ifdef SCHED_DEBUG

static void* callback(void* args)
{
	char *str = (char*) args;
	static int a = 0;
	printf("[%d] %s\n", a++, str);
	return NULL;
}

int main(int argc, const char *argv[])
{
	char *args = "hello world";
	sched_group_t *group = schedule_tmp_create();
	schedule_t *sched = schedule_tmp_insert(group, callback, (void*) args, SchedTmpLoop, 100000);

	int i, j;
	for(i = j = 0; 1; i++, j++) {
		sleep(1);
		if(i == 10) {
			schedule_tmp_delete(sched);
			/*
		}else if(i == 13) {
			schedule_tmp_update(sched, callback, (void*) args, 300, SchedTmpOnce);
			i = 0;
			*/
		}
		//if(j > 15)
		//	break;
		printf("[%d] main loop\n", j);
	}
	schedule_tmp_delete(sched);
	schedule_tmp_destroy_sched(sched);
	schedule_tmp_destroy_group(group);
	return 0;
}

#endif
