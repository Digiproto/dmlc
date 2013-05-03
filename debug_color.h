
/*
 * debug_color.h:
 *
 * Copyright (C) 2013 Oubang Shen <shenoubang@gmail.com>
 * Skyeye Develop Group, for help please send mail to
 * <skyeye-developer@lists.gro.clinux.org>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __DEBUG_COLOR_H__
#define __DEBUG_COLOR_H__  1

typedef enum
{
	RED,
	LIGHT_RED,
	BLACK,
	DARK_GRAY,
	BLUE,
	LIGHT_BLUE,
	GREEN,
	LIGHT_GREEN,
	CYAN,
	LIGHT_CYAN,
	PURPLE,
	LIGHT_PURPLE,
	BROWN,
	YELLOW,
	LIGHT_GRAY,
	WHITE
} COLOR_TYPE;

#define _debug_in_red(fmt,...)      printf("\033[0;31m"fmt"\033[0m", ## __VA_ARGS__)
#define _debug_in_light_red(fmt,...)    printf("\033[1;31m"fmt"\033[0m", ## __VA_ARGS__)
#define _debug_in_black(fmt,...)    printf("\033[0;30m"fmt"\033[0m", ## __VA_ARGS__)
#define _debug_in_dark_gray(fmt,...)    printf("\033[1;30m"fmt"\033[0m", ## __VA_ARGS__)
#define _debug_in_blue(fmt,...)     printf("\033[0;34m"fmt"\033[0m", ## __VA_ARGS__)
#define _debug_in_light_blue(fmt,...)   printf("\033[1;34m"fmt"\033[0m", ## __VA_ARGS__)
#define _debug_in_green(fmt,...)    printf("\033[0;32m"fmt"\033[0m", ## __VA_ARGS__)
#define _debug_in_light_green(fmt,...)  printf("\033[1;32m"fmt"\033[0m", ## __VA_ARGS__)
#define _debug_in_cyan(fmt,...)     printf("\033[0;36m"fmt"\033[0m", ## __VA_ARGS__)
#define _debug_in_light_cyan(fmt,...)   printf("\033[1;36m"fmt"\033[0m", ## __VA_ARGS__)
#define _debug_in_purple(fmt,...)   printf("\033[0;35m"fmt"\033[0m", ## __VA_ARGS__)
#define _debug_in_light_purple(fmt,...) printf("\033[1;35m"fmt"\033[0m", ## __VA_ARGS__)
#define _debug_in_brown(fmt,...)    printf("\033[0;33m"fmt"\033[0m", ## __VA_ARGS__)
#define _debug_in_yellow(fmt,...)   printf("\033[1;33m"fmt"\033[0m", ## __VA_ARGS__)
#define _debug_in_light_gray(fmt,...)   printf("\033[0;37m"fmt"\033[0m", ## __VA_ARGS__)
#define _debug_in_white(fmt,...)    printf("\033[1;37m"fmt"\033[0m", ## __VA_ARGS__)

void debug_color (COLOR_TYPE type, char *format, ...);
int debug_find_tail (char *format, ...);
int debug_add_child (char *format, ...);
int debug_create_node (char *format, ...);
int debug_create_node_list (char *format, ...);

#define DEBUG_CODE_COLOR

#ifdef	DEBUG_CODE_COLOR
#define DEBUG_COLOR				debug_color
#define DEBUG_FIND_TAIL			debug_find_tail
#define DEBUG_ADD_CHILD			debug_add_child
#define DEBUG_CREATE_NODE		debug_create_node
#define DEBUG_CREATE_NODE_LIST	debug_create_node_list
#else
#define DEBUG_COLOR
#define DEBUG_FIND_TAIL
#define DEBUG_ADD_CHILD
#define DEBUG_CREATE_NODE
#define DEBUG_CREATE_NODE_LIST
#endif

#endif							/* __DEBUG_COLOR_H__ */
