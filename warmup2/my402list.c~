#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "cs402.h"

#include "my402list.h"
/*
int main(int argc, char *argv[]){

	My402List list;
    	My402ListElem elem;

    	memset(&list, 0, sizeof(My402List));
    	(void)My402ListInit(&list);
	printf("len %d\tempty %d\n",My402ListLength(&list),My402ListEmpty(&list));
	int a = My402ListAppend(&list,(void *)1);
	a = My402ListAppend(&list,(void *)8);
	a = My402ListPrepend(&list,(void *)6);
	
	printf("len %d\tempty %d\n",My402ListLength(&list),My402ListEmpty(&list));
	printf("%d\n", (void *)(list.anchor.next->obj));
	printf("%d\n", (void *)(list.anchor.next->next->obj));
	printf("%d\n", (void *)(list.anchor.next->next->next->obj));
	elem = list.anchor;
	
	
	printf("\n");
	return 0;
}*/

int  My402ListLength(My402List* pList){
	return pList->num_members;
}

int  My402ListEmpty(My402List* pList){
	if(pList->num_members == 0)
		return TRUE;
	return FALSE;	
}

int  My402ListAppend(My402List* pList, void* obj){

	My402ListElem *temp = (My402ListElem *)malloc(sizeof(My402ListElem));	
	temp->obj = obj;
	My402ListElem *last = My402ListLast(pList);

	if(My402ListEmpty(pList)){
		pList->anchor.next = temp;
		pList->anchor.prev = temp;
		temp->next = &(pList->anchor);
		temp->prev = &(pList->anchor);				
	}
	else{
		last->next = temp;
		temp->next = &(pList->anchor);
		pList->anchor.prev = temp;
		temp->prev = last;				 			
	}	
	pList->num_members = pList->num_members + 1;
	return TRUE;
}

int  My402ListPrepend(My402List* pList, void* obj){

	My402ListElem *temp = (My402ListElem *)malloc(sizeof(My402ListElem));	
	temp->obj = obj;
	My402ListElem *first = My402ListFirst(pList);

	if(My402ListEmpty(pList)){
		pList->anchor.next = temp;
		pList->anchor.prev = temp;
		temp->next = &(pList->anchor);
		temp->prev = &(pList->anchor);				
	}
	else{
		pList->anchor.next = temp;
		temp->next = first;
		first->prev = temp;
		temp->prev = &(pList->anchor);				 			
	}	
	pList->num_members = pList->num_members + 1;
	return TRUE;
}

void My402ListUnlink(My402List* pList, My402ListElem* elem){	
	elem->prev->next = elem->next;
	elem->next->prev = elem->prev;

	elem->next = NULL;	
	elem->prev = NULL;
	free(elem);
	pList->num_members = pList->num_members - 1;
}

void My402ListUnlinkAll(My402List* pList){
	while(!(My402ListEmpty(pList))){
		My402ListUnlink(pList,pList->anchor.next);
	}
	pList->anchor.next = NULL;
	pList->anchor.prev = NULL;
	pList->num_members = 0;
}

int  My402ListInsertAfter(My402List* pList, void* obj, My402ListElem* elem){
	if(elem == NULL){
		My402ListAppend(pList,obj);
		return TRUE;
	}
		
	My402ListElem *temp = (My402ListElem *)malloc(sizeof(My402ListElem));	
	temp->obj = obj;

	temp->next = elem->next;
	temp->prev = elem;
	elem->next = temp;
	temp->next->prev = temp;

	pList->num_members = pList->num_members + 1;
	return TRUE;
}

int  My402ListInsertBefore(My402List* pList, void* obj, My402ListElem* elem){
	if(elem == NULL){
		My402ListPrepend(pList,obj);
		return TRUE;
	}
		
	My402ListElem *temp = (My402ListElem *)malloc(sizeof(My402ListElem));	
	temp->obj = obj;
	temp->prev = elem->prev;
	elem->prev = temp;
	temp->next = elem;
	temp->prev->next = temp;	

	pList->num_members = pList->num_members + 1;
	return TRUE;
}

My402ListElem *My402ListFirst(My402List* pList){
	return pList->anchor.next;
}

My402ListElem *My402ListLast(My402List* pList){
	return pList->anchor.prev;
}

My402ListElem *My402ListNext(My402List* pList, My402ListElem* elem){
	if(elem->next == &(pList->anchor))
		return NULL;
	return elem->next;
}

My402ListElem *My402ListPrev(My402List* pList, My402ListElem* elem){
	if(elem->prev == &(pList->anchor))
		return NULL;
	return elem->prev;
}

My402ListElem *My402ListFind(My402List* pList, void* obj){
	My402ListElem *elem = &(pList->anchor);
	while(My402ListNext(pList,elem)){
		elem = elem->next;
		if(elem->obj == obj)
			return elem;		
	}
	return NULL;
}

int My402ListInit(My402List* pList){
	memset(pList,0,sizeof(My402List));
	pList->num_members = 0;
	pList->anchor.obj = NULL;
	pList->anchor.next = NULL;
	pList->anchor.prev = NULL;
	return TRUE;
}





