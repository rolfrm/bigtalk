#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <icydb.h>
//#include <iron/full.h>
//#include <iron/utils.h>
//#include <iron/mem.h>
#include <stdlib.h>
#include <typeinfo>
//#include <memory>
#include <stdexcept>
#include <assert.h>
#define UNUSED __attribute__((unused))
int sqlcb(UNUSED void * user, int columns, char ** data, char ** name){

  for(int i = 0;i < columns; i++){
    //if( i == 1 || i == 2){
      printf("%i: '%s' %s\n", i, data[i], name[i]);
      //}
  }
  return 0;
}

void sqlite3_test3(sqlite3 * db){
  char * err = NULL;
  int rc = sqlite3_exec(db, "SELECT * from images", &sqlcb, NULL, &err);
  printf("SQLITE TEST3: %i %s\n", rc, err);
}

void sqlite3_test(sqlite3 * db){
  char * err;
  UNUSED int rc = sqlite3_exec(db, "SELECT * from images", &sqlcb, db, &err);

  sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &err);
  {
    sqlite3_stmt * stmt;
    const char * stmt_text = "SELECT image_id,width, height from images";
    sqlite3_prepare_v2(db, stmt_text, strlen(stmt_text) + 1, &stmt, NULL);
  next_row:
    int status = 0;
    
    
    while((status = sqlite3_step(stmt)) == SQLITE_BUSY){
      printf("Busy...\n");
    }
    if(status == SQLITE_ROW){
      int cols = sqlite3_column_count(stmt);
      
      for(int i = 0; i < cols; i++){
	printf("%i/%i: %i %s (%i)\n", i, cols, sqlite3_column_type(stmt, i), sqlite3_column_name(stmt,i), sqlite3_column_bytes(stmt, i));
	printf("    %i\n", sqlite3_column_int(stmt, i)); 
      }
      goto next_row;
    }
    sqlite3_finalize(stmt);
  }
  sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &err);
}


void sqlite3_test2(sqlite3 * db){
  //char * err;
  //int rc = sqlite3_exec(db, "UPDATE images width=?1 height=?2", &sqlcb, db, &err);
  //UNUSED(rc);

  {
    sqlite3_stmt * stmt;
    const char * stmt_text = "UPDATE images SET width=?1, height=?2 WHERE image_id=?3";
    int ec = sqlite3_prepare_v2(db, stmt_text, strlen(stmt_text) + 1, &stmt, NULL);
  next_row:
    int status = 0;
    int index = 589;
    sqlite3_bind_int(stmt, 1, index * 11);

    sqlite3_bind_int(stmt, 2, index * 12);
    sqlite3_bind_int(stmt, 3, 2);
    const char * err2 = sqlite3_errmsg(db);    
    while((status = sqlite3_step(stmt)) == SQLITE_BUSY){
      printf("Failure.. %s %i\n", err2, status);
      break;//return;
    }

    printf("STATUS: %i %i: %s\n", status, ec, err2);
    if(status == SQLITE_ROW){
      index++;
      sqlite3_bind_int(stmt, 1, index * 13);
      sqlite3_bind_int(stmt, 2, index * 17);
      goto next_row;
    }
    sqlite3_finalize(stmt);
  }
}

void print_stuff(void *, int, const char* msg){
  printf("SQL: %s\n", msg);
}

class Allocator{
public:

  template<typename T>
  T * create(size_t count){
    return (T *)calloc(count, sizeof(T));
  }

   template<typename T>
  T * create(){
     return create<T>(1);
  }
  
  template<typename T>
  T * resize(T * p, size_t new_size){
    return (T *) realloc(p, new_size * sizeof(T));
  }
  template<typename T>
  void free(T * p){
    std::free(p);
  }

  template<typename T>
  T * clone(const T * ptr, size_t count){
    T * out = create<T>(count);
    for(size_t i = 0; i < count; i++)
      out[i] = ptr[i];
    return out;
  }

  char * clone(const char * ptr){
    return clone(ptr, strlen(ptr) + 1);
  }

};

static Allocator array;

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


template<typename T>
class view{

  T * ptr;
  size_t count;
public:
  size_t flags;
  view(T * ptr, size_t count){
    this->ptr = ptr;
    this->count = count;
  }

  view(){
    flags = view_mem_flag;
  }

  T& operator[](size_t index){
    if(index >= count)
      throw  std::runtime_error("Index out of range");
    return ptr[index];
  }
  T * raw(){
    return ptr;
  }

  const static size_t view_mem_flag = 123456711;
};

class typed_box
{
public:
  void * ptr;
  const std::type_info * type;

  typed_box(void * ptr, const std::type_info * type){
    this->ptr = ptr;
    this->type = type;
  }

};

class table{
  struct table_info{
    column_base ** columns;
    size_t column_count;
    size_t row_count;
    char * name;
  };

  void push_column(column_base * col){
    info->columns = array.resize(info->columns,  (info->column_count += 1));
    info->columns[info->column_count - 1] = col;
    col->resize(info->row_count);
  }

  table_info * info;

  
public:


  table(): current_view(NULL, NULL){
    info = array.create<table_info>();
    info->name = NULL;
  }

  table(const char * name): table(){
    info->name = array.clone(name);
  }
  
  void print_csv(){
    printf("Table '%s'\n", info->name);
    for(size_t i = 0; i < info->column_count; i++){
      if(i != 0)
	printf(" ,");
      printf("%s", info->columns[i]->name);
    }
    printf("\n");
    for(size_t r = 0; r < info->row_count; r++){
      printf("%3i |", r);
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
	}else{
	  printf("unsupported(%s)", t.name());
	}
      }
      printf("\n");
    }

    
    printf("\n");
  }
  
  column_base * get_column(size_t index){
    if(index >= info->column_count)
      throw "Error";
    return info->columns[index];
  }

  template<typename T>
  column_base * add_column(const char * name){
    column<T> * new_column = array.create<column<T>>(1);
    new (new_column) column<T>();
    new_column->name = array.clone(name, strlen(name) + 1);
    push_column(new_column);
    return new_column;
  }

  void resize(size_t new_row_count){
    printf("new size: %i\n", new_row_count);
    for(size_t i = 0; i < info->column_count; i++)
      info->columns[i]->resize(new_row_count);
    info->row_count = new_row_count;
  }
  size_t add_row(){
    size_t r = info->row_count;
    resize(r + 1);
    printf("add_row: %i / %i\n", r, info->row_count);
    return r;
  }
  void dispose()
  {
    
  }
  
  static table create(const char * name){
    
    return table(name);
  }
  
  typed_box current_view;
  
  template<typename T>
  T * get_view(){
    /*if(current_view.type == &typeid(T)){
      printf("Readjust content..");
      ((size_t *)current_view.ptr)[0] = info->row_count;
      return (T *)current_view.ptr;
      }*/
    if(current_view.ptr != NULL){
      array.free(current_view.ptr);
      current_view = typed_box(NULL, &typeid(void *));
    }
    size_t size = sizeof(void *) + sizeof(view<int>) * info->column_count;
    if(size > sizeof(T)){
      throw std::runtime_error("Invalid size for a table view.");
    }
    printf(">> %i :: %i\n", sizeof(T), info->row_count);
    auto newview = array.create<T>();
    new(newview) T();
    current_view = typed_box(newview, &typeid(T));
    void ** ptrs = (void **) newview;
    ((size_t *) ptrs)[0] = info->row_count;
    view<int> * views = (view<int> *) (ptrs + 1);
    for(size_t i = 0; i < info->column_count; i++){
      assert(views[i].flags == view<int>::view_mem_flag);
      views[i] = view<int>((int *)info->columns[i]->get_view(), info->row_count);
    }
    return newview;
  }
};

class table_index{
  size_t column;
  table t;
public:
  table_index(table t, size_t column){
    this->t = t;
    this->column = column;
  }

};

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

void table_builder_test(){
  table t = table::create("loc");
  t.add_column<int>("id");
  t.add_column<float>("x");
  t.add_column<float>("y");
  
  
  t.get_column(0)->is_index = true;
  printf("Column name: %s %i\n", t.get_column(1)->get_column_type().name(), t.get_column(1)->get_column_type() == typeid(float));
  printf("Column name: %s\n", t.get_column(2)->get_column_type().name());
  printf("Column name: %s\n", t.get_column(0)->get_column_type().name());
  t.resize(10);
  position_table_view * v1 = t.get_view<position_table_view>();
  printf("Count: %i\n", v1->count);
  position_table_view * v2 = t.get_view<position_table_view>();
  assert(v1 == v2);
  v1->x[0] = 1234;
  v1->x[8] = 1234;
  printf("Count: %f\n", v1->x[0]);
  assert(v1->count == 10);

  table entities = table::create("entities");
  entities.add_column<int>("ID");
  entities.add_column<float>("value");
  entities.resize(10);
  auto v3 = entities.get_view<entities_table_view>();
  for(int i = 0; i < 10; i++){
    v3->id[i] = i + 1;
    v3->value[i] = 100 + (i % 3) * 13;
  }

  table positions = table::create("postions");
  positions.add_column<int>("ID");
  positions.add_column<float>("x");
  positions.add_column<float>("y");
  positions.resize(10);
  auto v4 = positions.get_view<position_table_view>();
  for(int i = 0; i < 10; i++){
    v4->id[i] = i + 1;
    v4->x[i] = (float)(i % 5 + (i + 3) % 13);
    v4->y[i] = (float)((i + 4) % 5 + (i + 41) % 13);
  }

  entities.print_csv();
  positions.print_csv();
}

extern "C"{
void usleep(int);
}

struct cons_view {
  size_t count;

  // the value of this cons.
  // may be another cons.
  view<size_t> value;
  
  // the type of this cons.
  // is another cons.
  view<size_t> type;

  // the next cons.
  view<size_t> next;
  view<bool> freed;
};

table my_table;

class cons{
  size_t index;
public:
  void check_freed(){
    cons_view * view = my_table.get_view<cons_view>();
    if(view->freed[index])
      throw new std::runtime_error("Cannot access a freed node");
  }

  cons(size_t index): index(index){

  }
  cons(){

  }

  void set_type(cons tp){
    cons_view * view = my_table.get_view<cons_view>();
    view->type[index] = tp.index;
  }
  cons get_type(){
     cons_view * view = my_table.get_view<cons_view>();
    return cons(view->type[index]);
  }

  cons get_next(){
    cons_view * view = my_table.get_view<cons_view>();
    return cons(view->next[index]);    
  }

  size_t & get_value(){
    cons_view * view = my_table.get_view<cons_view>();
    return view->value[index];    
  }

  cons get_value_as_cons(){
    return cons(get_value());
  }

  void set_value(cons cns){
    get_value() = cns.index;
  }
  
  void set_value(size_t value){
    cons_view * view = my_table.get_view<cons_view>();
    view->value[index] = value;
  }
  
  void set_next(cons next){
    cons_view * view = my_table.get_view<cons_view>();
    view->next[index] = next.index;
  }

  bool operator!=(cons & b){
    return index != b.index;
  }
  bool operator==(cons & b){
    return index == b.index;
  } 

};
void print_thing(UNUSED cons args){
  printf("hello world\n");
}
class ast {
public:
  

  cons add_cons(){
    size_t index = my_table.add_row();
    cons_view * view = my_table.get_view<cons_view>();
    printf("VIEW: %i\n", view->count);
    view->value[index] = 0;
    view->next[index] = 0;
    view->type[index] = 0;
    view->type[index] = 0;
    return cons(index);
  }

  void print(){
    my_table.print_csv();
  }

  cons null;
  cons fcn_type;
  cons cons_type;
  cons integer_type;
  void build(){
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

  void eval(cons code){
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
};

void bigtalk_v1(){
  my_table = table::create("cons");
  my_table.add_column<size_t>("value");
  my_table.add_column<size_t>("type");
  my_table.add_column<size_t>("next");
  my_table.add_column<bool>("free");
  ast a = ast();
  a.build();
  a.print();

  
  /*type.type = type;
  type.commit();

  cons symbol = a.add_cons();
  symbol.type = type;
  cons cons_type = a.add_cons();
  
  cons char_type = a.add_cons();
  char_type.type = type;
  cons name_variable = a.add_cons();
  name_variable.type = symbol;
  cons size_variable = a.add_cons();
  cons ptr_variable = a.add_cons();

  cons char_ptr_type = a.add_cons();
  char_ptr_type.type = type;
  cons char_ptr_type_1 = a.add_cons();
  char_ptr_type.next = char_ptr_type_1;
  
  cons char_ptr_type_name_1 = a.add_cons();
  char_ptr_type_name_1.type = cons_type.index;
  char_ptr_type_name_1.value = name_variable.index;

  cons char_ptr_type_name_2 = a.add_cons();
  char_ptr_type_name_1.next = char_ptr_type_name_2.index;
  //char_ptr_type_name_1.value*/
  
}

int main(){
  sqlite3_initialize();
  sqlite3_config(SQLITE_CONFIG_LOG, print_stuff, NULL);
  sqlite3_log(1, "Log test...\n");
  sqlite3 * db;
  int rc = sqlite3_open("game.sqldb", &db);
  if(rc){
    printf("ERROR\n");
  }

  printf("DB 1? %p\n", db);
 
  sqlite3_test2(db);
  sqlite3_test(db);
  
  sqlite3_close(db);
  table_builder_test();
  bigtalk_v1();

}
