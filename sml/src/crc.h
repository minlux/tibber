#ifndef CYCLIC_REDUNDANT_CHECK_H
#define CYCLIC_REDUNDANT_CHECK_H

/* -- Includes ------------------------------------------------------------ */
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

/* -- Defines ------------------------------------------------------------- */


/* -- Types --------------------------------------------------------------- */


/* -- Prototypes ---------------------------------------------------------- */
uint8_t crc_calc_crc8(uint8_t seed, uint8_t const * data, uint32_t length);
uint16_t crc_calc_crc16(uint16_t seed, uint8_t const * data, uint32_t length);
uint16_t crc_calc_crc16_reflected(uint16_t seed, uint8_t const * data, uint32_t length);
uint32_t crc_calc_crc32(uint32_t seed, uint8_t const * data, uint32_t length);
uint32_t crc_calc_crc32_reflected(uint32_t seed, uint8_t const * data, uint32_t length);


/* -- Implementation ------------------------------------------------------ */




#ifdef __cplusplus
} /* end of extern "C" */
#endif


#endif

