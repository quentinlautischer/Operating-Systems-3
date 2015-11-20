// vector.c
// Code from https://www.happybearsoftware.com/implementing-a-dynamic-array
#include "vector.h"

void vector_init(VectorArray *vector) {
  // initialize size and capacity
  vector->size = 0;
  vector->capacity = VECTOR_INITIAL_CAPACITY;

  // allocate memory for vector->data
  vector->data = malloc(sizeof(int) * vector->capacity);
}

void vector_append(VectorArray *vector, int value) {
  // make sure there's room to expand into
  vector_double_capacity_if_full(vector);

  // append the value and increment vector->size
  vector->data[vector->size++] = value;
}

int vector_get(VectorArray *vector, int index) {
  if (index >= vector->size || index < 0) {
    printf("Index %d out of bounds for VectorArray of size %d\n", index, vector->size);
    exit(1);
  }
  return vector->data[index];
}

void vector_set(VectorArray *vector, int index, int value) {
  // zero fill the VectorArray up to the desired index
  while (index >= vector->size) {
    vector_append(vector, 0);
  }

  // set the value at the desired index
  vector->data[index] = value;
}

void vector_double_capacity_if_full(VectorArray *vector) {
  if (vector->size >= vector->capacity) {
    // double vector->capacity and resize the allocated memory accordingly
    vector->capacity *= 2;
    vector->data = realloc(vector->data, sizeof(int) * vector->capacity);
  }
}

int vector_size(VectorArray *vector) {
	return vector->size;
}

void vector_free(VectorArray *vector) {
  free(vector->data);
}
