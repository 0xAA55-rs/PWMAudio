/*
 * my_string.c
 *
 *  Created on: Jun 1, 2025
 *      Author: 0xaa55
 */

#include "my_string.h"
#include <inttypes.h>

#pragma GCC optimize ("no-tree-loop-distribute-patterns")

void *memset(void * dst, int val, size_t len)
{
  uint32_t *ptr_dst = dst;
  if (((size_t)dst & 0x3) == 0)
  {
    union {
      uint8_t u8[4];
      uint32_t u32;
    } v4;
    v4.u8[0] = (uint8_t) val;
    v4.u8[1] = (uint8_t) val;
    v4.u8[2] = (uint8_t) val;
    v4.u8[3] = (uint8_t) val;
    while (len >= 4)
    {
      *ptr_dst ++ = v4.u32;
      len -= 4;
    }
  }
  uint8_t *ptr_dst_ = (uint8_t *)ptr_dst;
  while (len >= 1)
  {
    *ptr_dst_ ++ = (uint8_t) val;
    len -= 1;
  }
  return dst;
}

void *memcpy(void *__restrict dst, const void *__restrict src, size_t len)
{
  uint32_t *ptr_dst = dst;
  const uint32_t *ptr_src = src;
  if (dst == src) return dst;
  if (((size_t)dst & 0x3) == 0 && ((size_t)src & 0x3) == 0)
  {
    while (len >= 4)
    {
      *ptr_dst++ = *ptr_src++;
      len -= 4;
    }
  }
  uint8_t *ptr_dst_ = (uint8_t *)ptr_dst;
  uint8_t *ptr_src_ = (uint8_t *)ptr_src;

  while (len >= 1)
  {
    *ptr_dst_++ = *ptr_src_++;
    len -= 1;
  }
  return dst;
}

void *memmove(void * dst, const void * src, size_t len)
{
  uint32_t *ptr_dst = dst;
  const uint32_t *ptr_src = src;
  if (dst == src) return dst;
  if (dst < src)
  {
    return memcpy(dst, src, len);
  }
  else
  {
    uint32_t *ptr_dst_end = (uint32_t *)((uint8_t*)dst + len);
    uint32_t *ptr_src_end = (uint32_t *)((uint8_t*)src + len);
    if (((size_t)ptr_dst_end & 0x3) == 0 && ((size_t)ptr_src_end & 0x3) == 0)
    {
      while (len >= 4)
      {
        *--ptr_dst = *--ptr_src;
        len -= 4;
      }
    }
    uint8_t *ptr_dst_end_ = (uint8_t *)ptr_dst_end;
    uint8_t *ptr_src_end_ = (uint8_t *)ptr_src_end;
    while (len >= 1)
    {
      *--ptr_dst_end_ = *--ptr_src_end_;
      len -= 1;
    }
  }
  return dst;
}


