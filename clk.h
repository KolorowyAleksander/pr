#pragma once
#include <stdio.h>

struct logical_clock {
	int value;
};

void init_clock();
void increase_clock();
void cmp_and_increase(int other_clock);
int get_clock_value();
