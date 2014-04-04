
/*
 * debug_color.c:
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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "debug_color.h"

/*
 * The max length of string of user input
 */
#define MAX_LENGTH 1024


/**
 * @brief out_bounds: charge the input contents is out bounds of buf
 *
 * @param buf: the buffer to store input contents
 *
 * @return: 0
 */
static int out_bounds (char *buf)
{
	/*
	 * if the last charachter is not NULL,
	 * the input contents maybe out bounds
	 * */
	if ((buf[MAX_LENGTH - 1]) != '\0') {
		fprintf (stderr, "The color log buf is out of bounds!\n");
	}

	return 0;
}

/**
 * @brief debug_color : print bug information with color
 *
 * @param type : the type of color
 * @param format : the format of bug information
 * @param ... : other arguments of bug
 */
void debug_color (COLOR_TYPE type, char *format, ...)
{
	static char buf[MAX_LENGTH];
	memset (buf, '\0', MAX_LENGTH);
	va_list args;
	va_start (args, format);
	vsprintf (buf, format, args);
	out_bounds (buf);
	va_end (args);
	switch (type) {
	case BLACK:
		_debug_in_black ("%s", buf);
		break;
	case BLUE:
		_debug_in_blue ("%s", buf);
		break;
	case BROWN:
		_debug_in_brown ("%s", buf);
		break;
	case CYAN:
		_debug_in_cyan ("%s", buf);
		break;
	case DARK_GRAY:
		_debug_in_dark_gray ("%s", buf);
		break;
	case GREEN:
		_debug_in_green ("%s", buf);
		break;
	case LIGHT_BLUE:
		_debug_in_light_blue ("%s", buf);
		break;
	case LIGHT_CYAN:
		_debug_in_light_cyan ("%s", buf);
		break;
	case LIGHT_GREEN:
		_debug_in_light_green ("%s", buf);
		break;
	case LIGHT_PURPLE:
		_debug_in_light_purple ("%s", buf);
		break;
	case LIGHT_RED:
		_debug_in_light_red ("%s", buf);
		break;
	case PURPLE:
		_debug_in_purple ("%s", buf);
		break;
	case RED:
		_debug_in_red ("%s", buf);
		break;
	default:
		fprintf (stderr, "Not supported color type %d\n", type);
		break;
	}
}


void dump_src_code(const char *fname, int line) {
#define CMD_BUF 256
	char command[CMD_BUF];
	snprintf(command, CMD_BUF, "sed -ne '%dp' %s", line, fname);
	system(command);
#undef CMD_BUF
}

void pretty_indication(int space, int size, char indictor) {
	int i = 0;
	char fmt[128];   

	while(i < space) {
    	printf(" ");
        i++;
	}
    i = 0;
	while(i < size) {
		printf("%c", indictor);
		i++;
	}
	printf("\n");
}


/**
 * @brief debug_cyan : print buf information with cyan
 *
 * @param format :  the format of print bug information
 * @param ... : other arguments
 *
 * @return : 0 - return success
 */
int debug_cyan(char* format, ...) {
	char buf[MAX_LENGTH];
	memset (buf, '\0', MAX_LENGTH);
	va_list args;
	va_start (args, format);
	vsprintf (buf, format, args);
	out_bounds (buf);
	va_end (args);

	_debug_in_cyan ("%s", buf);

	return 0;
}

/**
 * @brief debug_purple : print buf information with purple
 *
 * @param format :  the format of print bug information
 * @param ... : other arguments
 *
 * @return : 0 - return success
 */
int debug_purple(char* format, ...) {
	char buf[MAX_LENGTH];
	memset (buf, '\0', MAX_LENGTH);
	va_list args;
	va_start (args, format);
	vsprintf (buf, format, args);
	out_bounds (buf);
	va_end (args);

	_debug_in_purple ("%s", buf);

	return 0;
}

/**
 * @brief debug_light_red : print buf information with light_red color
 *
 * @param format :  the format of print bug information
 * @param ... : other arguments
 *
 * @return : 0 - return success
 */
int debug_light_red(char* format, ...) {
	char buf[MAX_LENGTH];
	memset (buf, '\0', MAX_LENGTH);
	va_list args;
	va_start (args, format);
	vsprintf (buf, format, args);
	out_bounds (buf);
	va_end (args);

	_debug_in_light_red ("%s", buf);

	return 0;
}

/**
 * @brief debug_blue: print buf information with blue color
 *
 * @param format :  the format of print bug information
 * @param ... : other arguments
 *
 * @return : 0 - return success
 */
int debug_blue(char *format, ...)
{
	char buf[MAX_LENGTH];
	memset (buf, '\0', MAX_LENGTH);
	va_list args;
	va_start (args, format);
	vsprintf (buf, format, args);
	out_bounds (buf);
	va_end (args);

	_debug_in_blue ("%s", buf);

	return 0;
}

/**
 * @brief debug_green: print buf information with green color
 *
 * @param format :  the format of print bug information
 * @param ... : other arguments
 *
 * @return : 0 - return success
 */
int debug_green(char* format, ...)
{
	char buf[MAX_LENGTH];
	memset (buf, '\0', MAX_LENGTH);
	va_list args;
	va_start (args, format);
	vsprintf (buf, format, args);
	out_bounds (buf);
	va_end (args);

	_debug_in_green ("%s", buf);

	return 0;
}

/**
 * @brief debug_red: print buf information with red color
 *
 * @param format :  the format of print bug information
 * @param ... : other arguments
 *
 * @return : 0 - return success
 */
int debug_red(char *format, ...)
{
	char buf[MAX_LENGTH];
	memset (buf, '\0', MAX_LENGTH);
	va_list args;
	va_start (args, format);
	vsprintf (buf, format, args);
	out_bounds (buf);
	va_end (args);

	_debug_in_red ("%s", buf);

	return 0;
}

/**
 * @brief debug_proc: print buf information with light red color in proc
 *
 * @param format :  the format of print bug information
 * @param ... : other arguments
 *
 * @return : 0 - return success
 */
int debug_proc (char *format, ...)
{
	char buf[MAX_LENGTH];
	memset (buf, '\0', MAX_LENGTH);
	va_list args;
	va_start (args, format);
	vsprintf (buf, format, args);
	out_bounds (buf);
	va_end (args);

	_debug_in_light_red ("\t%s", buf);
	return 0;
}

/**
 * @brief debug_black: print buf information with light black color
 *
 * @param format :  the format of print bug information
 * @param ... : other arguments
 *
 * @return : 0 - return success
 */
int debug_black (char *format, ...)
{
	char buf[MAX_LENGTH];
	memset (buf, '\0', MAX_LENGTH);
	va_list args;
	va_start (args, format);
	vsprintf (buf, format, args);
	out_bounds (buf);
	va_end (args);

	_debug_in_black ("\t%s", buf);
	return 0;
}

/**
 * @brief debug_brown: print buf information with brown black color
 *
 * @param format :  the format of print bug information
 * @param ... : other arguments
 *
 * @return : 0 - return success
 */
int debug_brown(char* format, ...) {
	char buf[MAX_LENGTH];
	memset (buf, '\0', MAX_LENGTH);
	va_list args;
	va_start (args, format);
	vsprintf (buf, format, args);
	out_bounds (buf);
	va_end (args);

	_debug_in_brown ("\t%s", buf);
	return 0;
}
