#include "map.h"
#include <stdlib.h>

static size_t insert(struct mapitem *items, size_t nitems, mapkey key, void *val) {
  size_t mask = nitems - 1;

  for (size_t i = key & mask;; i = (i + 1) & mask) {
    struct mapitem item = items[i];

    if (item.key == key) {
      items[i].val = val;
      return 0;
    } else if (!item.val) {
      items[i].key = key;
      items[i].val = val;
      return 1;
    }
  }
}

void map_init(struct map *map) {
  map->capacity = 0;
  map->used     = 0;
  map->items    = NULL;
}

void map_free(struct map *map) {
  map->capacity = 0;
  map->used     = 0;
  free(map->items);
  map->items = NULL;
}

void *map_get(struct map *map, mapkey key) {
  if (!map->items) {
    return NULL;
  }

  return map->items[key & (map->capacity - 1)].val;
}

error map_set(struct map *map, mapkey key, void *val) {
  if (!map->items) {
    map->capacity     = 1;
    map->used         = 1;
    map->items        = malloc(1 * sizeof *map->items);

    if (!map->items) {
      return "malloc failure";
    }

    map->items[0].key = key;
    map->items[0].val = val;

    return NULL;
  }

  if (map->used == map->capacity) {
    size_t          oldCapacity = map->capacity,
                    newCapacity = map->capacity << 1;
    struct mapitem *oldItems    = map->items,
                   *newItems    = malloc(newCapacity * sizeof *newItems);

    if (!newItems) {
      return "malloc failure";
    }

    memset(newItems, 0, newCapacity * sizeof *newItems);

    for (size_t i = 0; i < oldCapacity; ++i) {
      struct mapitem item = oldItems[i];

      if (item.val != 0) {
        insert(newItems, newCapacity, item.key, item.val);
      }
    }

    free(oldItems);

    map->capacity = newCapacity;
    map->items    = newItems;
  }
 
  map->used += insert(map->items, map->capacity, key, val);

  return NULL;
}

