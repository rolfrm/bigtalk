#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include <assert.h>
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

void test_bigtalk(){
  current_table = table::create("cons");
  current_table.add_column<size_t>("value");
  current_table.add_column<size_t>("type");
  current_table.add_column<size_t>("next");
  current_table.add_column<bool>("free");

  free_table = table::create("free_cons");
  free_table.add_column<size_t>("index");
  
  
  ast a = ast();
  a.build();
  

  cons cs[3];
  for(cons & con : cs){
    con = a.add_cons();
  }
   for(cons & con : cs){
     con.dispose();
  }
  
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
