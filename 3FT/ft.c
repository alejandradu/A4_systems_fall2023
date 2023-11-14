/*--------------------------------------------------------------------*/
/* ft.c                                                               */
/* Author: Christopher Moretti                                        */
/*--------------------------------------------------------------------*/

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "dynarray.h"
#include "path.h"
#include "nodeFT.h"
#include "ft.h"
#include "a4def.h"


/*
  A File Tree is a representation of a hierarchy of directories and
  files: the File Tree is rooted at a directory, directories
  may be internal nodes or leaves, and files are always leaves.
*/

/* Global static flags */
/* 1. a flag for being in an initialized state */
static boolean isInitialized = FALSE;
/* 2. a pointer to the root of the hierarchy */
static Node_T oNRoot = NULL;
/* 3. a counter for the files */
static size_t fileCounter = 0;
/* 4. a counter for the directories */
static size_t dirCounter = 0;
/* 4. a counter for all the nodes */
static size_t NodeCounter = 0;



/* --------------------------------------------------------------------

  The FT_traversePath and FT_findNode functions modularize the common
  functionality of going as far as possible down an FT towards a path
  and returning either the node of however far was reached or the
  node if the full path was reached, respectively.
*/

/*
  Traverses the FT to find a node with absolute path pcPath. Returns a
  int SUCCESS status and sets *poNResult to be the node, if found.
  Otherwise, sets *poNResult to NULL and returns with status:
  * INITIALIZATION_ERROR if the FT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root's path is not a prefix of pcPath
  * NO_SUCH_PATH if no node with pcPath exists in the hierarchy
  * MEMORY_ERROR if memory could not be allocated to complete request
 */
static int FT_traversePath(Path_T oPPath, Node_T *poNFurthest) {
   int iStatus;
   Path_T oPPrefix = NULL;
   Node_T oNCurr;
   Node_T oNChild = NULL;
   size_t ulDepth;
   size_t i;
   size_t ulChildID;
   boolean isFile;

   assert(oPPath != NULL);
   assert(poNFurthest != NULL);

   /* root is NULL -> won't find anything */
   if(oNRoot == NULL) {
      *poNFurthest = NULL;
      return SUCCESS;
   }

   iStatus = Path_prefix(oPPath, 1, &oPPrefix);
   if(iStatus != SUCCESS) {
      *poNFurthest = NULL;
      return iStatus;
   }

   if(Path_comparePath(Node_getPath(oNRoot), oPPrefix)) {
      Path_free(oPPrefix);
      *poNFurthest = NULL;
      return CONFLICTING_PATH;
   }
   Path_free(oPPrefix);
   oPPrefix = NULL;

   oNCurr = oNRoot;
   ulDepth = Path_getDepth(oPPath);
   for(i = 2; i <= ulDepth; i++) { 
      iStatus = Path_prefix(oPPath, i, &oPPrefix);
      if(iStatus != SUCCESS) {
         *poNFurthest = NULL;
         return iStatus;
      }
      if(Node_hasChild(oNCurr, oPPrefix, &isFile,&ulChildID)) {
         /* go to that child and continue with next prefix */
         Path_free(oPPrefix);
         oPPrefix = NULL;
         if (isFile) {
            iStatus = Node_getFileChild(oNCurr, ulChildID, &oNChild);
            if(iStatus != SUCCESS) {
               *poNFurthest = NULL;
               return iStatus;
            }
            oNCurr = oNChild;
            break;
         } else {
            iStatus = Node_getDirChild(oNCurr, ulChildID, &oNChild);
            if(iStatus != SUCCESS) {
               *poNFurthest = NULL;
               return iStatus;
            }
            oNCurr = oNChild;
         }
         
      }
      else {
         /* oNCurr doesn't have child with path oPPrefix:
            this is as far as we can go */
         break;
      }
   }

  Path_free(oPPrefix);
  *poNFurthest = oNCurr;
  return SUCCESS;

}

/*
  Traverses the FT to find a node with absolute path pcPath. Returns a
  int SUCCESS status and sets *poNResult to be the node, if found.
  Otherwise, sets *poNResult to NULL and returns with status:
  * INITIALIZATION_ERROR if the FT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root's path is not a prefix of pcPath
  * NO_SUCH_PATH if no node with pcPath exists in the hierarchy
  * MEMORY_ERROR if memory could not be allocated to complete request
  * NOT_A_DIRECTORY if the node at pcPath is not of the correct type
 */
static int FT_findNode(const char *pcPath, Node_T *poNResult, boolean isFile) {
   Path_T oPPath = NULL;
   Node_T oNFound = NULL;
   int iStatus;

   assert(pcPath != NULL);
   assert(poNResult != NULL);

   if(!isInitialized) {
      *poNResult = NULL;
      return INITIALIZATION_ERROR;
   }

   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS) {
      *poNResult = NULL;
      return iStatus;
   }

   iStatus = FT_traversePath(oPPath, &oNFound);
   if(iStatus != SUCCESS) {
      Path_free(oPPath);
      *poNResult = NULL;
      return iStatus;
   }

   if(oNFound == NULL) {
      Path_free(oPPath);
      *poNResult = NULL;
      return NO_SUCH_PATH;
   }

   if(Path_comparePath(Node_getPath(oNFound), oPPath) != 0) {
      Path_free(oPPath);
      *poNResult = NULL;
      return NO_SUCH_PATH;
   }

   /* check the type of the node at path matched */
   assert(oNFound != NULL);
   if(((isFile) && (!Node_isFile(oNFound))) || ((!isFile) && (Node_isFile(oNFound)))) {
      Path_free(oPPath);
      *poNResult = NULL;
      return NOT_A_DIRECTORY;
   }

   Path_free(oPPath);
   *poNResult = oNFound;
   return SUCCESS;
}

/* TODO: make the function more concise?? break up the functions?? */
/* static function for remove?? */

/*--------------------------------------------------------------------*/

/* Encapsulation of the algorithm to insert a new node accounting
for the differences between a file and a directory */
static int FT_insertions(const char *pcPath, boolean isFile, void* FileContent, size_t fileLength) {
    int iStatus;
    Path_T oPPath = NULL;
    Node_T oNFirstNew = NULL;
    Node_T oNCurr = NULL;
    size_t ulDepth, ulIndex;
    size_t ulNewNodes = 0;

    assert(pcPath != NULL);
 
     /* validate initialization */
    if(!isInitialized) {
        return INITIALIZATION_ERROR; 
    }    
    /* create path for the (potential) insertion */
    iStatus = Path_new(pcPath, &oPPath);    
    if(iStatus != SUCCESS) {
        return iStatus;
    }    
 
    /* find the closest ancestor of oPPath already in the tree */
    iStatus= FT_traversePath(oPPath, &oNCurr);
    if(iStatus != SUCCESS)
    {
        Path_free(oPPath);
        return iStatus;
    }
  
    assert(oNCurr != NULL);
     /* fails if the closest ancestor is a file */
    if(Node_isFile(oNCurr)) {
        Path_free(oPPath);
        return NOT_A_DIRECTORY;
    }
  
     /* no ancestor and root not NULL, pcPath isn't underneath root. */
    if(oNCurr == NULL && oNRoot != NULL) {
        Path_free(oPPath);
        return CONFLICTING_PATH;
    }
  
    ulDepth = Path_getDepth(oPPath);
    if(oNCurr == NULL) /* new root */
        ulIndex = 1;
    else {
        /* index to insert the node */
        ulIndex = Path_getDepth(Node_getPath(oNCurr))+1;
 
         /* fails if there is already a node (any type) with that 
        path at that depth */
        if(ulIndex == ulDepth+1 && !Path_comparePath(oPPath,
                                         Node_getPath(oNCurr))) {
           Path_free(oPPath);
           return ALREADY_IN_TREE;
        }
    }

    /* starting at oNCurr, build rest of the path one level at a time */
    while(ulIndex <= ulDepth) {
        Path_T oPPrefix = NULL;
        Node_T oNNewNode = NULL;
  
        /* generate a Path_T for this level */
        iStatus = Path_prefix(oPPath, ulIndex, &oPPrefix);
        if(iStatus != SUCCESS) {
           Path_free(oPPath);
           if(oNFirstNew != NULL){
               if (isFile)
                  (void) Node_File_free(oNFirstNew);
               else {
                  (void) Node_Dir_free(oNFirstNew);
               }
           } 
           /*assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));*/
           return iStatus;
        }
  
        /* insert the new node for this level */
        iStatus = Node_new(oPPrefix, oNCurr, isFile, FileContent, fileLength, &oNNewNode);
        if(iStatus != SUCCESS) {
           Path_free(oPPath);
           Path_free(oPPrefix);
           if(oNFirstNew != NULL) {
              if(isFile)
                 (void) Node_File_free(oNFirstNew);
              else
                 (void) Node_Dir_free(oNFirstNew);
           }
           /*assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));*/
           return iStatus;
        }
  
        /* set up for next level */
        Path_free(oPPrefix);
        oNCurr = oNNewNode;
        ulNewNodes++;
        if(oNFirstNew == NULL)
           oNFirstNew = oNCurr;
        ulIndex++;
    }

    Path_free(oPPath);
    /* update FT state variables to reflect insertion */
    if(oNRoot == NULL)
      oNRoot = oNFirstNew;
    if(isFile)
        fileCounter += ulNewNodes;
    else
        dirCounter += ulNewNodes;

   /*assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));*/
   return SUCCESS;

}


/*
  Performs a pre-order traversal of the tree rooted at n,
  inserting each payload to DynArray_T d beginning at index i.
  Returns the next unused index in d after the insertion(s).
*/

static size_t FT_preOrderTraversal(Node_T oNRoot, DynArray_T AllNodesArray, size_t index) {
   size_t c;
   size_t fileChildIndex;

   assert(AllNodesArray != NULL);

   if(oNRoot != NULL) {
      (void) DynArray_set(AllNodesArray, index, oNRoot);
      index++;
      for( fileChildIndex = 0; fileChildIndex < Node_getNumChildrenFiles(oNRoot); fileChildIndex++) {
         int iStatus;
         Node_T oNChild = NULL;
         iStatus = Node_getDirChild(oNRoot,fileChildIndex, &oNChild);
         assert(iStatus == SUCCESS);
         (void) DynArray_set(AllNodesArray, index, oNChild);
         index++;
      }
      for(c = 0; c < Node_getNumChildrenDirs(oNRoot); c++) {
         int iStatus;
         Node_T oNChild = NULL;
         iStatus = Node_getDirChild(oNRoot,c, &oNChild);
         assert(iStatus == SUCCESS);
         index = FT_preOrderTraversal(oNChild, AllNodesArray, index);
      }
   }
   return index;
}


/*
  Alternate version of strlen that uses pulAcc as an in-out parameter
  to accumulate a string length, rather than returning the length of
  oNNode's path, and also always adds one addition byte to the sum.
*/

/*not modified for FT*/
static void FT_strlenAccumulate(Node_T oNNode, size_t *pulAcc) {
   assert(pulAcc != NULL);

   if(oNNode != NULL)
      *pulAcc += (Path_getStrLength(Node_getPath(oNNode)) + 1);
}


/*
  Alternate version of strcat that inverts the typical argument
  order, appending oNNode's path onto pcAcc, and also always adds one
  newline at the end of the concatenated string.
*/
/*not yet modified for FT*/
static void FT_strcatAccumulate(Node_T oNNode, char *pcAcc) {
   assert(pcAcc != NULL);

   if(oNNode != NULL) {
      strcat(pcAcc, Path_getPathname(Node_getPath(oNNode)));
      strcat(pcAcc, "\n");
   }
}



/*
   Inserts a new directory into the FT with absolute path pcPath.
   Returns SUCCESS if the new directory is inserted successfully.
   Otherwise, returns:
   * INITIALIZATION_ERROR if the FT is not in an initialized state
   * BAD_PATH if pcPath does not represent a well-formatted path
   * CONFLICTING_PATH if the root exists but is not a prefix of pcPath
   * NOT_A_DIRECTORY if a proper prefix of pcPath exists as a file
   * ALREADY_IN_TREE if pcPath is already in the FT (as dir or file)
   * MEMORY_ERROR if memory could not be allocated to complete request
*/
int FT_insertDir(const char *pcPath) {

    assert(pcPath != NULL);

    return FT_insertions(pcPath, FALSE, NULL, 0);
}


/*
  Returns TRUE if the FT contains a directory with absolute path
  pcPath and FALSE if not or if there is an error while checking.
*/
boolean FT_containsDir(const char *pcPath) {
    int iStatus;
    Node_T oNFound = NULL;

    assert(pcPath != NULL);

    iStatus = FT_findNode(pcPath, &oNFound, FALSE);
    return (iStatus == SUCCESS);
}


/*
  Removes the FT hierarchy (subtree) at the directory with absolute
  path pcPath. Returns SUCCESS if found and removed.
  Otherwise, returns:
  * INITIALIZATION_ERROR if the FT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root exists but is not a prefix of pcPath
  * NO_SUCH_PATH if absolute path pcPath does not exist in the FT
  * NOT_A_DIRECTORY if pcPath is in the FT as a file not a directory
  * MEMORY_ERROR if memory could not be allocated to complete request
*/
int FT_rmDir(const char *pcPath) {
   int iStatus;
   Node_T oNFound = NULL;

   assert(pcPath != NULL);
   /*assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));*/

   iStatus = FT_findNode(pcPath, &oNFound, FALSE);

   if(iStatus != SUCCESS)
       return iStatus;

   dirCounter -= Node_Dir_free(oNFound);
   if(dirCounter == 0)
      oNRoot = NULL;

   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/
   return SUCCESS;
}


/*
   Inserts a new file into the FT with absolute path pcPath, with
   file contents pvContents of size ulLength bytes.
   Returns SUCCESS if the new file is inserted successfully.
   Otherwise, returns:
   * INITIALIZATION_ERROR if the FT is not in an initialized state
   * BAD_PATH if pcPath does not represent a well-formatted path
   * CONFLICTING_PATH if the root exists but is not a prefix of pcPath,
                      or if the new file would be the FT root
   * NOT_A_DIRECTORY if a proper prefix of pcPath exists as a file
   * ALREADY_IN_TREE if pcPath is already in the FT (as dir or file)
   * MEMORY_ERROR if memory could not be allocated to complete request
*/
int FT_insertFile(const char *pcPath, void *pvContents,
                  size_t ulLength) {
    
    assert(pcPath != NULL);

    return FT_insertions(pcPath, TRUE, pvContents, ulLength);
}

/*
  Returns TRUE if the FT contains a file with absolute path
  pcPath and FALSE if not or if there is an error while checking.
*/
boolean FT_containsFile(const char *pcPath) {
    int iStatus;
    Node_T oNFound = NULL;

    assert(pcPath != NULL);

    iStatus = FT_findNode(pcPath, &oNFound, TRUE);
    return (iStatus == SUCCESS);
}

/*
  Removes the FT file with absolute path pcPath.
  Returns SUCCESS if found and removed.
  Otherwise, returns:
  * INITIALIZATION_ERROR if the FT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root exists but is not a prefix of pcPath
  * NO_SUCH_PATH if absolute path pcPath does not exist in the FT
  * NOT_A_FILE if pcPath is in the FT as a directory not a file
  * MEMORY_ERROR if memory could not be allocated to complete request
*/
int FT_rmFile(const char *pcPath) {
   int iStatus;
   Node_T oNFound = NULL;

   assert(pcPath != NULL);
   /*assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));*/

   iStatus = FT_findNode(pcPath, &oNFound, TRUE);

   if(iStatus != SUCCESS)
       return iStatus;

   fileCounter -= Node_File_free(oNFound);
   if(fileCounter == 0)
      oNRoot = NULL;

   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/
   return SUCCESS;
}

/*
  Returns the contents of the file with absolute path pcPath.
  Returns NULL if unable to complete the request for any reason.

  Note: checking for a non-NULL return is not an appropriate
  contains check, because the contents of a file may be NULL.
*/
void *FT_getFileContents(const char *pcPath) {
    int iStatus;
    Node_T oNFound = NULL;

    assert(pcPath != NULL);

    iStatus = FT_findNode(pcPath, &oNFound, TRUE);  
    /* file with given path does not exist */
    if(iStatus != SUCCESS)
        return NULL;

    return Node_getContent(oNFound);
}

/*
  Replaces current contents of the file with absolute path pcPath with
  the parameter pvNewContents of size ulNewLength bytes.
  Returns the old contents if successful. (Note: contents may be NULL.)
  Returns NULL if unable to complete the request for any reason.
*/
void *FT_replaceFileContents(const char *pcPath, void *pvNewContents,
                             size_t ulNewLength) {
    
    int iStatus;
    Node_T oNFound = NULL;

    assert(pcPath != NULL);

    iStatus = FT_findNode(pcPath, &oNFound, TRUE);  
    /* file with given path does not exist */
    if(iStatus != SUCCESS)
        return NULL;

    return Node_ReplaceFileContent(oNFound, pvNewContents, ulNewLength);
}

/*
  Returns SUCCESS if pcPath exists in the hierarchy,
  Otherwise, returns:
  * INITIALIZATION_ERROR if the FT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root's path is not a prefix of pcPath
  * NO_SUCH_PATH if absolute path pcPath does not exist in the FT
  * MEMORY_ERROR if memory could not be allocated to complete request

  When returning SUCCESS,
  if path is a directory: sets *pbIsFile to FALSE, *pulSize unchanged
  if path is a file: sets *pbIsFile to TRUE, and
                     sets *pulSize to the length of file's contents

  When returning another status, *pbIsFile and *pulSize are unchanged.
*/
int FT_stat(const char *pcPath, boolean *pbIsFile, size_t *pulSize) {
   Path_T oPPath = NULL;
   Node_T oNFound = NULL;
   boolean isFile;
   int iStatus;

   assert(pcPath != NULL);
   assert(pbIsFile != NULL);
   assert(pulSize != NULL);

   if(!isInitialized) {
      return INITIALIZATION_ERROR;
   }

   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS) {
      return iStatus;
   }

   iStatus = FT_traversePath(oPPath, &oNFound);
   if(iStatus != SUCCESS)
   {
      Path_free(oPPath);
      return iStatus;
   }

   if(oNFound == NULL) {
      Path_free(oPPath);
      return NO_SUCH_PATH;
   }

   if(Path_comparePath(Node_getPath(oNFound), oPPath) != 0) {
      Path_free(oPPath);
      return NO_SUCH_PATH;
   }

   Path_free(oPPath);
   assert(oNFound != NULL);
   isFile = Node_isFile(oNFound);
   if (isFile) {
      *pbIsFile = TRUE;
      *pulSize = Node_FileLength(oNFound);
   } else {
      *pbIsFile = FALSE;
   }
   return SUCCESS;
}

/*
  Sets the FT data structure to an initialized state.
  The data structure is initially empty.
  Returns INITIALIZATION_ERROR if already initialized,
  and SUCCESS otherwise.
*/
int FT_init(void) {
   if(isInitialized)
      return INITIALIZATION_ERROR;

   isInitialized = TRUE;
   oNRoot = NULL;
   fileCounter = 0;
   dirCounter = 0;
   NodeCounter = 0;

   return SUCCESS;
}

/*
  Removes all contents of the data structure and
  returns it to an uninitialized state.
  Returns INITIALIZATION_ERROR if not already initialized,
  and SUCCESS otherwise.
*/
int FT_destroy(void) {

   if(!isInitialized)
      return INITIALIZATION_ERROR;

   if(oNRoot) {
      NodeCounter -= Node_Dir_free(oNRoot);
      oNRoot = NULL;
   }

   isInitialized = FALSE;

   return SUCCESS;
}

/*
  Returns a string representation of the
  data structure, or NULL if the structure is
  not initialized or there is an allocation error.

  The representation is depth-first with files
  before directories at any given level, and nodes
  of the same type ordered lexicographically.

  Allocates memory for the returned string,
  which is then owned by client!
*/

/*not yet modified for FT*/
char *FT_toString(void) {
   DynArray_T nodes;
   size_t totalStrlen = 1;
   char *result = NULL;

   if(!isInitialized)
      return NULL;

   nodes = DynArray_new(NodeCounter);
   (void) FT_preOrderTraversal(oNRoot, nodes, 0);

   DynArray_map(nodes, (void (*)(void *, void*)) FT_strlenAccumulate,
                (void*) &totalStrlen);

   result = malloc(totalStrlen);
   if(result == NULL) {
      DynArray_free(nodes);
      return NULL;
   }
   *result = '\0';

   DynArray_map(nodes, (void (*)(void *, void*)) FT_strcatAccumulate,
                (void *) result);

   DynArray_free(nodes);

   return result;
}



