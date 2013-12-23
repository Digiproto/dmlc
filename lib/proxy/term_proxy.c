/*
 * Copyright (C)
 * 2011 - Michael.Kang blackfin.kang@gmail.com
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

#include <simics/dev_iface.h>
#include <simics/base_types.h>
#include <simics/core/object_model.h>
#include <simics/core/object_class.h>
#include <simics/core/object_iface.h>
#include <simics/core/object_resource.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <signal.h>

#include "term_proxy.h"
#include "thread_tmp.h"

#ifndef SKYEYE_BIN
const char* default_bin_dir = "/opt/skyeye/bin/";
#else
char* default_bin_dir = SKYEYE_BIN;
#endif

#ifndef __MINGW32__
const char* uart_prog = "uart_instance";
#else
const char* uart_prog = "uart_instance.exe";
#define	EADDRINUSE	WSAEADDRINUSE
char* ReplaceAllChar(char* src, char oldChar, char newChar){
	char * head=src;
	while(*src!='\0'){
		if(*src==oldChar) *src=newChar;
		src++;
	}
	return head;
}
#endif

#if 0
static exception_t term_proxy_raise(conf_object_t* object, int line_no)
{
	/* Get vic interrupt interface */
	general_signal_intf* vic_signal = (general_signal_intf*)SKY_get_interface(object, GENERAL_SIGNAL_INTF_NAME);
	vic_signal->raise_signal(vic_signal->conf_obj, line_no);

	return 0;
}
#endif

/*
static exception_t term_proxy_down(conf_object_t* object, int line_no)
{
	return 0;
}
*/

static exception_t __attribute__((unused)) term_proxy_read(conf_object_t *obj, void* buf, size_t count)
{
	term_proxy_t *dev = (term_proxy_t*) obj;
	rec_t* receive = dev->receive;
	int i = 0;
	char* rec_buf = buf;
	if (dev->attached && receive->mutex == 1) {
		/* move the charaters in buffer */
		while(i < count && (receive->rec_tail < receive->rec_count)){
			if(receive->rec_head >= receive->rec_tail){
				/* No data */
				receive->rec_head = 0;
				receive->rec_tail = 0;
				return Not_found_exp;
			}else{
				rec_buf[i++] = receive->rec_buf[receive->rec_head++];
				if(receive->rec_head == receive->rec_tail){
					/* data have been read*/
					receive->rec_head = 0;
					receive->rec_tail = 0;
				}
			}
		}
		DEBUG_UART("rec_buf %s\n", rec_buf);

		receive->mutex = 0;
	}else{
		return Not_found_exp;
	}

	return No_exp;
}

static exception_t term_proxy_write(conf_object_t *obj, void* buf, size_t count)
{
	int ret = -1;
	term_proxy_t *dev = (term_proxy_t*) obj;
	if(dev->attached){
		ret = term_write(dev->socket, buf, count);
		if(ret < 0)
			return Invarg_exp;
	}
	return No_exp;
}

static int serial_write(conf_object_t *obj, int value)
{
	return term_proxy_write(obj, &value, 1);
}

#ifdef __MINGW32__
static DWORD create_term(term_proxy_t* dev_uart, int port){
#else
static pid_t create_term(term_proxy_t* dev_uart, int port){
#endif
	pid_t pid;

	char port_str[32];
	sprintf(port_str, "%d", port);

#ifdef __MINGW32__
	default_bin_dir = getenv("SKYEYEBIN");
#endif

	// link a xterm path
	int len = strlen(default_bin_dir) + strlen(uart_prog) + 1;
	char *uart_instance_prog = MM_ZALLOC(sizeof(char) * len);
	sprintf(uart_instance_prog, "%s%s", default_bin_dir, uart_prog);

#ifndef __MINGW32__
	switch (pid = fork())
	{
		case -1:
		{
			perror("The fork failed!");
			break;
		}
		case 0:
		{
#ifdef DBG_XTERM
			execlp("xterm","xterm", "-hold", "-e", uart_instance_prog, dev_uart->obj_name, port_str, (char *)NULL);
#else
			execlp("xterm","xterm", "-e", uart_instance_prog, dev_uart->obj_name, port_str, (char *)NULL);
#endif
			perror("exec xterm");
			fprintf(stderr, "SKYEYE Error: We need xterm to run the console of uart. "
					"Please check if you installed it correctly.\n");
			_exit (-1);
		}
		default:
			break;
	}
	return pid;
#else
	char cmdline[2048] = "start ";
	ReplaceAllChar(uart_instance_prog, '/', '\\');
	strcat(cmdline, uart_instance_prog);
	strcat(cmdline, " ");
	strcat(cmdline, dev_uart->obj_name);
	strcat(cmdline, " ");
	strcat(cmdline, port_str);
	if (system(cmdline) != 0) {
		SKYEYE_ERR("ERROR: system failed!");
		return -1;
	}
	return 0;
#endif // MINGW32
}

static int term_proxy_receive(term_proxy_t* dev_uart)
{
	int res;
	rec_t* receive = dev_uart->receive;

#ifndef __MINGW32__
	struct pollfd fds;
#define POLL_TIMEOUT -1         /* wait forever ? FIXME ? */
	fds.fd = dev_uart->socket;
	fds.events = POLLIN|POLLPRI;
#if HOST_OS_SOLARIS9            /* { */
	fds.events |= POLLRDNORM|POLLRDBAND;
#endif                          /* } */
	fds.revents = 0;
	res = poll(&fds, 1, POLL_TIMEOUT);
#else                           //__MINGW32__
	fd_set fdread;
	FD_ZERO(&fdread);
	FD_SET(dev_uart->socket, &fdread);
	select(0, &fdread, NULL, NULL, NULL);
#endif                          //__MINGW32__

#ifndef __MINGW32__
	if (fds.revents & POLLIN) {
#else
	if (FD_ISSET(dev_uart->socket, &fdread)) {
#endif
		if(receive->rec_tail >= receive->rec_count) {
			fprintf(stderr, "Overflow for uart link.\n");
		} else {
			res = term_read(dev_uart->socket,
					receive->rec_buf + receive->rec_tail,
					receive->rec_count - receive->rec_tail);
		}
		if (res == 0) {
			/* a read of 0 bytes is an EOF */
			dev_uart->attached = 0;
#ifndef __MINGW32__
			close(dev_uart->socket);
#else
			closesocket(dev_uart->socket);
			WSACleanup();
#endif
		} else if (res < 0) {
			/* FIXME!!! I think we need a mechanism that
			 * notice outside terminal have disconnected
			 */
			perror("read");
			dev_uart->attached = 0;
		} else {
			/* expand receive buf */
			receive->rec_tail += res;
			receive->mutex = 1;
			DEBUG_UART("recieve %s\n", receive->rec_buf);
		}
	}
	return 0;
}

static void* create_uart_console(void* obj) {
	term_proxy_t* dev_uart = (term_proxy_t*) obj;
	struct hostent * hp;
	int on, length;
	struct sockaddr_in server, from;
	char * froms;

#ifndef __MINGW32__
	int term_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (term_socket < 0)
		SKYEYE_ERR("opening stream socket");

	/* enable the reuse of this socket if this process dies */
	if (setsockopt(term_socket, SOL_SOCKET, SO_REUSEADDR, (uint8_t*)&on, sizeof(on))<0)
		SKYEYE_ERR("turning on REUSEADDR");
#else
	int nsize;
	int err;
	struct timeval tv;
	WSADATA wsaData;
	SOCKET term_socket = INVALID_SOCKET;
	BOOL bOptVal = FALSE;
	int bOptLen = sizeof(BOOL);

	/*
	 * initiates use of the Winsock DLL by a process
	 * shenoubang modified it 2012-12-4
	 */
	err = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		printf("WSAStartup failed with error: %d\n", err);
		return (void*) 1;
	}
	/* creates a socket that is bound to a specific transport service provider */
	term_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (term_socket == INVALID_SOCKET) {
		SKYEYE_ERR("socket function failed with error: %u\n", WSAGetLastError());
		WSACleanup();
		return (void*) 1;
	}
	bOptVal = TRUE;
	err = setsockopt(term_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &bOptVal, bOptLen);
	if (err == SOCKET_ERROR) {
		SKYEYE_ERR("setsockopt for RESUEADDR failed with error: %u\n", WSAGetLastError());
		return (void*) 1;
	}
#endif
retry:
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(0); /* bind to an OS selected local port */

#ifndef __MINGW32__
	if (bind(term_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
#else
	if (bind(term_socket, (SOCKADDR *)&server, sizeof(server)) < 0) {
#endif
		switch (errno) {
		case EAGAIN:
			goto retry;
		case EADDRINUSE:
			SKYEYE_ERR("Port is already in use\n");
		default:
			SKYEYE_ERR("binding tcp stream socket");
		}
	}

	length = sizeof(server);
	if (getsockname(term_socket, (struct sockaddr *) &server, (socklen_t*) &length) == -1)
		SKYEYE_ERR("getting socket name");

	listen(term_socket, 1);
	/* Create the client xterm */
	dev_uart->uart_console_pid = create_term(dev_uart, ntohs(server.sin_port));
	/* main loop */
	do {
		if (!dev_uart->attached) {
			printf("Waiting for connection to %s: %d\n", dev_uart->obj_name, ntohs(server.sin_port));
			length = sizeof(from);
			/* save the uart socket */
			dev_uart->socket = accept(term_socket, (struct sockaddr *)&from, (socklen_t*)&length);

			hp = gethostbyaddr((char *)&from.sin_addr, 4, AF_INET);
			if (hp == (struct hostent *)0) {
				froms = inet_ntoa(from.sin_addr);
				fprintf(stderr,"cant resolve hostname for %s\n", froms);
			} else {
				froms = hp->h_name;
			}
			dev_uart->attached = 1;
		} else {
			/* begin receive data. */
			term_proxy_receive(dev_uart);
		} //if (fds.revents & POLLIN)
	} while (1);

	return (void*) 0;
}

/*
 * obj: the object of the uart_leon2 class
 * obj2: the object which support the interface
 * port: The interface name
 * index: unused
 */
static int serail_device_set(conf_object_t *obj, conf_object_t *obj2, const char *port, int index)
{
	term_proxy_t* dev = (term_proxy_t*) obj;

	if(dev->serial_device.obj == obj2 && dev->serial_device.port == port){
		return No_exp;
	}
	dev->serial_device.obj = obj2;
	if(port)
		dev->serial_device.port = port;
	else
		dev->serial_device.port = NULL;
	if(dev->serial_device.port)
		dev->serial_device.serial_dev = (serial_device_interface_t*)SIM_get_port_interface(obj2, "serial_device", port);
	else
		dev->serial_device.serial_dev = (serial_device_interface_t*)SIM_get_interface(obj2, "serial_device");

	return No_exp;
}

static conf_object_t* term_proxy_new_instance(const char *objname) {
	term_proxy_t *proxy = (term_proxy_t*) MM_ZALLOC(sizeof(*proxy));
	SIM_object_register(&proxy->obj, objname);
	rec_t* receive = MM_ZALLOC(sizeof(rec_t));
	proxy->obj_name = strdup(objname);

	/* initial recevie */
	proxy->receive = receive;
	receive->rec_buf = MM_ZALLOC(MAX_REC_NUM);
	receive->rec_head = 0;
	receive->rec_tail = 0;
	receive->mutex = 0;
	receive->rec_count = MAX_REC_NUM;

	proxy->attached = 0;
	proxy->socket = -1;
	proxy->mod = 3;

	thread_tmp_t *thread = thread_tmp_init(0, ThreadTmpNoTimer);
	thread_tmp_create(thread, create_uart_console, (void*)(proxy));
	//pthread_t pthread_id;
	//create_thread(create_uart_console, (void*)(&proxy->obj), &pthread_id);

	return &proxy->obj;
}

/*
exception_t free_uart_term(conf_object_t* dev){
	uart_term_device* dev_uart = dev->obj;
#ifndef __MINGW32__
	kill(dev_uart->uart_console_pid, SIGTERM);
#else
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dev_uart->uart_console_pid);
	if(hProcess == NULL)
		return Unknown_exp;
	TerminateProcess(hProcess, 0);
		
#endif
	skyeye_free(dev_uart->receive);
	skyeye_free(dev_uart->obj);
	skyeye_free(dev_uart);

	return No_exp;
}
*/

static const struct ConnectDescription term_proxy_connects[] = {
	[0] = (struct ConnectDescription) {
			.name = "serial_device",
			.set = serail_device_set,
			.get = NULL,
	},
	[1] = {}
};

static const serial_device_interface_t term_proxy_iface = {
	.write = serial_write,
	.receive_ready = NULL
};

static const struct InterfaceDescription term_proxy_ifaces[] =  {
	[0] = (struct InterfaceDescription) {
			.name = "serial_device",
			.iface = &term_proxy_iface,
	},
	[1] = {}
};

static const struct AttributeDescription term_proxy_attributes[] = {
	[0] = {}
};

static const class_resource_t term_proxy_resource = {
	.ifaces = term_proxy_ifaces,
	.connects = term_proxy_connects,
	.attributes = term_proxy_attributes,
};

static const class_data_t term_proxy_data = {
	.new_instance = term_proxy_new_instance,
	.resources = &term_proxy_resource,
};

void init_local(void) {
	static conf_class_t conf_class = {
		.cls_name = "term_proxy",
		.cls_data = &term_proxy_data,
	};
	SIM_register_conf_class(conf_class.cls_name, &conf_class);
}
