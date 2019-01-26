#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include <cassert>
#include <typeinfo>
#include <new>
#include "allocator.hpp"
#include "typed_box.hpp"
#include "view.hpp"
#include "column.hpp"
#include "table.hpp"
#include "bigtalk.hpp"
void allocator_test(){
  int array_count = 10;
  int * arrays[array_count];
  for(int j = 0; j < array_count; j++){ 
    int * arr = array.create<int>(10);
    for(int i = 0; i < 10; i++){
      arr[i] = 1;
    }
    arrays[j] = arr;
  }

  for(int j = 0; j < array_count; j++){ 
    int * arr = array.resize(arrays[j], 1024);
    for(int i = 0; i < 10; i++){
      assert(arr[i] == 1);
    }
    arrays[j] = arr;
  }

  for(int j = 0; j < array_count; j++){ 
    array.free(arrays[j]);
    arrays[j] = NULL;
  }
  
  const char * imastr = "hello world";
  char * str_clone = array.clone(imastr);
  assert(strcmp(str_clone, imastr) == 0);
  array.free(str_clone);
  printf("test allocator pass\n");
}

void test_view(){
  int * ptr = array.create<int>(10);
  view v1(ptr, 10);
  v1[5] = 10;
  int invalid_indices[] = {-1, 10, 1000, -1000};
  for(int x: invalid_indices){
    bool got_exc = false;
    try{
      v1[x] = 10;
    }catch(...){
      got_exc = true;
    }
    if(!got_exc)
      throw "test failed";
  }
  
  printf("test view PASS\n");
}


struct position_table_view {
  size_t count;
  view<int> id;
  view<float> x;
  view<float> y;
};


struct entities_table_view {
  size_t count;
  view<int> id;
  view<float> value;
};


void test_table(){
  table t = table::create("loc");
  t.add_column<int>("id");
  t.add_column<float>("x");
  t.add_column<float>("y");
  
  
  t.get_column(0)->is_index = true;
  printf("Column name: %s %i\n", t.get_column(1)->get_column_type().name(), t.get_column(1)->get_column_type() == typeid(float));
  printf("Column name: %s\n", t.get_column(2)->get_column_type().name());
  printf("Column name: %s\n", t.get_column(0)->get_column_type().name());
  t.resize(10);
  position_table_view v1;
  t.get_view<position_table_view>(v1);
  
  v1.x[0] = 1234;
  v1.x[8] = 1234;
  printf("Count: %f\n", v1.x[0]);
  assert(v1.count == 10);

  table entities = table::create("entities");
  entities.add_column<int>("ID");
  entities.add_column<float>("value");
  entities.resize(10);
  auto v3 = entities.get_view<entities_table_view>();
  for(int i = 0; i < 10; i++){
    v3.id[i] = i + 1;
    v3.value[i] = 100 + (i % 3) * 13;
  }

  table positions = table::create("postions");
  positions.add_column<int>("ID");
  positions.add_column<float>("x");
  positions.add_column<float>("y");
  positions.resize(10);
  auto v4 = positions.get_view<position_table_view>();
  for(int i = 0; i < 10; i++){
    v4.id[i] = i + 1;
    v4.x[i] = (float)(i % 5 + (i + 3) % 13);
    v4.y[i] = (float)((i + 4) % 5 + (i + 41) % 13);
  }

  entities.print_csv();
  positions.print_csv();

  for(int j = 0; j < 3; j++)
  {
    table_index x_index(positions, j);
    position_table_view view2 = x_index.get_view<position_table_view>();
    assert(view2.count == v4.count);
    for(size_t i = 1; i < view2.count; i++){
      printf("%i %f %f\n", view2.id[i], view2.x[i], view2.y[i]);
      if(j == 0)
	assert(view2.id[i - 1] < view2.id[i]);
      if(j == 1)
	assert(view2.x[i - 1] < view2.x[i]);
      if(j == 2)
	assert(view2.y[i - 1] < view2.y[i]);
    }
    printf("\n");

    x_index.dispose();
  }
  
  printf("test table PASS\n");
}


size_t root = 0;
size_t cons_type = 0;
size_t name_type = 0;
size_t string_type = 0;
size_t symbol_type = 0;
size_t integer_type = 0;
size_t symbol_ref_type = 0;
void printit(size_t id, const char * name, void * userptr){
  if(strcmp(name, "root") == 0){
      root = id;
  }else if(strcmp(name, "cons_type") == 0){
    cons_type = id;
  }else if(strcmp(name, "name_type") == 0){
    name_type = id;
  }else if(strcmp(name, "string_type") == 0){
    string_type = id;
  }else if(strcmp(name, "symbol_type") == 0){
    symbol_type = id;
  }else if(strcmp(name, "integer_type") == 0){
    integer_type = id;
  }else if(strcmp(name, "symbol_ref_type") == 0){
    symbol_ref_type = id;
  }
  printf("%i %s %i\n", id, name, userptr);
}

void get_string_value(size_t id, char * outbuffer){
  size_t * buf = (size_t *) outbuffer;
  while(id != 0){
    ccons fst = bigtalk_get_ccons(id);
    assert(fst.type == string_type);
    buf[0] = fst.value;
    buf++;
    id = fst.next;
    
  }
}

void print_conses(size_t id){
  char nameis[100];
  printf("(", id);
  
  bool fst = true;
  while(id != 0){
    if(!fst)
      printf(" ");
    
    fst =false;
    ccons fst = bigtalk_get_ccons(id);
    if(fst.type == cons_type){
      print_conses(fst.value);
    }else if(fst.type == name_type){

      get_string_value(fst.next, nameis);
      printf("name%i:%s",id, nameis);
      break;
    }else if(fst.type == symbol_ref_type){

      ccons nxt2 = bigtalk_get_ccons(fst.value);
      ccons nxt = bigtalk_get_ccons(nxt2.next);
      get_string_value(nxt.next, nameis);
      printf("%s", nameis);
     
    }else if(fst.type == integer_type){
      printf("%i", fst.value);
    }else{
      printf("[%i(%i %i %i)]", fst.value, fst.type, fst.id, id);
    }
    id = fst.next;
  }
  printf(")");
}

void test_bigtalk(){
  auto bt = bigtalk_initialize();
  
  bigtalk_iterate_meta(bt, printit, NULL);
  
  cons cs[3];
  for(cons & con : cs){
    con = bt->ast.add_cons();
  }
  for(cons & con : cs){
    con.dispose();
  }

  bigtalk_iterate_meta(bt, printit, NULL);
  assert(root != 0);
  //bigtalk_get_cons(bt, root);
  print_conses(root);
  printf("\n");
  ccons adding_cons = bigtalk_get_ccons(23);
  while(adding_cons.next != 0){
    adding_cons = bigtalk_get_ccons(adding_cons.next);
  }
  ccons newcons = ccons();
  newcons.value = 111;
  newcons.type = integer_type;
  bigtalk_set_ccons(&newcons);

  adding_cons.next = newcons.id;
  bigtalk_set_ccons(&adding_cons);
  print_conses(root);
  printf("\n");
  bigtalk_eval(19);
  newcons.value = 112;
  bigtalk_set_ccons(&newcons);
  bigtalk_eval(19);
  newcons.value = 113;
  bigtalk_set_ccons(&newcons);
  bigtalk_eval(19);
  printf("\n");
  //a.print();
  printf("Bigtalk test PASS\n");
}
  
int test(){
  allocator_test();
  test_view();
  test_table();
  test_bigtalk();
  return 0;
}
