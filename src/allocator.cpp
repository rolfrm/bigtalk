#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "allocator.hpp"

void * Allocator::alloc0(size_t bytes){
  return calloc(bytes, 1);
}

void Allocator::free(void * ptr){
  ::free(ptr);
}

void * Allocator::realloc(void * ptr, size_t new_size){
  return ::realloc(ptr, new_size);
}

char * Allocator::clone(const char * ptr){
  if(ptr == NULL) return NULL;
  size_t byte_len = strlen(ptr) + 1;
  char * d = this->create<char>(byte_len);
  memcpy(d, ptr, byte_len);
  return d;
}

Allocator array = Allocator();
