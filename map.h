#ifndef MAP_H
#define MAP_H
#include <stdint.h>
#include <string.h>
#include "error.h"

struct map {
  uint8_t    *key;
  size_t      keylen;
  void       *value;
  struct map *lesser,
             *greater;
};

void   map_init(struct map *map);
void   map_free(struct map *map);
void  *map_get(struct map *map, void *key, size_t keylen);
error  map_set(struct map *map, void *key, size_t keylen, void *item);

#endif
