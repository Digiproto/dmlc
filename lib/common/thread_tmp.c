#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "thread_tmp.h"

//#define THREAD_TMP_DEBUG

#define zalloc(n) memset(malloc(n), 0, n)
#define pwarn(fmt, ...) \
	do{ \
		fprintf(stderr, fmt, ##__VA_ARGS__); \
	}while(0)

struct thread_tmp {
	pthread_t             tid;
	pthread_mutex_t       mutex;
	thread_tmp_callback_t cb;
	void                 *cb_args;
	void                 *ret;
	/* timer mode */
	thread_tmp_mode       mode;
	unsigned int          ms;
	/* status */
	thread_tmp_status     status;
};

/**
 * @brief thread_main_timer : make the thread run with sleeping
 *
 * @param args : the struct with thread
 *
 * @return : nothing to return
 */
static void* thread_main_timer(void* args) {
	thread_tmp_t *thread = (thread_tmp_t*) args;
	thread_tmp_mode mode;
	thread_tmp_callback_t cb;
	void *cb_args, *ret;
	unsigned int ms;

	pthread_mutex_lock(&thread->mutex);
	thread->status = ThreadTmpRun;
	pthread_mutex_unlock(&thread->mutex);
	
	do{
		pthread_mutex_lock(&thread->mutex);
		cb = thread->cb;
		cb_args = thread->cb_args;
		//pthread_mutex_unlock(&thread->mutex);

		if(cb)
			ret = cb(cb_args);

		//pthread_mutex_lock(&thread->mutex);
		thread->ret = ret;
		mode = thread->mode;
		ms = thread->ms;
		pthread_mutex_unlock(&thread->mutex);

		/* if call pthread_cancel() */
		pthread_testcancel();

		if(mode == ThreadTmpLoop)
			usleep(ms);
		else
			break;
	}while(1);

	pthread_mutex_lock(&thread->mutex);
	thread->status = ThreadTmpStop;
	pthread_mutex_unlock(&thread->mutex);

	return NULL;
}

/**
 * @brief thread_main_no_timer : make thread run with no time sleep
 *
 * @param args : the struct of thread
 *
 * @return : nothing to return
 */
static void* thread_main_no_timer(void* args) {
	thread_tmp_t *thread = (thread_tmp_t*) args;
	thread_tmp_callback_t cb;
	void *cb_args, *ret;

	pthread_mutex_lock(&thread->mutex);
	cb = thread->cb;
	cb_args = thread->cb_args;
	pthread_mutex_unlock(&thread->mutex);

	ret = cb(cb_args);

	pthread_mutex_lock(&thread->mutex);
	thread->ret = ret;
	pthread_mutex_unlock(&thread->mutex);

	return NULL;
}

/**
 * @brief thread_tmp_init : initialize thread
 *
 * @param mode : the mode to set thread
 * @param ms : the time to set thread
 *
 * @return : thread struct
 */
thread_tmp_t* thread_tmp_init(thread_tmp_mode mode, unsigned int ms)
{
	thread_tmp_t* thread = zalloc(sizeof(*thread));
	pthread_mutex_init(&thread->mutex, NULL);

	int ret = thread_tmp_chmod_timer(thread, ms, mode);
	if(ret) {
		pthread_mutex_destroy(&thread->mutex);
		free(thread);
		return NULL;
	}
	return thread;
}

/**
 * @brief thread_tmp_create : create a new thread
 *
 * @param thread : the thread struct
 * @param cb : callback functin for thread
 * @param cb_args : arguments of callback function
 *
 * @return : 0 - create thread succussful
 *			-1 - create thread failed
 */
int thread_tmp_create(thread_tmp_t *thread, thread_tmp_callback_t cb, void *cb_args)
{
	int ret;

	if(!thread) {
		pwarn("thread_tmp_t have not been initial\n");
		return -1;
	}

	pthread_mutex_lock(&thread->mutex);
	thread->cb = cb;
	thread->cb_args = cb_args;
	if(thread->mode != ThreadTmpNoTimer)
		ret = pthread_create(&thread->tid, NULL, thread_main_timer, thread);
	else
		ret = pthread_create(&thread->tid, NULL, thread_main_no_timer, thread);
	pthread_mutex_unlock(&thread->mutex);

	if(ret) {
		errno = ret;
		perror("thread_tmp_create");
		pthread_mutex_destroy(&thread->mutex);
		free(thread);
		return ret;
	}
	return 0;
}

/**
 * @brief thread_tmp_stop : stop one thread
 *
 * @param thread : the thread to be stopped
 *
 * @return : 0 - success in stopping
 */
int thread_tmp_stop(thread_tmp_t *thread)
{
	pthread_mutex_lock(&thread->mutex);
	pthread_cancel(thread->tid);
	thread->status = ThreadTmpStop;
	pthread_mutex_unlock(&thread->mutex);
	return 0;
}

/**
 * @brief thread_tmp_destroy : destroy thread
 *
 * @param thread : the thread to be destroied
 *
 * @return : -1 - faield to destroy
 *			0 - success in destroing
 */
int thread_tmp_destroy(thread_tmp_t *thread)
{
	if(!thread) {
		pwarn("thread_tmp_t have not been initial\n");
		return -1;
	}

	pthread_mutex_destroy(&thread->mutex);
	free(thread);
	return 0;
}

/**
 * @brief thread_tmp_chmod_timer : change the mode of thread
 *
 * @param thread : the thread to be changed
 * @param mode : new mode of thread
 * @param ms : new timer of thread
 *
 * @return : -1 - failed to change mode
 *			0 - succuss to change mode
 */
int thread_tmp_chmod_timer(thread_tmp_t *thread, thread_tmp_mode mode, unsigned int ms)
{
	if(!thread) {
		pwarn("thread_tmp_t have not been initial\n");
		return -1;
	}
	pthread_mutex_lock(&thread->mutex);
	thread->ms = ms;
	thread->mode = mode;
	pthread_mutex_unlock(&thread->mutex);
	return 0;
}

/**
 * @brief thread_tmp_check_status : check the status of thread
 *
 * @param thread : the thread to be ckecked
 *
 * @return : the status of thread
 */
thread_tmp_status thread_tmp_check_status(thread_tmp_t *thread)
{
	thread_tmp_status s;
	pthread_mutex_lock(&thread->mutex);
	s = thread->status;
	pthread_mutex_unlock(&thread->mutex);
	return s;
}

// only for test
#ifdef THREAD_TMP_DEBUG
void* callback(void* args)
{
	static int a = 0;
	printf("hello world[%d]\n", a++);
	return NULL;
}

int main(int argc, const char *argv[])
{
	thread_tmp_t *thread = thread_tmp_init(1000000, ThreadTmpLoop);
	thread_tmp_create(thread, callback, NULL);

	long long i;
	for(i = 1; i >= 0; i++) {
		if((i & 0xfffffff) == 0) {
			thread_tmp_chmod_timer(thread, 1000, ThreadTmpOnce);
		}
	}
	return 0;
}
#endif
