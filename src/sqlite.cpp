#include <sqlite3.h>
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


void sqlite_run_test(){

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
}
