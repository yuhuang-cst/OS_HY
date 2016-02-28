#include "list.h"
#include "stdio.h"
int main()
{
	list_entry_t* list_p1 = new list_entry_t();
	list_init(list_p1);
	list_entry_t* list_p2 = new list_entry_t();
	list_add(list_p1, list_p2);
	if (list_empty(list_p1)){
		printf("list is empty");
	}
	return 0;
}