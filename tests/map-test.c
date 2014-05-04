// ../map.o
#include "../map.h"
#include "assert.h"

int main() {
  {
    struct map map;

    map_init(&map);

    assertEq("Map starts with zero used", map.used, 0);
    assertEq("Map starts with zero capacity", map.capacity, 0);

    assertError("map_set (1)", map_set(&map, 0, (void*)123));
    assertEq("Map now has one used", map.used, 1);
    assertEq("Map now has one capacity", map.capacity, 1);
    assertEq("Key is now retrievable", map_get(&map, 0), (void*)123);

    assertError("map_set (2)", map_set(&map, 123, (void*)456));
    assertEq("Map now has two used", map.used, 2);
    assertEq("Map now has two capacity", map.capacity, 2);
    assertEq("New key is now retrievable", map_get(&map, 123), (void*)456);
    assertEq("Old key is still retrievable (1)", map_get(&map, 0), (void*)123);

    assertError("map_set existing", map_set(&map, 0, (void*)789));
    assertEq("Map still has two used", map.used, 2);
    assertEq("Map now has four capacity", map.capacity, 4);
    assertEq("Key has updated value", map_get(&map, 0), (void*)789);
    assertEq("Old key is still retrievable (2)", map_get(&map, 123), (void*)456);
  }
}

