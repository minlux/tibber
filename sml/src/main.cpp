//---------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief SML decoder.

   Usage %s [-i <input-file>] [-o <output-file>]
   
   <input-file> specifies path to SML data (eg. read from tibber pulse)
   <input-file> can also be omitted, then data is read from stdin

   <output-file> specifies the path where to write the decoded (JSON) data
   <output-file> can also be omitted, then data is written to stdout


   Using stdin and stdout gives you the option to use this tool toghether with other linux command line tools
   to pipe input and output data

   ```
   curl -s http://admin:C0P5-B8D2@192.168.178.27/data.json?node_id=1 | ./sml/build/sml
   ```

   ```
   curl -s http://admin:C0P5-B8D2@192.168.178.27/data.json?node_id=1 | ./sml/build/sml | curl -H 'Content-Type: application-json' -X POST --data @- http://localhost:8080/smarty/b25
   ```
*/
//---------------------------------------------------------------------------------------------------------------------


/* -- Includes ------------------------------------------------------------ */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sml.h"




/* -- Defines ------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------- */

/* -- (Module-)Global Variables ------------------------------------------- */

/* -- Function Prototypes ------------------------------------------------- */

/* -- Implementation ------------------------------------------------------ */
static void usage(const char* cmd)
{
   fprintf(stderr, "Usage %s [-i <input-file>] [-o <output-file>]\r\n\n", cmd);
}



int main(int argc, char * argv[])
{
   SML sml;

   //evaluate command line arguments
   //-i <input-file> (default: NULL)
   char const* inFilename = NULL;
   for (int i = 1; i < (argc - 1); ++i)
   {
      if ((argv[i][0] == '-') && (argv[i][1] == 'i'))
      {
         inFilename = argv[i + 1];
         break;
      }
   }

   //-o <output-file> (default: NULL)
   char const* outFilename = NULL;
   for (int i = 1; i < (argc - 1); ++i)
   {
      if ((argv[i][0] == '-') && (argv[i][1] == 'o'))
      {
         outFilename = argv[i + 1];
         break;
      }
   }



   //read input
   {
      FILE *f = inFilename ? fopen(inFilename, "rb") : stdin; //if input file is given as "-" (dash) read from stdin
      if (f == NULL)
      {
         fprintf(stderr, "Failed to open input file %s!\n", inFilename);
         return -2;
      }
      uint8_t buffer[4096]; //should be enough
      size_t count;

      //read file to buffer
      count = fread(buffer, 1, sizeof(buffer), f);
      if (inFilename) fclose(f); //close only regular files, not stdin

      //process data
      sml.push(buffer, count);
   }


   //write output
   {
      FILE *f = outFilename ? fopen(outFilename, "wb") : stdout; //if output file is given as "-" (dash) write to stdout 
      if (f == NULL)
      {
         fprintf(stderr, "Failed to open output file %s!\n", outFilename);
         return -1;
      }

      //get json
      std::string json = sml.json().dump();

      //write to output file
      fwrite(json.c_str(), 1, json.length(), f);
      fwrite("\r\n", 1, 2, f); //add new-line
      if (outFilename) fclose(f); //close only regular files, not stdout
   }


   return 0;
}

