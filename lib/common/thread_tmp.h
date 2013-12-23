#ifndef __THREAD_TMP_H__
#define __THREAD_TMP_H__

typedef void* (*thread_tmp_callback_t)(void*);
typedef struct thread_tmp thread_tmp_t;

typedef enum thread_tmp_mode {
	ThreadTmpOnce,
	ThreadTmpLoop,
	ThreadTmpNoTimer,
}thread_tmp_mode;

typedef enum thread_tmp_status {
	ThreadTmpRun,
	ThreadTmpStop,
}thread_tmp_status;

thread_tmp_t* thread_tmp_init(thread_tmp_mode mode, unsigned int ms);
int thread_tmp_create(thread_tmp_t *thread, thread_tmp_callback_t cb, void *arg);
int thread_tmp_stop(thread_tmp_t *thread);
int thread_tmp_destroy(thread_tmp_t *thread);
int thread_tmp_chmod_timer(thread_tmp_t *thread, thread_tmp_mode mode, unsigned int ms);
thread_tmp_status thread_tmp_check_status(thread_tmp_t *thread);

#endif
