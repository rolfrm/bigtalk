struct bigtalk_context;

typedef struct{
  size_t id;
  size_t value;
  size_t type;
  size_t next;
}ccons;


bigtalk_context * bigtalk_initialize();
typedef void (* iterate_meta_f)(size_t id, const char * name, void * userptr);
void bigtalk_iterate_meta(bigtalk_context * bt,iterate_meta_f f , void * userptr);
ccons bigtalk_get_ccons(size_t id);
