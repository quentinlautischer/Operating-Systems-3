// vector.h
// Code from https://www.happybearsoftware.com/implementing-a-dynamic-array

#ifndef VECTORARRAY_H 
#define VECTORARRAY_H

#include <stdio.h>
#include <stdlib.h>

#define VECTOR_INITIAL_CAPACITY 100

// Define a vector type
typedef struct {
  int size;      // slots used so far
  int capacity;  // total available slots
  int *data;     // array of integers we're storing
} VectorArray;

void vector_init(VectorArray *vector);

void vector_append(VectorArray *vector, int value);

int vector_get(VectorArray *vector, int index);

void vector_set(VectorArray *vector, int index, int value);

void vector_double_capacity_if_full(VectorArray *vector);

void vector_free(VectorArray *vector);

int vector_size(VectorArray *vector);

#endif 
