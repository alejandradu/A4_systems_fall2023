/*--------------------------------------------------------------------*/
/* checkerDT.c                                                        */
/* Author:                                                            */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "checkerDT.h"
#include "dynarray.h"
#include "path.h"
#include "dt.h"


boolean CheckerDT_Node_isValid(Node_T oNNode) {
   Node_T oNParent;
   Path_T oPNPath;
   Path_T oPPPath;

   /* Sample check: a NULL pointer is not a valid node */
   if(oNNode == NULL) {
      fprintf(stderr, "A node is a NULL pointer\n");
      return FALSE;
   }

   /* Sample check: parent's path must be the longest possible
      proper prefix of the node's path */
   oNParent = Node_getParent(oNNode);
   if(oNParent != NULL) {
      oPNPath = Node_getPath(oNNode);
      oPPPath = Node_getPath(oNParent);

      if(Path_getSharedPrefixDepth(oPNPath, oPPPath) !=
         Path_getDepth(oPNPath) - 1) {
         fprintf(stderr, "P-C nodes don't have P-C paths: (%s) (%s)\n",
                 Path_getPathname(oPPPath), Path_getPathname(oPNPath));
         return FALSE;
      }
   }
   return TRUE;
}

/* Validate Node_getChild calls
*  Return TRUE if all assigned pointers to children are not
*  null, return false otherwise */
static boolean check_GetChildNull(Node_T oNNode) {
    Node_T oNChild = NULL;
    size_t ulIndex = 0;
    for (ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++){
        Node_getChild(oNNode, ulIndex, &oNChild);
        if (oNChild == NULL) {
            fprintf(stderr, "Detected a NULL node \n");
            return FALSE;
        }
    }
    return TRUE;
}

/* Validate toString calls
* Assumes that node_toString works. Return TRUE if the string
* representation of the tree contains the string representation
* of all nodes. Return FALSE otherwise */
static boolean check_toStringComplete(Node_T oNNode) {
    char *stringContain;
    stringContain= strstr((const char*)DT_toString(), 
        Node_toString(oNNode)); 
    if (stringContain == NULL) {
        fprintf(stderr,
         "DT_toString does not print all the nodes in the DT\n");
        return FALSE;
    }
    return TRUE;
}

/* Validate uniqueness of paths */
static boolean check_UniquePaths(Node_T oNNode) {
    size_t ulIndex;
    size_t ulIndex2;

    for (ulIndex = 0; ulIndex < Node_getNumChildren(oNNode)-1; 
         ulIndex++) {
        Node_T oNChild = NULL;
        Path_T pathChild1 = NULL;
        Node_getChild(oNNode, ulIndex, &oNChild);
        pathChild1 = Node_getPath(oNChild);
        /* compare paths against all other children */
        for (ulIndex2 = ulIndex +1; 
             ulIndex2 < Node_getNumChildren(oNNode); ulIndex2++){
           Node_T oNChild2 = NULL;
           Path_T pathChild2 = NULL;
           Node_getChild(oNNode, ulIndex2, &oNChild2);
           pathChild2 = Node_getPath(oNChild2);
            if (!Path_comparePath(pathChild1, pathChild2)){
                fprintf(stderr, 
                        "Detected two identical paths in the DT\n");
                return FALSE;
            }
        }
    }
    return TRUE;
}

/* Validate lexicographic order of children */
static boolean check_lexOrder(Node_T oNNode) {
    size_t ulIndex;

    for (ulIndex = 0; ulIndex < Node_getNumChildren(oNNode)-1;
         ulIndex++) {
        Node_T oNChild1 = NULL;
        Path_T pathChild1 = NULL;
        Node_T oNChild2 = NULL;
        Path_T pathChild2 = NULL;
        Node_getChild(oNNode, ulIndex, &oNChild1);
        Node_getChild(oNNode, ulIndex+1, &oNChild2);
        pathChild1 = Node_getPath(oNChild1);
        pathChild2 = Node_getPath(oNChild2);
        if (Path_comparePath(pathChild1, pathChild2)>0){
           fprintf(stderr, 
                  "Children are not arranged in lexicographic order\n");
           return FALSE;
        }
    }
    return TRUE;
}


/* Count the number of valid nodes in a tree rooted at oNRoot
* recursively. Is not affected by the length field of the DT */
static size_t countValidNodes(Node_T oNRoot) {
    size_t count = 0;
    size_t i;

    if (oNRoot == NULL) {
        return 0;
    }

    /* Check if the current node is valid */
    if (CheckerDT_Node_isValid(oNRoot)) {
        count++;
    }

    /* Recursively count the valid nodes in each child subtree */
    for (i = 0; i < Node_getNumChildren(oNRoot); i++) {
        Node_T oNChild = NULL;
        int iStatus = Node_getChild(oNRoot, i, &oNChild);

        if (iStatus == SUCCESS) {
            count += countValidNodes(oNChild);
        }
    }

    return count;
}


/*
   Performs a pre-order traversal of the tree rooted at oNNode.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise.
*/
static boolean CheckerDT_treeCheck(Node_T oNNode) {
   size_t ulIndex;

    if(oNNode!= NULL) {

        /* Sample check on each node: node must be valid */
        /* If not, pass that failure back up immediately */
        if(!CheckerDT_Node_isValid(oNNode))
            return FALSE;

        /* check all getchild calls return not null */
        if (Node_getNumChildren(oNNode) > 0) {
            if(!check_GetChildNull(oNNode))
            return FALSE;
        }

        /* check if toString returns the path names of all nodes,
        assuming that node_toString works*/
        if(!check_toStringComplete(oNNode)) 
            return FALSE;

        if (Node_getNumChildren(oNNode) > 1) {
            /* check if every path of each node's children is unique*/
            if(!check_UniquePaths(oNNode)) 
                return FALSE;
            /* check if the children are in lexicographic order */
            if(!check_lexOrder(oNNode))
                return FALSE;
        }


        /* Recur on every child of oNNode */
        for(ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); 
            ulIndex++) {
            Node_T oNChild = NULL;
            int iStatus = Node_getChild(oNNode, ulIndex, &oNChild);
   
            if(iStatus != SUCCESS) {
               fprintf(stderr, "getNumChildren claims more"
                                    "children than getChild returns\n");
               return FALSE;
            }

            /* if recurring down one subtree results in a failed check
               farther down, passes the failure back up immediately */
            if(!CheckerDT_treeCheck(oNChild))
               return FALSE;

        }

    }
    return TRUE;
}



boolean CheckerDT_isValid(boolean bIsInitialized, Node_T oNRoot,
                          size_t ulCount) {

   size_t totalCount;

   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
   if(!bIsInitialized)
      if(ulCount != 0) {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }

    /* check minimal size */
    if (oNRoot == NULL && ulCount != 0) {
        fprintf(stderr, "ulCount is not 0, but the root is NULL\n");
        return FALSE;
    }

    /* NEW: check length field agrees with node count */
    totalCount = countValidNodes(oNRoot);
    if (totalCount != ulCount) {
        fprintf(stderr, 
          "DT length does not equal total number of nodes detected \n");
        fprintf(stderr, "Length: %lu. Nodes detected: %lu\n",
                ulCount, totalCount);
        return FALSE;
    }   

   /* Now checks invariants recursively at each node from the root. */
   return CheckerDT_treeCheck(oNRoot);
}
