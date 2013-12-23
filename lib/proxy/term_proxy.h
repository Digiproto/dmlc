#ifndef __SKYEYE_UART_TERM_H__
#define __SKYEYE_UART_TERM_H__

#ifndef __MINGW32__
/* linux head file */
#include <poll.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
typedef char BOOL;
#else
#include <windows.h>
#include <direct.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

typedef struct receive_t{
	uint8_t* rec_buf;
	int      rec_head;
	int      rec_tail;
	int      rec_count;
	int      mutex;
}rec_t;

typedef struct term_proxy {
	conf_object_t  obj;
	rec_t*         receive;
	const char*    obj_name;
	int            mod;
#ifndef __MINGW32__
	int            socket;
	pid_t          uart_console_pid;
#else
	SOCKET         socket;
	DWORD          uart_console_pid;
#endif
	BOOL           attached;
	struct {
		conf_object_t *obj;
		const char *port;
		const serial_device_interface_t* serial_dev;
	}serial_device;
}term_proxy_t;

#define MAX_REC_NUM 1024
//#define DBG_UART
#ifdef DBG_UART
#define DEBUG_UART(fmt, ...) \
	do{ \
		fprintf(stderr, "%s: " fmt, __func__, ## __VA_ARGS__); \
	}while(0)
#else
#define DEBUG_UART(fmt, ...) \
	do{ \
	}while(0)
#endif
#define SKYEYE_ERR(fmt, ...) \
	do{ \
		fprintf(stderr, "%s: " fmt, __func__, ## __VA_ARGS__); \
	}while(0)

#ifndef __MINGW32__
#define term_write(fd, buf, len) \
	write((fd), (buf), (len))
#define term_read(fd, buf, len) \
	read((fd), (buf), (len))
#else
#define term_write(fd, buf, len) \
	send((fd), (buf), (len), 0)
#define term_read(fd, buf, len) \
	recv((fd), (buf), (len), 0)
#endif

#endif
