#pragma once

#include <stdarg.h>
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
