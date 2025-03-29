//---------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief Smart Message Language (SML) ist ein Kommunikationsprotokoll für Stromzähler.
*/
//---------------------------------------------------------------------------------------------------------------------
#ifndef SMART_MESSAGE_LANGUAGE_H_
#define SMART_MESSAGE_LANGUAGE_H_

/* -- Includes ------------------------------------------------------------ */
#include <stdint.h>
#include <stdarg.h>
#include "sml_parser.h"
#include "nlohmann/json.hpp"



/* -- Defines ------------------------------------------------------------- */
// #define SML_UART_RX_DEBUG     //uncomment to enable some kind of debugging


/* -- Function Prototypes ------------------------------------------------- */

/* -- Types --------------------------------------------------------------- */
class SML
{
public:
   SML() { manufacturer[0] = 0; serial[0] = 0; power1W = 0; energy1Wh = 0; strom1mA[0] = 0; strom1mA[1] = 0; strom1mA[2] = 0; }
   void push(const uint8_t * data, uint32_t len);
   nlohmann::json json() const;
   uint32_t hash() const;

   inline const char * getManufacturer() const { return manufacturer; }
   inline const char * getSerialNumber() const { return serial; }

   inline int32_t getPower1W() const { return power1W; }
   inline uint32_t getEnergy1Wh() const { return energy1Wh; }
   inline uint32_t getStrom1mA(uint32_t phase) const { return strom1mA[phase]; } //phase: L1 .. L3

private:
   SmlParser parser;
   char manufacturer[8];
   char serial[12];
   int32_t power1W;
   uint32_t energy1Wh;
   int32_t strom1mA[3];
};


/* -- Global Variables ---------------------------------------------------- */

/* -- Function Prototypes ------------------------------------------------- */

/* -- Implementation ------------------------------------------------------ */



#endif
