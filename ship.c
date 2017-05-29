#include "def.h"
#include "utils.h"
#include "clk.h"


#define WAITING_MIN 300 * 1000 // usec
#define WAITING_MAX 500 * 1000 // usec


struct clk * lam_clk; // lamport clock for the ship
int my_tid; // ship id
int h; 	    // requested tugboats
int H;      // all tugboats required
int n;      // number of ships total
int s_tids[SLAVENUM]; // other ships ids


void add_to_queue(int tid, int c, int h)
{
	log_m("Recieved entering from: %d, clock: %d\n",
	      tid, clk_getval(lam_clk));

	// TODO: add to queue
	clk_inc(lam_clk);
	pvm_initsend(PvmDataDefault);
	clk_pkclk(lam_clk);
	pvm_send(tid, MSG_OK);
}

void remove_from_queue(int tid)
{
	log_m("Recieved leaving from: %d, clock: %d\n",
	       tid, clk_getval(lam_clk));
	// TODO: remove from queue
}

void request_from_other(int tid)
{
	clk_inc(lam_clk);
	pvm_initsend(PvmDataDefault);
	clk_pkclk(lam_clk);
	pvm_pkint(&h, 1, 1);
	pvm_send(tid, MSG_TAKE);
}

void leaving()
{
	log_m("Leaving, clock: %d\n", clk_getval(lam_clk));

	int i;
	for(i = 0; i < n; i++) {
		clk_inc(lam_clk);

		pvm_initsend(PvmDataDefault);
		clk_pkclk(lam_clk);
		pvm_send(s_tids[i], MSG_FREE);
	}
}

void entering()
{
	int bufid, tag, s_tid, s_c, s_h, permissions;
	log_m("Entering: clock: %d\n", clk_getval(lam_clk));

	// TODO: clear queue
	int i;
	for(i = 0; i < n; i++) { // sending request to all other processes
		if (s_tids[i] != my_tid) {
			request_from_other(s_tids[i]);
		}
	}

	bool can_enter = false;
	permissions = 1; // i allow myself to enter
	while(!can_enter) {
		bufid = pvm_recv(-1, -1);
		pvm_bufinfo(bufid, NULL, &tag, &s_tid);
		s_c = clk_upk_and_cmp(lam_clk);

		switch(tag) {
			case MSG_TAKE: {
				pvm_upkint(&s_h, 1, 1);
				add_to_queue(s_tid, s_c, s_h);
				break;
			} case MSG_FREE: {
				remove_from_queue(s_tid);
				break;
			} case MSG_OK: {
				permissions++;
			}
		}

		// TODO: check if can enter if(permissions == n && all in front can enter)
		can_enter = true;
	}
}

void idle()
{
	double left = (rand()%(WAITING_MAX - WAITING_MIN)) + WAITING_MIN; // us
	int bufid, tag, s_tid, s_c, s_h; // s means other ship
	struct timeval t;
	struct timespec t1, t2;

	log_m("Waiting: %f us, clock: %d\n", left, clk_getval(lam_clk));

	while(left > 0) {
		clock_gettime(CLOCK_MONOTONIC, &t1);

		t.tv_sec = 0;
		t.tv_usec = left;
		if ((bufid = pvm_trecv(-1, -1, &t)) > 0) {
			pvm_bufinfo(bufid, NULL, &tag, &s_tid);
			s_c = clk_upk_and_cmp(lam_clk);

			switch(tag) {
				case MSG_TAKE: {
					pvm_upkint(&s_c, 1, 1);
					add_to_queue(s_tid, s_c, s_h);
					break;
				} case MSG_FREE: {
					remove_from_queue(s_tid);
					break;
				}
			}
		}

		clock_gettime(CLOCK_MONOTONIC, &t2);
		left -= (t2.tv_sec - t1.tv_sec) * 1e6 +
			(t2.tv_nsec - t1.tv_nsec) * 1e-3;
	}
}

void sailing()
{
	log_m("Sailing!\n");

	int i;
	for(i = 0; i < 3; i++) { // this will be infinite loop sometime
		idle();
		entering();
		idle();
		leaving();
	}
}

int main(int argc, char** argv)
{
	my_tid = pvm_mytid();  // initiating this as a pvm process
	lam_clk = clk_make(); // initiating global lamport clock
	srand(time(NULL) * my_tid); // this gives different seed

	pvm_recv(-1, MSG_INIT); // initiating global state
	pvm_upkint(&n, 1, 1);
	pvm_upkint(s_tids, n, 1);
	pvm_upkint(&h, 1, 1);
	pvm_upkint(&H, 1, 1);

	sailing();

	clk_free(lam_clk);
	pvm_exit();
}
