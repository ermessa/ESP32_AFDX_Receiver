#ifndef _AFDX_H_
#define _AFDX_H_

#include "stdint.h"

/// @brief AFDX STRUCTURE
typedef struct
{
    uint16_t id;        //VIRTUAL LINK ID
    uint32_t timestamp; //TIMESTAMP IN MILISSECONDS
    char payload[50];   //FRAME DATA
    uint32_t crc32;     //CHECKSUM
}AfdxFrame_t;

/// @brief VALIDATE AFDX PAYLOAD
/// @param frame FRAME STRUCTURE POINTER
/// @return TRUE IF IS VALID, OTHERWISE RETURNS FALSE
bool AfdxValidateFrame(const AfdxFrame_t *frame);

/// @brief PRINT AFDX STRUCTURE FIELDS
/// @param frame FRAME STRUCTURE POINTER
void AfdxPrintFrame(const AfdxFrame_t *frame);

/// @brief PARSE AFDX PAYLOAD INTO SEPARATE VARIABLES
/// @param frame POINTER TO AFDX FRAME
/// @param missionCode OUTPUT POINTER FOR MISSION CODE
/// @param altitude OUTPUT POINTER FOR ALTITUDE
/// @param timeMs OUTPUT POINTER FOR TIMESTAMP
void AfdxParsePayload(const AfdxFrame_t *frame, int *missionCode, float *altitude, uint32_t *timeMs);

#endif