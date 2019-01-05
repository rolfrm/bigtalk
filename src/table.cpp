#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include <assert.h>
#include <typeinfo>
#include <stdexcept>
#include "allocator.hpp"
#include "typed_box.hpp"
#include "view.hpp"
#include "column.hpp"
#include "table.hpp"

void table::push_column(column_base * col){
  info->columns = array.resize(info->columns,  (info->column_count += 1));
  info->columns[info->column_count - 1] = col;
  col->resize(info->row_count);
}

table::table(): current_view(NULL, NULL){
  info = array.create<table::table_info>();
  info->name = NULL;
}

table::table(const char * name) : table(){
  info->name = array.clone(name);
}

void table::print_csv(){
    printf("Table '%s'\n", info->name);
    for(size_t i = 0; i < info->column_count; i++){
      if(i != 0)
	printf(" ,");
      printf("%s", info->columns[i]->name);
    }
    printf("\n");
    for(size_t r = 0; r < info->row_count; r++){
      printf("%3i  ", r);
      for(size_t c = 0; c < info->column_count; c++){
	if(c != 0)
	  printf(" ,");
	void * d = info->columns[c]->get_view();
	const std::type_info & t = info->columns[c]->get_column_type();
	if(t == typeid(int)){
	  printf("%i", ((int *) d)[r]);
	}else if(t == typeid(float)){
	  printf("%f", ((float *) d)[r]);
	}else if(t == typeid(size_t)){
	  printf("%i", ((size_t *) d)[r]);
	}else if(t == typeid(bool)){
	  printf("[%s]", ((bool *) d)[r] ? "x" : " ");
	}else{
	  printf("unsupported(%s)", t.name());
	}
      }
      printf("\n");
    }

    
    printf("\n");

}

column_base * table::get_column(size_t index){
  if(index >= info->column_count)
    throw "Error";
  return info->columns[index];
}

void table::resize(size_t new_row_count){
  for(size_t i = 0; i < info->column_count; i++)
    info->columns[i]->resize(new_row_count);
  info->row_count = new_row_count;
}

size_t table::add_row(){
  size_t r = info->row_count;
  resize(r + 1);
  printf("add_row: %i / %i\n", r, info->row_count);
  return r;
}

void table::dispose()
{
  
}


table table::create(const char * name){
  return table(name);
}


size_t table::get_column_count(){
  return info->column_count;
}

template<typename T>
int sort_things(const size_t * _a, const size_t * _b, T * values){
  auto a = values[*_a];
  auto b = values[*_b];
  if(a < b) return -1;
  if(a > b) return 1;
  return 0;
}

typedef int (*compare_f)(const void *, const void *, void *);

void table_index::update_index(){
  column_base * c = t.get_column(column);
  void * raw = c->get_view();
  const std::type_info & ctype = c->get_column_type();
  for(size_t i = 0; i < current_index_size; i++)
    current_index[i] = i;
  
  if(ctype == typeid(float)){
    float * values = (float *) raw;
    qsort_r(current_index, current_index_size, sizeof(size_t), (compare_f) sort_things<float>, values);
  }else if(ctype == typeid(int)){
    int * values = (int *) raw;
    qsort_r(current_index, current_index_size, sizeof(size_t), (compare_f) sort_things<int>, values);
  }
  else if(ctype == typeid(bool)){
    bool * values = (bool *) raw;
    qsort_r(current_index, current_index_size, sizeof(size_t), (compare_f) sort_things<bool>, values);
  }else {
    throw std::runtime_error("Unable to create an index of this kind");
  }
  
}
