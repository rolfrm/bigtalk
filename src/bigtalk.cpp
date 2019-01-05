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
  

cons ast::add_cons(){
  size_t index = current_table.add_row();
  cons_view view = current_table.get_view<cons_view>();
  printf("VIEW: %i\n", view.count);
  view.value[index] = 0;
  view.next[index] = 0;
  view.type[index] = 0;
  view.type[index] = 0;
  return cons(index);
}

void ast::print(){
  current_table.print_csv();
}

void print_thing(__attribute__((unused)) cons args){
  printf("hello world\n");
}

void ast::build(){
  cons null = add_cons();
  cons root = add_cons();
  root.set_next(null);
  cons type = add_cons();
  cons symbol_type = add_cons();
  cons integer_type = add_cons();
  integer_type.set_type(type);
  cons_type = add_cons();
  fcn_type = add_cons();
  cons print_fcn = add_cons();
  print_fcn.set_type(fcn_type);
  print_fcn.set_value((size_t) &print_thing);
  symbol_type.set_type(type);
  type.set_type(type);
  cons_type.set_type(type);
  root.set_type(cons_type);

  try{
    cons code1 = add_cons();
    code1.set_value(print_fcn);
    code1.set_type(cons_type);

    cons code2 = add_cons();
    code2.set_value(123);
    code2.set_type(integer_type);
    code1.set_next(code2);
      
    eval(code1);
  }catch (std::runtime_error e){
      
  }
}

void ast::eval(cons code){
  if(code.get_type() != cons_type)
    throw std::runtime_error("function must be in a cons.");
    
  cons fcn = code.get_value_as_cons();
  if(fcn.get_type() != fcn_type)
    throw std::runtime_error("can only execute function types");
    
  cons next = code.get_next();
  while(next != null){

    next = next.get_next();
  }

  void (* fptr)(cons args) = (void (*)(cons args)) fcn.get_value();
  fptr(null);
    
    
}
