#ifndef CRC32_H_
#define CRC32_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INITIAL_CRC 0x0

uint32_t crc32(const uint8_t *data, size_t length, uint32_t crc);

#ifdef __cplusplus
}
#endif

#endif // CRC32_H_