#ifndef __DFS_STACK_H
#define __DFS_STACK_H

#include <stdlib.h>
#include <stdio.h>
#include "graph.h"

//Implement a stack for DFS
// start by adding the very first targets
// loop or recursion:
// look at top of stack, are there dependencies that are not resolved.
// if yes find first such dependency and add, repeat for top of stack.
// main.o : main.c --> just execute here, not really a dependent

typedef struct block_stack {
  int max_capacity;
  int top_index;
  target top;
  target *items;
} stack;

stack *createStack(int max_capacity) {
  stack *st = malloc(sizeof(stack));

  st->max_capacity = max_capacity;
  st->top_index = -1;
  st->items = malloc(sizeof(target) * max_capacity);

  return st;
}

int isEmpty(stack *st) {
  return st->top_index == -1;
}

int isFull(stack *st) {
  return st->top_index == st->max_capacity-1;
}

void push(stack *st, target targ) {
  if(!isFull(st)) {
    st->items[++st->top_index] = targ;
  }
}

target peek(stack *st) {
  if(isEmpty(st)) {
    exit(!EXIT_SUCCESS);
  }
  return st->items[st->top_index];
}


target pop(stack *st) {
  if(isEmpty(st)) {
    exit(!EXIT_SUCCESS);
  }
  return st->items[st->top_index--];
}

#endif
