#ifndef _CRC32_H_
#define _CRC32_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @brief INIT CRC32
void crc32Init(void);

/// @brief CALCULATE CHECKSUM
/// @param data INPUT DATA
/// @param len DATA SIZE
/// @return CHECKSUM RESULT
uint32_t crc32(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif
#endif