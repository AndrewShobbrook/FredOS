#include "stddef.h"
#include "stdint.h"

struct ZoneHeader {
  void *min_addr;
  void *max_addr;
}
