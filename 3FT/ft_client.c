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

/* dummy */

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
  assert(FT_containsDir("1root/2child/3gkid") == FALSE);
  assert(FT_rmDir("1root/2child/3gkid") == INITIALIZATION_ERROR);
  assert(FT_insertFile("1root/2child/3gkid/4ggk",NULL,0) ==
         INITIALIZATION_ERROR);
  assert(FT_containsFile("1root/2child/3gkid/4ggk") == FALSE);
  assert(FT_rmFile("1root/2child/3gkid/4ggk") == INITIALIZATION_ERROR);
  assert((temp = FT_toString()) == NULL);
  assert(FT_destroy() == INITIALIZATION_ERROR);

  /* After initialization, the data structure is empty, so
     contains* should still return FALSE for any non-NULL string,
     and toString should return the empty string.
  */
  assert(FT_init() == SUCCESS);
  assert(FT_containsDir("1root/2child/3gkid") == FALSE);
  assert(FT_containsFile("1root/2child/3gkid/4ggk") == FALSE);
  assert((temp = FT_toString()) != NULL);
  assert(!strcmp(temp,""));
  free(temp);

  FT_insertDir("1root/2second/3third");
  FT_insertFile("1root/2second/3file", NULL, 0);
  FT_rmDir("1root/2second");
  FT_rmDir("1root");

  return 0;
}