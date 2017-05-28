#pragma once

struct clk;

struct clk *clk_init();

void clk_free(struct clk * clock);

void clk_succ(struct clk * clock);

void clk_cmp(struct clk* clock, int other_clock);

int clk_getval(struct clk * clock);
