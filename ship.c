#include "def.h"
#include "clk.h"


struct request {
	int clock; // when the request happened
	int h; 	   // number of tugboats requested
	int tid;   // which process requested
};

struct request queue[SLAVENUM];

struct clk * clock;

void add_to_queue(int tid, int n, int h)
{
	printf("Recieved entering from: %d, clock: %d\n",
	       tid,
	       clock_getval(clock));

	// TODO: add to queue
	clk_inc(clock);
	int c = clk_getval(clock);
	pvm_initsend(PvmDataDefault);
	pvm_pkint(&c, 1, 1);
	pvm_send(tid, MSG_OK);
}

void remove_from_queue(int tid, int n)
{
	printf("Recieved leaving from: %d, clock: %d\n",
	       tid,
	       clk_getval(clock));
	// TODO: remove from queue
}

void leaving(int* s_tids, int n)
{
	printf("Leaving, clock: %d\n", clk_getval(clock));

	int i;
	for(i = 0; i < n; i++) {
		int c = clk_getval(clock);
		clk_inc(clock);

		pvm_initsend(PvmDataDefault);
		pvm_pkint(&c, 1, 1);
		pvm_send(s_tids[i], MSG_FREE);
	}
}

void entering(int* s_tids, int n, int h, int H)
{
	printf("Entering: clock: %d\n", clk_getval(clock));
	int bufid, tag, s_tid, c, h, permissions;

	// TODO: clear queue
	int i;
	for(i = 0; i < n; i++) {
		clk_inc(clock);
		int now = clk_getval(clock);
		pvm_initsend(PvmDataDefault);
		pvm_pkint(&now, 1, 1);
		pvm_pkint(&h, 1, 1);
		pvm_send(s_tids[i], MSG_TAKE);
	}

	bool can_enter = false;
	permissions = 1; // i allow myself to enter
	while(!can_enter) {
		bufid = pvm_recv(-1, -1);
		pvm_bufinfo(bufid, NULL, &tag, &s_tid);
		pvm_upkint(&c, 1, 1);

		clk_cmp(clock, c);

		switch(tag) {
			case MSG_TAKE: {
				add_to_queue(s_tid, n, h);
			} case MSG_FREE: {
				remove_from_queue(s_tid, n);
			} case MSG_OK: {
				permissions++;
			}
		}

		// TODO: check if can enter
		can_enter = true;
	}
}

void idle(double duration, int n)
{
	int bufid, tag, s_tid, c, h;
	struct timeval t, t1, t2;

	printf("Waiting: clock: %d\n", clk_getval(clock));

	while(duration > 0) {
		gettimeofday(&t1, NULL);

		t.tv_usec = duration;
		if ((bufid = pvm_trecv(-1, -1, &t)) > 0) {
			pvm_bufinfo(bufid, NULL, &tag, &s_tid);
			pvm_upkint(&c, 1, 1);

			clk_cmp(clock, c);

			switch(tag) {
				case MSG_TAKE: {
					pvm_upkint(&h, 1, 1);
					add_to_queue(s_tid, n, h);
					break;
				} case MSG_FREE: {
					remove_from_queue(s_tid, n);
					break;
				}
			}
		}

		gettimeofday(&t2, NULL);
		duration -= (t2.tv_usec - t1.tv_usec);
	}
}

void sailing(int* s_tids, int n, int h, int H)
{
	printf("Sailing: %d\n", my_tid);

	int i;
	for(i = 0; i < 3; i++) { // this will be infinite loop sometime
		idle(2, n);
		entering(s_tids, n, h, H);
		idle(2, n);
		leaving(s_tids, n);
	}
}

int main(int argc, char** argv)
{
	int my_tid, h, H, n;
	int s_tids[SLAVENUM]; // other ships

	my_tid = pvm_mytid();  // initiating this as a pvm process

	srand(time(NULL));

	// recieving initial structure
	pvm_recv(-1, MSG_INIT);
	pvm_upkint(&n, 1, 1);
	pvm_upkint(s_tids, n, 1);
	pvm_upkint(&h, 1, 1);
	pvm_upkint(&H, 1, 1);

	int i;
	for(i = 0; i < n; i++) {
		queue[i].clock = -1;
		queue[i].h = -1;
		queue[i].tid = s_tids[i];
	}

	clock = clk_init();

	sailing(s_tids, n, h, H);

	pvm_exit();
}
