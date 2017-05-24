#include "clk.h"

struct logical_clock _c; 

void init_clock() {
	_c.value = 0;
}

void increase_clock() {
	_c.value++;
}

void cmp_and_increase(int other_clock) {
	if (_c.value > other_clock) {
		_c.value++;
	} else {
		_c.value = other_clock++;
	}
}

int get_clock_value() {
	return _c.value;
}	
