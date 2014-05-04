#ifndef MAP_H
#define MAP_H
#include <stdint.h>
#include <string.h>
#include "error.h"

typedef uint64_t mapkey;

struct mapitem {
  mapkey  key;
  void   *val;
};

struct map {
  size_t          capacity,
                  used;
  struct mapitem *items;
};

void   map_init(struct map *map);
void  *map_get(struct map *map, mapkey key);
error  map_set(struct map *map, mapkey key, void *item);

#endif
