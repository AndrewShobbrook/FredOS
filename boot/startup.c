#include "mm/buddy.h"
#include <limine.h>
#include <stddef.h>
#include <stdint.h>

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
};

static volatile struct limine_efi_system_table_request sys_table_request = {
    .id = LIMINE_EFI_SYSTEM_TABLE_REQUEST,
    .revision = 0,
};

static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0,
};

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST, .revision = 0};

enum MEMORY_ALLOC_TYPE {
  MEMORY_ALLOC_TYPE_BUDDY = 0,
  MEMORY_ALLOC_TYPE_SLAB = 1,
  MEMORY_ALLOC_TYPE_HYBRID_BUDDY_SLAB = 2,
};

enum MEMORY_USAGE_TYPE {
  MEMORY_USAGE_TYPE_KERNEL = 0,
  MEMORY_USAGE_TYPE_USERSPACE = 1,
  MEMORY_USAGE_TYPE_DRIVER = 2
};

void *memcpy(void *dest, const void *src, size_t n) {
  uint8_t *pdest = (uint8_t *)dest;
  const uint8_t *psrc = (const uint8_t *)src;

  for (size_t i = 0; i < n; i++) {
    pdest[i] = psrc[i];
  }

  return dest;
}

void *memset(void *s, int c, size_t n) {
  uint8_t *p = (uint8_t *)s;

  for (size_t i = 0; i < n; i++) {
    p[i] = (uint8_t)c;
  }

  return s;
}

void *memmove(void *dest, const void *src, size_t n) {
  uint8_t *pdest = (uint8_t *)dest;
  const uint8_t *psrc = (const uint8_t *)src;

  if (src > dest) {
    for (size_t i = 0; i < n; i++) {
      pdest[i] = psrc[i];
    }
  } else if (src < dest) {
    for (size_t i = n; i > 0; i--) {
      pdest[i - 1] = psrc[i - 1];
    }
  }

  return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const uint8_t *p1 = (const uint8_t *)s1;
  const uint8_t *p2 = (const uint8_t *)s2;

  for (size_t i = 0; i < n; i++) {
    if (p1[i] != p2[i]) {
      return p1[i] < p2[i] ? -1 : 1;
    }
  }

  return 0;
}

void hcf() {
  asm("cli");
  for (;;) {
    asm("hlt");
  }
}

struct info_block_header {
  uint32_t usable_area; // Amount of area available after 4096 byte header.
  enum MEMORY_ALLOC_TYPE memory_alloc_type;
  enum MEMORY_USAGE_TYPE memory_usage_type;
};

struct Kernel {
  uint8_t *sys_framebuffer_addr;
  uint64_t sys_framebuffer_width;
  uint64_t sys_framebuffer_height;
  uint64_t sys_framebuffer_pitch;
  uint16_t sys_framebuffer_bpp;
  uint16_t padding; // PADDING

  struct info_block_header
      *allocation_info_block; // This is set to a 4096 byte block on the
                              // largest block of usable memory, will hold
                              // information for buddies and slabs
};

static volatile struct Kernel kernel = {0};

void _start(void) {
  if (framebuffer_request.response == NULL) {
    hcf();
  }
  struct limine_framebuffer_response framebuffers =
      *framebuffer_request.response;
  if (framebuffers.framebuffer_count == 0 || framebuffers.framebuffers == NULL)
    return;
  memset(framebuffers.framebuffers[0]->address, 0xffee66, 100000);

  if (rsdp_request.response->address == NULL)
    return;

  if (memmap_request.response == NULL)
    return;
  struct limine_memmap_response memmap_response = *memmap_request.response;

  uint64_t largest_usable_memory = 0;
  uint8_t *largest_usable_memory_addr = NULL;
  for (uint64_t i = 0; i < memmap_response.entry_count; i++) {
    if (memmap_response.entries[i]->type == LIMINE_MEMMAP_USABLE) {
      largest_usable_memory = memmap_response.entries[i]->length;
      largest_usable_memory_addr = (uint8_t *)memmap_response.entries[i]->base;
    }
  }
  if (largest_usable_memory / 4096 > 4) {
    kernel.allocation_info_block =
        (struct info_block_header *)largest_usable_memory_addr;
  }

  hcf();
}
