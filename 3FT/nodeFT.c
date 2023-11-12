/*--------------------------------------------------------------------*/
/* nodeDT.c                                                           */
/* Author: Christopher Moretti                                        */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dynarray.h"
#include "nodeDT.h"
#include "checkerDT.h"

/* A node in a FT */
struct node {
   /* the object corresponding to the node's absolute path */
   Path_T oPPath;
   /* this node's parent */
   Node_T oNParent;
   /* the object containing links to this node's directory children */
   DynArray_T oDChildren;
   /* the object containing links to this node's File children */
   DynArray_T oFChildren;
   /* a boolean, true if this is a file, false if this is a directory*/
   boolean isFile;
   /* a void pointer to file content*/
   void* FileContent;
};


int Node_new(Path_T oPPath, Node_T oNParent, boolean isFile, void* FileContent, Node_T *poNResult) {
   Node_T psNew;
   Path_T oPParentPath = NULL;
   Path_T oPNewPath = NULL;
   size_t ulParentDepth;
   size_t ulIndex;
   int iStatus;

   assert(oPPath != NULL);
   assert(oNParent == NULL || CheckerDT_Node_isValid(oNParent));

   /* allocate space for a new node */
   psNew = malloc(sizeof(struct node));
   if(psNew == NULL) {
      *poNResult = NULL;
      return MEMORY_ERROR;
   }

   /* set the new node's path */
   iStatus = Path_dup(oPPath, &oPNewPath);
   if(iStatus != SUCCESS) {
      free(psNew);
      *poNResult = NULL;
      return iStatus;
   }
   psNew->oPPath = oPNewPath;

   /* validate and set the new node's parent */
   if(oNParent != NULL) {
      size_t ulSharedDepth;

      oPParentPath = oNParent->oPPath;
      ulParentDepth = Path_getDepth(oPParentPath);
      ulSharedDepth = Path_getSharedPrefixDepth(psNew->oPPath,
                                                oPParentPath);
      /* parent must be an ancestor of child */
      if(ulSharedDepth < ulParentDepth) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return CONFLICTING_PATH;
      }

      /* parent must be exactly one level up from child */
      if(Path_getDepth(psNew->oPPath) != ulParentDepth + 1) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return NO_SUCH_PATH;
      }

      /* parent must not already have child with this path */
      if(Node_hasChild(oNParent, oPPath, &ulIndex)) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return ALREADY_IN_TREE;
      }
   }
   else {
      /* new node must be root */
      /* can only create one "level" at a time */
      if(Path_getDepth(psNew->oPPath) != 1) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return NO_SUCH_PATH;
      }
   }
   psNew->oNParent = oNParent;

   /* initialize the new node */
   psNew->oDChildren = DynArray_new(0);
   if(psNew->oDChildren == NULL) {
      Path_free(psNew->oPPath);
      free(psNew);
      *poNResult = NULL;
      return MEMORY_ERROR;
   }

   /* Link into parent's children list */
   if(oNParent != NULL) {
      iStatus = Node_addChild(oNParent, psNew, ulIndex);
      if(iStatus != SUCCESS) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return iStatus;
      }
   }

    /*specifying if this node is a file or not and initialize accordingly*/
   psNew->isFile = isFile;

   if (isFile) {
    psNew->FileContent = FileContent;
    psNew->oDChildren = NULL;
    psNew->oFChildren = NULL;
   }

   

   *poNResult = psNew;

   assert(oNParent == NULL || CheckerDT_Node_isValid(oNParent));
   assert(CheckerDT_Node_isValid(*poNResult));

   return SUCCESS;
}