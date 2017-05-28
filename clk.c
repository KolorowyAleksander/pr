#include "clk.h"
#include <stdlib.h>
#include <pvm3.h>


// this is enclosing a single int if we want to ie check for overflow or stuff
struct clk {
	int count;
};

void clk_make(struct clk * clock)
{
	clock = malloc(sizeof * clock);
	clock->count = 0;
}

void clk_free(struct clk * clock)
{
	free(clock);
}

void clk_inc(struct clk * clock)
{
	clock->count++;
}

void clk_pkclk(struct clk * clock)
{
	pvm_pkint(&(clock->count), 1, 1);
}

void clk_upk_and_cmp(struct clk * clock)
{
	int _c;
	pvm_upkint(&_c, 1, 1);

	clock->count = clock->count > _c ? clock->count++ : _c++;
}

int clk_getval(struct clk * clock)
{
	return clock->count;
}
