#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "afdx.h"
#include "crc32.h"
#include "arpa/inet.h"
#include "esp_log.h"

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

void AfdxParsePayload(const AfdxFrame_t *frame, int payloadLen, int *missionCode, float *altitude, uint32_t *timeMs)
{
    char payloadCpy[50];

    if (payloadLen >= sizeof(payloadCpy))
    {
        payloadLen = sizeof(payloadCpy) - 1;
    }

    memcpy(payloadCpy, frame->payload, payloadLen);
    payloadCpy[payloadLen] = '\0';
    ESP_LOGI("PARSER", "Gross payload: %s", payloadCpy);
    
    int parsed = sscanf (payloadCpy, "M%d|ALT:%f ft|T:%lu ms", missionCode, altitude, timeMs);
    if (parsed != 3)
    {
        ESP_LOGW("PARSER", "Error regarding payload parsing: '%s'", payloadCpy);
        *missionCode = -1;
        *altitude = -1.0f;
        *timeMs = 0;
    }
}