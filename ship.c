#include "def.h"
#include "clk.h"


struct request {
	int clock; // when the request happened
	int h; 	   // number of tugboats requested
	int tid;   // which process requested
};

struct request queue[SLAVENUM];

struct clk * clock;

void leaving(int* s_tids, int n)
{
	int c = clk_getval(clock);
	clk_inc(clock);

	int i;
	for(i = 0; i < n; i++) {
		pvm_initsend(PvmDataDefault);
		pvm_pkint(&c, 1, 1);
		pvm_send(s_tids[i], MSG_FREE);
	}
}

void add_to_queue(int tid, int n, int h)
{
	int c = clk_getval(clock);
	// add to queue
	pvm_initsend(PvmDataDefault);
	pvm_pkint(&c, 1, 1);
	pvm_send(tid, MSG_OK);
}

void remove_from_queue(int tid, int n)
{
	int i;
	for(i = 0; i < n; i++) {
		if(queue[i].tid == tid) {
			queue[i].clock = -1;
			break;
		}
	}
}

void entering(int* s_tids, int n, int h, int H)
{
	int i;
	for(i = 0; i < n; i++) {
		queue[i].clock = -1;
	}

	int c = clk_getval(clock);

	for(i = 0; i < n; i++) {
		pvm_initsend(PvmDataDefault);
		pvm_pkint(&c, 1, 1);
		pvm_pkint(&h, 1, 1);
		pvm_send(s_tids[i], MSG_TAKE);
	}

	bool can_enter = false;
	int permissions = 1; // i allow myself to enter
	while(!can_enter) {
		int bufid = pvm_recv(-1, -1);
		int tag;
		int s_tid;
		int c;
		int h;
		pvm_bufinfo(bufid, NULL, &tag, &s_tid);
		pvm_upkint(&c, 1, 1);

		clk_cmp(clock, c);

		switch(tag) {
		case MSG_TAKE:
			add_to_queue(s_tid, n, h);
		case MSG_FREE:
			remove_from_queue(s_tid, n);
		case MSG_OK:
			permissions++;
		}

		//here i check if i can enter cs and eventually set can_enter to true
		can_enter = true;
	}
}

void wait_for(int duration, int n)
{
	int bufid, time_left = duration;
	time_t t1, t2;

	struct timeval t = {
		.tv_sec = time_left,
		.tv_usec = 0
	};

	while(time_left > 0) {
		t1 = time(NULL);
		if((bufid = pvm_trecv(-1, -1, &t)) > 0) {
			t2 = time(NULL);
			int tag;   // message tag
			int s_tid; // sender id
			int c;     // clock value
			int h;
			pvm_bufinfo(bufid, NULL, &tag, &s_tid);
			pvm_upkint(&c, 1, 1);

			clk_cmp(clock, c);

			switch(tag) {
			case MSG_TAKE:
				pvm_upkint(&h, 1, 1);
				add_to_queue(s_tid, n, h);
				break;
			case MSG_FREE:
				remove_from_queue(s_tid, n);
				break;
			}

			time_left -= (t2 - t1);
			t.tv_sec = time_left;
		} else {
			time_left = 0;
		}
	}
}

void sailing(int* s_tids, int n, int h, int H)
{
	int i;
	for(i = 0; i < 3; i++) { // this will be infinite loop
		wait_for(2, n);
		entering(s_tids, n, h, H);
		wait_for(2, n);
		leaving(s_tids, n);
	}
}

int main(int argc, char** argv) {
	int my_tid;
	int h, H, n;
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

	clock = init_clock();
	sailing(s_tids, n, h, H);

	pvm_exit();
}
