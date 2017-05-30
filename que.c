#include "que.h"


void que_init(struct que * queue, int * tids, int n)
{
        int i = 0;
        for(i = 0; i < n; i++) {
                queue[i].tid = tids[i];
                queue[i].c = 0;
        }
}

void que_add(struct que * queue, int n, int tid, int clk, int h)
{
        int i = 0;
        for(i = 0; i < n; i++) {
                if(queue[i].tid == tid) {
                        queue[i].c = clk;
                        queue[i].h = h;
                        break;
                }
        }
}

void que_remove(struct que * queue, int n, int tid)
{
        int i = 0;
        for(i = 0; i < n; i++) {
                if(queue[i].tid == tid) {
                        queue[i].c = -1;
                }
        }
}

int que_count_before(struct que * queue, int n, int tid)
{
        int h = 0; // total sum
        int c;     // tids clock
        int i = 0;

        for(i = 0; i < n; i++) {
                if(queue[i].tid == tid) {
                        c = queue[i].c;
                }
        }

        for(i = 0; i < n; i++) {
                if(queue[i].tid != tid && queue[i].c < c) {
                        h += queue[i].h;
                }
        }

        return h;
}
