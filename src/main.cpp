#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <bigtalk_api.h>
#define BUFFER_SIZE 256
#define UNUSED __attribute__((unused))
int test();
void print_meta(size_t id, const char * name, UNUSED void * userptr){
  printf("{ \"call\":\"get_meta\", \"id\":\"%i\", \"name\":\"%s\" }\n", id, name);
}

void print_get_cons(size_t id, size_t next, size_t type, size_t value, UNUSED void * userptr){
  printf("{ \"call\":\"get\", \"id\":\"%i\", \"next\":\"%i\", \"type\":\"%i\", \"value\":\"%i\"}\n", id, next, type, value);
}

void websocket_mode(){
  
  auto bt_ctx = bigtalk_initialize();
  setbuf( stdout, NULL );
  //printf( "Query String: %s\n", getenv( "QUERY_STRING" ) );
  print_meta(0, "", NULL);
  char * line = NULL;
  size_t line_len;
  while( 1 ) {
    // read a char at a time, blocks until each char is available
    size_t l = getline(&line, &line_len, stdin);
    line[l - 1] = 0;
    l -= 1;
    if(strcmp("get_meta", line) == 0){
      bigtalk_iterate_meta(bt_ctx, print_meta, NULL);
      continue;
    }else{
      char * start = strstr(line, "get_sub");
      if(start != NULL){

	char * subline = start + strlen("get_sub ");
	char * err;
      next_it:
	long long id = strtoll(subline, &err, 10);
	if(err == subline)
	  goto error;
	bigtalk_get_cons(bt_ctx, (size_t) id, print_get_cons, NULL);
	char * next = strstr(subline, ",");
	if(next != NULL){
	  subline = next + 1;
	  goto next_it;
	}
	
	continue;
      }

      printf( "got line: '%s'\n", line );
      continue;
    error:
      printf("error parsing %s\n", line);
      
    }
  }

}
int main(){
  test();
  websocket_mode();

  return 0;
}
