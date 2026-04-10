//---------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief Smart Message Language (SML) ist ein Kommunikationsprotokoll für Stromzähler.
*/
//---------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------ */
#include <string.h> //memset
#include "sml_parser.h"
#include "crc.h" //utilties

/* -- Defines ------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------- */

/* -- Function Prototypes ------------------------------------------------- */

/* -- Implementation ------------------------------------------------------ */

SmlParser::SmlParser()
{
   listLen = 0;
   currentState = SML_START;
   stateLen = 4;
   currentLevel = 0;
   crc = 0xFFFF;
   crcMine = 0xFFFF;
   crcReceived = 0x0000;
}


void SmlParser::crc16(uint8_t byte)
{
   crc = crc_calc_crc16(crc, &byte, 1);
}


void SmlParser::setState(sml_states_t state, unsigned int byteLen)
{
   currentState = state;
   stateLen = byteLen;
}


void SmlParser::pushListBuffer(uint8_t byte)
{
   if (listLen < SML_PARSER_MAX_LIST_SIZE)
   {
      listBuffer[listLen++] = byte;
   }
}


void SmlParser::reduceList()
{
   if (nodes[currentLevel] > 0)
   {
      nodes[currentLevel]--;
   }
}


void SmlParser::checkMagicByte(uint8_t byte)
{
   unsigned int size = 0;
   while ((currentLevel > 0) && (nodes[currentLevel] == 0))
   {
      /* go back in tree if no nodes remaining */
      // SML_TREELOG(currentLevel, "back to previous list\n");
      currentLevel--;
   }
   if ((byte > 0x70) && (byte < 0x7F))
   {
      /* new list */
      size = byte & 0x0F;
      reduceList();
      if (currentLevel < (SML_PARSER_MAX_TREE_SIZE - 1)) currentLevel++;
      nodes[currentLevel] = (uint8_t)size;
      // SML_TREELOG(currentLevel, "LISTSTART on level %i with %i nodes\n", currentLevel, size);
      setState(SML_LISTSTART, size);
      // @todo workaround for lists inside obis lists
      if (size > 5)
      {
         memset(listBuffer, 0, SML_PARSER_MAX_LIST_SIZE);
         listLen = 0;
      }
      else
      {
         pushListBuffer(size);
         pushListBuffer(currentState);
      }
   }
   else if ((byte >= 0x01) && (byte <= 0x6F) && (nodes[currentLevel] > 0))
   {
      if (byte == 0x01)
      {
         /* no data, get next */
         // SML_TREELOG(currentLevel, " Data %i (empty)\n", nodes[currentLevel]);
         pushListBuffer(0);
         pushListBuffer(currentState);
         if (nodes[currentLevel] == 1)
         {
            setState(SML_LISTEND, 1);
            // SML_TREELOG(currentLevel, "LISTEND\n");
         }
         else
         {
            setState(SML_NEXT, 1);
         }
      }
      else
      {
         size = (byte & 0x0F) - 1;
         setState(SML_DATA, size);
         if ((byte & 0xF0) == 0x50)
         {
            setState(SML_DATA_SIGNED_INT, size);
         }
         else if ((byte & 0xF0) == 0x60)
         {
            setState(SML_DATA_UNSIGNED_INT, size);
         }
         else if ((byte & 0xF0) == 0x00)
         {
            setState(SML_DATA_OCTET_STRING, size);
         }
         // SML_TREELOG(currentLevel,
         //             " Data %i (length = %i%s): ", nodes[currentLevel], size,
         //             (currentState == SML_DATA_SIGNED_INT)     ? ", signed int"
         //             : (currentState == SML_DATA_UNSIGNED_INT) ? ", unsigned int"
         //             : (currentState == SML_DATA_OCTET_STRING) ? ", octet string"
         //                                                       : "");
         pushListBuffer(size);
         pushListBuffer(currentState);
      }
      reduceList();
   }
   else if (byte == 0x00)
   {
      /* end of block */
      reduceList();
      // SML_TREELOG(currentLevel, "End of block at level %i\n", currentLevel);
      if (currentLevel == 0)
      {
         setState(SML_NEXT, 1);
      }
      else
      {
         setState(SML_BLOCKEND, 1);
      }
   }
   else if ((byte >= 0x80) && (byte <= 0x8F))
   {
      // MSB bit is set, another TL byte will follow
      setState(SML_HDATA, (byte & 0x0F) << 4);
   }
   else if ((byte == 0x1B) && (currentLevel == 0))
   {
      /* end sequence */
      setState(SML_END, 3);
   }
   else
   {
      /* Unexpected Byte */
      // SML_TREELOG(currentLevel,
      //             "UNEXPECTED magicbyte >%02X< at currentLevel %i\n", byte,
      //             currentLevel);
      setState(SML_UNEXPECTED, 4);
   }
}


SmlParser::sml_states_t SmlParser::push(uint8_t currentByte)
{
   uint8_t size;
   if (stateLen > 0) stateLen--;
   crc16(currentByte);
   switch (currentState)
   {
   case SML_UNEXPECTED:
   case SML_CHECKSUM_ERROR:
   case SML_FINAL:
   case SML_START:
      currentState = SML_START;
      currentLevel = 0; // Reset current level at the begin of a new transmission to prevent problems
      if (currentByte != 0x1b)
      {
         setState(SML_UNEXPECTED, 4);
      }
      if (stateLen == 0)
      {
         // SML_TREELOG(0, "START\n");
         /* completely clean any garbage from crc checksum */
         crc = 0xFFFF;
         currentByte = 0x1b;
         crc16(currentByte);
         crc16(currentByte);
         crc16(currentByte);
         crc16(currentByte);
         setState(SML_VERSION, 4);
      }
      break;

   case SML_VERSION:
      if (currentByte != 0x01)
      {
         setState(SML_UNEXPECTED, 4);
      }
      if (stateLen == 0)
      {
         setState(SML_BLOCKSTART, 1);
      }
      break;

   case SML_END:
      if (currentByte != 0x1b)
      {
         // SML_LOG("UNEXPECTED char >%02X< at SML_END\n", currentByte);
         setState(SML_UNEXPECTED, 4);
      }
      if (stateLen == 0)
      {
         setState(SML_CHECKSUM, 4);
      }
      break;

   case SML_CHECKSUM:
      // SML_LOG("CHECK: %02X\n", currentByte);
      if (stateLen == 2)
      {
         crcMine = crc ^ 0xFFFF;
      }
      if (stateLen == 1)
      {
         crcReceived += currentByte;
      }
      if (stateLen == 0)
      {
         crcReceived = crcReceived | (currentByte << 8);
         // SML_LOG("Received checksum: %02X\n", crcReceived);
         // SML_LOG("Calculated checksum: %02X\n", crcMine);
         if (crcMine == crcReceived)
         {
            setState(SML_FINAL, 4);
         }
         else
         {
            setState(SML_CHECKSUM_ERROR, 4);
         }
         crc = 0xFFFF;
         crcReceived = 0x000; /* reset CRC */
      }
      break;

   case SML_HDATA:
      size = stateLen + currentByte - 1;
      setState(SML_DATA, size);
      pushListBuffer(size);
      pushListBuffer(currentState);
      // SML_TREELOG(currentLevel, " Data (length = %i): ", size);
      break;

   case SML_DATA:
   case SML_DATA_SIGNED_INT:
   case SML_DATA_UNSIGNED_INT:
   case SML_DATA_OCTET_STRING:
      // SML_LOG("%02X ", currentByte);
      pushListBuffer(currentByte);
      if ((nodes[currentLevel] == 0) && (stateLen == 0))
      {
         // SML_LOG("\n");
         // SML_TREELOG(currentLevel, "LISTEND on level %i\n", currentLevel);
         currentState = SML_LISTEND;
      }
      else if (stateLen == 0)
      {
         currentState = SML_DATAEND;
         // SML_LOG("\n");
      }
      break;

   case SML_DATAEND:
   case SML_NEXT:
   case SML_LISTSTART:
   case SML_LISTEND:
   case SML_BLOCKSTART:
   case SML_BLOCKEND:
      checkMagicByte(currentByte);
      break;
   }

   return currentState;
}


bool SmlParser::checkObisCode(const char code[6])
{
   return (memcmp(code, &listBuffer[2], 6) == 0);
}


const char * SmlParser::getManufacturer(char *buffer, unsigned int size)
{
   unsigned int i = 0;
   unsigned int pos = 0;
   unsigned int len = 0;
   while (i < listLen)
   {
      len = (int)listBuffer[i];
      i++;
      pos++;
      if (pos == 6)
      {
         /* get manufacturer at position 6 in list */
         len = (len > size - 1) ? size : len;
         memcpy(buffer, &listBuffer[i + 1], len);
         buffer[len] = 0; //zero termination
         return buffer;
      }
      i += len + 1;
   }
   buffer[0] = 0; //zero termination
   return buffer;
}



uint32_t SmlParser::getUnit(sml_units_t unit)
{
   uint8_t i = 0;
   uint8_t pos = 0;
   uint8_t size = 0;
   uint8_t y = 0;
   uint8_t skip = 0;
   int8_t exp = 0; //scale exponent 10^exp
   sml_states_t type;
   while (i < listLen)
   {
      pos++;
      size = (int)listBuffer[i++];
      type = (sml_states_t)listBuffer[i++];
      if ((type == SML_LISTSTART) && (size > 0))
      {
         // skip a list inside an obis list
         skip = size;
         while (skip > 0)
         {
            size = (int)listBuffer[i++];
            type = (sml_states_t)listBuffer[i++];
            i += size;
            skip--;
         }
         size = 0;
      }
      if ((pos == 4) && (listBuffer[i] != unit))
      {
         /* return unknown (-1) if unit does not match */
         return 0;
      }
      if (pos == 5)
      {
         exp = listBuffer[i];
      }
      if (pos == 6)
      {
         y = size;
         // initialize 64bit signed integer based on MSB from received value
         uint64_t val = ((type == SML_DATA_SIGNED_INT) && (listBuffer[i] & (1 << 7))) ? ~0uLL : 0uLL;
         for (y = 0; y < size; y++)
         {
            // left shift received bytes to 64 bit signed integer
            val = (val << 8) | listBuffer[i + y];
         }
         //scale
         if (exp == -2) //10^-2
         {
            return (uint32_t)((41LL * (int64_t)val) >> 12);
         }
         if (exp == -4) //10^-4
         {
            return (uint32_t)((13LL * (int64_t)val) >> 17);
         }
         //todo - also implement other scale factors
         //otherwise
         return (uint32_t)val;
      }
      i += size;
   }
   return 0;
}


