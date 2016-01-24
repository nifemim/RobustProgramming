#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * error return values
 */
#define LE_NONE 0	/* no errors */
#define LE_TOOMANYLISTS -1	/* too many linked lists */
#define LE_NOMEMORY -2 		/* error in malloc */
#define LE_INTINCON -3		/* internal inconsistency */
#define LE_BADTICKET -4		/* bad ticket */
#define LE_BADCALL -5

/*
 * checking error return values
 */
#define LE_ISERROR(x) ((x) < 0)

/*
 * ticket used to access linked lists
 */
typedef long int LTICKET;

/*
 * error buffer, accessed as extern variable and defined by the caller
 */
extern char le_errbuf[256];

/*
 * linked list management
 */
LTICKET create_list(void* head_data);	/* create a list */
int delete_list(LTICKET ticket);		/* delete a list */
int append_to_list(LTICKET ticket, void* data);	/* append a data node to the list */
int insert_head(LTICKET ticket, void* data);	/* insert a data node at the head of the list */
int remove_head(LTICKET ticket);	/* remove the head node from the linked list */
int remove_tail(LTICKET ticket);	/* remove the tail node from the linked list */
int visit_nodes(LTICKET ticket);	/* visit all the nodes in the linked list */
