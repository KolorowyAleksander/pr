#include "def.h"
#include <time.h>


int main(int argc, char ** argv) {
	srand(time(NULL));

	int my_tid;         // master pvm id
	int n;              // number of ships total
	int tids[SLAVENUM]; // all ships id's
	int h[SLAVENUM];    // number of tugboats needed by ship i
	int H = 0; 	    // number of tugboats needed by all ships

	my_tid = pvm_mytid();

	n = pvm_spawn(SLAVENAME, NULL, PvmTaskDefault, "", SLAVENUM, tids);

	int i;
	for(i = 0; i < n; i++) {
		h[i] = rand() % 9 + 1;
		H += h[i];
	}

	fprintf(stdout, "spawned %d tasks\n", n);
	fprintf(stdout, "Sum of h: %d, H: %d.\n", H, H-h[0]);

	H -= h[0];

	char recieved_string[100];
	for(i = 0; i < n; i++) {
		pvm_initsend(PvmDataDefault);
		pvm_pkint(&n, 1, 1);
		pvm_pkint(tids, n, 1);
		pvm_pkint(&h[i], 1, 1);
		pvm_pkint(&H, 1, 1);
		pvm_send(tids[i], MSG_INIT);
	}

	while(true) { // recieving log messages
		char s[MAX_MSG_SIZE];
		pvm_recv(-1, MSG_LOG);
		pvm_upkstr(s);
		fprintf(stdout, s);
	}

	pvm_exit();
}
