#include "utils.h"

#include <inttypes.h>
#include <stdio.h>

GString *qsubber_utils_calculate_hash_for_file(const char *file) {
  GString *retval;
  guint64 hash, fsize;
  FILE *handle;

  handle = fopen(file, "r");

  fseek(handle, 0, SEEK_END);
  fsize = ftell(handle);
  fseek(handle, 0, SEEK_SET);

  hash = fsize;

  for(gint64 tmp = 0, i = 0; i < 65536/sizeof(tmp) && fread((char*)&tmp, sizeof(tmp), 1, handle); hash += tmp, i++);
  fseek(handle, (long)MAX(0, fsize - 65536), SEEK_SET);
  for(gint64 tmp = 0, i = 0; i < 65536/sizeof(tmp) && fread((char*)&tmp, sizeof(tmp), 1, handle); hash += tmp, i++);

  retval = g_string_new(NULL);

  g_string_printf(retval, "%" PRIx64, hash);

  return retval;
}