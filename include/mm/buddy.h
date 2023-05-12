#include <stddef.h>
#include <stdint.h>

typedef enum BuddyFlags {
  BUDDY_USED = 1 << 1,
  BUDDY_IS_PAGE = 1 << 2,
  BUDDY_ZONE_NORMAL = 1 << 3,
  BUDDY_ZONE_HIGH = 1 << 4,
} BuddyFlags;

struct BuddyNode {
  struct BuddyNode *left_child;
  struct BuddyNode *right_child;
  void *addr;
  BuddyFlags buddy_flags;
};

struct BuddyAllocationHeader {};
