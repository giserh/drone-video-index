// tetrartree.cpp : implementation file
//


#include "tetrartree.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <assert.h>
#include  <math.h>
#include<limits>

using namespace std;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Global variants
extern FILE* index_file; 
extern int index_root_id; //The id of root index node.


//Copy a branch to another branch: b1 <- b2
void CopyBranch(pTETRARTREEBRANCH b1, pTETRARTREEBRANCH b2)
{
	b1->childid=b2->childid;
	for(int i=0; i<MBR_NUMB; i++)
	{
		for(int j=0;j<SIDES_NUMB;j++)
		{
			b1->mbrs[i].bound[j]=b2->mbrs[i].bound[j];
		}
	}
	b1->orientation.min = b2->orientation.min;
	b1->orientation.max = b2->orientation.max;
}

//Copy an index node to another index node: n1 <- n2
void Copy(pTETRARTREENODE n1,pTETRARTREENODE n2)
{
	n1->taken=n2->taken;
	n1->nodeid=n2->nodeid;
	n1->level=n2->level;
	n1->count=n2->count;
	for(int i=0;i<MAXCARD;i++)
		CopyBranch(&n1->branch[i],&n2->branch[i]);
}


void InitBranch(pTETRARTREEBRANCH p)
{
	p->childid=0;
	for(int i=0; i<MBR_NUMB; i++)
	{
		for(int j=0;j<SIDES_NUMB;j++)
		{
			p->mbrs[i].bound[j]=0.0;
		}
	}
	p->orientation.min = 0.0;
	p->orientation.max = 0.0;
}

void InitNode(pTETRARTREENODE p)
{
	p->taken=0;
	p->nodeid=0;
	p->count=0;
	p->level=-1;
	for(int j=0;j<MAXCARD;j++)
	{
		InitBranch(&(p->branch[j]));
	}
}



void FindRoot(TETRARTREENODE* rootp)
{
	fseek(index_file,(index_root_id-1)*sizeof(TETRARTREENODE),0);
	fread(rootp,sizeof(TETRARTREENODE),1,index_file);
	
	/**
	 * Print root index node information.
	 */
	/*printf("nodeid=%d \t level=%d \t count=%d\n\n", rootp->nodeid, rootp->level, rootp->count);
	for(int c=0; c<rootp->count; c++)
	{
		for(int i=0; i<MBR_NUMB; i++)
		{
			for(int j=0; j<SIDES_NUMB; j++)
			{
				printf("%.6f\t", rootp->branch[c].mbrs[i].bound[j]);
			}
			printf("\n");
		}
		printf("\n\n");
	}*/
	
	
	/**
	 * Print TetrR-tree hierachical information.
	 */
	/*printf("level=%d \t count=%d\n\n", rootp->level, rootp->count);
	printf("Its child node info:\n");
    for(int c=0; c<rootp->count; c++)
	{
		TETRARTREENODE node;
		fseek(index_file,(rootp->branch[c].childid-1)*sizeof(TETRARTREENODE),0);
		fread(&node,sizeof(TETRARTREENODE),1,index_file);
		printf("nodeid=%d \t count=%d\n", node.nodeid, node.count);
	}*/
}



/**
 * Reset (empty) the node in the index file.
 */
void EmptyNode(pTETRARTREENODE p)
{
	int node=p->nodeid;
	InitNode(p);
	fseek(index_file,(node-1)*sizeof(TETRARTREENODE),0);
	fwrite(p,sizeof(TETRARTREENODE),1,index_file);
	fflush(index_file);
}

/**
 * Initialize a TETRARTREEPARTITION structure.
 */
void _TetraRTreeInitPart( TETRARTREEPARTITION *p, int maxrects, int minfill)
{
 int i;
 assert(p);

 p->count[0] = p->count[1] = 0;
 for(int i=0; i<MBR_NUMB; i++)
 {
	 for(int j=0;j<SIDES_NUMB;j++)
	 {
		 p->cover[0][i].bound[j]=0;
		 p->cover[1][i].bound[j]=0;
	 }
 }
 p->coverOrientation[0].min = 0.0;
 p->coverOrientation[0].max = 0.0;
 p->coverOrientation[1].min = 0.0;
 p->coverOrientation[1].max = 0.0;
 
 p->alloverScore[0] = p->alloverScore[1] = (REALTYPE)0;
 p->total = maxrects;//最大分支数
 p->minfill = minfill;
 for (i=0; i<maxrects; i++)
 {
  p->taken[i] = FALSE;
  p->partition[i] = -1;
 }
}


/**
 * Combine two rectangles, make one that includes both.
 */
 MBR CombineMBR(MBR *rc1, MBR *rc2)
{
 int i, j;
 MBR new_rect;
 
 // Sanity checks
 if (INVALID_RECT(rc1))
  return *rc2;

 if (INVALID_RECT(rc2))
  return *rc1;

 for (i = 0; i < DIMS_NUMB; i++)
 {
  new_rect.bound[i] = MIN(rc1->bound[i], rc2->bound[i]);
  j = i + DIMS_NUMB;
  new_rect.bound[j] = MAX(rc1->bound[j], rc2->bound[j]);
 } 
 return new_rect;
}



/**
 * Combine a list of rectangles, make one that includes all of them.
 */
MBR EncloseMBRs(MBR* mbrs)
{
	MBR rect;
	for(int j=0; j<MBR_NUMB; j++)
	{
		if(j==0) 
		{
			for(int i=0; i<SIDES_NUMB; i++) 
				rect.bound[i]=mbrs[j].bound[i];
		}
		else rect = CombineMBR(&rect, &(mbrs[j]));
	}
	return rect;
}


/**
 * Combine two lists (four) of rectangles (mbrs)
 * To make one list (four) of mbrs that includes both.
 */
REALTYPE CombineMBRTetrad(MBR *new_mbrs, MBR *mbrs1, MBR *mbrs2)
{
	std::set<int> available_indexes = {0, 1, 2, 3};
	REALTYPE sum_waste = 0.0;
	
	for(size_t i=0; i<MBR_NUMB; i++)
	{
		REALTYPE min_waste = DOUBLE_MAX;
		int min_idx = 0;
		MBR min_mbr;
		for(size_t j=0; j<MBR_NUMB; j++)
		{
			if(available_indexes.find(j) != available_indexes.end()) //if exist
			{
				MBR cover_mbr = CombineMBR(&(mbrs1[i]), &(mbrs2[j]));
				REALTYPE waste = RectSphericalVolume(&cover_mbr) 
				                 - RectSphericalVolume(&(mbrs1[i])) 
								 - RectSphericalVolume(&(mbrs2[j]));
				if(waste < min_waste)
				{
					min_idx = j;
					min_waste = waste;
					min_mbr = cover_mbr;
				}
			}
		}
		new_mbrs[i] = min_mbr; //Update new_mbrs[i] (i.e., mbrs1[i])
		sum_waste += min_waste;
		available_indexes.erase(min_idx);
	}
	return sum_waste;
}



/** 
 * Calculate the angle between two directions in clockwise direction.
 * The return value is in [0, 2PI]
 */
REALTYPE CalculateAngleClockwise(REALTYPE thetaS, REALTYPE thetaE) 
{
	//angle is in [0, PI) x1*y1+x2*y2
	REALTYPE angle = acos(sin(thetaS)*sin(thetaE) + cos(thetaS)*cos(thetaE));
	
	/**
	 * cross multipleply x1*y2+x2*y1 = |a|*|b|*sin<a,b> right hand rule, 
	 * counter clockwise
	 */
	REALTYPE cross = sin(thetaS)*cos(thetaE) + cos(thetaS)*sin(thetaE);  
	if(cross>0) return 2*3.1415926-angle; 
	else return angle;
}



/**
 * Combine two MBOs, make one that includes both.
 */
MBO CombineMBO(MBO* mbo1, MBO* mbo2)
{
	MBO new_mbo = {0,0};
	REALTYPE AngleS1E2 = CalculateAngleClockwise(mbo1->min, mbo2->max);
	REALTYPE AngleS2E1 = CalculateAngleClockwise(mbo2->min, mbo1->max);
	if(AngleS1E2 > AngleS2E1)
	{
		new_mbo.min = mbo1->min;
		new_mbo.max = mbo2->max;
	}
	else
	{
		new_mbo.min = mbo2->min;
		new_mbo.max = mbo1->max;
	}
	return new_mbo;
}



/**
 * Calculate the aggregated information of all the branches of a node,
 * and add the aggregated information into a branch.
 *
 * @para[in,out] TETRARTREEBRANCH *br: the pointer of a TetraR-tree branch.
 * @para[in] TETRARTREENODE *node: the pointer of a TetraR-tree node.
 *
 */
 void TetraRTreeNodeCover(TETRARTREEBRANCH *br, TETRARTREENODE *node)
{
	assert(node);
	assert(br);
	
	/**
	 * pointer childid.
	 */
	br->childid = node->nodeid;
	
	/**
	 * mbr.
	 */
	for(int i=0; i<MBR_NUMB; i++) // br->mbrs <= node->branch[0].mbrs
		br->mbrs[i] = node->branch[0].mbrs[i];
	for(int i=1; i<node->count; i++)
	{
		CombineMBRTetrad(br->mbrs, br->mbrs, node->branch[i].mbrs);
	}
	
	/**
	 * orientation.
	 */
	br->orientation = node->branch[0].orientation;
	for(int i=1; i<node->count; i++)
	{
		br->orientation = CombineMBO(&(br->orientation), 
		                             &(node->branch[i].orientation));
	}
}





/**
 * Calculate the n-dimensional volume of the bounding sphere of a rectangle.
 * The exact volume of the bounding sphere for the given MBR.
 */
 REALTYPE RectSphericalVolume( pMBR mbr )
{
 int i;
 REALTYPE sum_of_squares=0, radius;

 if (INVALID_RECT(mbr))
  return (REALTYPE) 0;
 
 for (i=0; i<DIMS_NUMB; i++) {
  REALTYPE half_extent = (mbr->bound[i+DIMS_NUMB] - mbr->bound[i]) / 2;
  sum_of_squares += half_extent * half_extent;
 }
 radius = (REALTYPE)sqrt(sum_of_squares);
 return (REALTYPE)(pow(radius, DIMS_NUMB) * UnitSphereVolume);
}



/**
 * Load branch buffer with branches from full node plus the extra branch.
 */
void _RTreeGetBranches(HTETRARTREEROOT root, 
                              TETRARTREENODE *node, 
							  TETRARTREEBRANCH *br)
{
 int i;

 assert(node && br);
 
 /* load the branch buffer */
 for (i=0; i<MAXKIDS(node); i++)
 {
  assert(node->branch[i].childid); /* n should have every entry full */
  CopyBranch(&(root->BranchBuf[i]),&(node->branch[i]));
 }

 CopyBranch(&(root->BranchBuf[MAXKIDS(node)]), br);
 root->BranchCount = MAXKIDS(node) + 1;

 /* calculate mbr containing all in the set */
 /** Useless for nodes' partition.
 for(i=0; i<MBR_NUMB; i++) root->CoverSplit[i] = root->BranchBuf[0].mbrs[i];
 for (i=1; i<MAXKIDS(node)+1; i++)
 {
  //--root->CoverSplit = RTreeCombineRect(&root->CoverSplit, &root->BranchBuf[i].mbr);
  CombineMBRTetrad(root->CoverSplit, root->CoverSplit, root->BranchBuf[i].mbrs);
 }

 root->CoverSplitArea = RectSphericalVolume(root->CoverSplit);*/
 EmptyNode(node); //Important
}

 

 

/**
 * Put a branch in one of the groups.
 */
void _RTreeClassify(HTETRARTREEROOT root, int i, int group, 
                           TETRARTREEPARTITION *p)
{
 assert(p);
 assert(!p->taken[i]);//前提都尚未分组

 p->partition[i] = group;//第i个分支属于第group组
 p->taken[i] = TRUE;//第i个分支已分过组

 ////////////////////////////////////////////////////////////计算两组的mbr值总和
 if (p->count[group] == 0)//当第group组分支数为零时
 {
	 for(int j=0; j<MBR_NUMB; j++)
	 {
		 p->cover[group][j] = root->BranchBuf[i].mbrs[j];
	 }
 }
 else
 {
	 //--p->cover[group] = RTreeCombineRect(&root->BranchBuf[i].mbr, &p->cover[group]);
	 CombineMBRTetrad(p->cover[group], p->cover[group], root->BranchBuf[i].mbrs);
 }

 p->count[group]++;
}



/**
 * Pick two rects from set to be the first elements of the two groups.
 * Pick the two that waste the most area if covered by a single rectangle.
 */
void _TetraRTreePickSeeds(HTETRARTREEROOT root, TETRARTREEPARTITION *p)
{
 int i, j, seed0=0, seed1=0;
 REALTYPE worst, spatial_waste;

 worst = 0 - std::numeric_limits<float>::max();
 
 //MBR rect_i, rect_j, combine_rect;
 //REALTYPE area_i, area_j;
 MBR cover_mbrs[MBR_NUMB];
 
 for (i=0; i<p->total-1; i++)//扫描各个分支，两两组合，计算waste{area(A+B)-area(A)-area(B)}值最大的一对
 {
  for (j=i+1; j<p->total; j++)
  {	  
	  /**
       * Optimize based on the waste spaces of four mbrs of TetrR-tree node
	   */
	  spatial_waste = CombineMBRTetrad(cover_mbrs, root->BranchBuf[i].mbrs, root->BranchBuf[j].mbrs);
	  
	  /**
       * Combine orientation into the optimization.
	   */
	   /*MBO combined_mbo = CombineMBO(&(root->BranchBuf[i].orientation), &(root->BranchBuf[j].orientation));
	
	   REALTYPE angle_waste = CalculateAngleClockwise(combined_mbo.min, combined_mbo.max) - 
	                          CalculateAngleClockwise(root->BranchBuf[i].orientation.min, 
			                                          root->BranchBuf[i].orientation.max) - 
							  CalculateAngleClockwise(root->BranchBuf[j].orientation.min, 
			                                          root->BranchBuf[j].orientation.max);
	  spatial_waste = spatial_waste*0.5 + angle_waste*0.5; // update
	  */
	
	  
	   /**
		* Optimize based on the waste area of the mbrs of quadrilaterals
		* Same as R-trees
		*/
		/*rect_i = EncloseMBRs(root->BranchBuf[i].mbrs);
		rect_j = EncloseMBRs(root->BranchBuf[j].mbrs);
		combine_rect = CombineMBR(&rect_i, &rect_j);
		area_i = RectSphericalVolume(&rect_i);
		area_j = RectSphericalVolume(&rect_j);
		spatial_waste = RectSphericalVolume(&combine_rect) - area_i - area_j;*/
	  	  
	   if (spatial_waste > worst)
	   {
		worst = spatial_waste;
		seed0 = i;
		seed1 = j;
	   }
  }
 }
 _RTreeClassify(root, seed0, 0, p);
 _RTreeClassify(root, seed1, 1, p);
}



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
							 int minfill )
{
 int i;
 REALTYPE biggestDiff;
 int group, chosen=0, betterGroup=0;
 assert(p);

 _TetraRTreeInitPart(p, root->BranchCount, minfill);
 _TetraRTreePickSeeds(root, p);
 
 MBR* pcover_group0 = &(p->cover[0][0]);
 MBR* pcover_group1 = pcover_group0 + MBR_NUMB;
 while (p->count[0] + p->count[1] < p->total && 
  p->count[0] < p->total - p->minfill && 
  p->count[1] < p->total - p->minfill)
 {
  biggestDiff = (REALTYPE)-1;
  for (i=0; i<p->total; i++)
  {
   if (!p->taken[i])//作用在于此，更新作用，使得while能有效进行下去
   {
    MBR *r = root->BranchBuf[i].mbrs;;
	MBR rect_0[MBR_NUMB], rect_1[MBR_NUMB];
    REALTYPE growth0, growth1, diff;
    
	growth0 = CombineMBRTetrad(rect_0, r, pcover_group0);
	growth1 = CombineMBRTetrad(rect_1, r, pcover_group1);
    diff = growth1 - growth0;
    if (diff >= 0)//选择距离（面积）增加更少的
     group = 0;
    else
    {
     group = 1;
     diff = -diff;//因为此时diff是负值，而后面需要用其绝对值
    }
    if (diff > biggestDiff)
    {
     biggestDiff = diff;
     chosen = i;
     betterGroup = group;
    }
    else if (diff==biggestDiff && p->count[group]<p->count[betterGroup])//考虑两因素：首先距离，其次两组分支数，原则尽量紧凑，平衡
    {
     chosen = i;
     betterGroup = group;
    }
   }//end if taken
  }//end for
  _RTreeClassify(root, chosen, betterGroup, p);//对于每一个分支都要全盘考虑，也即是从剩余的分支中出发，根据增加的面积最少，挑选其中的分支
                                               //这是第一种方法；第二种则仅考虑自身，从各分支出发。
 }//end while

 /* if one group too full, put remaining rects in the other */
 if (p->count[0] + p->count[1] < p->total)
 {
  if (p->count[0] >= p->total - p->minfill)
   group = 1;
  else
   group = 0;
  
  for (i=0; i<p->total; i++)
  {
   if (!p->taken[i])
    _RTreeClassify(root, i, group, p);
  }
 }
 
 assert(p->count[0] + p->count[1] == p->total);
 assert(p->count[0] >= p->minfill && p->count[1] >= p->minfill);
}




/**
 * Copy branches from the buffer into two nodes according to the partition.
 */
void _RTreeLoadNodes(HTETRARTREEROOT root, TETRARTREENODE *n, 
                            TETRARTREENODE *q, TETRARTREEPARTITION *p)
{
 int i;
 assert(n && q && p);

 for (i=0; i<p->total; i++)
 {
  assert(p->partition[i] == 0 || p->partition[i] == 1);
  if (p->partition[i] == 0)
   AddBranch(root, &root->BranchBuf[i], n, NULL);
  else if (p->partition[i] == 1)
   AddBranch(root, &root->BranchBuf[i], q, NULL);
 }
}




/**
 * Split a node.
 * Divides the nodes branches and the extra one between two nodes.
 * Old node is one of the new ones, and one really new one is created.
 * Tries more than one method for choosing a partition, uses best result.
 */
void SplitNode(HTETRARTREEROOT root, TETRARTREENODE *node, 
               TETRARTREEBRANCH *br, TETRARTREENODE **new_node)
{
 TETRARTREEPARTITION *p; //TODO: make it as output otherwise it will make recomputations for the allover MBRs/MBO info.
 int level;
 assert(node && br);
 
 /* load all the branches into a buffer (root->BranchBuf), initialize old node */
 level = node->level; //
 _RTreeGetBranches(root, node, br);


 /* find partition */
 p = &(root->Partitions[0]); //Get the partition address.

 /* Note: use MINFILL(n) below since node was cleared by GetBranches() */
 _RTreeMethodZero(root, p, (level>0 ? MINNODEFILL : MINLEAFFILL));


 /* put branches from buffer into 2 nodes according to chosen partition */
 int i,ftaken=2;
 long filelen;
 fseek(index_file,0L,SEEK_END);
 filelen=ftell(index_file);
 for(i=0;i<(int)filelen;i=i+sizeof(TETRARTREENODE))
 {
	 fseek(index_file,i,0);
	 fread(&ftaken,sizeof(int),1,index_file);//
	 if(ftaken==0) break;
 }
 node->nodeid=i/sizeof(TETRARTREENODE)+1;
 node->taken=1;
 node->level=level;
	 
 *new_node = (TETRARTREENODE*)malloc(sizeof(TETRARTREENODE));
 InitNode(*new_node);
 (*new_node)->level = level;

 for(;i<(int)filelen;i=i+sizeof(TETRARTREENODE))
 {
	 fseek(index_file,i,0);
	 fread(&ftaken,sizeof(int),1,index_file);//
	 if(ftaken==0 && (i/sizeof(TETRARTREENODE)+1)!=(unsigned int)(node->nodeid)) break;
 }
 (*new_node)->nodeid=i/sizeof(TETRARTREENODE)+1;
 (*new_node)->taken=1;


 _RTreeLoadNodes(root, node, *new_node, p);

 assert(node->count+(*new_node)->count == p->total);
}


/**
 * Add a branch to a node.  Split the node if necessary.
 * Returns 0 if node not split.  Old node updated.
 * Returns 1 if node split, sets *new_node to address of new node.
 * Old node updated, becomes one of two.
 */
 int AddBranch(HTETRARTREEROOT root, TETRARTREEBRANCH *br, 
               TETRARTREENODE *node, TETRARTREENODE **new_node)
{
 int i;
 assert(br && node);
 
 if (node->count < MAXKIDS(node))  /* split won&apos;t be necessary */
 {
  for (i = 0; i < MAXKIDS(node); i++)  /* find empty branch */
  {
   if (node->branch[i].childid == 0)
   {
    CopyBranch(&(node->branch[i]),br);
    node->count++; 
	CombineMBRTetrad(node->branch[i].mbrs, node->branch[i].mbrs, br->mbrs); //Update 
    node->branch[i].orientation = CombineMBO(&(br->orientation), &(node->branch[i].orientation));
	fseek(index_file,(node->nodeid-1)*sizeof(TETRARTREENODE),0);
	fwrite(node,sizeof(TETRARTREENODE),1,index_file);
	/*std::cout << "nodeid: " << node->nodeid
					  << "\t cout: " << node->count
					  << "\t level: " << node->level << std::endl;*/
	fflush(index_file);
    break;
   }
  }

  return 0;
 }
 
 assert(new_node);
 SplitNode(root, node, br, new_node);
 return 1;
}



/**
 * Pick a branch.  Pick the one that will need the smallest increase
 * in area to accomodate the new rectangle.  This will result in the
 * least total area for the covering rectangles in the current node.
 * In case of a tie, pick the one which was smaller before, to get
 * the best resolution when searching.
 */
int RTreePickBranch( TETRARTREEBRANCH * pbranch, TETRARTREENODE *node)
{
 //MBR *r;
 MBR * mbrs = pbranch->mbrs;
 //MBO mbo = pbranch->orientation;
 
 int i, first_time = 1;
 REALTYPE increase, bestIncr=(REALTYPE)-1, area, bestArea=0;
 int best=0;
 assert(mbrs && node);
 
 //MBR rect_mbrs, rect_branchmbrs, combine_rect;
 MBR tmp_rects[MBR_NUMB];
 
 for (i=0; i<MAXKIDS(node); i++)
 {
  if (node->branch[i].childid)
  {
   /**
    * Optimize based on the waste spaces of four mbrs of TetrR-tree node
	*/
	increase = CombineMBRTetrad(tmp_rects, mbrs, node->branch[i].mbrs);
	
	/**
     * Combine orientation into the optimization
	 */
	/*MBO combined_mbo = CombineMBO(&mbo, &(node->branch[i].orientation));
	
	REALTYPE angle_increase = CalculateAngleClockwise(combined_mbo.min, combined_mbo.max) - 
	                          CalculateAngleClockwise(node->branch[i].orientation.min, 
			                                          node->branch[i].orientation.max);
	increase = increase*0.5 + angle_increase*0.5; // update
	*/
	
   /**
    * Calculate sum of the areas of the four MBRs of branch i.
	*/
   area = 0.0;
   for(int j=0; j<MBR_NUMB; j++)
	   area += RectSphericalVolume(&(node->branch[i].mbrs[j]));
   
   
   /**
    * Optimize based on the waste area of the mbrs of quadrilaterals
	* Same as R-trees
	*/
	/*rect_mbrs = EncloseMBRs(mbrs);
	rect_branchmbrs = EncloseMBRs(node->branch[i].mbrs);
	combine_rect = CombineMBR(&rect_mbrs, &rect_branchmbrs);
	area = RectSphericalVolume(&rect_branchmbrs);
	increase = RectSphericalVolume(&combine_rect) - area;*/
	
	
   if (increase < bestIncr || first_time)
   {
    best = i;
    bestArea = area;
    bestIncr = increase;
    first_time = 0;
   }
   else if (increase == bestIncr && area < bestArea)
   {
    best = i;
    bestArea = area;
    bestIncr = increase;
   }
  }
 }
 return best;
}


void CopyMBR(MBR* mbr1, MBR* mbr2)
{
	for(int j=0;j<SIDES_NUMB;j++)
	{
		mbr1->bound[j] = mbr2->bound[j];
	}
}


void CopyMBRs(MBR* mbrs1, MBR* mbrs2)
{
	for(int i=0; i<MBR_NUMB; i++)
	{
		CopyMBR(&(mbrs1[i]), &(mbrs2[i]));
	}
}



void CopyMBO(MBO* mbo1, MBO* mbo2)
{
	mbo1->min = mbo2->min;
	mbo1->max = mbo2->max;
}


void PARTITION2Branches(HTETRARTREEROOT root, 
                        TETRARTREEBRANCH *b1, int nodeid1,
						TETRARTREEBRANCH *b2, int nodeid2)
{
	b1->childid = nodeid1;
	CopyMBRs(b1->mbrs, root->Partitions[0].cover[0]);
	CopyMBO(&(b1->orientation), &(root->Partitions[0].coverOrientation[0]));
	
	b2->childid = nodeid2;
	CopyMBRs(b2->mbrs, root->Partitions[0].cover[1]);
	CopyMBO(&(b2->orientation), &(root->Partitions[0].coverOrientation[1]));
}



/**
 * Inserts a new data rectangle into the index structure.
 * Recursively descends tree, propagates splits back up.
 * Returns 0 if node was not split.  Old node updated.
 * If node was split, returns 1 and sets the pointer pointed to by
 * new_node to point to the new node.  Old node updated to become one of two.
 * The level argument specifies the number of steps up from the leaf
 * level to insert; e.g. a data rectangle goes in at level = 0.
 */
 int _RTreeInsertRect(HTETRARTREEROOT root, TETRARTREEBRANCH *pbranch,  
                      TETRARTREENODE *node, TETRARTREENODE **new_node, int level)
{
 int i;
 TETRARTREEBRANCH b;
 InitBranch(&b);
 TETRARTREENODE *n2;

 //assert(pbranch->mbr && node && new_node);
 assert(level >= 0 && level <= node->level);//because node begin from the root

 /* Still above level for insertion, go down tree recursively */
 if (node->level > level)
 {
  i = RTreePickBranch(pbranch, node);
  TETRARTREENODE child;
  InitNode(&child);
  fseek(index_file,(node->branch[i].childid-1)*sizeof(TETRARTREENODE),0);
  fread(&child,sizeof(TETRARTREENODE),1,index_file);

  if (!_RTreeInsertRect(root, pbranch, &child, &n2, level))
  {
   /* child was not split */
   /*if(node->branch[i].childid==0)
   {
	   node->branch[i].childid=pbranch->childid;
   }
   CombineMBRTetrad(node->branch[i].mbrs, node->branch[i].mbrs, pbranch->mbrs);
   node->branch[i].orientation = CombineMBO(&(pbranch->orientation), &(node->branch[i].orientation));

   fseek(index_file,(node->nodeid-1)*sizeof(TETRARTREENODE),0);
   fwrite(node,sizeof(TETRARTREENODE),1,index_file);
   //std::cout << "nodeid: " << node->nodeid
   //				  << "\t cout: " << node->count
   //				  << "\t level: " << node->level << std::endl;
   fflush(index_file);*/
   return 0;
  }
  
  /* child was split */
  //TetraRTreeNodeCover(&(node->branch[i]), &child);
  //TetraRTreeNodeCover(&b, n2);
  /**
   * Copy the allover info of the two groups (in root->partions) to the two branches:
   * node->branch[i] and b.
   */
  PARTITION2Branches(root, &(node->branch[i]), child.nodeid, &b, n2->nodeid);
  
  return AddBranch(root, &b, node, new_node);
 } 
 else if (node->level == level) /* Have reached level for insertion. Add mbr, split if necessary */
 {
  CopyBranch(&b,pbranch);

  /* child field of leaves contains tid of data record */

  return AddBranch(root, &b, node, new_node);
 }
 
 /* Not supposed to happen */
 assert (FALSE);
 return 0;
}









void build_index(FILE* &object_file, FILE* &index_file) 
{
	std::vector<REALTYPE> quadrilateral(8);
	REALTYPE timestamp;
	REALTYPE azimuth;
	
	int countdataid = 0; //initionalize zero
	
	
	while(!feof(object_file))
	{
		fscanf(object_file, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", 
		&quadrilateral[0], &quadrilateral[1], &quadrilateral[2], &quadrilateral[3], 
		&quadrilateral[4], &quadrilateral[5], &quadrilateral[6], &quadrilateral[7], 
		&azimuth, &timestamp);
		countdataid++;
		
		TETRARTREEBRANCH branch;
		/** 
		 * Set the branch to contain the quadrilateral object.
         */		 
		branch.childid = countdataid; 
		for(int i=0; i<4; i++) //for each point of the quadrilateral
		{
			branch.mbrs[i].bound[0] = quadrilateral[i*2+1]; //minX <- Longitude
			branch.mbrs[i].bound[1] = quadrilateral[i*2+0]; //minY <- Latitude
			branch.mbrs[i].bound[2] = quadrilateral[i*2+1]; //maxX <- Longitude
			branch.mbrs[i].bound[3] = quadrilateral[i*2+0]; //maxY <- Latitude
		}
		/*for(int i=0; i<MBR_NUMB; i++)
		{
			for(int j=0; j<SIDES_NUMB; j++)
			{
				printf("%.6f\t", branch.mbrs[i].bound[j]);
			}
			printf("\n");
		}*/
		branch.orientation.min = azimuth;
		branch.orientation.max = azimuth;

		
		TETRARTREEROOT root; 
		root.root_node=(TETRARTREENODE*)malloc(sizeof(TETRARTREENODE));
		InitNode(root.root_node);
		
		fseek(index_file,0L,SEEK_END);
		long leng=ftell(index_file);
		if(leng==0) 
		{   
			/*Write the first index node.*/
			root.root_node->taken=1;
			root.root_node->nodeid=1; 
			index_root_id = root.root_node->nodeid;
			root.root_node->level=0;
			root.root_node->count=1;
			CopyBranch(&(root.root_node->branch[0]), &branch);
			fseek(index_file,0L,0); 
			fwrite(root.root_node,sizeof(TETRARTREENODE),1,index_file);
			/*std::cout << "nodeid: " << root.root_node->nodeid
					  << "\t cout: " << root.root_node->count
					  << "\t level: " << root.root_node->level << std::endl;*/
			fflush(index_file);
		}
		else
		{
			FindRoot(root.root_node); //Read the root node from disk.
			
			/**
			 * In the following, insert the branch into the index.
			 */
			TETRARTREENODE *newroot;
			TETRARTREENODE *newnode;
			TETRARTREEBRANCH b;
			InitBranch(&b);
			
			if (_RTreeInsertRect(&root, &branch, root.root_node, &newnode, 0))  
			{
				/* root is splitted */
				int ftaken;
				
				
			
				/* Create a new root, and tree grows taller */
				newroot=(TETRARTREENODE*)malloc(sizeof(TETRARTREENODE)); 
				InitNode(newroot);
				newroot->taken=1;
				newroot->level = root.root_node->level + 1;
				
				/* Find the first index node slot on the disk */
				fseek(index_file,0L,SEEK_END);
				leng=ftell(index_file);
				int i;
				for(i=0;i<(int)leng;i=i+sizeof(TETRARTREENODE))
				{
				 fseek(index_file,i,0);
				 fread(&ftaken,sizeof(int),1,index_file);
				 if(ftaken==0) break;
				}
				
				newroot->nodeid=i/sizeof(TETRARTREENODE)+1;
				index_root_id = newroot->nodeid; // Update index root node id.
				
				/*Add root.root_node as a branch of newroot*/
				//TetraRTreeNodeCover(&b, root.root_node);
				//fseek(index_file,0L,SEEK_END);
				//AddBranch(&root, &b, newroot, NULL);
				PARTITION2Branches(&root, &(newroot->branch[0]), root.root_node->nodeid, 
				                          &b, newnode->nodeid);
				
				/*Add newnode as a branch of newroot*/
				//TetraRTreeNodeCover(&b, newnode);
				AddBranch(&root, &b, newroot, NULL);

				fseek(index_file,((root.root_node->nodeid)-1)*sizeof(TETRARTREENODE),0);
				fwrite(root.root_node,sizeof(TETRARTREENODE),1,index_file);
				/*std::cout << "nodeid: " << root.root_node->nodeid
					  << "\t cout: " << root.root_node->count
					  << "\t level: " << root.root_node->level << std::endl;*/
				fflush(index_file);

				fseek(index_file,((newnode->nodeid)-1)*sizeof(TETRARTREENODE),0);
				fwrite(newnode,sizeof(TETRARTREENODE),1,index_file);
				/*std::cout << "nodeid: " << newnode->nodeid
					  << "\t cout: " << newnode->count
					  << "\t level: " << newnode->level << std::endl;*/
				fflush(index_file);

				Copy(root.root_node , newroot);
				delete newroot;
			 }//END IF
		 }//end else if leng==0
		 delete root.root_node;
	} //end while result->next()
}



