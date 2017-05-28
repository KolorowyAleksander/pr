#pragma once

struct clk;

// initialize clock
void clk_make(struct clk * clock);

// free memory
void clk_free(struct clk * clock);

// increase clock value
void clk_inc(struct clk * clock);

// takes a clock structure and packs it into pvm buffer
void clk_pkclk(struct clk * clock);

// unpacks from pvm buffer, and compares recieved value with clock
void clk_upk_and_cmp(struct clk * clock);

// recieve underlying value
int clk_getval(struct clk * clock);
