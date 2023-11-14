/*--------------------------------------------------------------------*/
/* nodeDT.c                                                           */
/* Author: Christopher Moretti                                        */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dynarray.h"
#include "nodeFT.h"
#include "a4def.h"
#include "path.h"



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
   /* length of the file contents */
   size_t ulContLength;
};

/*-------------------------------------------------------------------------*/

/*
  Compares the string representation of oNfirst with a string
  pcSecond representing a node's path.
  Returns <0, 0, or >0 if oNFirst is "less than", "equal to", or
  "greater than" pcSecond, respectively.
*/
static int Node_compareString(const Node_T oNFirst,
                                 const char *pcSecond) {
   assert(oNFirst != NULL);
   assert(pcSecond != NULL);

   return Path_compareString(oNFirst->oPPath, pcSecond);
}

/*-------------------------------------------------------------------------*/

static int Node_addChild(Node_T oNParent, Node_T oNChild,
                         size_t ulIndex) {
   assert(oNParent != NULL);
   assert(oNChild != NULL);
   assert(!oNParent->isFile);

    if (oNChild->isFile){
        if(DynArray_addAt(oNParent->oFChildren, ulIndex, oNChild))
            return SUCCESS;
        else
            return MEMORY_ERROR;
    } else {
        if(DynArray_addAt(oNParent->oDChildren, ulIndex, oNChild))
            return SUCCESS;
        else
            return MEMORY_ERROR;
    }
}

/*-------------------------------------------------------------------------*/

int Node_new(Path_T oPPath, Node_T oNParent, 
      boolean isFile, void* FileContent, size_t ulContLength, Node_T *poNResult) {
   Node_T psNew;
   Path_T oPParentPath = NULL;
   Path_T oPNewPath = NULL;
   size_t ulParentDepth;
   size_t ulIndex;
   boolean childIsFile;
   int iStatus;

   assert(oPPath != NULL);
   /* assert(oNParent == NULL); PARENT CAN BE NULL IF IT'S THE ROOT - maybe validate another way */

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
      if(Node_hasChild(oNParent, oPPath, &childIsFile, &ulIndex)) {
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
        /*psNew->FileContent = FileContent;*/
        psNew->oDChildren = NULL;
        psNew->oFChildren = NULL;

        /* HERE: would it be useful to track the lenght of the
        file content?? */
    
        /* allocate memory for contents */
        psNew->FileContent = malloc(ulContLength);
        if(psNew->FileContent == NULL) {
            Path_free(psNew->oPPath);
            free(psNew);
            *poNResult = NULL;
            return MEMORY_ERROR;
        } else {
            /* copy (void) contents to the memory space */
            memcpy(psNew->FileContent, FileContent, ulContLength);
            psNew->ulContLength = ulContLength;
        }
    
   } else {
        psNew->oDChildren = DynArray_new(0);
        if(psNew->oDChildren == NULL) {
            Path_free(psNew->oPPath);
            free(psNew);
            *poNResult = NULL;
            return MEMORY_ERROR;
        }
        psNew->oFChildren = DynArray_new(0);
        if(psNew->oFChildren == NULL) {
            Path_free(psNew->oPPath);
            free(psNew);
            *poNResult = NULL;
            return MEMORY_ERROR;
        }
        psNew->FileContent = NULL;
   }

   *poNResult = psNew;

   /* assert(oNParent == NULL); */

   return SUCCESS;
}

/*-------------------------------------------------------------------------*/

boolean Node_isFile(Node_T oNNode) {
    assert(oNNode != NULL);

    return oNNode->isFile;
}

/*-------------------------------------------------------------------------*/

void *Node_getContent(Node_T oNNode) {
    assert (oNNode != NULL);

    /* this content might be NULL 
    HERE: should this dereference? */
    return oNNode->FileContent;
}

/*-------------------------------------------------------------------------*/


/*
  Replaces current contents of the file with absolute path pcPath with
  the parameter pvNewContents of size ulNewLength bytes.
  Returns the old contents if successful. (Note: contents may be NULL.)
  Returns NULL if unable to complete the request for any reason.
*/
void *Node_ReplaceFileContent(Node_T oNNode, void* NewFileContent, size_t ulNewLength) {
    void* oldContent;
    void* temp;
    
    assert(oNNode != NULL);
    assert(oNNode->isFile);

    /* reorder pointers */
    oldContent = oNNode->FileContent;

    /* re-allocate memory for contents */
    temp = realloc(oNNode->FileContent, ulNewLength);
    if(temp == NULL) {
        /* revert reordering */
        oNNode->FileContent = oldContent;
        return NULL;
    } else {
        oNNode->FileContent = temp;
        /* copy new contents */
        memcpy(oNNode->FileContent, NewFileContent, ulNewLength);
        oNNode->ulContLength = ulNewLength;
    }

    return oldContent;
}

/*--------------------------------------------------------------------------*/


boolean Node_hasChild(Node_T oNParent, Path_T oPPath, boolean *pisFile,
                         size_t *pulChildID) {
   boolean hasDirChild; 
   boolean hasFileChild;
   assert(oNParent != NULL);
   assert(oPPath != NULL);
   assert(pulChildID != NULL);
   assert(oNParent->isFile != TRUE);

    /* *pulChildID is the index into oNParent->oDChildren */
    hasDirChild = DynArray_bsearch(oNParent->oDChildren,
            (char*) Path_getPathname(oPPath), pulChildID,
            (int (*)(const void*,const void*)) Node_compareString);
    
    if (hasDirChild) {
        *pisFile = FALSE;
        return hasDirChild;
    } else {
        /* *pulChildID is the index into oNParent->oDChildren */
        hasFileChild = DynArray_bsearch(oNParent->oFChildren,
            (char*) Path_getPathname(oPPath), pulChildID,
            (int (*)(const void*,const void*)) Node_compareString);
        *pisFile = hasFileChild;
        return (hasFileChild);
    }   
}

/*-------------------------------------------------------------------------*/

size_t Node_Dir_free(Node_T oNNode) {
   size_t ulIndex;
   size_t ulCount = 0;

   assert(oNNode != NULL);
   assert(!oNNode->isFile);

    /* remove from parent's list */
    if(oNNode->oNParent != NULL) {
        if(DynArray_bsearch(
                oNNode->oNParent->oDChildren,
                oNNode, &ulIndex,
                (int (*)(const void *, const void *)) Node_compare)
            )
            (void) DynArray_removeAt(oNNode->oNParent->oDChildren,
                                    ulIndex);
    }

    /* remove file children*/
    while(DynArray_getLength(oNNode->oFChildren) != 0) {
        ulCount += Node_File_free(DynArray_get(oNNode->oFChildren, 0));
    }
    DynArray_free(oNNode->oFChildren);

    /* recursively remove directory children */
    while(DynArray_getLength(oNNode->oDChildren) != 0) {
        ulCount += Node_Dir_free(DynArray_get(oNNode->oDChildren, 0));
    }
    DynArray_free(oNNode->oDChildren);

    /* remove path */
    Path_free(oNNode->oPPath);

    /* finally, free the struct node */
    free(oNNode);
    ulCount++;
    return ulCount;
}

/*-------------------------------------------------------------------------*/

size_t Node_File_free(Node_T oNNode) {
   size_t ulIndex;
   size_t ulCount = 0;

   assert(oNNode != NULL);
   assert(oNNode->isFile);

    /* Free space of file content */
    free(oNNode->FileContent);

    /* remove from parent's list */
    if(oNNode->oNParent != NULL) {
        if(DynArray_bsearch(
                oNNode->oNParent->oFChildren,
                oNNode, &ulIndex,
                (int (*)(const void *, const void *)) Node_compare)
            )
            (void) DynArray_removeAt(oNNode->oNParent->oFChildren,
                                    ulIndex);
    }

    /* remove path */
    Path_free(oNNode->oPPath);

    /* free the struct node */
    free(oNNode);
    ulCount++;
    return ulCount;
}

/*-------------------------------------------------------------------------*/

Path_T Node_getPath(Node_T oNNode) {
   assert(oNNode != NULL);

   return oNNode->oPPath;
}

/*-------------------------------------------------------------------------*/

size_t Node_getNumChildrenFiles(Node_T oNParent) {
   assert(oNParent != NULL);
   assert(!oNParent->isFile);

   return DynArray_getLength(oNParent->oFChildren);
}

/*-------------------------------------------------------------------------*/

size_t Node_getNumChildrenDirs(Node_T oNParent) {
   assert(oNParent != NULL);
   assert(!oNParent->isFile);

   return DynArray_getLength(oNParent->oDChildren);
}

/*-------------------------------------------------------------------------*/

int  Node_getFileChild(Node_T oNParent, size_t ulChildID,
                   Node_T *poNResult) {

   assert(oNParent != NULL);
   assert(poNResult != NULL);
   assert(!oNParent->isFile);

   /* ulChildID is the index into oNParent->oDChildren */
   if(ulChildID >= Node_getNumChildrenFiles(oNParent)) {
      *poNResult = NULL;
      return NO_SUCH_PATH;
   }
   else {
      *poNResult = DynArray_get(oNParent->oFChildren, ulChildID);
      return SUCCESS;
   }
}

/*-------------------------------------------------------------------------*/

int  Node_getDirChild(Node_T oNParent, size_t ulChildID,
                   Node_T *poNResult) {

   assert(oNParent != NULL);
   assert(poNResult != NULL);
   assert(!oNParent->isFile);

   /* ulChildID is the index into oNParent->oDChildren */
   if(ulChildID >= Node_getNumChildrenDirs(oNParent)) {
      *poNResult = NULL;
      return NO_SUCH_PATH;
   }
   else {
      *poNResult = DynArray_get(oNParent->oDChildren, ulChildID);
      return SUCCESS;
   }
}

/*-------------------------------------------------------------------------*/


Node_T Node_getParent(Node_T oNNode) {
   assert(oNNode != NULL);

   return oNNode->oNParent;
}

/*-------------------------------------------------------------------------*/

int Node_compare(Node_T oNFirst, Node_T oNSecond) {
   assert(oNFirst != NULL);
   assert(oNSecond != NULL);

   return Path_comparePath(oNFirst->oPPath, oNSecond->oPPath);
}

/*-------------------------------------------------------------------------*/

char* Node_toString(Node_T oNNode) {
   char *copyPath;

   assert(oNNode != NULL);

   copyPath = malloc(Path_getStrLength(Node_getPath(oNNode))+1);
   if(copyPath == NULL)
      return NULL;
   else
      return strcpy(copyPath, Path_getPathname(Node_getPath(oNNode)));
}

/*-------------------------------------------------------------------------*/

size_t Node_FileLength(Node_T oNNode) {
   assert (oNNode != NULL);
   assert (oNNode->isFile);

   return (oNNode->ulContLength);
}