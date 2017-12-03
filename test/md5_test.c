/* MDDRIVER.C - test driver for MD2, MD4 and MD5
 */

#ifndef MD
#define MD 5
#endif

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "md5.h"

/* Length of test block, number of test blocks.
 */
#define TEST_BLOCK_LEN 1000
#define TEST_BLOCK_COUNT 1000

/* static void MDString PROTO_LIST ((char *)); */
/* static void MDTimeTrial PROTO_LIST ((void)); */
/* static void MDTestSuite PROTO_LIST ((void)); */
/* static void MDFile PROTO_LIST ((char *)); */
/* static void MDFilter PROTO_LIST ((void)); */
/* static void MDPrint PROTO_LIST ((unsigned char [16])); */


#if MD == 5
#define MDCTX MD5_CTX
#define MDInit MD5Init
#define MDUpdate MD5Update
#define MDFinal MD5Final
#endif

/* Prints a message digest in hexadecimal.
 */
static void MDPrint (unsigned char digest[16])
{
  unsigned int i;

  for (i = 0; i < 16; i++)
      printf ("%02x", digest[i]);
}

/* Digests a string and prints the result.
 */
static void MDString (unsigned char *string)
{
  unsigned char digest[16];

  pmp_get_md5 (string, digest);

  printf ("MD%d (\"%s\") = ", MD, string);
  MDPrint (digest);
  printf ("\n");
}



/* Main driver.

Arguments (may be any combination):
  -sstring - digests string
 */
int main (int argc, char *argv[])
{
  int i;

  if (argc > 1)
      for (i = 1; i < argc; i++)
          if (argv[i][0] == '-' && argv[i][1] == 's')
              MDString ((unsigned char*)argv[i] + 2);

  return (0);
}




