#include "linklib.h"

char le_errbuf[256];

int main(int argc, char *argv[]) {

	LTICKET ref = create_list("Nifemi");
	append_to_list(ref, "Segun");
	append_to_list(ref, "Mobola");
	visit_nodes(ref);
	delete_list(ref);
	int check = visit_nodes(ref);
	
	if(LE_ISERROR(check))
		printf("%s\n", le_errbuf);
	return 0;
}
