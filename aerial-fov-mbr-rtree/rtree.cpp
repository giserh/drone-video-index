// rtree.cpp : implementation file
//


#include "rtree.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
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
extern int index_node_read_num;
extern int index_node_write_num;
extern int taken_read_num;
extern double index_node_read_time;
extern double index_node_write_time;
extern double taken_read_time;
extern long long RTreeCombineRect_invoke_num;
extern double RTreeCombineRect_invoke_time;

//Copy a branch to another branch: b1 <- b2
void CopyBranch(pRTREEBRANCH b1, pRTREEBRANCH b2)
{
	b1->childid=b2->childid;
	for(int j=0;j<SIDES_NUMB;j++)
	{
		b1->mbr.bound[j]=b2->mbr.bound[j];
	}
}

//Copy an index node to another index node: n1 <- n2
void Copy(pRTREENODE n1,pRTREENODE n2)
{
	n1->taken=n2->taken;
	n1->nodeid=n2->nodeid;
	n1->level=n2->level;
	n1->count=n2->count;
	for(int i=0;i<MAXCARD;i++)
		CopyBranch(&n1->branch[i],&n2->branch[i]);
}


void InitBranch(pRTREEBRANCH p)
{
	p->childid=0;
	for(int i=0;i<SIDES_NUMB;i++) p->mbr.bound[i]=0;
}

void InitNode(pRTREENODE p)
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

void EmptyNode(pRTREENODE p)//与前者不同
{
	int node=p->nodeid;
	InitNode(p);
	
	clock_t tStart = clock();
	fseek(index_file,(node-1)*sizeof(RTREENODE),0);
	fwrite(p,sizeof(RTREENODE),1,index_file);
    index_node_write_num++;
	/*std::cout << "nodeid: " << p->nodeid
					  << "\t cout: " << p->count
					  << "\t level: " << p->level << std::endl;*/
	fflush(index_file);
	index_node_write_num++;
	index_node_write_time += (double)(clock() - tStart)/CLOCKS_PER_SEC;
}

/**
 * Initialize a RTREEPARTITION structure.
 */
static void _RTreeInitPart( RTREEPARTITION *p, int maxrects, int minfill)
{
 int i;
 assert(p);

 p->count[0] = p->count[1] = 0;
 for(int j=0;j<SIDES_NUMB;j++)
	{
		p->cover[0].bound[j]=0;
		p->cover[1].bound[j]=0;
	}
 p->area[0] = p->area[1] = (REALTYPE)0;
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
 RTREEMBR RTreeCombineRect( RTREEMBR *rc1, RTREEMBR *rc2 )//两MBR叠加
{
    clock_t tStart = clock();
    
    int i, j;
    RTREEMBR new_rect;

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
    
    RTreeCombineRect_invoke_time += (double)(clock() - tStart)/CLOCKS_PER_SEC;
    RTreeCombineRect_invoke_num ++;
    
    return new_rect;
}








 /**
 * Find the smallest rectangle that includes all rectangles in branches of a node.
 */
 RTREEMBR RTreeNodeCover(RTREENODE *node)
{
	int i;
	RTREEMBR mbr;
	assert(node);

	mbr = node->branch[0].mbr;;

	for (i = 1; i < MAXCARD; i++)
	{
		if (node->branch[i].childid)
		{
			mbr = RTreeCombineRect(&mbr, &(node->branch[i].mbr));
		}
	}
	return mbr;
}





/**
 * Calculate the n-dimensional volume of the bounding sphere of a rectangle.
 * The exact volume of the bounding sphere for the given RTREEMBR.
 */
 REALTYPE RTreeRectSphericalVolume( pRTREEMBR mbr )
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
static void _RTreeGetBranches(HRTREEROOT root, RTREENODE *node, RTREEBRANCH *br)
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
 root->CoverSplit = root->BranchBuf[0].mbr;

 for (i=1; i<MAXKIDS(node)+1; i++)
 {
  root->CoverSplit = RTreeCombineRect(&root->CoverSplit, &root->BranchBuf[i].mbr);
 }

 root->CoverSplitArea = RTreeRectSphericalVolume(&root->CoverSplit);
 EmptyNode(node); //Important
}

 

 

/**
 * Put a branch in one of the groups.
 */
static void _RTreeClassify(HRTREEROOT root, int i, int group, RTREEPARTITION *p)
{
 assert(p);
 assert(!p->taken[i]);//前提都尚未分组

 p->partition[i] = group;//第i个分支属于第group组
 p->taken[i] = TRUE;//第i个分支已分过组

 ////////////////////////////////////////////////////////////计算两组的mbr值总和
 if (p->count[group] == 0)//当第group组分支数为零时
 {
	 p->cover[group] = root->BranchBuf[i].mbr;
 }
 else
 {
	  p->cover[group] = RTreeCombineRect(&root->BranchBuf[i].mbr, &p->cover[group]);
 }

 
 p->area[group] = RTreeRectSphericalVolume(&(p->cover[group]));
 p->count[group]++;
}



/**
 * Pick two rects from set to be the first elements of the two groups.
 * Pick the two that waste the most area if covered by a single rectangle.
 */
static void _RTreePickSeeds(HRTREEROOT root, RTREEPARTITION *p)
{
 int i, j, seed0=0, seed1=0;
 REALTYPE worst, waste;

 worst = 0 - std::numeric_limits<float>::max();
 
 for (i=0; i<p->total-1; i++)//扫描各个分支，两两组合，计算waste{area(A+B)-area(A)-area(B)}值最大的一对
 {
  for (j=i+1; j<p->total; j++)
  {
	  RTREEMBR cover = RTreeCombineRect(&(root->BranchBuf[i].mbr), &(root->BranchBuf[j].mbr));
	  waste = RTreeRectSphericalVolume(&cover) - RTreeRectSphericalVolume(&(root->BranchBuf[i].mbr)) - RTreeRectSphericalVolume(&(root->BranchBuf[j].mbr));

   if (waste > worst)
   {
    worst = waste;
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
static void _RTreeMethodZero(HRTREEROOT root, RTREEPARTITION *p, int minfill )
{
 int i;
 REALTYPE biggestDiff;
 int group, chosen=0, betterGroup=0;
 assert(p);

 _RTreeInitPart(p, root->BranchCount, minfill);
 _RTreePickSeeds(root, p);

 while (p->count[0] + p->count[1] < p->total && 
  p->count[0] < p->total - p->minfill && 
  p->count[1] < p->total - p->minfill)
 {
  biggestDiff = (REALTYPE)-1;
  for (i=0; i<p->total; i++)
  {
   if (!p->taken[i])//作用在于此，更新作用，使得while能有效进行下去
   {
    RTREEMBR *r, rect_0, rect_1;
    REALTYPE growth0, growth1, diff;
    
    r = &root->BranchBuf[i].mbr;
    rect_0 = RTreeCombineRect(r, &p->cover[0]);
    rect_1 = RTreeCombineRect(r, &p->cover[1]);
    growth0 = RTreeRectSphericalVolume(&rect_0) - p->area[0]; //计算第i个分支与两组的距离
    growth1 = RTreeRectSphericalVolume(&rect_1) - p->area[1];
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
static void _RTreeLoadNodes(HRTREEROOT root, RTREENODE *n, RTREENODE *q, RTREEPARTITION *p)
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
void SplitNode(HRTREEROOT root, RTREENODE *node, RTREEBRANCH *br, RTREENODE **new_node)
{
 RTREEPARTITION *p;
 int level;
 assert(node && br);
 
 /* load all the branches into a buffer, initialize old node */
 level = node->level; //关键!!!!!!!!!!!!!!!!!!!!!!!!记住原来root的层数和nodeid号，否则清空，后果不堪设想!!!
 _RTreeGetBranches(root, node, br);


 /* find partition */
 p = &(root->Partitions[0]);///////可见实际分配的内存是在root中，在一开始

 /* Note: can&apos;t use MINFILL(n) below since node was cleared by GetBranches() */
 _RTreeMethodZero(root, p, (level>0 ? MINNODEFILL : MINLEAFFILL));


 /* put branches from buffer into 2 nodes according to chosen partition */
 int i,ftaken=2;
 long filelen;
 
 clock_t tStart1 = clock();
 fseek(index_file,0L,SEEK_END);
 filelen=ftell(index_file);
 for(i=0;i<(int)filelen;i=i+sizeof(RTREENODE))
 {
	 fseek(index_file,i,0);
	 fread(&ftaken,sizeof(int),1,index_file);//
     index_node_read_num++;
	 if(ftaken==0) break;
 }
 taken_read_num++;
 taken_read_time += (double)(clock() - tStart1)/CLOCKS_PER_SEC;
	
 node->nodeid=i/sizeof(RTREENODE)+1;
 node->taken=1;
 node->level=level;
	 
 *new_node = (RTREENODE*)malloc(sizeof(RTREENODE));
 InitNode(*new_node);
 (*new_node)->level = level;

 clock_t tStart = clock();
 for(;i<(int)filelen;i=i+sizeof(RTREENODE))
 {
	 fseek(index_file,i,0);
	 fread(&ftaken,sizeof(int),1,index_file);//
	 if(ftaken==0 && (i/sizeof(RTREENODE)+1)!=(unsigned int)(node->nodeid)) break;
 }
 taken_read_num++;
 taken_read_time += (double)(clock() - tStart)/CLOCKS_PER_SEC;
 
 (*new_node)->nodeid=i/sizeof(RTREENODE)+1;
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
 int AddBranch(HRTREEROOT root, RTREEBRANCH *br, RTREENODE *node, RTREENODE **new_node)
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
	
	clock_t tStart = clock();
	fseek(index_file,(node->nodeid-1)*sizeof(RTREENODE),0);
	fwrite(node,sizeof(RTREENODE),1,index_file);
	fflush(index_file);
	index_node_write_num++;
	index_node_write_time += (double)(clock() - tStart)/CLOCKS_PER_SEC;
	/*std::cout << "nodeid: " << node->nodeid
					  << "\t cout: " << node->count
					  << "\t level: " << node->level << std::endl;*/
					  
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
int RTreePickBranch( RTREEMBR *mbr, RTREENODE *node)
{
 RTREEMBR *r;
 int i, first_time = 1;
 REALTYPE increase, bestIncr=(REALTYPE)-1, area, bestArea=0;
 int best=0;
 RTREEMBR tmp_rect;
 assert(mbr && node);

 for (i=0; i<MAXKIDS(node); i++)
 {
  if (node->branch[i].childid)
  {
   r = &node->branch[i].mbr;
   area = RTreeRectSphericalVolume(r);
   tmp_rect = RTreeCombineRect(mbr, r);
   increase = RTreeRectSphericalVolume(&tmp_rect) - area;
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



/**
 * Inserts a new data rectangle into the index structure.
 * Recursively descends tree, propagates splits back up.
 * Returns 0 if node was not split.  Old node updated.
 * If node was split, returns 1 and sets the pointer pointed to by
 * new_node to point to the new node.  Old node updated to become one of two.
 * The level argument specifies the number of steps up from the leaf
 * level to insert; e.g. a data rectangle goes in at level = 0.
 */
 int _RTreeInsertRect(HRTREEROOT root, RTREEBRANCH *pbranch,  RTREENODE *node, RTREENODE **new_node, int level)
{//root为树根；mbr为需插入的项值；tid为项标识；node为当前找到的节点；
 int i;
 RTREEBRANCH b;
 InitBranch(&b);
 RTREENODE *n2;

 //assert(pbranch->mbr && node && new_node);
 assert(level >= 0 && level <= node->level);//because node begin from the root

 /* Still above level for insertion, go down tree recursively */
 if (node->level > level)
 {
  i = RTreePickBranch(&(pbranch->mbr), node);
  RTREENODE child;
  InitNode(&child);
  
  clock_t tStart1 = clock();
  fseek(index_file,(node->branch[i].childid-1)*sizeof(RTREENODE),0);
  fread(&child,sizeof(RTREENODE),1,index_file);
  index_node_read_num++;
  index_node_read_time += (double)(clock() - tStart1)/CLOCKS_PER_SEC;

  if (!_RTreeInsertRect(root, pbranch, &child, &n2, level))
  {
   /* child was not split */
   node->branch[i].mbr = RTreeCombineRect(&(pbranch->mbr), &(node->branch[i].mbr));

   if(node->branch[i].childid==0)
   {
	   node->branch[i].childid=pbranch->childid;
   }

   clock_t tStart = clock();
   fseek(index_file,(node->nodeid-1)*sizeof(RTREENODE),0);
   fwrite(node,sizeof(RTREENODE),1,index_file);
   fflush(index_file);
   index_node_write_num++;
   index_node_write_time += (double)(clock() - tStart)/CLOCKS_PER_SEC;
   /*std::cout << "nodeid: " << node->nodeid
					  << "\t cout: " << node->count
					  << "\t level: " << node->level << std::endl;*/
   return 0;
  }
  
  /* child was split */
  node->branch[i].mbr = RTreeNodeCover(&child);
  node->branch[i].childid=child.nodeid;

  b.childid = n2->nodeid;
  b.mbr = RTreeNodeCover(n2);
  
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





void FindRoot(RTREENODE* rootp)
{
	clock_t tStart = clock();
	fseek(index_file,(index_root_id-1)*sizeof(RTREENODE),0);
	fread(rootp,sizeof(RTREENODE),1,index_file);
    index_node_read_num++;
	index_node_read_time += (double)(clock() - tStart)/CLOCKS_PER_SEC;
}




 void Quadrilateral2MBR(std::vector<REALTYPE>& quadrilateral, RTREEMBR& mbr)
 {
	 REALTYPE minLat = std::min(std::min(std::min(quadrilateral[0], quadrilateral[2]), quadrilateral[4]), quadrilateral[6]);
	 REALTYPE minLng = std::min(std::min(std::min(quadrilateral[1], quadrilateral[3]), quadrilateral[5]), quadrilateral[7]);
	 REALTYPE maxLat = std::max(std::max(std::max(quadrilateral[0], quadrilateral[2]), quadrilateral[4]), quadrilateral[6]);
	 REALTYPE maxLng = std::max(std::max(std::max(quadrilateral[1], quadrilateral[3]), quadrilateral[5]), quadrilateral[7]);
	 mbr.bound[0] = minLng;
	 mbr.bound[1] = minLat;
	 mbr.bound[2] = maxLng;
	 mbr.bound[3] = maxLat;
 }



void build_index(FILE* &object_file, FILE* &index_file) 
{
	std::vector<REALTYPE> quadrilateral(8);
	REALTYPE timestamp;
	REALTYPE azimuth;
	
	int countdataid = 0; //initionalize zero
	std::cout << "MAXCARD: " << MAXCARD <<'\n';
	
	while(!feof(object_file))
	{
		fscanf(object_file, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", 
		&quadrilateral[0], &quadrilateral[1], &quadrilateral[2], &quadrilateral[3], 
		&quadrilateral[4], &quadrilateral[5], &quadrilateral[6], &quadrilateral[7], 
		&azimuth, &timestamp);
		countdataid++;
		
		RTREEBRANCH branch;
		InitBranch(&branch);
		branch.childid = countdataid; 
		Quadrilateral2MBR(quadrilateral, branch.mbr);

		
		RTREEROOT root; //root为根结点
		root.root_node=(RTREENODE*)malloc(sizeof(RTREENODE));
		InitNode(root.root_node);
		  
		fseek(index_file,0L,SEEK_END);
		long leng=ftell(index_file);
		if(leng==0) 
		{   
			root.root_node->taken=1;
			root.root_node->nodeid=1; //赋值
			index_root_id = root.root_node->nodeid;
			root.root_node->level=0;
			root.root_node->count=1;
			CopyBranch(&(root.root_node->branch[0]), &branch);
			
			clock_t tStart = clock();
			fseek(index_file,0L,0); //存储
			fwrite(root.root_node,sizeof(RTREENODE),1,index_file);
			fflush(index_file);
            index_node_write_num++;
			index_node_write_time += (double)(clock() - tStart)/CLOCKS_PER_SEC;
			/*std::cout << "nodeid: " << root.root_node->nodeid
					  << "\t cout: " << root.root_node->count
					  << "\t level: " << root.root_node->level << std::endl;*/
		}//end if leng==0
		else
		{
			FindRoot(root.root_node);
			////////////////////////////////////插入/////////////////////////////////////////////////
  
			RTREENODE *newroot;
			RTREENODE *newnode;
			RTREEBRANCH b;
			InitBranch(&b);
			
			/* root split */
			if (_RTreeInsertRect(&root, &branch, root.root_node, &newnode, 0))  //返回来的两节点已写到文件中了
			{
				int ftaken;
				newroot=(RTREENODE*)malloc(sizeof(RTREENODE)); /* grow a new root, & tree taller */
				InitNode(newroot);
				newroot->taken=1;
				newroot->level = root.root_node->level + 1;

				fseek(index_file,0L,SEEK_END);
				leng=ftell(index_file);
				int i;
				
				clock_t tStart1 = clock();
				for(i=0;i<(int)leng;i=i+sizeof(RTREENODE))
				{
				 fseek(index_file,i,0);
				 fread(&ftaken,sizeof(int),1,index_file);
				 if(ftaken==0) break;
				}
				taken_read_num++;
				taken_read_time += (double)(clock() - tStart1)/CLOCKS_PER_SEC;
	
				newroot->nodeid=i/sizeof(RTREENODE)+1;
				index_root_id = newroot->nodeid;
				std::cout << "index_root_id: " << index_root_id << std::endl;
				b.mbr = RTreeNodeCover(root.root_node);
				b.childid = root.root_node->nodeid;
				fseek(index_file,0L,SEEK_END);
				AddBranch(&root, &b, newroot, NULL);

				b.mbr = RTreeNodeCover(newnode);
				b.childid = newnode->nodeid;
				AddBranch(&root, &b, newroot, NULL);
				
				clock_t tStart = clock();
				fseek(index_file,((root.root_node->nodeid)-1)*sizeof(RTREENODE),0);
				fwrite(root.root_node,sizeof(RTREENODE),1,index_file);
                index_node_write_num++;
				/*std::cout << "nodeid: " << root.root_node->nodeid
					  << "\t cout: " << root.root_node->count
					  << "\t level: " << root.root_node->level << std::endl;*/
				fflush(index_file);

				fseek(index_file,((newnode->nodeid)-1)*sizeof(RTREENODE),0);
				fwrite(newnode,sizeof(RTREENODE),1,index_file);
                index_node_write_num++;
				/*std::cout << "nodeid: " << newnode->nodeid
					  << "\t cout: " << newnode->count
					  << "\t level: " << newnode->level << std::endl;*/
				fflush(index_file);
				index_node_write_time += (double)(clock() - tStart)/CLOCKS_PER_SEC;

				Copy(root.root_node , newroot);
				delete newroot;
			 }//END IF
		 }//end else if leng==0
		 delete root.root_node;
	} //end while result->next()
}



