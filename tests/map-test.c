// ../map.o
#include "../map.h"
#include "assert.h"

int main() {
  {
    struct map map;

    map_init(&map);

    char key1[] = "This is a really long key.",
         key2[] = "Another key.";

    assertError("map_set (1)", map_set(&map, key1, sizeof key1, (void*)123));
    assertEq("Key is now retrievable", map_get(&map, key1, sizeof key1), (void*)123);

    assertError("map_set (2)", map_set(&map, key2, sizeof key2, (void*)456));
    assertEq("New key is now retrievable", map_get(&map, key2, sizeof key2), (void*)456);
    assertEq("Old key is still retrievable (1)", map_get(&map, key1, sizeof key1), (void*)123);

    assertError("map_set existing", map_set(&map, key1, sizeof key1, (void*)789));
    assertEq("Key has updated value", map_get(&map, key1, sizeof key1), (void*)789);
    assertEq("Old key is still retrievable (2)", map_get(&map, key2, sizeof key2), (void*)456);

    assertError("map_set small key (1)", map_set(&map, (void*)1, 0, (void*)10));
    assertError("map_set small key (2)", map_set(&map, (void*)2, 0, (void*)20));
    assertEq("Can retrieve short key (1)", map_get(&map, (void*)1, 0), (void*)10);
    assertEq("Can retrieve short key (2)", map_get(&map, (void*)2, 0), (void*)20);

    map_free(&map);
  }
}

