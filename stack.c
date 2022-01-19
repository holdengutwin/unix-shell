/*
Partner 1: Theodore Buckley
trb702
11279739

Partner 2:
Holden Gutwin
11273492
hcg369 

CMPT 332 - Assignment 1b
*/

#include <stdlib.h>
#include <stdio.h>

#include "stack.h"

/*
Function that dynamically allocates memory for a new Stack, initializes its values, 
and returns a pointer to the newly allocated and initialized Stack
*/
Stack *init_stack() {
	Stack *newStack = (Stack*) malloc(sizeof(Stack));
	newStack->head = NULL;
	newStack->n_items = 0;
	return newStack;
}

/*
Pushes an item to the stack
*/
void push(Stack *target, void *data) {
	LinkedNode *newNode = (LinkedNode*) malloc(sizeof(LinkedNode));
	newNode->item = data;
	if (target->head == NULL) {
		target->head = newNode;
	}
	else {
		LinkedNode *oldHead = target->head;
		target->head = newNode;
		newNode->next = oldHead;
	}
	target->n_items++;
}

/*
Pops an item off the stack and returns it
If the stack is empty, returns NULL
*/
void *pop(Stack *target) {
	if (target->head == NULL) {
		return NULL;
	}

	else {
		LinkedNode *oldHead = target->head;
		void *oldItem = oldHead->item;
		target->head = oldHead->next;
		free(oldHead);
		target->n_items--;
		return oldItem;
	}

}

/*
Returns the top item from the stack without removing it
*/
void *peek(Stack *target) {

	if (target->head == NULL) {
		return NULL;
	}
	return target->head->item;
}

int stack_size(Stack *target) {
	return target->n_items;
}
/*
Basic stack testing
*/
/*
int main()
{
	int a = 1;
	int b = 2;
	int c = 3;
	Stack *myStack = init_stack();
	push(myStack, &a);
	push(myStack, &b);
	push(myStack, &c);
	printf("peeked %d\n", *(int*)peek(myStack));
	printf("popped %d\n", *(int*)pop(myStack));
	printf("popped %d\n", *(int*)pop(myStack));
	printf("popped %d\n", *(int*)pop(myStack));
	free(myStack);
}
*/
