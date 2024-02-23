#include <gmp.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "runtime/alloc.h"
#include "runtime/arena.h"
#include "runtime/header.h"

extern "C" {

REGISTER_ARENA(youngspace, YOUNGSPACE_ID);
REGISTER_ARENA(oldspace, OLDSPACE_ID);
REGISTER_ARENA(alwaysgcspace, ALWAYSGCSPACE_ID);

char *youngspace_ptr() {
  return arenaStartPtr(&youngspace);
}

char *oldspace_ptr() {
  return arenaStartPtr(&oldspace);
}

char **young_alloc_ptr() {
  return arenaEndPtr(&youngspace);
}

char **old_alloc_ptr() {
  return arenaEndPtr(&oldspace);
}

char youngspace_collection_id() {
  return getArenaCollectionSemispaceID(&youngspace);
}

char oldspace_collection_id() {
  return getArenaCollectionSemispaceID(&oldspace);
}

size_t youngspace_size(void) {
  return arenaSize(&youngspace);
}

bool youngspaceAlmostFull(size_t threshold) {
  char *nextBlock = *(char **)youngspace.block_start;
  if (nextBlock) {
    // not on the last block, so short circuit and assume that we can keep
    // allocating for now.
    return false;
  }
  ptrdiff_t freeBytes = youngspace.block_end - youngspace.block;
  size_t totalBytes
      = youngspace.num_blocks * (BLOCK_SIZE - sizeof(memory_block_header));
  return (totalBytes - freeBytes) * 100 > threshold * 95;
}

void koreAllocSwap(bool swapOld) {
  arenaSwapAndClear(&youngspace);
  arenaClear(&alwaysgcspace);
  if (swapOld) {
    arenaSwapAndClear(&oldspace);
  }
}

void setKoreMemoryFunctionsForGMP() {
  mp_set_memory_functions(koreAllocMP, koreReallocMP, koreFree);
}

__attribute__((always_inline)) void *koreAllocToken(size_t requested) {
  size_t size = (requested + 7) & ~7;
  return koreArenaAlloc(&youngspace, size < 16 ? 16 : size);
}

__attribute__((always_inline)) void *koreAllocOld(size_t requested) {
  return koreArenaAlloc(&oldspace, requested);
}

__attribute__((always_inline)) void *koreAllocTokenOld(size_t requested) {
  size_t size = (requested + 7) & ~7;
  return koreArenaAlloc(&oldspace, size < 16 ? 16 : size);
}

__attribute__((always_inline)) void *koreAllocAlwaysGC(size_t requested) {
  return koreArenaAlloc(&alwaysgcspace, requested);
}

void *koreResizeLastAlloc(void *oldptr, size_t newrequest, size_t last_size) {
  newrequest = (newrequest + 7) & ~7;
  last_size = (last_size + 7) & ~7;

  if (oldptr != *arenaEndPtr(&youngspace) - last_size) {
    MEM_LOG(
        "May only reallocate last allocation. Tried to reallocate %p to %zd\n",
        oldptr, newrequest);
    exit(255);
  }

  ssize_t increase = newrequest - last_size;
  if (arenaResizeLastAlloc(&youngspace, increase)) {
    return oldptr;
  }

  void *newptr = koreAlloc(newrequest);
  memcpy(newptr, oldptr, last_size);
  return newptr;
}

void *koreAllocMP(size_t requested) {
  auto *_new = (string *)koreAllocToken(sizeof(string) + requested);
  init_with_len(_new, requested);
  return _new->data;
}

void *koreReallocMP(void *ptr, size_t old_size, size_t new_size) {
  auto *_new = (string *)koreAllocToken(sizeof(string) + new_size);
  size_t min = old_size > new_size ? new_size : old_size;
  memcpy(_new->data, ptr, min);
  init_with_len(_new, new_size);
  return _new->data;
}

void koreFree(void *ptr, size_t size) { }

__attribute__((always_inline)) void *koreAllocInteger(size_t requested) {
  auto *result = (mpz_hdr *)koreAlloc(sizeof(mpz_hdr));
  init_with_len(result, sizeof(mpz_hdr) - sizeof(blockheader));
  return &result->i;
}

__attribute__((always_inline)) void *koreAllocFloating(size_t requested) {
  auto *result = (floating_hdr *)koreAlloc(sizeof(floating_hdr));
  init_with_len(result, sizeof(floating_hdr) - sizeof(blockheader));
  return &result->f;
}

__attribute__((always_inline)) void *koreAllocIntegerOld(size_t requested) {
  auto *result = (mpz_hdr *)koreAllocOld(sizeof(mpz_hdr));
  init_with_len(result, sizeof(mpz_hdr) - sizeof(blockheader));
  return &result->i;
}

__attribute__((always_inline)) void *koreAllocFloatingOld(size_t requested) {
  auto *result = (floating_hdr *)koreAllocOld(sizeof(floating_hdr));
  init_with_len(result, sizeof(floating_hdr) - sizeof(blockheader));
  return &result->f;
}
}
