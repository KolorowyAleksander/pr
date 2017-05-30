#pragma once

// list ordered by logical clock value
struct que {
        int tid;
        int c;
        int h;
};

void que_init(struct que * queue, int * tids, int n);

void que_add(struct que * queue, int n, int tid, int clk, int h);

void que_remove(struct que * queue, int n, int tid);

int que_count_before(struct que * queue, int n, int tid);
