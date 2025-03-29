/* -- Includes ------------------------------------------------------------ */
#include "crc.h"

/* -- Defines ------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------- */

//CRC8
//Polynomial: 0x7
//4-bit lookup table
static const uint8_t crc8_lut[16] =
{
   0x00u, 0x07u, 0x0Eu, 0x09u, 0x1Cu, 0x1Bu, 0x12u, 0x15u,
   0x38u, 0x3Fu, 0x36u, 0x31u, 0x24u, 0x23u, 0x2Au, 0x2Du,
};


//CRC16 CCIT ZERO
//Polynomial: 0x1021
//4-bit lookup table
static const uint16_t crc16_lut[16] =
{
   0x0000u, 0x1021u, 0x2042u, 0x3063u, 0x4084u, 0x50A5u, 0x60C6u, 0x70E7u,
   0x8108u, 0x9129u, 0xA14Au, 0xB16Bu, 0xC18Cu, 0xD1ADu, 0xE1CEu, 0xF1EFu
};
//CRC16 CCIT ZERO
//Polynomial: 0x1021
//4-bit lookup table, reflected
static const uint16_t crc16_reflected_lut[16] =
{
   0x0000u, 0x1081u, 0x2102u, 0x3183u, 0x4204u, 0x5285u, 0x6306u, 0x7387u,
   0x8408u, 0x9489u, 0xA50Au, 0xB58Bu, 0xC60Cu, 0xD68Du, 0xE70Eu, 0xF78Fu
};


//CRC32
//Polynomial: 0x4C11DB7
//4-bit lookup table
static const uint32_t crc32_lut[16] =
{
   0x00000000uL, 0x04C11DB7uL, 0x09823B6EuL, 0x0D4326D9uL, 0x130476DCuL, 0x17C56B6BuL, 0x1A864DB2uL, 0x1E475005uL,
   0x2608EDB8uL, 0x22C9F00FuL, 0x2F8AD6D6uL, 0x2B4BCB61uL, 0x350C9B64uL, 0x31CD86D3uL, 0x3C8EA00AuL, 0x384FBDBDuL
};
//CRC32
//Polynomial: 0x4C11DB7
//4-bit lookup table, reflected
static const uint32_t crc32_reflected_lut[16] =
{
   0x00000000uL, 0x1DB71064uL, 0x3B6E20C8uL, 0x26D930ACuL, 0x76DC4190uL, 0x6B6B51F4uL, 0x4DB26158uL, 0x5005713CuL,
   0xEDB88320uL, 0xF00F9344uL, 0xD6D6A3E8uL, 0xCB61B38CuL, 0x9B64C2B0uL, 0x86D3D2D4uL, 0xA00AE278uL, 0xBDBDF21CuL
};



/* -- Implementation ------------------------------------------------------ */
#define GET_NIBBLE(word, start) (((word) >> (start)) & 0xF)


uint8_t crc_calc_crc8(uint8_t seed, uint8_t const * data, uint32_t length)
{
   uint32_t crc = seed;
   for (uint32_t i = 0; i < length; ++i)
   {
      uint32_t dataByte = data[i];
      uint32_t crcNibble;
      uint32_t dataNibble;
      uint32_t idx;

      //1st nibble
      //calculate index into lookup table
      crcNibble = GET_NIBBLE(crc, 4); //xor the upper 4 bits of the crc ...
      dataNibble = GET_NIBBLE(dataByte, 4); //... with the upper 4 bits for the data-byte.
      idx = crcNibble ^ dataNibble;; //This produces the index into the lookup table
      //update crc according to the respective value from lookup table
      crc = (crc << 4) ^ crc8_lut[idx]; //shift out the 4 upper bits of the crc, and xor the remainder with the value from the lookup table

      //2nd nibble
      crcNibble = GET_NIBBLE(crc, 4); //xor the upper 4 bits of the crc ...
      dataNibble = GET_NIBBLE(dataByte, 0); //... with the lower 4 bits for the data-byte.
      idx = crcNibble ^ dataNibble;; //This produces the index into the lookup table
      //update crc according to the respective value from lookup table
      crc = (crc << 4) ^ crc8_lut[idx]; //shift out the 4 upper bits of the crc, and xor the remainder with the value from the lookup table
   }
   return (uint8_t)crc;
}



uint16_t crc_calc_crc16(uint16_t seed, uint8_t const * data, uint32_t length)
{
   uint32_t crc = seed;
   for (uint32_t i = 0; i < length; ++i)
   {
      uint32_t dataByte = data[i];
      uint32_t crcNibble;
      uint32_t dataNibble;
      uint32_t idx;

      //1st nibble
      //calculate index into lookup table
      crcNibble = GET_NIBBLE(crc, 12); //xor the upper 4 bits of the crc ...
      dataNibble = GET_NIBBLE(dataByte, 4); //... with the upper 4 bits for the data-byte.
      idx = crcNibble ^ dataNibble;; //This produces the index into the lookup table
      //update crc according to the respective value from lookup table
      crc = (crc << 4) ^ crc16_lut[idx]; //shift out the 4 upper bits of the crc, and xor the remainder with the value from the lookup table

      //2nd nibble
      crcNibble = GET_NIBBLE(crc, 12); //xor the upper 4 bits of the crc ...
      dataNibble = GET_NIBBLE(dataByte, 0); //... with the lower 4 bits for the data-byte.
      idx = crcNibble ^ dataNibble;; //This produces the index into the lookup table
      //update crc according to the respective value from lookup table
      crc = (crc << 4) ^ crc16_lut[idx]; //shift out the 4 upper bits of the crc, and xor the remainder with the value from the lookup table
   }
   return (uint16_t)crc;
}


uint16_t crc_calc_crc16_reflected(uint16_t seed, uint8_t const * data, uint32_t length)
{
   uint32_t crc = seed;
   for (uint32_t i = 0; i < length; ++i)
   {
      uint32_t dataByte = data[i];
      uint32_t crcNibble;
      uint32_t dataNibble;
      uint32_t idx;

      //1st nibble
      //calculate index into lookup table
      crcNibble = GET_NIBBLE(crc, 0); //xor the lower 4 bits of the crc ...
      dataNibble = GET_NIBBLE(dataByte, 0); //... with the lower 4 bits for the data-byte.
      idx = crcNibble ^ dataNibble;; //This produces the index into the lookup table
      //update crc according to the respective value from lookup table
      crc = (crc >> 4) ^ crc16_reflected_lut[idx]; //shift out the 4 upper bits of the crc, and xor the remainder with the value from the lookup table

      //2nd nibble
      crcNibble = GET_NIBBLE(crc, 0); //xor the lower 4 bits of the crc ...
      dataNibble = GET_NIBBLE(dataByte, 4); //... with the upper 4 bits for the data-byte.
      idx = crcNibble ^ dataNibble;; //This produces the index into the lookup table
      //update crc according to the respective value from lookup table
      crc = (crc >> 4) ^ crc16_reflected_lut[idx]; //shift out the 4 upper bits of the crc, and xor the remainder with the value from the lookup table
   }
   return (uint16_t)crc;
}



uint32_t crc_calc_crc32(uint32_t seed, uint8_t const * data, uint32_t length)
{
   uint32_t crc = seed;
   for (uint32_t i = 0; i < length; ++i)
   {
      uint32_t dataByte = data[i];
      uint32_t crcNibble;
      uint32_t dataNibble;
      uint32_t idx;

      //1st nibble
      //calculate index into lookup table
      crcNibble = GET_NIBBLE(crc, 28); //xor the upper 4 bits of the crc ...
      dataNibble = GET_NIBBLE(dataByte, 4); //... with the upper 4 bits for the data-byte.
      idx = crcNibble ^ dataNibble;; //This produces the index into the lookup table
      //update crc according to the respective value from lookup table
      crc = (crc << 4) ^ crc32_lut[idx]; //shift out the 4 upper bits of the crc, and xor the remainder with the value from the lookup table

      //2nd nibble
      crcNibble = GET_NIBBLE(crc, 28); //xor the upper 4 bits of the crc ...
      dataNibble = GET_NIBBLE(dataByte, 0); //... with the lower 4 bits for the data-byte.
      idx = crcNibble ^ dataNibble;; //This produces the index into the lookup table
      //update crc according to the respective value from lookup table
      crc = (crc << 4) ^ crc32_lut[idx]; //shift out the 4 upper bits of the crc, and xor the remainder with the value from the lookup table
   }
   return crc;
}


uint32_t crc_calc_crc32_reflected(uint32_t seed, uint8_t const * data, uint32_t length)
{
   uint32_t crc = seed;
   for (uint32_t i = 0; i < length; ++i)
   {
      uint32_t dataByte = data[i];
      uint32_t crcNibble;
      uint32_t dataNibble;
      uint32_t idx;

      //1st nibble
      //calculate index into lookup table
      crcNibble = GET_NIBBLE(crc, 0); //xor the lower 4 bits of the crc ...
      dataNibble = GET_NIBBLE(dataByte, 0); //... with the lower 4 bits for the data-byte.
      idx = crcNibble ^ dataNibble;; //This produces the index into the lookup table
      //update crc according to the respective value from lookup table
      crc = (crc >> 4) ^ crc32_reflected_lut[idx]; //shift out the 4 upper bits of the crc, and xor the remainder with the value from the lookup table

      //2nd nibble
      crcNibble = GET_NIBBLE(crc, 0); //xor the lower 4 bits of the crc ...
      dataNibble = GET_NIBBLE(dataByte, 4); //... with the upper 4 bits for the data-byte.
      idx = crcNibble ^ dataNibble;; //This produces the index into the lookup table
      //update crc according to the respective value from lookup table
      crc = (crc >> 4) ^ crc32_reflected_lut[idx]; //shift out the 4 upper bits of the crc, and xor the remainder with the value from the lookup table
   }
   return crc;
}

