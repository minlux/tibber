//---------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief Smart Message Language (SML) ist ein Kommunikationsprotokoll für Stromzähler.

   Adapted from https://github.com/olliiiver/sml_parser
   For OBIS codes search the internet for "iTrona, Beschreibung SML Datenprotokoll für SMART METER"
   and the "Obis-Kennzahlen-System_2.0.pdf" page 6, in doc folder.
*/
//---------------------------------------------------------------------------------------------------------------------
#ifndef SML_PARSER_H_
#define SML_PARSER_H_

/* -- Includes ------------------------------------------------------------ */
#include <stdint.h>



/* -- Defines ------------------------------------------------------------- */
#define SML_PARSER_MAX_LIST_SIZE    (80)
#define SML_PARSER_MAX_TREE_SIZE    (10)



/* -- Function Prototypes ------------------------------------------------- */

/* -- Types --------------------------------------------------------------- */
class SmlParser
{
public:
   typedef enum {
      SML_START,
      SML_END,
      SML_VERSION,
      SML_NEXT,
      SML_LISTSTART,
      SML_LISTEND,
      SML_DATA,
      SML_HDATA,
      SML_DATAEND,
      SML_BLOCKSTART,
      SML_BLOCKEND,
      SML_CHECKSUM,
      SML_CHECKSUM_ERROR, /* calculated checksum does not match */
      SML_UNEXPECTED,     /* unexpected byte received */
      SML_FINAL,          /* final state, checksum OK */
      SML_DATA_SIGNED_INT,
      SML_DATA_UNSIGNED_INT,
      SML_DATA_OCTET_STRING,
   } sml_states_t;

   typedef enum {
      SML_YEAR = 1,
      SML_MONTH = 2,
      SML_WEEK = 3,
      SML_DAY = 4,
      SML_HOUR = 5,
      SML_MIN = 6,
      SML_SECOND = 7,
      SML_DEGREE = 8,
      SML_DEGREE_CELSIUS = 9,
      SML_CURRENCY = 10,
      SML_METRE = 11,
      SML_METRE_PER_SECOND = 12,
      SML_CUBIC_METRE = 13,
      SML_CUBIC_METRE_CORRECTED = 14,
      SML_CUBIC_METRE_PER_HOUR = 15,
      SML_CUBIC_METRE_PER_HOUR_CORRECTED = 16,
      SML_CUBIC_METRE_PER_DAY = 17,
      SML_CUBIC_METRE_PER_DAY_CORRECTED = 18,
      SML_LITRE = 19,
      SML_KILOGRAM = 20,
      SML_NEWTON = 21,
      SML_NEWTONMETER = 22,
      SML_PASCAL = 23,
      SML_BAR = 24,
      SML_JOULE = 25,
      SML_JOULE_PER_HOUR = 26,
      SML_WATT = 27,
      SML_VOLT_AMPERE = 28,
      SML_VAR = 29,
      SML_WATT_HOUR = 30,
      SML_VOLT_AMPERE_HOUR = 31,
      SML_VAR_HOUR = 32,
      SML_AMPERE = 33,
      SML_COULOMB = 34,
      SML_VOLT = 35,
      SML_VOLT_PER_METRE = 36,
      SML_FARAD = 37,
      SML_OHM = 38,
      SML_OHM_METRE = 39,
      SML_WEBER = 40,
      SML_TESLA = 41,
      SML_AMPERE_PER_METRE = 42,
      SML_HENRY = 43,
      SML_HERTZ = 44,
      SML_ACTIVE_ENERGY_METER_CONSTANT_OR_PULSE_VALUE = 45,
      SML_REACTIVE_ENERGY_METER_CONSTANT_OR_PULSE_VALUE = 46,
      SML_APPARENT_ENERGY_METER_CONSTANT_OR_PULSE_VALUE = 47,
      SML_VOLT_SQUARED_HOURS = 48,
      SML_AMPERE_SQUARED_HOURS = 49,
      SML_KILOGRAM_PER_SECOND = 50,
      SML_KELVIN = 52,
      SML_VOLT_SQUARED_HOUR_METER_CONSTANT_OR_PULSE_VALUE = 53,
      SML_AMPERE_SQUARED_HOUR_METER_CONSTANT_OR_PULSE_VALUE = 54,
      SML_METER_CONSTANT_OR_PULSE_VALUE = 55,
      SML_PERCENTAGE = 56,
      SML_AMPERE_HOUR = 57,
      SML_ENERGY_PER_VOLUME = 60,
      SML_CALORIFIC_VALUE = 61,
      SML_MOLE_PERCENT = 62,
      SML_MASS_DENSITY = 63,
      SML_PASCAL_SECOND = 64,
      SML_RESERVED = 253,
      SML_OTHER_UNIT = 254,
      SML_COUNT = 255
   } sml_units_t;


   SmlParser();
   sml_states_t push(uint8_t byte); //push received byte into parser

   bool checkObisCode(const char code[6]); //compare 6 byte obis code; return true if equals!

   const char * getManufacturer(char * buffer, unsigned int size); //get zero terminated manufacturer string
   inline int32_t getWatt() { return (int32_t)getUnit(SML_WATT); }
   inline uint32_t getWattHours() { return getUnit(SML_WATT_HOUR); }
   inline int32_t getMilliAmps() { return (int32_t)getUnit(SML_AMPERE); }


   //obis codes
   static constexpr const char * const OBIS_MANUFACTURER    = "\x81\x81\xC7\x82\x03\xFF";
   static constexpr const char * const OBIS_SERIENNUMMER    = "\x01\x00\x00\x00\x00\xFF";

   static constexpr const char * const OBIS_WIRKLEISTUNG    = "\x01\x00\x01\x07\x00\xFF"; //Summe aller Phasen, in W
   // static constexpr const char * const OBIS_WIRKLEISTUNG_L1 = "\x01\x00\x15\x07\x00\xFF"; //Phase L1, in W as int32_t
   // static constexpr const char * const OBIS_WIRKLEISTUNG_L2 = "\x01\x00\x29\x07\x00\xFF"; //Phase L2, in W as int32_t
   // static constexpr const char * const OBIS_WIRKLEISTUNG_L3 = "\x01\x00\x3D\x07\x00\xFF"; //Phase L3, in W as int32_t

   static constexpr const char * const OBIS_STROM_L1        = "\x01\x00\x1F\x07\x00\xFF"; //Phase L1, in mA as int32_t
   static constexpr const char * const OBIS_STROM_L2        = "\x01\x00\x33\x07\x00\xFF"; //Phase L2, in mA as int32_t
   static constexpr const char * const OBIS_STROM_L3        = "\x01\x00\x47\x07\x00\xFF"; //Phase L3, in mA as int32_t

   static constexpr const char * const OBIS_WIRKENERGIE     = "\x01\x00\x01\x08\x00\xFF"; //Summe aller Tarife, in Wh
   static constexpr const char * const OBIS_WIRKENERGIE_T1  = "\x01\x00\x01\x08\x01\xFF"; //Tarif 1 as uint32_t
   static constexpr const char * const OBIS_WIRKENERGIE_T2  = "\x01\x00\x01\x08\x02\xFF"; //Tarif 2 as uint32_t
   // static constexpr const char * const OBIS_WIRKENERGIE_T3  = "\x01\x00\x01\x08\x03\xFF"; //Tarif 3 as uint32_t


private:
   uint32_t getUnit(sml_units_t unit);
   void crc16(uint8_t byte);
   void setState(sml_states_t state, unsigned int byteLen);
   void pushListBuffer(uint8_t byte);
   void reduceList(void);
   void checkMagicByte(uint8_t byte);




private:
   uint8_t nodes[SML_PARSER_MAX_TREE_SIZE];
   uint8_t listBuffer[SML_PARSER_MAX_LIST_SIZE]; /* keeps a list as length + state + data  */
   unsigned int listLen;
   sml_states_t currentState;
   unsigned int stateLen;
   unsigned int currentLevel;
   uint16_t crc;
   uint16_t crcMine;
   uint16_t crcReceived;
};


/* -- Global Variables ---------------------------------------------------- */

/* -- Function Prototypes ------------------------------------------------- */

/* -- Implementation ------------------------------------------------------ */



#endif
