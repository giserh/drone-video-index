#ifndef  RTREE_H
#define  RTREE_H

#include <vector>
#include <stdio.h>
#include "basics.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif


#define K 3
#define  METHODS  1
#define  PAGE_SIZE    4096 //4KB
#define  DIMS_NUMB    2       // number of dimensions
#define  SIDES_NUMB   2*DIMS_NUMB//min max

/* max branching factor of a node */
//Assume one index node takes one page
#define MAXCARD (int)((PAGE_SIZE-(4*sizeof(int))) / sizeof(RTREEBRANCH))


#ifndef INVALD_RECT
  #define INVALID_RECT(x) ((x)->bound[0] > (x)->bound[DIMS_NUMB])//min>max,则无效矩形
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



/*
* If passed to a tree search, this callback function will be called
* with the ID of each data mbr that overlaps the search mbr
* plus whatever user specific pointer was passed to the search.
* It can terminate the search early by returning 0 in which case
* the search will return the number of hits found up to that point.
*/


typedef struct _RTREEMBR //MBR
{
 REALTYPE bound[SIDES_NUMB]; /* xmin,ymin,...,xmax,ymax,... */
}RTREEMBR,*pRTREEMBR;



typedef struct _RTREEBRANCH//branch
{
	RTREEMBR mbr; 
    /* id of child node for non-leaf node; 
	   file offset of object for leaf node */
	int childid;
}RTREEBRANCH,*pRTREEBRANCH;



typedef struct _RTREENODE // node structure
{
 int taken;//标记第ID号是否被占，1占，0未占
 int nodeid; //自身节点的ID
 //int parentid;//父节点号
 int count;//分支数计数
 int level; /* 0 is leaf, others positive */
 RTREEBRANCH  branch[MAXCARD];
}RTREENODE,*pRTREENODE; 




typedef struct _RTREEPARTITION/////////////////////////////////////////////////////为了分裂之需
{
 int   partition[MAXCARD+1];//对应root结构中的分支，标号对应
 int   total;//实际分支数
 int   minfill;//最小容纳的面积
 int   taken[MAXCARD+1];//标明对应分支是否被占，即是否进行分组
 int   count[2];//两组各有的实际分支数
 RTREEMBR cover[2];//两组各分支的MBR值总和
 REALTYPE area[2];//两组各分支的MBR值对应的总面积
} RTREEPARTITION;



typedef struct _RTREEROOT
{
 RTREENODE*  root_node;
 RTREEBRANCH  BranchBuf[MAXCARD+1];//实际存放的各待分裂分支
 int    BranchCount;
 RTREEMBR  CoverSplit;
 REALTYPE  CoverSplitArea;
 RTREEPARTITION Partitions[METHODS];
} RTREEROOT, * HRTREEROOT;



//-----------------------------------for query processing-------------------------------------//
typedef struct _IR2TREEPRINODE
{
	int nodeid;
	REALTYPE distance;
	struct _IR2TREEPRINODE *next;
} IR2TREEPRINODE,*IR2TREEPRINODEp;

typedef struct
{
	IR2TREEPRINODEp front;
	IR2TREEPRINODEp rear;
	IR2TREEPRINODEp minpoint;
} IR2TREEPRIQUEUE,*IR2TREEPRIQUEUEp;


typedef struct _IR2TREELISTNODE
{
	int nodeid;
	struct _IR2TREELISTNODE *next;
}IR2TREELISTNODE,*IR2TREELISTNODEp;



//-------------------------------dataset----------------------------//



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
* Calculate the 2-dimensional area of a rectangle
*/
REALTYPE RTreeRectArea( RTREEMBR *mbr );

/**
* Calculate the n-dimensional volume of a rectangle
*/
REALTYPE RTreeRectVolume( RTREEMBR *mbr );


/**
* Calculate the n-dimensional volume of the bounding sphere of a rectangle
* The exact volume of the bounding sphere for the given RTREEMBR.
*/
REALTYPE RTreeRectSphericalVolume( RTREEMBR *mbr );



/**
* Combine two rectangles, make one that includes both.
*/
RTREEMBR RTreeCombineRect( RTREEMBR *rc1, RTREEMBR *rc2 );


/**
* Find the smallest rectangle that includes all rectangles in branches of a node.
*/
RTREEMBR RTreeNodeCover( pRTREENODE node );


/**
* Pick a branch.  Pick the one that will need the smallest increase
* in area to accomodate the new rectangle.  This will result in the
* least total area for the covering rectangles in the current node.
* In case of a tie, pick the one which was smaller before, to get
* the best resolution when searching.
*/
int RTreePickBranch( RTREEMBR *mbr, pRTREENODE node);


/**
* Add a branch to a node.  Split the node if necessary.
* Returns 0 if node not split.  Old node updated.
* Returns 1 if node split, sets *new_node to address of new node.
* Old node updated, becomes one of two.
*/
int AddBranch(HRTREEROOT root, RTREEBRANCH *br, pRTREENODE node, pRTREENODE *new_node);


/** 
* Insert a data rectangle into an index structure.
* RTreeInsertRect provides for splitting the root;
* returns 1 if root was split, 0 if it was not.
* The level argument specifies the number of steps up from the leaf
* level to insert; e.g. a data rectangle goes in at level = 0.
* _RTreeInsertRect does the recursion.
*/
int RTreeInsert(HRTREEROOT root, RTREEMBR *data_mbr, void* data_id, int level);


void FindRoot(RTREENODE* rootp);

void InitBranch(pRTREEBRANCH p);

void InitNode(pRTREENODE p);

void CopyBranch(pRTREEBRANCH b1,pRTREEBRANCH b2);

int _RTreeInsertRect(HRTREEROOT root, RTREEBRANCH *pbranch,  RTREENODE *node, RTREENODE **new_node, int level);

REALTYPE CalculateAngleClockwise(REALTYPE thetaS, REALTYPE thetaE);


void Copy(pRTREENODE n1,pRTREENODE n2);

void FindRoot(RTREENODE* rootp);


REALTYPE CalculateAlloverScoreWaste(RTREEBRANCH *branch1, RTREEBRANCH *branch2);

void Quadrilateral2MBR(std::vector<REALTYPE>& quadrilateral, RTREEMBR& mbr);

void build_index(FILE* &object_file, FILE* &index_file); 

//---------------query processing-----------------//
/*REALTYPE MinDist(RTREEMBR* mbrp, REALTYPE qlat, REALTYPE qlng);

bool MBRContainCircle(RTREEMBR* mbrp, REALTYPE qlat, REALTYPE qlng, REALTYPE qradius);

int NodeOverlapCheck(RTREEBRANCH* branchp, REALTYPE qlat, REALTYPE qlng, REALTYPE qradius);

int ObjectOverlapCheck(REALTYPE plat, REALTYPE plng, REALTYPE pthetas, REALTYPE pthetae, REALTYPE pR, REALTYPE qlat, REALTYPE qlng, REALTYPE qradius);

void ReportResults(RTREENODE* node);
*/


#ifdef __cplusplus
}
#endif

#endif /* IR2TREE_H_INCLUDED */