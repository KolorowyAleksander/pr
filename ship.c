#include "def.h"
#include "clk.h"

#define WAITING_MIN 300 * 1000 // usec
#define WAITING_MAX 500 * 1000 // usec

struct clk * lam_clk;

void log_to_master(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char s[MAX_MSG_SIZE];

	vsprintf(s, fmt, args);
	pvm_initsend(PvmDataDefault);
	pvm_pkstr(s);
	pvm_send(pvm_parent(), MSG_LOG);
}

void add_to_queue(int tid, int n, int h)
{
	log_to_master("Recieved entering from: %d, clock: %d\n",
	       tid,
	       clk_getval(lam_clk));

	// TODO: add to queue
	clk_inc(lam_clk);
	pvm_initsend(PvmDataDefault);
	clk_pkclk(lam_clk);
	pvm_send(tid, MSG_OK);
}

void remove_from_queue(int tid, int n)
{
	log_to_master("Recieved leaving from: %d, clock: %d\n",
	       tid,
	       clk_getval(lam_clk));
	// TODO: remove from queue
}

void leaving(int* s_tids, int n)
{
	log_to_master("Leaving, clock: %d\n", clk_getval(lam_clk));

	int i;
	for(i = 0; i < n; i++) {
		clk_inc(lam_clk);

		pvm_initsend(PvmDataDefault);
		clk_pkclk(lam_clk);
		pvm_send(s_tids[i], MSG_FREE);
	}
}

void entering(int* s_tids, int n, int h, int H)
{
	log_to_master("Entering: clock: %d\n", clk_getval(lam_clk));
	int bufid, tag, s_tid, permissions;

	// TODO: clear queue
	int i;
	for(i = 0; i < n; i++) {
		clk_inc(lam_clk);
		pvm_initsend(PvmDataDefault);
		clk_pkclk(lam_clk);
		pvm_pkint(&h, 1, 1);
		pvm_send(s_tids[i], MSG_TAKE);
	}

	bool can_enter = false;
	permissions = 1; // i allow myself to enter
	while(!can_enter) {
		bufid = pvm_recv(-1, -1);
		pvm_bufinfo(bufid, NULL, &tag, &s_tid);
		clk_upk_and_cmp(lam_clk);

		switch(tag) {
			case MSG_TAKE: {
				int other_h;
				pvm_upkint(&other_h, 1, 1);
				add_to_queue(s_tid, n, other_h); // clock!
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

void idle(int n)
{
	double duration = (rand()%(WAITING_MAX - WAITING_MIN)) + WAITING_MIN;
	int bufid, tag, s_tid, h;
	struct timeval t;
	struct timespec t1, t2;

	log_to_master("Waiting for: %f us, clock: %d\n",
		      duration,
		      clk_getval(lam_clk));

	while(duration > 0) {
		clock_gettime(CLOCK_MONOTONIC, &t1);

		t.tv_sec = 0;
		t.tv_usec = duration;
		if ((bufid = pvm_trecv(-1, -1, &t)) > 0) {
			pvm_bufinfo(bufid, NULL, &tag, &s_tid);
			clk_upk_and_cmp(lam_clk);

			switch(tag) {
				case MSG_TAKE: {
					int other_h;
					pvm_upkint(&other_h, 1, 1);
					add_to_queue(s_tid, n, other_h); // clock!
					break;
				} case MSG_FREE: {
					remove_from_queue(s_tid, n);
					break;
				}
			}
		}

		clock_gettime(CLOCK_MONOTONIC, &t2);
		double elapsed_time = (t2.tv_sec - t1.tv_sec) * 1e6
			+ (t2.tv_nsec - t1.tv_nsec) * 1e-3;
		duration -= elapsed_time;
		log_to_master("Elapsed time: %f, time left: %f\n", elapsed_time, duration);
	}
}

void sailing(int* s_tids, int n, int h, int H)
{
	log_to_master("Sailing!\n");

	int i;
	for(i = 0; i < 3; i++) { // this will be infinite loop sometime
		idle(n);
		entering(s_tids, n, h, H);
		idle(n);
		leaving(s_tids, n);
	}
}

int main(int argc, char** argv)
{
	int my_tid, h, H, n;
	int s_tids[SLAVENUM]; // other ships

	my_tid = pvm_mytid();  // initiating this as a pvm process

	pvm_recv(-1, MSG_INIT); // recieving initial data about other processes
	pvm_upkint(&n, 1, 1);
	pvm_upkint(s_tids, n, 1);
	pvm_upkint(&h, 1, 1);
	pvm_upkint(&H, 1, 1);

	srand(time(NULL) * h);

	lam_clk = clk_make();
	sailing(s_tids, n, h, H);
	clk_free(lam_clk);

	pvm_exit();
}
