#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "afdx.h"

bool AfdxValidateFrame(const AfdxFrame_t *frame)
{
    return frame->id >= 100 && frame->id <= 200;
}

void AfdxPrintFrame(const AfdxFrame_t *frame)
{
    printf("----------AFDX Frame Received-----------\n");
    printf("VL ID       : %u\n", frame->id);
    printf("timestamp   : %lu ms\n", frame->timestamp);
    printf("payload     : %s\n", frame->payload);
    printf("----------------------------------------\n");
}