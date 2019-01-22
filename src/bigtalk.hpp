
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

extern table free_table;
extern table current_table;

class cons{
  size_t index;
  bool disposed;
public:
  void check_freed();
  cons(size_t index);
  cons();
  void set_type(cons tp);
  cons get_type();
  cons get_next();
  size_t & get_value();
  cons get_value_as_cons();
  void set_value(cons cns);
  void set_value(size_t value);
  void set_next(cons next);
  size_t get_index();
  bool operator!=(cons & b);
  bool operator==(cons & b);
  void dispose();
};

class ast {
public:
  cons add_cons();
  void print();
  cons null;
  cons fcn_type;
  cons cons_type;
  cons integer_type;
  cons type_type;
  cons root;
  cons root_end;
  cons symbol_type;
  cons string_type;
  cons name_type;
  cons names;
  cons names_last;
  void build();
  cons eval(cons code);
  void root_add(cons c);
  cons stringify(const char * str);
  static ast * current;
};



class bigtalk_context{

  virtual ~bigtalk_context();
public:
  table free_table;
  table current_table;
  ::ast ast;

  void make_current();
};

typedef struct{
  size_t id;
  size_t value;
  size_t type;
  size_t next;
}ccons;

bigtalk_context * bigtalk_initialize();
void bigtalk_iterate_meta(bigtalk_context * bt, void (* f)(size_t id, const char * name, void * userptr), void * userptr);
void bigtalk_get_cons(bigtalk_context * bt, size_t id, void (* f)(size_t id, size_t next, size_t type, size_t value, void * userdata), void * userdata);
ccons bigtalk_get_ccons(bigtalk_context * bt, size_t id);

