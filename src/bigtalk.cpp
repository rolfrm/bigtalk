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

void ast::build(){
  current = this;
  cons null = add_cons();
  cons root = add_cons();
  root.set_next(null);
  cons type = add_cons();
  cons symbol_type = add_cons();
  integer_type = add_cons();
  integer_type.set_type(type);
  cons_type = add_cons();
  fcn_type = add_cons();
  cons print_fcn = add_cons();
  print_fcn.set_type(fcn_type);
  print_fcn.set_value((size_t) &print_thing);

  cons add_fcn = add_cons();
  add_fcn.set_type(fcn_type);
  add_fcn.set_value((size_t) &add_things);

  symbol_type.set_type(type);
  type.set_type(type);
  cons_type.set_type(type);
  root.set_type(cons_type);

  try{
    // (print 123 1233 12333 (+ 15 32))
    cons code1 = add_cons();
    code1.set_value(print_fcn);
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
  }catch (std::runtime_error e){
    printf("ERROR: %s\n", e.what());
  }
}

cons ast::eval(cons code){
  if(code.get_type() != cons_type){
    return code;
  }
    
  cons fcn = code.get_value_as_cons();
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
