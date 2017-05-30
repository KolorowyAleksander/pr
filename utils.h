#pragma once

#include <stdarg.h>
#include <time.h>
#include <pvm3.h>

void log_m(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char s[MAX_MSG_SIZE];

	vsprintf(s, fmt, args);
	pvm_initsend(PvmDataDefault);
	pvm_pkstr(s);
	pvm_send(pvm_parent(), MSG_LOG);
}

// this code was found here:
// https://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/
// it's no different how anyone would implementing, but since we copied it
// we wan't to say it's not actually our code
struct timespec timespec_diff(struct timespec start, struct timespec end)
{
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}
