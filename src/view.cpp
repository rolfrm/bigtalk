#include <stdexcept>
#include <stdlib.h>
#include <assert.h>
#include "view.hpp"
void view_base::check_range(size_t index){
  if(index >= count)
    throw std::runtime_error("Index out of range");
}

void view_base::check_flag(){
  assert(flags == view_mem_flag);
}
