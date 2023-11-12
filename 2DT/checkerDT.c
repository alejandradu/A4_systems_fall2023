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

   /* NEW: if the path */

   return TRUE;
}

/* NEW: check all getchild calls return not null */
static boolean check_GetChildNull(Node_T oNNode) {
    Node_T oNChild = NULL;
    size_t ulIndex = 0;
    for (ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++) {
        Node_getChild(oNNode, ulIndex, &oNChild);
        if (oNChild == NULL) {
            fprintf(stderr, "Detected a NULL node \n");
            return FALSE;
        }
    }
    return TRUE;
}

/* NEW: check if toString returns the path names of all nodes,
assuming that node_toString works*/
static boolean check_toStringComplete(Node_T oNNode) {
    char *stringContain;
    stringContain= strstr((const char*)DT_toString(), Node_toString(oNNode)); 
    if (stringContain == NULL) {
        fprintf(stderr, "DT_toString does not print all the nodes in the DT\n");
        return FALSE;
    }
    return TRUE;
}

/* NEW: check if every path of each node's children is unique */
static boolean check_UniquePaths(Node_T oNNode) {
    size_t ulIndex;
    size_t ulIndex2;

    for (ulIndex = 0; ulIndex < Node_getNumChildren(oNNode)-1; ulIndex++) {
        Node_T oNChild = NULL;
        Path_T pathChild1 = NULL;
        Node_getChild(oNNode, ulIndex, &oNChild);
        pathChild1 = Node_getPath(oNChild);
        /* do pairwise comparisons */
        for (ulIndex2 = ulIndex +1; ulIndex2 < Node_getNumChildren(oNNode); ulIndex2++){
           Node_T oNChild2 = NULL;
           Path_T pathChild2 = NULL;
           Node_getChild(oNNode, ulIndex2, &oNChild2);
           pathChild2 = Node_getPath(oNChild2);
            if (!Path_comparePath(pathChild1, pathChild2)){
                fprintf(stderr, "Detected two identical paths in the DT\n");
                return FALSE;
            }
        }
    }
    return TRUE;
}

/* NEW: check if the children are arranged in lexicographic order */
static boolean check_lexOrder(Node_T oNNode) {
    size_t ulIndex;

    for (ulIndex = 0; ulIndex < Node_getNumChildren(oNNode)-1; ulIndex++) {
        Node_T oNChild1 = NULL;
        Path_T pathChild1 = NULL;
        Node_T oNChild2 = NULL;
        Path_T pathChild2 = NULL;
        Node_getChild(oNNode, ulIndex, &oNChild1);
        Node_getChild(oNNode, ulIndex+1, &oNChild2);
        pathChild1 = Node_getPath(oNChild1);
        pathChild2 = Node_getPath(oNChild2);
        if (Path_comparePath(pathChild1, pathChild2)>0){
           fprintf(stderr, "Children are not arranged in lexicographic order\n");
           return FALSE;
        }
    }
    return TRUE;
}


/*
   Performs a pre-order traversal of the tree rooted at oNNode.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise.

   You may want to change this function's return type or
   parameter list to facilitate constructing your checks.
   If you do, you should update this function comment.

   TODO: change function description

   THIS IS FOR LOWER-LEVEL DT FUNCTIONS
*/
static size_t CheckerDT_treeCheck(Node_T oNNode, size_t ptotalCount, boolean *result, size_t ulCount) {
   size_t ulIndex;

   /*fprintf(stderr, "ulCount (from isValid) %lu, my count (from treecheck) %lu \n", ulCount, ptotalCount);*/
   if (ulCount > 0) {
    fprintf(stderr, "ulCount is %lu, while total number of nodes detected is %lu\n", ulCount, ptotalCount);
    if (ulCount == ptotalCount){
                fprintf(stderr, "ERROR \n");
                fprintf(stderr, "ulCount is %lu, while total number of nodes detected is %lu\n", ulCount, ptotalCount);
            *result = FALSE;
    }
   }

    if(oNNode!= NULL && *result != FALSE) {

        /* Sample check on each node: node must be valid */
        /* If not, pass that failure back up immediately */
        if(!CheckerDT_Node_isValid(oNNode))
            *result = FALSE;

        /* NEW: check all getchild calls return not null */
        /* QUESTION: isn't this contained in the first function here? */
        if (Node_getNumChildren(oNNode) > 0) {
            if(!check_GetChildNull(oNNode))
            *result = FALSE;
        }

        /* NEW: check if toString returns the path names of all nodes,
        assuming that node_toString works*/
        if(!check_toStringComplete(oNNode)) 
            *result = FALSE;

        if (Node_getNumChildren(oNNode) > 1) {
            /* NEW: check if every path of each node's children is unique*/
            if(!check_UniquePaths(oNNode)) 
                *result = FALSE;
            /* NEW: check if the children are arranged in lexicographic order */
            if(!check_lexOrder(oNNode))
                *result = FALSE;
        }

        /* add to node count */
        ptotalCount++;

        /* Recur on every child of oNNode */
        for(ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++) {
            Node_T oNChild = NULL;
            int iStatus = Node_getChild(oNNode, ulIndex, &oNChild);
   
            if(iStatus != SUCCESS) {
               fprintf(stderr, "getNumChildren claims more children than getChild returns\n");
               *result = FALSE;
            }

            /* if recurring down one subtree results in a failed check
               farther down, passes the failure back up immediately */
            if(!CheckerDT_treeCheck(oNChild, ptotalCount, result, ulCount))
               *result = FALSE;

            /* NEW: update index mimic DT_preOrderTraversal */
            ptotalCount = CheckerDT_treeCheck(oNChild, ptotalCount, result, ulCount);
      }
   }

   return ptotalCount;
}


boolean CheckerDT_isValid(boolean bIsInitialized, Node_T oNRoot,
                          size_t ulCount) {

   size_t totalCount = 0;
   boolean treecheck_result = TRUE;   /* INIT TO THIS? */

   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
   if(!bIsInitialized) {
      if(ulCount != 0) {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }
   }

   totalCount = CheckerDT_treeCheck(oNRoot, totalCount, &treecheck_result, ulCount);

   /* NEW: check if ulCount equals the total number of nodes detected*/
   /* if (ulCount != totalCount){
            fprintf(stderr, "ERROR ulCount does not equal total number of nodes detected \n");
            fprintf(stderr, "ulCount is %lu, while total number of nodes detected is %lu\n", ulCount, totalCount);
            return FALSE;
        } 
    if (treecheck && (ulCount > 0)) {
        fprintf(stderr, "ulCount %lu, my count %lu \n", ulCount, totalCount);
        if (ulCount != totalCount){
            fprintf(stderr, "ulCount does not equal total number of nodes detected \n");
            fprintf(stderr, "ulCount is %lu, while total number of nodes detected is %lu\n", ulCount, totalCount);
            return FALSE;
        }  
    }
*/
   /* Now checks invariants recursively at each node from the root. */
   return treecheck_result;
                          
}