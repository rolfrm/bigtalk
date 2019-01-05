
class Allocator{
public:

  void * alloc0(size_t byte_size);
  void * realloc(void *, size_t new_size);
  void free(void * ptr);
  char * clone(const char * ptr);
  
  template<typename T>
  T * create(size_t count){
    return (T *)alloc0(count * sizeof(T));
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
    free((void *) p);
  }

  template<typename T>
  T * clone(const T * ptr, size_t count){
    T * out = create<T>(count);
    for(size_t i = 0; i < count; i++)
      out[i] = ptr[i];
    return out;
  }
};

extern Allocator array;
