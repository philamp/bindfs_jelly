#include <sqlite3ext.h>
#include <stdint.h>
#include <stddef.h>
SQLITE_EXTENSION_INIT1
int supercompare (void*,int al,const void* av,int bl,const void* bv);

/* Insert your extension code here */

#ifdef _WIN32
__declspec(dllexport)
#endif
/* TODO: Change the entry point name so that "extension" is replaced by
** text derived from the shared library filename as follows:  Copy every
** ASCII alphabetic character from the filename after the last "/" through
** the next following ".", converting each character to lowercase, and
** discarding the first three characters if they are "lib".
*/
int sqlite3_extension_init(
  sqlite3 *db, 
  char **pzErrMsg, 
  const sqlite3_api_routines *pApi
){
  int rc = SQLITE_OK;
  SQLITE_EXTENSION_INIT2(pApi);
  /* Insert here calls to
  **     sqlite3_create_function_v2(),
  **     sqlite3_create_collation_v2(),
  **     sqlite3_create_module_v2(), and/or
  **     sqlite3_vfs_register()
  ** to register the new features that your extension adds.
  */
  sqlite3_create_collation_v2(db, "supercollate", SQLITE_UTF8, NULL, supercompare, NULL);


  return rc;
}

void depth_encode(sqlite3_context* ctx, int nbargs, sqlite3_value** args){
    int len = sqlite3_value_bytes(args[0]);
    const unsigned char* v = sqlite3_value_text(args[0]);
    int nbsl = 0;
    // check if len != 0
    for(int i = 0; i < len; i++){
        if (v[i] == '/'){
            nbsl++;
        };
    }
    // remove final slash
    if (v[len-1] == '/'){
        nbsl--;
        len--;
    }

    char* result = malloc(5 + len)



}

int supercompare (void* pArg,int al,const void* av,int bl,const void* bv){
    static const uint8_t C_EOS = 0xFF;
    static const uint8_t C_SLH = 0x00;
    static const uint8_t C_OLDSL = '/';
    static const uint8_t C_OLDEOS = '\\';
    static const uint8_t SLXOR = C_OLDSL ^ C_SLH;
    static const uint8_t EOXOR = C_OLDEOS ^ C_EOS;
    const char *aa = av;
    const char *bb = bv;
    uint8_t xa;
    uint8_t xb;
    int ll = (al < bl) ? al : bl;
    for (int i = 0; i < ll; i++)
    {
        // read next char. '\0' is mapped to 255, the rest is in decreasing ascii order
        xa = aa[i];
        xb = bb[i];
        if (xa == xb) {
            continue;
        }
        // swap '/' with 0
        if (xa == C_OLDSL || xa == C_SLH) xa^=SLXOR;
        if (xb == C_OLDSL || xb == C_SLH) xb^=SLXOR;
        // swap '\' with 255
        if (xa == C_OLDEOS || xa == C_EOS) xa^=EOXOR;
        if (xb == C_OLDEOS || xb == C_EOS) xb^=EOXOR;
        return (xa < xb) ? -1 : 1;
    }
    return al - bl;
}



