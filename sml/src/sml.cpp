//---------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief Smart Message Language (SML) ist ein Kommunikationsprotokoll für Stromzähler.
*/
//---------------------------------------------------------------------------------------------------------------------


/* -- Includes ------------------------------------------------------------ */
#include "crc.h" //utilities
#include "sml.h"




/* -- Defines ------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------- */

/* -- Function Prototypes ------------------------------------------------- */

/* -- Implementation ------------------------------------------------------ */




void SML::push(const uint8_t * data, uint32_t len)
{
   while (len-- > 0)
   {
      const SmlParser::sml_states_t state = parser.push((uint8_t)*data++);
      if (state == SmlParser::SML_LISTEND)
      {
         if (parser.checkObisCode(SmlParser::OBIS_MANUFACTURER))
         {
            parser.getManufacturer(manufacturer, sizeof(manufacturer) - 1);
            continue;
         }
         if (parser.checkObisCode(SmlParser::OBIS_SERIENNUMMER))
         {
            parser.getManufacturer(serial, sizeof(serial) - 1);
            continue;
         }
         if (parser.checkObisCode(SmlParser::OBIS_WIRKLEISTUNG))
         {
            power1W = parser.getWatt();
            continue;
         }
         if (parser.checkObisCode(SmlParser::OBIS_WIRKENERGIE))
         {
            energy1Wh = parser.getWattHours();
            continue;
         }
         if (parser.checkObisCode(SmlParser::OBIS_STROM_L1))
         {
            strom1mA[0] = parser.getMilliAmps();
            continue;
         }
         if (parser.checkObisCode(SmlParser::OBIS_STROM_L2))
         {
            strom1mA[1] = parser.getMilliAmps();
            continue;
         }
         if (parser.checkObisCode(SmlParser::OBIS_STROM_L3))
         {
            strom1mA[2] = parser.getMilliAmps();
            continue;
         }
      }
   }
}


nlohmann::json SML::json() const
{
   nlohmann::json  obj;
   obj["meter"] = manufacturer;
   obj["serial"] = serial;
   obj["power"] = power1W;
   obj["energy"] = energy1Wh;
#if 0
   {
        obj["current"] = nlohmann::json::array();
        obj["current"].push_back(this->strom1mA[0]);
        obj["current"].push_back(this->strom1mA[1]);
        obj["current"].push_back(this->strom1mA[2]);
   }
#endif
   return obj;
}


uint32_t SML::hash() const
{
   uint32_t crc = 0xFFFFFFFFuL;
   crc = crc_calc_crc32(crc, (uint8_t *)&power1W, sizeof(power1W));
   crc = crc_calc_crc32(crc, (uint8_t *)&energy1Wh, sizeof(energy1Wh));
   crc = crc_calc_crc32(crc, (uint8_t *)&strom1mA, sizeof(strom1mA));
   // crc = crc_calc_crc32(crc, (uint8_t *)&rxCount, sizeof(rxCount));
   return crc;
}
