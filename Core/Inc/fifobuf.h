/*
 * fifobuf.h
 *
 *  Created on: May 30, 2025
 *      Author: 0xaa55
 */

#ifndef INC_FIFOBUF_H_
#define INC_FIFOBUF_H_

#include <stddef.h>
#include <inttypes.h>

typedef struct fifobuf_s
{
  size_t position;
  size_t length;
  size_t capacity;
  uint8_t buffer[0];
}fifobuf;

void fifobuf_init(fifobuf *fb, size_t capacity);
size_t fifobuf_write(fifobuf *fb, void *data, size_t len);
size_t fifobuf_read(fifobuf *fb, void *buffer, size_t len);
size_t fifobuf_peek(fifobuf *fb, void *buffer, size_t len);
void *fifobuf_map_read(fifobuf *fb, size_t len);
void *fifobuf_map_write(fifobuf *fb, size_t len);
size_t fifobuf_get_remaining_space(fifobuf *fb);
void fifobuf_clear(fifobuf *fb);

#define DEFINE_FIFOBUF(prefix, name, size) union { fifobuf name; uint8_t _placeholder[size + sizeof(fifobuf)]; } prefix

#endif /* INC_FIFOBUF_H_ */
