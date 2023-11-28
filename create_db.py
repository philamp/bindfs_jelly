#!/usr/bin/env python3
import sqlite3

# Path to your SQLite database file #TODO: to change
db_path = '/root/bindfs_jelly.db'

print("coucou")
# UPDATE new_virtual_mappings SET (virtual_fullpath = '
# WHERE virtual_fullpath COLLATE CUSTOM_COLLATE!! BETWEEN 'path/' AND 'path0';

# SQL command to create the table if it doesn't exist
create_table_sql = '''
CREATE TABLE IF NOT EXISTS main_mapping (
    virtual_fullpath TEXT PRIMARY KEY COLLATE SCLIST,
    actual_fullpath TEXT UNIQUE,
    jginfo_cache_ready BOOLEAN,
    jginfo_rd_torrent_folder TEXT,
    jginfo_rclone_cache_item TEXT
);
'''

insert_data = '''
INSERT INTO main_mapping (virtual_fullpath, actual_fullpath) VALUES 
(depenc('/a/b/d'), "/mounts/rar2fs_remote_realdebrid/movies/Walk.The.Line.2005.MULTi.1080p.BluRay.x264-NOEX/Walk.The.Line.2005.MULTi.1080p.BluRay.x264-NOEX.mkv")
, (depenc('/a/b/e'), "/mounts/rar2fs_remote_realdebrid/movies/Walk.The.Line.2005.MULTi.1080p.BluRay.x264-NOEX/Walk.The.Line.2005.MULTi.1080p.BluRay.x264-NOEX.nfo")
, (depenc('/a'), null)
, (depenc('/a/t'), null)
, (depenc('/a/b'), null)
, (depenc('/a/b/c'), null)
, (depenc('/a/v'), null)
, (depenc('/a/b/c/d'), "/mounts/rar2fs_remote_realdebrid/movies/Watcher.2022.1080p.WEBRip.x265-RARBG/Watcher.2022.1080p.WEBRip.x265-RARBG.mp4");
'''

# usage of supercustomcollate:
# SELECT * FROM main_mapping 
# WHERE virtual_fullpath COLLATE SUPERCUSTOMCOLLATE BETWEEN 'path/' AND 'path0';

# Function to create the table
def create_table():
    try:
        # Connect to the SQLite database
        conn = sqlite3.connect(db_path)
        conn.enable_load_extension(True)
        conn.load_extension("/root/dev/bindfs_jelly/sqlite_ext/supercollate.so")
        cursor = conn.cursor()
        
        # Execute the SQL command
        cursor.execute(create_table_sql)
        cursor.execute(insert_data)
        
        # Commit the changes and close the connection
        conn.commit()
        conn.close()

        print("Table created successfully.")
    except sqlite3.Error as e:
        print(f"An error occurred: {e}")

# Create the table
create_table()