#include "clk.h"
#include <stdlib.h>


struct clk {
	int count;
};

struct clk *clk_init()
{
	struct clk * _c = malloc(sizeof * _c);
	_c.count = 0;

	return _c;
}

void clk_free(struct clk * clock)
{
	free(clock);
}

void clk_inc(struct clk * clock)
{
	clock->count++;
}

void clk_cmp(struct clk* clock, int other_clock)
{
	if (clock->count > other_clock) {
		clock->count++;
	} else {
		clock->count = other_clock++;
	}
}

int clk_getval(struct clk * clock)
{
	clock->count;
}
