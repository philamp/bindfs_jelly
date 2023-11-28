#include <sqlite3ext.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
SQLITE_EXTENSION_INIT1
int supercomparelist (void* pArg,int al,const void* av,int bl,const void* bv);
void depth_encode(sqlite3_context* ctx, int nbargs, sqlite3_value** args);
void depth_decode(sqlite3_context* ctx, int nbargs, sqlite3_value** args);
int supercomparedepth (void* pArg,int al,const void* av,int bl,const void* bv);
int supercompare_sub (void* pArg,int al,const void* av,int bl,const void* bv, int start_i);

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
  sqlite3_create_collation_v2(db, "sclist", SQLITE_UTF8, NULL, supercomparelist, NULL);
  
  sqlite3_create_collation_v2(db, "scdepth", SQLITE_UTF8, NULL, supercomparedepth, NULL);

  sqlite3_create_function_v2(db, "depenc", 1, SQLITE_UTF8, NULL, depth_encode, NULL, NULL, NULL);

  sqlite3_create_function_v2(db, "depdec", 1, SQLITE_UTF8, NULL, depth_decode, NULL, NULL, NULL);

  return rc;
}

void depth_decode(sqlite3_context* ctx, int nbargs, sqlite3_value** args){
    const unsigned char* v_orig = sqlite3_value_text(args[0]);
    int len = strlen(v_orig);
    if (len < 6) {
        const char* empty = "";
        sqlite3_result_text(ctx, empty, -1, SQLITE_STATIC); // Return the result
    }

    char* dest = malloc(len - 4); // Allocate memory for the new string
    if (!dest) {
        sqlite3_result_error_nomem(ctx);
        return; // Return if memory allocation fails
    }

    strcpy(dest, v_orig + 5); // Copy starting from the 6th character
    sqlite3_result_text(ctx, dest, -1, free); // Return the result
}

void depth_encode(sqlite3_context* ctx, int nbargs, sqlite3_value** args){
    const unsigned char* v_orig = sqlite3_value_text(args[0]);
    int len = strlen((char*)v_orig);
    int nbsl = 0;
    
    for(int i = 0; i < len; i++) {
        if (v_orig[i] == '/') {
            nbsl++;
        }
    }

    // Create a modifiable copy of v
    unsigned char* v = malloc(len + 1); // +1 for null terminator
    if (!v) {
        sqlite3_result_error_nomem(ctx);
        return;
    }
    strcpy((char*)v, (char*)v_orig);

    if (len > 0 && v[len - 1] == '/') {
        v[len - 1] = '\0'; // Safe to modify now
        nbsl--;
        len--;
    }

    // Allocate memory for the result
    char* result = malloc(6 + len); // depth (4) + # (1) + path (len) + null term (1)
    if (!result) {
        free(v); // Free the copied string
        sqlite3_result_error_nomem(ctx);
        return;
    }

    snprintf(result, 5, "%04x", nbsl); // Convert depth to hex
    result[4] = '#'; // Append '#'
    strcpy(result + 5, (char*)v); // Copy modified v to result

    free(v); // Free the copied string
    sqlite3_result_text(ctx, result, -1, free); // Return the result

}

int supercompare_sub (void* pArg,int al,const void* av,int bl,const void* bv, int start_i){
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
    for (int i = start_i; i < ll; i++)
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

int supercomparedepth (void* pArg,int al,const void* av,int bl,const void* bv){
    return supercompare_sub (pArg,al,av,bl,bv,5);
}

int supercomparelist (void* pArg,int al,const void* av,int bl,const void* bv){
    return supercompare_sub (pArg,al,av,bl,bv,0);
}



