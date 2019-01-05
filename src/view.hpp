
class view_base{
private:
  const static size_t view_mem_flag = 123456711;
  size_t flags;
protected:
  size_t count;
  void check_range(size_t index);

  view_base(){
    flags = view_mem_flag;
  }

public:
  void check_flag();
};

template<typename T>
class view : public view_base{

  T * ptr;
  size_t * index;
public:

  view(T * ptr, size_t count) : view(){
    this->ptr = ptr;
    this->count = count;
  }

  view(){
    index = NULL;
  }

  T& operator[](size_t index){
    check_range(index);
    if(this->index == NULL)
      return ptr[index];
    return ptr[this->index[index]];
  }
  T * raw(){
    return ptr;
  }

  view<T> with_index(size_t * index){
    view<T> v2 = *this;
    v2.index = index;
    return v2;
  }
  
};
