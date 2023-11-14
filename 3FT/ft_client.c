/*--------------------------------------------------------------------*/
/* ft_client.c                                                        */
/* Author: Christopher Moretti                                        */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ft.h"

/* Tests the FT implementation with an assortment of checks.
   Prints the status of the data structure along the way to stderr.
   Returns 0. */
int main(void) {
  enum {ARRLEN = 1000};
  char* temp;
  boolean bIsFile;
  size_t l;
  char arr[ARRLEN];
  arr[0] = '\0';

  /* Before the data structure is initialized:
     * insert*, rm*, and destroy should all return INITIALIZATION_ERROR
     * contains* should return FALSE
     * toString should return NULL.
  */
  assert(FT_insertDir("1root/2child/3gkid") == INITIALIZATION_ERROR);


  return 0;
}
