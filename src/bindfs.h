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

static const char cache_check[] = "/cache_check";
static const int lencache_check = sizeof(cache_check)-1;

static const char fallbackd[] = "fallback";
static const int lenfallbackd = sizeof(fallbackd)-1;

static const char remoted_[] = "/mounts/remote_";
static const int lenremoted_ = sizeof(remoted_)-1;

static const char latestsuffix[] = "_latest";
static const int lenlatestsuffix = sizeof(latestsuffix)-1;



static const char cache_check_readme[] = "/mounts/cache_check_readme.txt";


extern sqlite3 *sqldb;
export int delete_file(const char *path, int (*target_delete_func)(const char *));
export int bindfs_unlink(const char *path);