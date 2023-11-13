/*--------------------------------------------------------------------*/
/* nodeFT.h                                                           */
/* Author: Christopher Moretti                                        */
/*--------------------------------------------------------------------*/

#ifndef NODE_INCLUDED
#define NODE_INCLUDED

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "a4def.h"
#include "dynarray.h"
#include "path.h"



/* A Node_T is a node in a File Tree */
typedef struct node *Node_T;

/*
  Creates a new node in the File Tree, with path oPPath and
  parent oNParent, boolean isFile, FileContent. Returns an int SUCCESS status and sets *poNResult
  to be the new node if successful. Otherwise, sets *poNResult to NULL
  and returns status:
  * MEMORY_ERROR if memory could not be allocated to complete request
  * CONFLICTING_PATH if oNParent's path is not an ancestor of oPPath
  * NO_SUCH_PATH if oPPath is of depth 0
                 or oNParent's path is not oPPath's direct parent
                 or oNParent is NULL but oPPath is not of depth 1
  * ALREADY_IN_TREE if oNParent already has a child with this path
*/
int Node_new(Path_T oPPath, Node_T oNParent, 
      boolean isFile, void* FileContent, size_t ulContLength, Node_T *poNResult);

/* Takes in a Node and returns True if it is a file, false if it is a directory*/
boolean Node_isFile(Node_T oNNode);


/* Takes in a File Node and returns its file content*/
void Node_getContent(Node_T oNNode);

/*
  Replaces current contents of the file at the node oNNode with new FileContent 
  Returns the old contents if successful. (Note: contents may be NULL.)
  Returns NULL if unable to complete the request for any reason.
  Fails if a non-file Node is pluged into this function
*/
void *Node_ReplaceFileContent(Node_T oNNode, void* NewFileContent, size_t ulNewLength);

/*
  Returns TRUE if oNParent has a child with path oPPath. Returns
  FALSE if it does not.

  If oNParent has such a child, stores in *pulChildID the child's
  identifier (as used in Node_getChild). If oNParent does not have
  such a child, stores in *pulChildID the identifier that such a
  child _would_ have if inserted.

  stores TRUE in *pisFile if the child identified is a File, false if it is
  not a file or if it is not existent 
*/
boolean Node_hasChild(Node_T oNParent, Path_T oPPath, boolean *pisFile,
                         size_t *pulChildID);


/*
  Destroys and frees all memory allocated for the subtree rooted at
  oNNode, i.e., deletes this node and all its descendents. Returns the
  number of nodes deleted.
*/
size_t Node_Dir_free(Node_T oNNode);

/*
  Destroys and frees all memory allocated for the file of
  oNNode. Returns the number of nodes deleted.
*/
size_t Node_File_free(Node_T oNNode);

/* Returns the path object representing oNNode's absolute path. */
Path_T Node_getPath(Node_T oNNode); 

/* Returns the number of file children that oNParent has. */
size_t Node_getNumChildrenFiles(Node_T oNParent); 

/* Returns the number of Directory children that oNParent has. */
size_t Node_getNumChildrenDirs(Node_T oNParent); 

/*
  Returns an int SUCCESS status and sets *poNResult to be the child
  file node of oNParent with identifier ulChildID, if one exists.
  Otherwise, sets *poNResult to NULL and returns status:
  * NO_SUCH_PATH if ulChildID is not a valid child for oNParent
*/
int  Node_getFileChild(Node_T oNParent, size_t ulChildID,
                   Node_T *poNResult);

/*
  Returns an int SUCCESS status and sets *poNResult to be the child
  directory node of oNParent with identifier ulChildID, if one exists.
  Otherwise, sets *poNResult to NULL and returns status:
  * NO_SUCH_PATH if ulChildID is not a valid child for oNParent
*/
int  Node_getDirChild(Node_T oNParent, size_t ulChildID,
                   Node_T *poNResult);

/*
  Returns a the parent node of oNNode.
  Returns NULL if oNNode is the root and thus has no parent.
*/
Node_T Node_getParent(Node_T oNNode); /*works*/

/*
  Compares oNFirst and oNSecond lexicographically based on their paths.
  Returns <0, 0, or >0 if onFirst is "less than", "equal to", or
  "greater than" oNSecond, respectively.
*/
int Node_compare(Node_T oNFirst, Node_T oNSecond);

/*
  Returns a string representation for oNNode, or NULL if
  there is an allocation error.

  Allocates memory for the returned string, which is then owned by
  the caller!
*/
char *Node_toString(Node_T oNNode);

#endif