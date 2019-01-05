
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
  void build();
  void eval(cons code);
};

