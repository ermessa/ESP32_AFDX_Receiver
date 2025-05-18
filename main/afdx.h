#ifndef _AFDX_H_
#define _AFDX_H_

#include "stdint.h"

/// @brief AFDX STRUCTURE
typedef struct
{
    uint16_t id;        //VIRTUAL LINK ID
    uint32_t timestamp; //TIMESTAMP IN MILISSECONDS
    char payload[50];   //FRAME DATA
}AfdxFrame_t;

/// @brief THE FUNCTION VERIFY IF IS A VALID AFDX ID
/// @param frame FRAME STRUCTURE POINTER
/// @return TRUE IF IS VALID, OTHERWISE RETURNS FALSE
bool AfdxValidateFrame(const AfdxFrame_t *frame);

/// @brief THE FUNCTION PRINT AFDX STRUCTURE FIELDS
/// @param frame FRAME STRUCTURE POINTER
void AfdxPrintFrame(const AfdxFrame_t *frame);

#endif

