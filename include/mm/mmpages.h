enum PageStatusFlags {
  PAGE_MAPPED = 1 << 1,
  PAGE_LOCKED = 1 << 2,
};

struct Page {
  PageStatusFlags flags;
}
