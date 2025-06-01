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
  size_t head = (size_t)ptr_dst & 0x3;
  if (head)
  {
    uint8_t *ptr_dst_ = (uint8_t *)ptr_dst;
    while(head && len)
    {
      *ptr_dst_ ++ = (uint8_t) val;
      head --;
      len --;
    }
    ptr_dst = (uint32_t *)ptr_dst_;
  }
  if (len >= 4)
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
  while (len)
  {
    *ptr_dst_ ++ = (uint8_t) val;
    len --;
  }
  return dst;
}

void *memcpy(void *__restrict dst, const void *__restrict src, size_t len)
{
  uint32_t *ptr_dst = dst;
  const uint32_t *ptr_src = src;
  if (dst == src) return dst;
  size_t head = (size_t)dst & 0x3;
  if (head)
  {
    uint8_t *ptr_dst_ = (uint8_t *)ptr_dst;
    uint8_t *ptr_src_ = (uint8_t *)ptr_src;
    while(head && len)
    {
      *ptr_dst_++ = *ptr_src_++;
      head --;
      len --;
    }
    ptr_dst = (uint32_t *)ptr_dst_;
    ptr_src = (uint32_t *)ptr_src_;
  }
  while (len >= 4)
  {
    *ptr_dst++ = *ptr_src++;
    len -= 4;
  }
  if (len)
  {
    uint8_t *ptr_dst_ = (uint8_t *)ptr_dst;
    uint8_t *ptr_src_ = (uint8_t *)ptr_src;

    while (len)
    {
      *ptr_dst_++ = *ptr_src_++;
      len --;
    }
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
    size_t tail = (size_t)ptr_dst_end & 0x3;
    if (tail)
    {
      uint8_t *ptr_dst_end_ = (uint8_t *)ptr_dst_end;
      uint8_t *ptr_src_end_ = (uint8_t *)ptr_src_end;
      while (tail && len)
      {
        *--ptr_dst_end_ = *--ptr_src_end_;
        tail --;
        len --;
      }
      ptr_dst_end = (uint32_t *)ptr_dst_end_;
      ptr_src_end = (uint32_t *)ptr_src_end_;
    }
    while (len >= 4)
    {
      *--ptr_dst_end = *--ptr_src_end;
      len -= 4;
    }
    if (len)
    {
      uint8_t *ptr_dst_end_ = (uint8_t *)ptr_dst_end;
      uint8_t *ptr_src_end_ = (uint8_t *)ptr_src_end;
      while (len)
      {
        *--ptr_dst_end_ = *--ptr_src_end_;
        len --;
      }
    }
  }
  return dst;
}


