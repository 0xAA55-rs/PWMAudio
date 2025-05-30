/*
 * fifobuf.c
 *
 *  Created on: May 30, 2025
 *      Author: 0xaa55
 */

#include "fifobuf.h"
#include <stdlib.h>

#define MAX_SHIFT_BUFFER 32

void fifobuf_init(fifobuf *fb)
{
  memset(fb, 0, sizeof *fb);
}

size_t fifobuf_write(fifobuf *fb, void *data, size_t len)
{
  size_t ret = min(fifobuf_get_remaining_space(fb), len);
  if (!ret) return 0;

  uint8_t *ptr = data;
  size_t back_writable = (sizeof fb->buffer) - fb->position;
  size_t front_writable = fb->position;
  size_t to_write = ret;
  size_t back_write = min(back_writable, ret);
  if (!back_write)
  {
    back_write = min(front_writable, ret);
    fb->position = 0;
  }

  memcpy(&fb->buffer[fb->position], ptr, back_write);
  fb->position += back_write;
  fb->length += back_write;
  ptr += back_write;
  to_write -= back_write;

  if (to_write)
  {
    memcpy(&fb->buffer[0], ptr, to_write);
    fb->position = to_write;
    fb->length += to_write;
  }

  return ret;
}

size_t fifobuf_read(fifobuf *fb, void *buffer, size_t len)
{
  size_t read = fifobuf_peek(fb, buffer, len);
  fb->position += read;
  if (fb->position >= sizeof fb->buffer) fb->position -= sizeof fb->buffer;
  fb->length -= read;
  if (!fb->length) fb->position = 0;
  return read;
}

size_t fifobuf_peek(fifobuf *fb, void *buffer, size_t len)
{
  size_t ret = min(fb->length, len);
  if (!ret) return 0;

  uint8_t *ptr = data;
  size_t to_read = ret;
  size_t back_readable = (sizeof fb->buffer) - fb->position;
  size_t front_readable = fb->position;
  size_t back_read = min(back_readable, ret);
  if (back_read)
  {
    memcpy(&fb->buffer[fb->position], ptr, back_read);
    ptr += back_read;
    to_read -= back_read;
  }
  size_t front_read = min(front_readable, to_read);
  if (front_read)
  {
    memcpy(&fb->buffer[0], ptr, front_read);
  }
  return ret;
}

static void _memswap(void *buf1, void *buf2, size_t len)
{
  uint8_t *ptr1 = buf1;
  uint8_t *ptr2 = buf2;
  size_t remain = len;
  if ((size_t)ptr1 & 0x03 == 0 && (size_t)ptr2 & 0x03 == 0)
  {
    while (remain >= 4)
    {
      uint32_t t = *(uint32_t*)ptr1;
      *(uint32_t*)ptr1 = *(uint32_t*)ptr2;
      *(uint32_t*)ptr2 = t;
      ptr1 += 4;
      ptr2 += 4;
      remain -= 4;
    }
  }
  while (remain)
  {
    uint8_t t = *ptr1;
    *ptr1 = *ptr2;
    *ptr2 = t;
    ptr1 ++;
    ptr2 ++;
    remain --;
  }
}

int _fifobuf_is_data_contiguous(fifobuf *fb)
{
  size_t back_space = (sizeof fb->buffer) - fb->position;
  return back_space >= fb->length;
}

void _fifobuf_shift_data(fifobuf *fb, ssize_t shift)
{
  static const size_t sizeof_buffer = sizeof fb->buffer;
  uint8_t shift_buf[MAX_SHIFT_BUFFER];
  if (shift == 0) return;
  if (shift < 0)
  {
    while (shift < 0)
    {
      if (_fifobuf_is_data_contiguous(fb) && fb->position > 0)
      {
        ssize_t shiftable = min(-shift, (ssize_t)fb->position);
        size_t new_position = fb->position - (size_t)shiftable;
        memmove(&fb->buffer[new_position], &fb->buffer[fb->position], fb->length);
        fb->position = new_position;
        shift += shiftable;
      }
      if (shift < -MAX_SHIFT_BUFFER)
      {
        _fifobuf_shift_data(fb, -MAX_SHIFT_BUFFER);
        shift += MAX_SHIFT_BUFFER;
      }
      else if (shift < 0)
      {
        size_t to_shift = (size_t)-shift;
        memcpy(shift_buf, &fb->buffer[0], to_shift);
        memmove(&fb->buffer[0], &fb->buffer[to_shift], sizeof_buffer - to_shift);
        memcpy(&fb->buffer[sizeof_buffer - to_shift], shift_buf, to_shift);
        fb->position = (fb->position + sizeof_buffer - to_shift) % sizeof_buffer;
        break;
      }
    }
  }
  else
  {
    while (shift > 0)
    {
      size_t back_space = sizeof_buffer - (fb->position + fb->length);
      if (_fifobuf_is_data_contiguous(fb) && back_space > 0)
      {
        size_t shiftable = min((size_t)shift, back_space);
        size_t new_position = fb->position + shiftable;
        memmove(&fb->buffer[new_position], &fb->buffer[fb->position], fb->length);
        fb->position = new_position;
        shift -= shiftable;
      }
      if (shift > MAX_SHIFT_BUFFER)
      {
        _fifobuf_shift_data(fb, MAX_SHIFT_BUFFER);
        shift -= MAX_SHIFT_BUFFER;
      }
      else if (shift > 0)
      {
        memcpy(shift_buf, &fb->buffer[sizeof_buffer - shift], shift);
        memmove(&fb->buffer[shift], &fb->buffer[0],sizeof_buffer - shift);
        memcpy(&fb->buffer[0], shift_buf, shift);
        fb->position = (fb->position + shift) % sizeof_buffer;
        break;
      }
    }
  }
}

void _fifobuf_realign_data(fifobuf *fb)
{
  if (_fifobuf_is_data_contiguous(fb))
  {
    memmove(fb->buffer, &fb->buffer[fb->position], fb->length);
    fb->position = 0;
  }
  else
  {
    if (fb->position < MAX_SHIFT_BUFFER)
      _fifobuf_shift_data(fb, -(ssize_t)fb->position);
    else
      _fifobuf_shift_data(fb, (sizeof fb->buffer) - fb->position);
  }
}

void *fifobuf_map_read(fifobuf *fb, size_t len)
{
  if (fb->length < len) return NULL;

  size_t back_readable = (sizeof fb->buffer) - fb->position;

  if (back_readable < len)
    _fifobuf_realign_data(fb);
  void *ret = &fb->buffer[fb->position];
  fb->position += len;
  if (fb->position == sizeof fb->buffer) fb->position = 0;
  fb->length -= len;
  return ret;
}

void *fifobuf_map_write(fifobuf *fb, size_t len)
{
  size_t remain = fifobuf_get_remaining_space(&fb);
  if (remain < len) return NULL;

  size_t tail_writable = (fb->position + fb->length) % sizeof fb->buffer;

  if (tail_writable < len)
    _fifobuf_realign_data(fb);
  void *ret = &fb->buffer[fb->position + fb->length];
  fb->length += len;
  return ret;
}

size_t fifobuf_get_remaining_space(fifobuf *fb)
{
  return (sizeof fb->buffer) - fb->length;
}

void fifobuf_clear(fifobuf *fb)
{
  fb->position = 0;
  fb->length = 0;
}
