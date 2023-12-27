#include <cstring>
#include "bindfs.h"
#include <cstdio>
#include <errno.h>
#include <unistd.h>
#include <string>

class stating{
    public:

        stating(const stating&) = delete; // to avoid :  stating sting2 = sting1

        void operator=(const stating&) = delete; // sting2 = sting1


        sqlite3_stmt *stmt;
        stating(const char* query){
            int rc = sqlite3_prepare_v2(sqldb, query, -1, &stmt, NULL);
            if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(sqldb));
            }

        }
        
        int bind(int pos, const char* var){
            int rc = sqlite3_bind_text(stmt, pos, var, -1, SQLITE_TRANSIENT );
            if (rc != SQLITE_OK) {
                fprintf(stderr, "Failed to bind text: %s\n", sqlite3_errmsg(sqldb));
                return rc;
            }
            return rc;

        }

        int step(){
            return sqlite3_step(stmt);
        }

        ~stating(){
            sqlite3_finalize(stmt);
        }

};

class transact{

    bool need_rollback = false;

    public:

        transact(const transact&) = delete; 

        void operator=(const transact&) = delete; 

        transact(){
            int rc;
            stating sting_b("BEGIN");
            if((rc = sting_b.step()) != SQLITE_DONE){
                fprintf(stderr, "Failed to BEGIN %s\n", sqlite3_errmsg(sqldb));
                abort();
            }
            need_rollback = true;
        }

        int commit(){
            int rc;
            stating sting_b("COMMIT");
            if((rc = sting_b.step()) != SQLITE_DONE){
                fprintf(stderr, "Failed to COMMIT: %s\n", sqlite3_errmsg(sqldb));
                return rc;
            }
            need_rollback = false;
            return rc;
        }

        ~transact(){
            if(need_rollback){
                int rc;
                stating sting_b("ROLLBACK");
                if((rc = sting_b.step()) != SQLITE_DONE){
                    fprintf(stderr, "Failed to ROLLBACK: %s\n", sqlite3_errmsg(sqldb));
                    abort();
                }
                need_rollback = false;
            }
        }

};


export int bindfs_unlink(const char *path)
{
    // select
    static const char SQL_TARGET[] = "SELECT actual_fullpath FROM main_mapping WHERE virtual_fullpath = depenc( ? ) AND virtual_fullpath is NOT NULL";
    // DELETE
    static const char SQL_FILE_DELETE[] = "DELETE FROM main_mapping WHERE virtual_fullpath = depenc( ? )";

    if (strncmp(path, mrgsrcprefix, lenmrgsrcprefix) == 0) {
        path += lenmrgsrcprefix;

        int rc;

        std::string deltarget;

        // 0 BEGIN
        {
            transact trans;
        
        // 1 SELECT 
        {
            stating sting(SQL_TARGET);
            sting.bind(1, path);

            if((rc = sting.step()) == SQLITE_ROW){
                deltarget = (char*)sqlite3_column_text(sting.stmt, 0);
            }else{
                return -ENOENT;
            }
        }

        // 2 DELETE
        {
            stating sting(SQL_FILE_DELETE);
            sting.bind(1, path);

            if((rc = sting.step()) != SQLITE_DONE){
                return -EPERM;
            }
        }

        // 3 COMMIT
        
            trans.commit();
        }


        // 4 ACTUAL DELETE 
        if (strncmp(deltarget.c_str(), fallbackd, lenfallbackd) == 0) {
            delete_file(deltarget.c_str(), &unlink);
        }

        return 0;
    }


    return delete_file(path, &unlink);
}