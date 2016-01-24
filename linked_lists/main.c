#include "linklib.h"

char le_errbuf[256];

int main(int argc, char *argv[]) {
	printf("\n");
	/*
	 * testing the creation of lists and appending
	 */
	printf("%s\n\n", "Commands Test 1");
	LTICKET ref = create_list("Adewale");
	append_to_list(ref, "Olufemi");
	append_to_list(ref, "Nneka");
	append_to_list(ref, "Jacob");
	append_to_list(ref, "Adetola");
	append_to_list(ref, "Kemi");
	visit_nodes(ref);
	delete_list(ref);

	/*
	 * testing the head and tail removal as well as head insertion
	 */
	printf("%s\n\n", "Commands Test 2");
	LTICKET ref2 = create_list("Adewale");
	append_to_list(ref2, "Olufemi");
	append_to_list(ref2, "Nneka");
	append_to_list(ref2, "Jacob");
	append_to_list(ref2, "Adetola");
	append_to_list(ref2, "Kemi");
	visit_nodes(ref2);
	remove_head(ref2);
	visit_nodes(ref2);
	insert_head(ref2, "Chiamaka");
	visit_nodes(ref2);
	remove_tail(ref2);
	visit_nodes(ref2);
	delete_list(ref2);

	// int check = visit_nodes(ref);
	// if(LE_ISERROR(check))
	// 	printf("%s\n", le_errbuf);
	return 0;
}
