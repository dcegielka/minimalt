#include "map.h"
#include <stdlib.h>

void map_init(struct map *map) {
  map->key     = NULL;
  map->keylen  = 0;
  map->lesser  = NULL;
  map->greater = NULL;
}

void map_free(struct map *map) {
  if (map->keylen != 0) {
    free(map->key);
    map->keylen = 0;
  } else {
    map->key = 0;
  }

  if (map->lesser) {
    map_free(map->lesser);
    free(map->lesser);
  }

  if (map->greater) {
    map_free(map->greater);
    free(map->greater);
  }
}

static int compareKeys(void *key1, size_t keylen1, void *key2, size_t keylen2) {
  int comparison = keylen1 - keylen2;

  if (comparison == 0) {
    if (keylen1 == 0) {
      return key1 - key2;
    } else {
      return memcmp(key1, key2, keylen1);
    }
  }

  return comparison;
}

void *map_get(struct map *map, void *key, size_t keylen) {
  if (map->key == NULL) {
    return NULL;
  }

  for (;;) {
    int comparison = compareKeys(key, keylen, map->key, map->keylen);

    if (comparison < 0) {
      if (map->lesser) {
        map = map->lesser;
      } else {
        return NULL;
      }
    } else if (comparison > 0) {
      if (map->greater) {
        map = map->greater;
      } else {
        return NULL;
      }
    } else {
      return map->value;
    }
  }
}

static error setKey(struct map *map, void *key, size_t keylen, void *value) {
  if (keylen == 0) {
    map->key = key;
  } else {
    if (!(map->key = malloc(keylen))) {
      return "malloc failure";
    }
  }

  map->keylen = keylen;
  map->value  = value;

  memcpy(map->key, key, keylen);

  return NULL;
}

error map_set(struct map *map, void *key, size_t keylen, void *value) {
  if (map->key == NULL) {
    return setKey(map, key, keylen, value);
  }

  for (;;) {
    int comparison = compareKeys(key, keylen, map->key, map->keylen);

    if (comparison < 0) {
      if (map->lesser) {
        map = map->lesser;
      } else {
        map->lesser = malloc(sizeof *map->lesser);

        if (!map->lesser) {
          return "malloc failure";
        }

        return setKey(map->lesser, key, keylen, value);
      }
    } else if (comparison > 0) {
      if (map->greater) {
        map = map->greater;
      } else {
        map->greater = malloc(sizeof *map->greater);

        if (!map->greater) {
          return "malloc failure";
        }

        return setKey(map->greater, key, keylen, value);
      }
    } else {
      return setKey(map, key, keylen, value);
    }
  }
}

