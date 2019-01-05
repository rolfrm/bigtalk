class table{
  struct table_info{
    column_base ** columns;
    size_t column_count;
    size_t row_count;
    char * name;
  };

  void push_column(column_base * col);
  table_info * info;

public:

  
  table();
  table(const char * name);
  void print_csv();
  size_t get_column_count();
  size_t get_row_count(){
    return info->row_count;
  }
  column_base * get_column(size_t index);
  
  template<typename T>
  column_base * add_column(const char * name){
    column<T> * new_column = array.create<column<T>>(1);
    new (new_column) column<T>();
    new_column->name = array.clone(name, strlen(name) + 1);
    push_column(new_column);
    return new_column;
  }

  void resize(size_t new_row_count);
  size_t add_row();
  void dispose();
  
  static table create(const char * name);
  
  typed_box current_view;
  
  template<typename T>
  void get_view(T & theview){
    T * newview = &theview;
    void ** ptrs = (void **) newview;
    
    view<int> * views = (view<int> *) (ptrs + 1);
    
    ((size_t *) ptrs)[0] = info->row_count;

    for(size_t i = 0; i < info->column_count; i++){
      views[i].check_flag();

      views[i] = view<int>((int *)info->columns[i]->get_view(), info->row_count);
    }
  }

  template<typename T>
  T get_view(){
    T out = T();
    get_view(out);
    return out;
  }
};

class table_index{
  size_t column;
  table t;
  size_t * current_index;
  size_t current_index_size;

  void update_index();
public:
  table_index(table t, size_t column){
    this->t = t;
    this->column = column;
    current_index_size = 0;
  }

  template<typename T>
  void get_view(T & outview){
    size_t cnt = t.get_row_count();
    if(cnt > current_index_size){
      current_index = array.resize(current_index, cnt);
      current_index_size = cnt;
    }
    t.get_view<T>(outview);
    T * newview = &outview;
    void ** ptrs = (void **) newview;
    view<int> * views = (view<int> *) (ptrs + 1);
    for(size_t i = 0; i < t.get_column_count(); i++){
      views[i] = views[i].with_index(current_index);
    }
    update_index(); 
  }
  
  template<typename T>
  T get_view(){
    T out = T();
    get_view<T>(out);
    return out;
  }

  void dispose(){
    array.free(current_index);
  }

};
