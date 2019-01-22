#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include <assert.h>
#include <typeinfo>
#include <new>
#include <stdexcept>
#include "allocator.hpp"
#include "typed_box.hpp"
#include "view.hpp"
#include "column.hpp"
#include "table.hpp"
#include "bigtalk.hpp"

table current_table;
table free_table;
void cons::check_freed(){
  cons_view view = current_table.get_view<cons_view>();
  if(view.freed[index])
    throw new std::runtime_error("Cannot access a freed node");
}

cons::cons(size_t index): index(index){

}
cons::cons(){

}

void cons::set_type(cons tp){
  cons_view view = current_table.get_view<cons_view>();
  view.type[index] = tp.index;
}
cons cons::get_type(){
  cons_view view = current_table.get_view<cons_view>();
  return cons(view.type[index]);
}

cons cons::get_next(){
  cons_view view = current_table.get_view<cons_view>();
  return cons(view.next[index]);    
}

size_t & cons::get_value(){
  cons_view view = current_table.get_view<cons_view>();
  return view.value[index];    
}

cons cons::get_value_as_cons(){
  return cons(get_value());
}

void cons::set_value(cons cns){
  get_value() = cns.index;
}
  
void cons::set_value(size_t value){
  cons_view view = current_table.get_view<cons_view>();
  view.value[index] = value;
}
  
void cons::set_next(cons next){
  cons_view view = current_table.get_view<cons_view>();
  view.next[index] = next.index;
}

bool cons::operator!=(cons & b){
  return index != b.index;
}
bool cons::operator==(cons & b){
  return index == b.index;
}

size_t cons::get_index(){
  return index;
}

struct free_view{
  size_t count;
  view<size_t> index;
};

void cons::dispose(){
  disposed = true;
  auto view = current_table.get_view<cons_view>();
  view.freed[index] = true;
  auto free_index = free_table.add_row();
  auto free_col = free_table.get_view<free_view>();
  free_col.index[free_index] = index;
}

cons ast::add_cons(){

  size_t index;
  if(free_table.get_row_count() > 0){
    auto free_col = free_table.get_view<free_view>();
    index = free_col.index[free_col.count - 1];
    free_table.resize(free_table.get_row_count() - 1);
  }else{
    index = current_table.add_row();
  }
  cons_view view = current_table.get_view<cons_view>();
  view.value[index] = 0;
  view.next[index] = 0;
  view.type[index] = 0;
  view.freed[index] = 0;
  return cons(index);
}

void ast::print(){
  current_table.print_csv();
}

ast * ast::current;

cons print_thing(cons args){
  printf("hello world ");
  while(args != ast::current->null){
    cons val = args.get_value_as_cons();
    if(val.get_type() == ast::current->integer_type){
      printf("%i ", val.get_value());
    }else{
      printf("?? (%i)", val.get_type());
    }
    args = args.get_next();
  }
  printf("\n");
  return ast::current->null;
}

cons add_things(cons args){
  size_t sum = 0;
  while(args != ast::current->null){
    cons val = args.get_value_as_cons();
    if(val.get_type() == ast::current->integer_type){
      sum += val.get_value();
    }else{
      throw new std::runtime_error("invalid value type for '+'");
    }
    args = args.get_next();
  }

  auto result = ast::current->add_cons();
  result.set_value(sum);
  result.set_type(ast::current->integer_type);
  printf("Sum result: %i\n", sum);
  return result;
}

cons ast::stringify(const char * str){
  int len = strlen(str) + 1;
  size_t size_len  = (len - 1) / sizeof(size_t) + 1;
  size_t newv[size_len] = {0};
  memcpy(newv, str, len);
  cons start = add_cons();
  cons it = start;
  for(size_t i = 0; i < size_len; i++){
    it.set_value(newv[i]);
    it.set_type(string_type);
    cons it2 = add_cons();
    it.set_next(it2);
    it = it2;
  }
  return start;
}

void ast::root_add(cons thing){
  auto new_end = add_cons();
  root_end.set_next(new_end);
  root_end = new_end;
  
  root_end.set_value(thing);
  root_end.set_type(cons_type);
}

void ast::build(){
  current = this;
  null = add_cons();
  root = add_cons();
  root_end = root;
  
  type_type = add_cons();
  symbol_type = add_cons();
  integer_type = add_cons();
  integer_type.set_type(type_type);
  cons_type = add_cons();
  name_type = add_cons();
  names = add_cons();
  names_last = names;
  fcn_type = add_cons();
  string_type = add_cons();
  
  cons print_symbol = add_cons();
  print_symbol.set_type(symbol_type);
  
  {
    cons print_fcn = add_cons();
    print_fcn.set_type(fcn_type);
    print_fcn.set_value((size_t) &print_thing);
    print_symbol.set_value(print_fcn);

    cons print_symbol_name = add_cons();
    print_symbol_name.set_type(name_type);
    print_symbol_name.set_value(print_symbol);
    print_symbol_name.set_next(stringify("printprintprintt"));
    root_add(print_symbol_name);
    
  }


  
  cons add_fcn = add_cons();
  add_fcn.set_type(fcn_type);
  add_fcn.set_value((size_t) &add_things);

  cons add_symbol = add_cons();
  add_symbol.set_type(symbol_type);
  add_symbol.set_value(add_fcn);
  {
    cons add_symbol_name = add_cons();
    add_symbol_name.set_type(name_type);
    add_symbol_name.set_value(add_symbol);
    add_symbol_name.set_next(stringify("+"));
    root_add(add_symbol_name);
  }
  
  symbol_type.set_type(type_type);
  type_type.set_type(type_type);
  cons_type.set_type(type_type);
  root.set_type(cons_type);

  try{
    // (print 123 1233 12333 (+ 15 32))
    cons code1 = add_cons();
    code1.set_value(print_symbol);
    code1.set_type(cons_type);

    cons code2 = add_cons();
    code2.set_value(123);
    code2.set_type(integer_type);
    code1.set_next(code2);

    cons code3 = add_cons();
    code3.set_value(1233);
    code3.set_type(integer_type);
    code2.set_next(code3);
    code3.set_next(null);

    cons code4 = add_cons();
    code4.set_value(12333);
    code4.set_type(integer_type);
    code3.set_next(code4);

    cons code5 = add_cons();
    code5.set_value(add_fcn);
    code5.set_type(cons_type);

    cons code6 = add_cons();
    code6.set_type(integer_type);
    code6.set_value(15);
    code5.set_next(code6);

    cons code7 = add_cons();
    code7.set_type(integer_type);
    code7.set_value(32);
    code6.set_next(code7);

    cons code8 = add_cons();
    code8.set_type(cons_type);
    code8.set_value(code5);
    
    code4.set_next(code8);
    
    eval(code1);
    root_add(code1);

  }catch (std::runtime_error e){
    printf("ERROR: %s\n", e.what());
  }
}

cons ast::eval(cons code){
  if(code.get_type() != cons_type){
    return code;
  }
    
  cons fcn = code.get_value_as_cons();
  if(fcn.get_type() == symbol_type){
    fcn = fcn.get_value_as_cons();
  }
  if(fcn.get_type() != fcn_type)
    throw std::runtime_error("can only execute function types");
  cons arg0 = null;
  cons first = null;
  cons next = code.get_next();
  while(next != null){
    cons val;
    if(next.get_type() == cons_type){
      val = eval(next.get_value());
    }else{
      val = eval(next);
    }

    cons arg= add_cons();

    arg.set_value(val);
    arg0.set_next(arg);
    arg.set_type(cons_type);
    if(first == null)
      first = arg;
    arg0 = arg;
    next = next.get_next();
  }
  
  cons (* fptr)(cons args) = (cons (*)(cons args)) fcn.get_value();
  return fptr(first);
}

bigtalk_context::~bigtalk_context(){

}

void bigtalk_context::make_current(){
  ast::current = &this->ast;
  free_table = this->free_table;
  current_table = this->current_table;
}

bigtalk_context * bigtalk_initialize(){
  current_table = table::create("cons");
  current_table.add_column<size_t>("value");
  current_table.add_column<size_t>("type");
  current_table.add_column<size_t>("next");
  current_table.add_column<bool>("free");

  free_table = table::create("free_cons");
  free_table.add_column<size_t>("index");

  auto * bt = new bigtalk_context();
  bt->free_table = free_table;
  bt->current_table = current_table;
  bt->ast = ast();
  bt->make_current();
  bt->ast.build();
  return bt;
}

void bigtalk_iterate_meta(bigtalk_context * bt, void (* f)(size_t id, const char * name, void * userptr), void * userptr){
  ast * ast = &bt->ast;
  f(ast->null.get_index(), "null", userptr);
  f(ast->fcn_type.get_index(), "fcn_type", userptr);
  f(ast->cons_type.get_index(), "cons_type", userptr);
  f(ast->integer_type.get_index(), "integer_type", userptr);
  f(ast->type_type.get_index(), "type_type", userptr);
  f(ast->root.get_index(), "root", userptr);
  f(ast->symbol_type.get_index(), "symbol_type", userptr);
  f(ast->name_type.get_index(), "name_type", userptr);
  
}

void bigtalk_get_cons(__attribute__((unused)) bigtalk_context * bt, size_t id, void (* f)(size_t id, size_t next, size_t type, size_t value, void * userdata), void * userdata){

  cons cns = cons(id);
  f(cns.get_index(), cns.get_next().get_index(), cns.get_type().get_index(), cns.get_value(), userdata);
}

ccons bigtalk_get_ccons(size_t id){
  
  cons cns = cons(id);
  ccons c;
  c.id = id;
  c.value = cns.get_value();
  c.type = cns.get_type().get_index();
  c.next = cns.get_next().get_index();
  return c;
}
