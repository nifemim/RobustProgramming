#include "linklib.h"

/*
 * maximum sizes
 */
#define MAX_NODES 1024 /* max number of nodes per linked list */
#define MAX_LISTS 1024 /* max number of linked lists */

/*
 * offsets
 */
#define IOFFSET 0x1221 /* index offset */
#define NOFFSET 0x0502 /* initial nonce */

/*
 * error message functions
 */
#define ERROR_BUF(str) (void) strncpy(le_errbuf, str, sizeof(le_errbuf))
#define ERROR_BUF2(str, f) (void) sprintf(le_errbuf, str, f)
#define ERROR_BUF3(str, f_1, f_2) (void) sprintf(le_errbuf, str, f_1, f_2)

/*
 * linked list node
 */
typedef struct ListNode {
	void* data;
	struct ListNode *next;
} LNODE;

/*
 * linked list structure
 */
typedef struct LinkedList {
	LNODE *head;
	LTICKET ticket;
} LINKED_LIST;

/*
 * variables shared by libary routines
 */
static LINKED_LIST *lists[MAX_LISTS]; /* the array of linked lists */
static unsigned int nonce = NOFFSET; /* nonce must be non-zero, see README */

/*
 * creates a ticket
 * errors: LE_INTINCON	- index out of range
 *						- nonce out of range
 */
static LTICKET create_ticket(unsigned int index)
{
	if(index > MAX_LISTS) {
		ERROR_BUF3("create_ticket: index %u exceeds %d", index, MAX_LISTS);
		return LE_INTINCON;
	}

	unsigned int high;
	unsigned int low;

	high = (index + IOFFSET);

	if((high & 0x7fff) != index + IOFFSET) {
		ERROR_BUF3("create_ticket: index %u larger than %u", index, 
			0x7fff - IOFFSET);
		return LE_INTINCON;
	}

	low = nonce & 0xffff;
	if((low != nonce++) || low == 0) {
		ERROR_BUF3("create_ticket: generation number %u exceeds %u", nonce-1, 0xffff - NOFFSET);
		return LE_INTINCON;
	}

	LTICKET ret_val = (high << 16) | low;

	return ret_val;
}

/*
 * reads out the index and nonce of a ticket
 * errors: LE_INTINCON	- reference to null nodes
 *						- nonce is 0
 * errors: LE_BADTICKET	- ticket index out of range
 *						- reference to null or old linked list
 */
static int read_ticket(LTICKET ticket)
{
	register unsigned int index;
	LINKED_LIST *l;

	index = (ticket >> 16) & 0xffff - IOFFSET;

	if(index >= MAX_LISTS) {
		ERROR_BUF2("read_ticket: ticket index %u is out of range", index);
		return LE_BADTICKET;
	}

	if(lists[index] == NULL) {
		ERROR_BUF("read_ticket: there is no linked list at the ticket index");
		return LE_BADTICKET;
	}

	if(lists[index]->ticket != ticket) {
		ERROR_BUF3("read_ticket: ticket refers to old list (new=%u, old=%u)",
			(lists[index]->ticket & 0xffff) - NOFFSET, 
			(ticket & 0xffff) - NOFFSET);
		return LE_BADTICKET;
	}

	l = lists[index];

	if(((l->ticket) & 0xffff) == 0) {
		ERROR_BUF("read_ticket: internal inconsistentcy: nonce = 0");
		return LE_INTINCON;
	}

	return index;
}

/*
 * creates a linked list with a specific head node 
 * errors: 	LE_NOMEMORY	- error calling malloc
 *			LE_TOOMANYLISTS	- maximum number of linked lists
 */
LTICKET create_list(void* head_data)
{
	register int index;

	for(index = 0; index < MAX_LISTS; index++)
	{
		if(lists[index] == NULL)
			break;
	}

	if(index >= MAX_LISTS) {
		ERROR_BUF2("create_list: too many lists (maximum of %d)", MAX_LISTS);
		return LE_TOOMANYLISTS;
	}

	LINKED_LIST *list;
	LNODE *head;

	if((list = malloc(sizeof(LINKED_LIST))) == NULL) {
		ERROR_BUF("create_list: error allocating memory for list");
		return LE_NOMEMORY;
	}

	if((head = malloc(sizeof(LNODE))) == NULL) {
		ERROR_BUF("create_list: error allocating memory for head");
		free(list);
		return LE_NOMEMORY;
	}

	LTICKET ticket = create_ticket(index);

	if(LE_ISERROR(ticket)) {
		free(head);
		free(list);
		return ticket;
	}

	lists[index] = list;

	head->data = head_data;
	head->next = NULL;

	list->head = head;
	list->ticket = ticket;

	return ticket;
}

/*
 * deletes a linked list
 * errors: LE_INTINCON	- index out of range
 *						- nonce out of range
 */
int delete_list(LTICKET ticket)
{
	register int index;

	if(LE_ISERROR(index = read_ticket(ticket)))
		return index;

	LINKED_LIST *list = lists[index];
	
	LNODE *cur = list->head;
	while(cur != NULL) {
		LNODE *tmp = cur;
		cur = cur->next;
		free(tmp);
	}
	
	free(list);
	lists[index] = NULL;

	return LE_NONE;
}

/*
 * appends to a linked list
 * errors: 	LE_NOMEMORY	- error in malloc
 *			LE_INTINCON - nonce out of range
 */
int append_to_list(LTICKET ticket, void* data)
{
	register int index;

	if (LE_ISERROR(index = read_ticket(ticket)))
		return index;

	LINKED_LIST *list = lists[index];

	LNODE *node;
	if((node = malloc(sizeof(LNODE))) == NULL) {
		ERROR_BUF("append_to_list: error allocating memory");
		return LE_NOMEMORY;		
	}

	node->data = data;
	node->next = NULL;

	if(list->head) {
		LNODE *cur = list->head;
		while(cur->next) {
			cur = cur->next;
		}
		cur->next = node;
	} else {
		list->head = node;
	}

	return LE_NONE;
}

/*
 * inserts to a linked list
 * errors: 	LE_NOMEMORY	- error calling malloc
 *			LE_INTINCON	- previous head of linked list is NULL
 */
int insert_head(LTICKET ticket, void* data)
{
	register int index;

	if (LE_ISERROR(index = read_ticket(ticket)))
		return index;

	LINKED_LIST *list = lists[index];

	LNODE *node;
	if((node = malloc(sizeof(LNODE))) == NULL) {
		ERROR_BUF("insert_head: error allocating memory");
		return LE_NOMEMORY;		
	}

	LNODE *prev_head = list->head;

	node->data = data;
	node->next = prev_head;

	list->head = node;

	return LE_NONE;
}

int remove_head(LTICKET ticket)
{
	register int index;

	if (LE_ISERROR(index = read_ticket(ticket)))
		return index;

	LINKED_LIST *list = lists[index];
	LNODE *prev_head = list->head;

	if(prev_head == NULL) {
		ERROR_BUF("remove_head: no head to remove");
		return LE_BADCALL;
	}

	list->head = list->head->next;
	free(prev_head);

	return LE_NONE;
}

int remove_tail(LTICKET ticket)
{
	register int index;

	if (LE_ISERROR(index = read_ticket(ticket)))
		return index;

	LINKED_LIST *list = lists[index];

	if(list->head == NULL) {
		ERROR_BUF("remove_tail: no element in list to remove");
		return LE_BADCALL;
	}
	
	LNODE *cur = list->head;

	if(cur->next == NULL) {
		free(cur);
		list->head = NULL;
	} else {
		LNODE *prev = cur;
		cur = cur->next;
		while(cur->next) {
			cur = cur->next;
		}
		free(cur);
		prev->next = NULL;
	}

	return LE_NONE;
}

int visit_nodes(LTICKET ticket)
{
	register int index;

	if (LE_ISERROR(index = read_ticket(ticket)))
		return index;

	LINKED_LIST *list = lists[index];

	LNODE *node = list->head;

	printf("\n%s\n\n", "PRINTING NODE DATA");
	while(node) {
		printf("%s", node->data);
		node = node->next;
		if(node)
			printf("%s", " -> ");
	}

	printf("\n");

	return LE_NONE;
}
