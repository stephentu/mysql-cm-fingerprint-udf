#include <my_global.h>
#include <my_sys.h>

#include <mysql.h>
#include <m_ctype.h>
#include <m_string.h>
#include <stdlib.h>

#include <ctype.h>
#include <math.h>

#define ARRAY_LIST_INIT_CAP 10

typedef struct array_list {
  unsigned int count;
  unsigned int capacity;
  long long *elements;
} array_list_t;

array_list_t *new_array_list();
void add_array_list(array_list_t *list, long long elem);
void clear_array_list(array_list_t *list);
void free_array_list(array_list_t **ptr); 

typedef struct cm_fingerprint_ctx {
  array_list_t *list_ctx;
  char *res_ctx;
} cm_fingerprint_ctx_t;

#define P_TO_LIST(p) (((cm_fingerprint_ctx_t*)p)->list_ctx)
#define P_TO_RES(p)  (((cm_fingerprint_ctx_t*)p)->res_ctx)

#ifdef HAVE_DLOPEN

my_bool cm_fingerprint_init(UDF_INIT *initid, UDF_ARGS *args, char *message);

void cm_fingerprint_clear(UDF_INIT *initid, char *is_null, char *message);

void cm_fingerprint_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *message);

char *cm_fingerprint(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);

void cm_fingerprint_deinit(UDF_INIT *initid);

my_bool cm_fingerprint_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  if (args->arg_count != 1) {
    strcpy(message, "One arg only please");
    return 1;
  }

  args->arg_type[0] = INT_RESULT;

  cm_fingerprint_ctx_t *ctx = (cm_fingerprint_ctx_t*) malloc(sizeof(cm_fingerprint_ctx_t));  
  ctx->list_ctx = new_array_list();
  ctx->res_ctx  = NULL;

  initid->ptr = (char*) ctx;
  initid->maybe_null = 1;
  initid->max_length = 0x1 << 16; // 65KB
  initid->const_item = 0;

  return 0;
}

void cm_fingerprint_clear(UDF_INIT *initid, char *is_null, char *message) {
  clear_array_list(P_TO_LIST(initid->ptr));
}

void cm_fingerprint_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *message) {
  add_array_list(P_TO_LIST(initid->ptr), *((long long *)args->args[0]));
}

char *cm_fingerprint(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error) {
  
  array_list_t *list = P_TO_LIST(initid->ptr);
  unsigned int num_entries = ceil(log2(list->count));
  unsigned int bytes_needed = 2 * num_entries * sizeof(long long);
  unsigned int i, j, p;
  if (bytes_needed > initid->max_length) {
    *error = 1;
    return NULL;
  }

  P_TO_RES(initid->ptr) = (char*) malloc(sizeof(char) * bytes_needed);
  memset(P_TO_RES(initid->ptr), 0, bytes_needed);
  *length = bytes_needed;

  long long *fp_table = (long long *) P_TO_RES(initid->ptr);

  for (i = 0; i < list->count; i++) {
    j = 0;
    p = 0x1;
    while (j < num_entries) {
      if (i & p)
        fp_table[2 * j + 1] ^= list->elements[i];
      else
        fp_table[2 * j] ^= list->elements[i];
      j++;
      p <<= 1;
    }
  }
  
  return P_TO_RES(initid->ptr); 
}

void cm_fingerprint_deinit(UDF_INIT *initid) {
  free_array_list(&P_TO_LIST(initid->ptr));
  free(P_TO_RES(initid->ptr));
  free(initid->ptr);
}

array_list_t *new_array_list() {
  array_list_t *list = (array_list_t*) malloc(sizeof(array_list_t));
  list->count = 0;
  list->elements = (long long*) malloc(sizeof(long long) * ARRAY_LIST_INIT_CAP);
  list->capacity = ARRAY_LIST_INIT_CAP;
  return list;
}

void add_array_list(array_list_t *list, long long elem) {
  if (list->count == list->capacity) {
    long long *newbuf = (long long*) malloc(sizeof(long long) * list->capacity * 2);
    memcpy(newbuf, list->elements, list->count * sizeof(long long));
    free(list->elements);
    list->capacity *= 2;
    list->elements = newbuf;
  }
  list->elements[list->count++] = elem;
}

void clear_array_list(array_list_t *list) {
  list->count = 0;
}

void free_array_list(array_list_t **ptr) {
  free((*ptr)->elements);
  free(*ptr);
  *ptr = NULL;
}

#endif /* HAVE_DLOPEN */
