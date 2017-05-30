#include "def.h"
#include "utils.h"
#include "clk.h"
#include "que.h"


#define WAITING_MIN 3000 * 1000 // usec
#define WAITING_MAX 5000 * 1000 // usec


struct clk * lam_clk; // lamport clock for the ship
struct que queue [SLAVENUM];   // list head
int my_tid; // ship id
int h; 	    // requested tugboats
int H;      // all tugboats required
int n;      // number of ships total
int s_tids[SLAVENUM]; // other ships ids


void add_to_queue(int tid, int clk, int h)
{
	const char * fmt = "Recieved entering from: %d, clock: %d";
	log_m(fmt, tid, clk_getval(lam_clk));

	que_add(queue, n, tid, clk, h);
	clk_inc(lam_clk);
	pvm_initsend(PvmDataDefault);
	clk_pkclk(lam_clk);
	pvm_send(tid, MSG_OK);
}

void remove_from_queue(int tid)
{
	const char * fmt = "Recieved leaving from: %d, clock: %d";
	log_m(fmt, tid, clk_getval(lam_clk));
	que_remove(queue, n, tid);
}

void send_request(int tid)
{
	clk_inc(lam_clk);
	pvm_initsend(PvmDataDefault);
	clk_pkclk(lam_clk);
	pvm_pkint(&h, 1, 1);
	pvm_send(tid, MSG_TAKE);
}

void leaving()
{
	log_m("Leaving, clock: %d", clk_getval(lam_clk));

	int i;
	for(i = 0; i < n; i++) {
		if (s_tids[i] != my_tid) {
			clk_inc(lam_clk);
			pvm_initsend(PvmDataDefault);
			clk_pkclk(lam_clk);
			pvm_send(s_tids[i], MSG_FREE);
		}
	}
}

void entering()
{
	int bufid, tag, permissions, c, s_tid, s_c, s_h;
	log_m("Entering: clock: %d", clk_getval(lam_clk));

	c = clk_getval(lam_clk);

	int i;
	for(i = 0; i < n; i++) { // sending request to all other processes
		if (s_tids[i] != my_tid) {
			send_request(s_tids[i]);
		}
	}

	bool can_enter = n = 1 && h <= H - h ? true : false; // if only self
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

		if (que_count_before(queue, n, my_tid) <= H - h) {
			can_enter = true;
		}
	}
}

void idle()
{
	double left = (rand()%(WAITING_MAX - WAITING_MIN)) + WAITING_MIN; // us
	int bufid, tag, s_tid, s_c, s_h; // s means other ship
	struct timeval t;
	struct timespec t1, t2, difference;

	log_m("Waiting: %.1f us, clock: %d", left, clk_getval(lam_clk));

	while(left > 0) {
		clock_gettime(CLOCK_MONOTONIC, &t1);

		t.tv_sec = left / 10e6;
		t.tv_usec = fmod(left, 10e6);
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

		difference = timespec_diff(t1, t2);
		left -= difference.tv_sec * 1e6 + difference.tv_nsec * 1e-3;
	}
}

void sailing()
{
	log_m("Sailing!");

	while(true) {
		idle();     // waiting random amount of time
		entering(); // trying to enter critical section
		idle();     // waitin in critical section
		leaving();  // info to others that they can free resources
	}
}

int main(int argc, char** argv)
{	// init section
	my_tid = pvm_mytid();  // initiating this as a pvm process
	lam_clk = clk_make(); // initiating global lamport clock
	srand(time(NULL) * my_tid); // this gives different seed

	pvm_recv(-1, MSG_INIT); // initiating global state
	pvm_upkint(&n, 1, 1);
	pvm_upkint(s_tids, n, 1);
	pvm_upkint(&h, 1, 1);
	pvm_upkint(&H, 1, 1);

	que_init(queue, s_tids, n);

	// main section
	sailing();

	//end section (never gets called, because infinite loop)
	clk_free(lam_clk);
	pvm_exit();
}
