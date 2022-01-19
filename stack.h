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

/*
Structure that represents a node in a node chain
*/
typedef struct linkednode {
	struct linkednode *next;
	void *item;
} LinkedNode;

/*
Structure that represents a basic stack that holds, implemented using a node chain
The stack stores items of type void* (and how many items are in it) and can be pushed, popped, and peeked
Using the stack with another type is done by typecasting pointers of your desired type to void* before pushing and after peeking/popping
*/
typedef struct stack {
	LinkedNode *head;
	int n_items;
} Stack;

Stack *init_stack();

void push(Stack *target, void *data);

void *pop(Stack *target);

void *peek(Stack *target);

int stack_size(Stack *target);