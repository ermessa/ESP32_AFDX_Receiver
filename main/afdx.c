#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "afdx.h"
#include "crc32.h"
#include "arpa/inet.h"

bool AfdxValidateFrame(const AfdxFrame_t *frame)
{
    if (frame->id < 100 || frame->id > 200)
    {
        return false;
    }

    const uint8_t *rawData = (const uint8_t *)frame;
    uint32_t crcCalc = Crc32(rawData, sizeof(AfdxFrame_t) - sizeof(uint32_t));

    uint32_t crcReceived = ntohl(frame->crc32);

    return crcCalc == crcReceived;
}

void AfdxPrintFrame(const AfdxFrame_t *frame)
{
    printf("----------AFDX Frame Received-----------\n");
    printf("VL ID       : %u\n", frame->id);
    printf("timestamp   : %lu ms\n", frame->timestamp);
    printf("payload     : %s\n", frame->payload);
    printf("checksum    : 0x%lx\n", frame->crc32);
    printf("----------------------------------------\n");
}

void AfdxParsePayload(const AfdxFrame_t *frame, int *missionCode, float *altitude, uint32_t *timeMs)
{
    if (!frame || !missionCode || !altitude || !timeMs) return;

    sscanf (frame->payload, "M%d ALT:%fft T:%lu", missionCode, altitude, timeMs);
}