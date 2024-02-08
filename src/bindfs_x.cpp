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
    static const char SQL_TARGET[] = "SELECT actual_fullpath, jginfo_rd_torrent_folder, depdec(jginfo_rclone_cache_item) FROM main_mapping WHERE virtual_fullpath = depenc( ? ) AND actual_fullpath is NOT NULL";
    // DELETE
    static const char SQL_FILE_DELETE[] = "DELETE FROM main_mapping WHERE virtual_fullpath = depenc( ? )";
    // read torrent folder (files only)
    static const char SQL_TORRENT_FOLDER_LIST[] = "SELECT virtual_fullpath FROM main_mapping WHERE jginfo_rd_torrent_folder = ? AND actual_fullpath is NOT NULL";
    // delete all items (liklely folders) of this release (check if jellyfin delete them before)
    // static const char SQL_DELETE_CORRESPONDING_RELEASE_ITEMS[] = "DELETE FROM main_mapping WHERE jginfo_rd_torrent_folder = ?";

    if (strncmp(path, mrgsrcprefix, lenmrgsrcprefix) == 0) {
        path += lenmrgsrcprefix;

        // remap path to the first found "/" because virtual can now have suffix for readdir filtering (ex: virtual_bdmv)
        if(*path != '\0' && *path != '/' && strchr(path, '/') != NULL){
            path = strchr(path, '/');
        }

        if (*path == '\0' || strchr(path, '/') == NULL){
            return -EPERM;
        }

        int rc;

        std::string deltarget;
        std::string torrentfolder;
        std::string torrentfile;


        // 0 BEGIN
        {
            transact trans;
        
        // 1 SELECT 
        {
            stating sting(SQL_TARGET);
            sting.bind(1, path);

            if((rc = sting.step()) == SQLITE_ROW){
                deltarget = (char*)sqlite3_column_text(sting.stmt, 0);
                torrentfolder = sqlite3_column_type(sting.stmt, 1) == SQLITE_NULL ? "" : (char*)sqlite3_column_text(sting.stmt, 1);
                torrentfile = sqlite3_column_type(sting.stmt, 2) == SQLITE_NULL ? "" : (char*)sqlite3_column_text(sting.stmt, 2);
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


        // 4 ACTUAL DELETE if fallback file
        if (strncmp(deltarget.c_str(), fallbackd, lenfallbackd) == 0) {
            delete_file(deltarget.c_str(), &unlink);
        }else if(strncmp(torrentfolder.c_str(), remoted_, lenremoted_) == 0){
            // if no more file entries having this torrentfolder -> del source torrent file  (-> and del all virtual refering to the same release folder -> parent folder empty ? ): not needed if jellyfin deletes folders
            //printf("there is a remote torrentfolder corresponding to this file / -> if statement is working");
            // 0 BEGIN
            {
                transact trans;
            
            // 1 SELECT 
            {
                stating sting(SQL_TORRENT_FOLDER_LIST);
                sting.bind(1, torrentfolder.c_str());

                printf("\n -> Virtual deletion of a file %s \n..inside this torrent release: %s\n", torrentfile.c_str(), torrentfolder.c_str());

                if((rc = sting.step()) != SQLITE_ROW){
                    // it means there is no more file corresponding to this release
                    if(torrentfile != ""){
                        // we use unlink here as the torrent file path is not relative from the mount (contrary to fallback files)
                        unlink(torrentfile.c_str());
                        printf("\n -> Deletion of torrent release %s \n..having this last file: %s\n", torrentfolder.c_str(), torrentfile.c_str());
                        // this will trigger the deletion of the torrent release in the RD mount (rclone_rd fork)
                    }
                    //{
                        // dell all entries refering to the same release folder (likely subfolders of a multiple file release if it is)
                      //  stating delsting(SQL_DELETE_CORRESPONDING_RELEASE_ITEMS);
                        //delsting.bind(1, torrentfolder.c_str());

                    //}
                }
            }

            // 3 COMMIT
            
                trans.commit();
            }



        }else{
            delete_file(deltarget.c_str(), &unlink);
        }

        return 0;
    }


    return delete_file(path, &unlink);
}