#include "crc32.h"

#define BIT_REVERSED_CRC32 0xEDB88320

static uint32_t crc32Table[256];
static int tableInitialized = 0;

void crc32Init(void)
{
    uint32_t polynomial = BIT_REVERSED_CRC32;

    for (uint32_t i = 0; i < 256; i++)
    {
        uint32_t crc = i;

        for (uint32_t j = 0; j < 8; j++)
        {
            crc = (crc >> 1)^((crc & 1) ? polynomial : 0);
        }
        crc32Table[i] = crc;
    }
    tableInitialized = 1;
}

uint32_t crc32(const uint8_t *data, size_t len)
{
    if(!tableInitialized)
    {
        crc32Init();
    }

    uint32_t crc = 0xffffffff;

    for (size_t i = 0; i < len; i++)
    {
        uint8_t byte = data[i];
        uint32_t index = (crc ^ byte) & 0xff;
        crc = (crc >> 8) ^ crc32Table[index];
    }
    return crc ^ 0xffffffff;
}