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

