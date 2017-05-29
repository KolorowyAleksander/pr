#include "def.h"
#include <time.h>

// the master has to:
// give each ship a number of tugboats required
// sum all the tugboats
// create a total, then set number of tugboats available (lower than total)
int main(int argc, char ** argv) {
	srand(time(NULL)); // initiate prng

	int my_tid;
	int h[SLAVENUM];    // number of tugboats needed by ship i
	int H = 0; 	    // number of tugboats needed by all ships
	int tids[SLAVENUM];
	int nproc;
	char recieved_string[100];

	my_tid = pvm_mytid(); // just to start this as a pvm process

	nproc=pvm_spawn(SLAVENAME, NULL, PvmTaskDefault, "", SLAVENUM, tids);

	fprintf(stdout, "spawned %d tasks\n", nproc);

	int i;
	for(i = 0; i < nproc; i++) {
		h[i] = rand() % 100;
		H += h[i];
	}

	for(i = 0; i < nproc; i++) {
		pvm_initsend(PvmDataDefault);
		pvm_pkint(&nproc, 1, 1);
		pvm_pkint(tids, nproc, 1);
		pvm_pkint(&h[i], 1, 1);
		pvm_pkint(&H, 1, 1);
		pvm_send(tids[i], MSG_INIT);
	}



	while(true) {
		char s[MAX_MSG_SIZE];
		pvm_recv(-1, MSG_LOG);
		pvm_upkstr(s);
		fprintf(stdout, s);
	}

	pvm_exit();
}
