#include "list.h"
int main()
{
	list_entry_t* list_p1 = (list_entry_t*)malloc(sizeof(list_entry_t));
	list_p1->prev =  (list_entry_t*)malloc(sizeof(list_entry_t));
	list_p1->next =  (list_entry_t*)malloc(sizeof(list_entry_t));

	list_entry_t* list_p2 = (list_entry_t*)malloc(sizeof(list_entry_t));
	list_p2->prev =  (list_entry_t*)malloc(sizeof(list_entry_t));
	list_p2->next =  (list_entry_t*)malloc(sizeof(list_entry_t));

	list_init(list_p1);
	list_add(list_p1, list_p2);
	if (list_empty(list_p1)){
		printf("list is empty\n");
	}
	else{
		printf("list is not empty\n");
	}
	return 0;
}
