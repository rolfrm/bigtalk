struct bigtalk_context;
bigtalk_context * bigtalk_initialize();
void bigtalk_iterate_meta(bigtalk_context *, void (* f)(size_t id, const char * name, void * userdata), void * userdata);
void bigtalk_get_cons(bigtalk_context * bt, size_t id, void (* f)(size_t id, size_t next, size_t type, size_t value, void * userdata), void * userdata);
