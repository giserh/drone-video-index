#ifndef  TETRARTREE_H
#define  TETRARTREE_H

#include <vector>
#include <stdio.h>
#include "basics.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

#define  METHODS  1
#define  PAGE_SIZE    16384 //16384 //8192 //16KB//4096 //4KB
#define  DIMS_NUMB    2       // number of dimensions
#define  SIDES_NUMB   2*DIMS_NUMB//min max
#define  MBR_NUMB     4

/* max branching factor of a node */
//Assume one index node takes one page
//#define MAXCARD (int)((PAGE_SIZE-(4*sizeof(int))) / sizeof(TETRARTREEBRANCH))

#define MAXCARD 203

#ifndef INVALD_RECT
  #define INVALID_RECT(x) ((x)->bound[0] > (x)->bound[DIMS_NUMB])//min>max,����Ч����
#endif

#ifndef UnitSphereVolume
  #define UnitSphereVolume UnitSphereVolumes[DIMS_NUMB]
#endif

/**
 * Precomputed volumes of the unit spheres for the first few dimensions
 */
const double UnitSphereVolumes[] = {
 0.000000,  /* dimension   0 */
 2.000000,  /* dimension   1 */
 3.141593,  /* dimension   2 */
 4.188790,  /* dimension   3 */
 4.934802,  /* dimension   4 */
 5.263789,  /* dimension   5 */
 5.167713,  /* dimension   6 */
 4.724766,  /* dimension   7 */
 4.058712,  /* dimension   8 */
 3.298509,  /* dimension   9 */
 2.550164,  /* dimension  10 */
 1.884104,  /* dimension  11 */
 1.335263,  /* dimension  12 */
 0.910629,  /* dimension  13 */
 0.599265,  /* dimension  14 */
 0.381443,  /* dimension  15 */
 0.235331,  /* dimension  16 */
 0.140981,  /* dimension  17 */
 0.082146,  /* dimension  18 */
 0.046622,  /* dimension  19 */
 0.025807,  /* dimension  20 */
};


/**
 * The data structure of Minimum Bounding Rectangle (MBR)
 */
typedef struct _MBR
{
	/**
	 * minX, minY,...,Xmax, maxY,...
	 * i.e., minLng, minLat, maxLng, maxLat
	 */
	REALTYPE bound[SIDES_NUMB];
}MBR,*pMBR;




/**
 * The data structure of Minimum Bounding Orientation (MBO)
 */
typedef struct _MBO
{
	REALTYPE min;
	REALTYPE max;
}MBO, *pMBO;


/**
 * The data structure of TetraR-tree node branch.
 * A TetraR-tree node includes a set of branches.
 */
typedef struct _TETRARTREEBRANCH
{
	/**
	 * The four MBRs or points of the quadrilaterals in the subtree.
     */
	MBR mbrs[MBR_NUMB];

	MBO orientation; //MBO

    /**
     * For non-leaf node: id of child node;
	 * For leaf node: file offset of object
	 */
	int childid;
}TETRARTREEBRANCH,*pTETRARTREEBRANCH;



/**
 * The data structure of TetraR-tree node
 */
typedef struct _TETRARTREENODE
{
	/**
     * mark the node space in the disk is taken or not
	 * mark 1 if taken; otherwise mark 0.
	 */
	int taken;

	/**
	 * Node ID
	 */
	int nodeid;

	/**
	 * The number of child branches of the node
	 */
    int count;

	/**
	 * The level of the node in the tree.
	 * 0 if leaf node; otherwise positive.
	 */
    int level;
 TETRARTREEBRANCH  branch[MAXCARD];
}TETRARTREENODE,*pTETRARTREENODE;




/**
 * The index node loader that is used to store the information during partition.
 * Store the branches of a node to be splitted and the branch to be inserted.
 */
typedef struct _TETRARTREEPARTITION
{
	/*Store the group/partition number for each branch in the partition*/
	int   partition[MAXCARD+1];

    /*Number of total branches*/
	int   total;

    /*The minimum area it covers*/
	int   minfill;
	
	/*Mark each branch whether be taken, i.e., whether be grouped*/
	int   taken[MAXCARD+1];

    /*The number of branches for each group*/
	int   count[2];

    /*The allover four-MBRs of MBRs in the two groups*/
	MBR cover[2][MBR_NUMB];

    /*The allover MBOs of MBOs in the two groups*/
	MBO coverOrientation[2];

    /**
	 * The allover score (combination score of MBR area and orientation 
	 * difference) for each group
	 */
	REALTYPE alloverScore[2];
} TETRARTREEPARTITION;





/**
 * Store the root node information. 
 * It inludes subtree branch info used for partition.
 */
typedef struct _TETRARTREEROOT
{
	/*root node*/
	TETRARTREENODE*  root_node;
	
	/*branches to be partitioned*/
	TETRARTREEBRANCH  BranchBuf[MAXCARD+1];
	
	/*Actual branch number in BranchBuf*/
	int    BranchCount;
	
	/*The allover MBRs of the root node*/
	MBR  CoverSplit[MBR_NUMB];
	
	/*The area of the allover MBRs*/
	REALTYPE  CoverSplitArea;
	
	/*The allover information of the two groups*/
	TETRARTREEPARTITION Partitions[METHODS];
} TETRARTREEROOT, * HTETRARTREEROOT;





#ifndef NODECARD
  #define NODECARD MAXCARD
#endif
#ifndef LEAFCARD
  #define LEAFCARD MAXCARD
#endif

/* balance criteria for node splitting */
/* NOTE: can be changed if needed. */
#ifndef MINNODEFILL
  #define MINNODEFILL (NODECARD / 2)
#endif

#ifndef MINLEAFFILL
  #define MINLEAFFILL (LEAFCARD / 2)
#endif

#ifndef MAXKIDS
  #define MAXKIDS(n) ((n)->level > 0 ? NODECARD : LEAFCARD)
#endif

#ifndef MINFILL
  #define MINFILL(n) ((n)->level > 0 ? MINNODEFILL : MINLEAFFILL)
#endif





/**
 * Copy a branch to another branch: b1 <- b2
 */
void CopyBranch(pTETRARTREEBRANCH b1, pTETRARTREEBRANCH b2);


/**
 * Copy an index node to another index node: n1 <- n2
 */
void Copy(pTETRARTREENODE n1,pTETRARTREENODE n2);


/**
 * Copy an MBR to another MBR: mbr1 <- mbr2
 */
void CopyMBR(MBR* mbr1, MBR* mbr2);


/**
 * Copy a set(four) of MBRs to another set of MBRs: mbrs1 <- mbrs2
 */
void CopyMBRs(MBR* mbrs1, MBR* mbrs2);


/**
 * Copy an MBO to another MBS: mbo1 <- mbo2
 */
void CopyMBO(MBO* mbo1, MBO* mbo2);


/**
 * Copy the allover info of the two groups (in root->partions) to 
 * the two branches: b1 and b2. 
 * nodeid1 and nodeid2 are the node ids of b1 and b2.
 */
void PARTITION2Branches(HTETRARTREEROOT root, 
                        TETRARTREEBRANCH *b1, int nodeid1,
						TETRARTREEBRANCH *b2, int nodeid2);


/**
 * Calculate the n-dimensional volume of a rectangle
 */
REALTYPE RTreeRectVolume( MBR *mbr );


/**
 * Initialize a TetraRtree brach
 */
void InitBranch(pTETRARTREEBRANCH p);


/**
 * Initialize a TetraRtree node
 */
void InitNode(pTETRARTREENODE p);


/**
 * Reset (empty) the node in the index file.
 */
void EmptyNode(pTETRARTREENODE p);


/**
 * Calculate the n-dimensional volume of the bounding sphere of a rectangle.
 * The exact volume of the bounding sphere for the given MBR.
 */
REALTYPE RectSphericalVolume( pMBR mbr );


/**
 * Combine two rectangles/MBRs, make one that includes both.
 */
MBR CombineMBR(MBR *rc1, MBR *rc2);


/**
 * Combine two lists (four) of rectangles (mbrs)
 * To make one list (four) of mbrs that includes both.
 */
REALTYPE CombineMBRTetrad(MBR *new_mbrs, MBR *mbrs1, MBR *mbrs2);


/**
 * Calculate the angle between two directions in clockwise direction.
 * The return value is in [0, 2PI]
 */
REALTYPE CalculateAngleClockwise(REALTYPE thetaS, REALTYPE thetaE);


/**
 * Combine two MBOs, make one that includes both.
 */
MBO CombineMBO(MBO* mbo1, MBO* mbo2);


/**
 * Calculate the aggregated information of all the branches of a node,
 * and add the aggregated information into a branch.
 *
 * @para[in,out] TETRARTREEBRANCH *br: the pointer of a TetraR-tree branch.
 * @para[in] TETRARTREENODE *node: the pointer of a TetraR-tree node.
 *
 */
void TetraRTreeNodeCover(TETRARTREEBRANCH *br, TETRARTREENODE *node);


void FindRoot(TETRARTREENODE* rootp);




/*********************
  * Static functions
  ********************/

/**
 * Initialize a TeTraRTreePartition structure.
 */
void _TetraRTreeInitPart( TETRARTREEPARTITION *p,
                                 int maxrects, int minfill);


/**
 * Load branch buffer with branches from full node plus the extra branch.
 */
void _RTreeGetBranches(HTETRARTREEROOT root,
                              TETRARTREENODE *node,
							  TETRARTREEBRANCH *br);



/**
 * Put a branch in one of the groups.
 */
void _RTreeClassify(HTETRARTREEROOT root, int i, int group,
                           TETRARTREEPARTITION *p);



/**
 * Pick two rects from set to be the first elements of the two groups.
 * Pick the two that waste the most area if covered by a single rectangle.
 */
void _TetraRTreePickSeeds(HTETRARTREEROOT root, TETRARTREEPARTITION *p);



/**
 * Method #0 for choosing a partition:
 * As the seeds for the two groups, pick the two rects that would waste the
 * most area if covered by a single rectangle, i.e. evidently the worst pair
 * to have in the same group.
 * Of the remaining, one at a time is chosen to be put in one of the two groups.
 * The one chosen is the one with the greatest difference in area expansion
 * depending on which group - the mbr most strongly attracted to one group
 * and repelled from the other.
 * If one group gets too full (more would force other group to violate min
 * fill requirement) then other group gets the rest.
 * These last are the ones that can go in either group most easily.
 */
void _RTreeMethodZero(HTETRARTREEROOT root,
                             TETRARTREEPARTITION *p,
							 int minfill );

/**
 * Copy branches from the buffer into two nodes according to the partition.
 */
void _RTreeLoadNodes(HTETRARTREEROOT root, TETRARTREENODE *n,
                            TETRARTREENODE *q, TETRARTREEPARTITION *p);
















/**
 * Split a node.
 * Divides the nodes branches and the extra one between two nodes.
 * Old node is one of the new ones, and one really new one is created.
 * Tries more than one method for choosing a partition, uses best result.
 */
void SplitNode(HTETRARTREEROOT root, TETRARTREENODE *node,
               TETRARTREEBRANCH *br, TETRARTREENODE **new_node);


/**
 * Add a branch to a node.  Split the node if necessary.
 * Returns 0 if node not split.  Old node updated.
 * Returns 1 if node split, sets *new_node to address of new node.
 * Old node updated, becomes one of two.
 * 
 * @param root
 * @param br
 * @param node
 * @param new_node
 */
int AddBranch(HTETRARTREEROOT root, TETRARTREEBRANCH *br,
               TETRARTREENODE *node, TETRARTREENODE **new_node);


/**
 * Pick a branch.  Pick the one that will need the smallest increase
 * in area to accomodate the new rectangle.  This will result in the
 * least total area for the covering rectangles in the current node.
 * In case of a tie, pick the one which was smaller before, to get
 * the best resolution when searching.
 */
int RTreePickBranch( TETRARTREEBRANCH * pbranch, TETRARTREENODE *node);



/**
 * Inserts a new data rectangle into the index structure.
 * Recursively descends tree, propagates splits back up.
 * Return 0 if node was not split.  Old node updated.
 * If node was split, returns 1 and sets the pointer pointed to by
 * new_node to point to the new node.  Old node updated to become one of two.
 * The level argument specifies the number of steps up from the leaf
 * level to insert; e.g. a data rectangle goes in at level = 0.
 *
 * @param root[in, out]: root index node.
 * @param pbranch[in]:   the branch of new data to be inserted.
 * @param node[in, out]: the index node where the pbranch is to be inserted.
 * @param new_node[in, out]: a new created index node if node is splitted.
 * @param level[in]:     the index level of pbranch to be inserted.
 * 
 * @return 1 if node is splitted; 0 otherwise.
 */
int _RTreeInsertRect(HTETRARTREEROOT root, TETRARTREEBRANCH *pbranch,
                     TETRARTREENODE *node, TETRARTREENODE **new_node, int level);









void build_index(FILE* &object_file, FILE* &index_file);




#ifdef __cplusplus
}
#endif

#endif /* IR2TREE_H_INCLUDED */
