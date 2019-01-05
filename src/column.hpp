
class column_base {
public:
  const char * name;
  bool is_index;
  virtual const std::type_info & get_column_type() = 0;
  virtual void resize(size_t new_row_count) = 0;
  virtual void * get_view() = 0;
};

template<typename T>
class column : public column_base{
public:

  void * operator new(size_t s, column<T>*& self){
    for(size_t i = 0; i < s / sizeof(column<T>); i++){
      self[i] = column<T>();
    }
    return self;
  }

  virtual const std::type_info & get_column_type(){
    return typeid(T);
  }

  T * data;
  size_t current_count;
  size_t current_capacity;

  void ensure_size(size_t new_row_count){
    if(new_row_count < current_capacity)
      return;
    
    size_t next_capacity =  (new_row_count + 1) * 2;
    data = array.resize(data, next_capacity);
    current_capacity = next_capacity;    
  }
  
  virtual void resize(size_t new_row_count){
    size_t prev_count = new_row_count;
    ensure_size(new_row_count);
    current_count = new_row_count;
    for(size_t i = prev_count; i < new_row_count; i++){
      data[i] = T();
    }
  }

  virtual void * get_view(){
    return data;
  }
};
