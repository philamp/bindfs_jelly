#include <sqlite3.h>

#ifdef __cplusplus
#define export extern "C"
#else
#define export
#endif

static const char srcprefix[] = "/actual";
static const int lensrcprefix = sizeof(srcprefix)-1;

static const char mrgsrcprefix[] = "/virtual";
static const int lenmrgsrcprefix = sizeof(mrgsrcprefix)-1;

static const char fallbackd[] = "fallback";
static const int lenfallbackd = sizeof(mrgsrcprefix)-1;


extern sqlite3 *sqldb;
export int delete_file(const char *path, int (*target_delete_func)(const char *));
export int bindfs_unlink(const char *path);