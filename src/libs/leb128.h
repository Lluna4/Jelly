/*
* Brian R Taylor
* brian.taylor@bolderflight.com
* 
* Copyright (c) 2021 Bolder Flight Systems Inc
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the “Software”), to
* deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/

#ifndef SRC_LEB128_H_
#define SRC_LEB128_H_

#if defined(ARDUINO)
#include <Arduino.h>
#endif
#include <cstddef>
#include <cstdint>

namespace bfs {

std::size_t EncodeLeb128(int64_t val, uint8_t * const data,
                         const std::size_t len) {
  if (!data) {return 0;}
  bool negative = (val < 0);
  std::size_t i = 0;
  while (1) {
    /* Prevent buffer overflow */
    if (i < len) {
      uint8_t b = val & 0x7F;
      /* Ensure an arithmetic shift */
      val >>= 7;
      if (negative) {
        val |= (~0ULL << 57);
      }
      if (((val == 0) && (!(b & 0x40))) ||
          ((val == -1) && (b & 0x40))) {
        data[i++] = b;
        return i;
      } else {
        data[i++] = b | 0x80;
      }
    } else {
      return 0;
    }
  }
}

std::size_t DecodeLeb128(uint8_t const * const data, const std::size_t len,
                         int64_t * const val) {
  /* Null pointer check */
  if ((!data) || (!val)) {return 0;}
  int64_t res = 0;
  std::size_t shift = 0;
  std::size_t i = 0;
  while (1) {
    if (i < len) {
      uint8_t b = data[i++];
      uint64_t slice = b & 0x7F;
      res |= slice << shift;
      shift += 7;
      if (!(b & 0x80)) {
        if ((shift < 64) && (b & 0x40)) {
          *val = res | (-1ULL) << shift;
          return i;
        }
        *val = res;
        return i;
      }
    } else {
      return 0;
    }
  }
}

}  // namespace bfs

#endif  // SRC_LEB128_H_
