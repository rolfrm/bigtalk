#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <bigtalk_api.h>
#define BUFFER_SIZE 256
#define UNUSED __attribute__((unused))
int test();
void print_meta(size_t id, const char * name, UNUSED void * userptr){
  printf("{ \"call\":\"get_meta\", \"id\":\"%llu\", \"name\":\"%s\" }\n", id, name);
}

void print_get_cons(size_t id, size_t next, size_t type, size_t value){
  printf("{ \"call\":\"get\", \"id\":\"%llx\", \"next\":\"%llx\", \"type\":\"%llx\", \"value\":\"%llx\"}\n", id, next, type, value);
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
    ssize_t l = getline(&line, &line_len, stdin);
    if(l <= 0)continue;
    printf("L: %i\n", l);
    line[l - 1] = 0;
    l -= 1;
    if(strcmp("get_meta", line) == 0){
      bigtalk_iterate_meta(bt_ctx, print_meta, NULL);
      continue;
    }else if(strcmp("end", line) == 0){
      break;
    }else{
      char * start = strstr(line, "get_sub");
      if(start != NULL){

	char * subline = start + strlen("get_sub ");
	char * err;
      next_it:
	long long id = strtoll(subline, &err, 16);
	if(err == subline)
	  goto error;
	auto cc = bigtalk_get_ccons(id);
	print_get_cons(cc.id, cc.next, cc.type, cc.value);
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
int main(int argc, const char **argv){
  for(int i = 1; i < argc; i++){
    if(strcmp(argv[i], "--test") == 0){
      test();
      
      char * ptr;
      size_t size;
      FILE * newstdin = open_memstream(&ptr, &size);
      stdin =newstdin;
      fprintf(stdin, "get_meta\n");
      fprintf(stdin, "get_meta\n");
      fprintf(stdin, "get_sub 1\n");
      fprintf(stdin, "get_sub 11\n");
      fprintf(stdin, "get_sub 17\n");
      fprintf(stdin, "get_sub 25\n");
      fprintf(stdin, "end\n");
      websocket_mode();
      return 0;

    }
  }
  websocket_mode();

  return 0;
}
