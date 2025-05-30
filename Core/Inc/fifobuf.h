/*
 * fifobuf.h
 *
 *  Created on: May 30, 2025
 *      Author: 0xaa55
 */

#ifndef INC_FIFOBUF_H_
#define INC_FIFOBUF_H_

#ifndef FIFOBUF_SIZE
#define FIFOBUF_SIZE 512
#endif

#include <inttypes.h>

typedef struct fifobuf_s
{
  size_t position;
  size_t length;
  uint8_t buffer[FIFOBUF_SIZE];
}fifobuf;

void fifobuf_init(fifobuf *fb);
size_t fifobuf_write(fifobuf *fb, void *data, size_t len);
size_t fifobuf_read(fifobuf *fb, void *buffer, size_t len);
size_t fifobuf_peek(fifobuf *fb, void *buffer, size_t len);
void *fifobuf_map_read(fifobuf *fb, size_t len, size_t *remaining_space);
void *fifobuf_map_write(fifobuf *fb, size_t len, size_t *remaining_space);
size_t fifobuf_get_remaining_space(fifobuf *fb);
void fifobuf_clear(fifobuf *fb);

#endif /* INC_FIFOBUF_H_ */
