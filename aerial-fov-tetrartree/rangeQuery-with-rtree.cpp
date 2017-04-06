
#include "rtree.h"
#include "aerialfov.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stack>
#include <math.h>
#include <cmath> 
#include <time.h>
#include<limits>

using namespace std;

//Global variants
FILE* index_file;
int index_root_id; //The id of root index node.
std::ifstream object_file;

/** 
 * The number of pruned index nodes (leaf and non-leaf nodes).
 */
int index_node_access_num = 0;

/** 
 * The number of fovs that need to look up the object file.
 */
int fov_object_lookup_num = 0;


int pruned_index_node_num = 0;

/** 
 * Pruned fov number being checked by the MBR of fov.
 * No need to look up the object file.
 */
int pruned_fov_num_byMBR = 0; 

/** 
 * Pruned fov number being checked by ray-casting algorithm.
 * Need to look up the object file.
 */
int pruned_fov_num = 0; 






void CircleRangeQuery(REALTYPE qlat, REALTYPE qlng, REALTYPE qradius)
{
	RTREEROOT root; //root为根结点
	root.root_node=(RTREENODE*)malloc(sizeof(RTREENODE));
	InitNode(root.root_node);

	fseek(index_file,0L,SEEK_END);
	long leng=ftell(index_file);
	if(leng==0) 
	{
		printf("Sorry, the index is empty!\n");
	}
	else
	{
		FindRoot(root.root_node);
		RTREEBRANCH* RootBranch = (RTREEBRANCH*)malloc(sizeof(RTREEBRANCH));
		InitBranch(RootBranch);
		RootBranch->childid = root.root_node->nodeid;
		RootBranch->mbr = RTreeNodeCover(root.root_node);

		stack<RTREEBRANCH*> RangeQueryStack;
		RangeQueryStack.push(RootBranch);
		RTREENODE node;
		while(!RangeQueryStack.empty())
		{
			RTREEBRANCH* branchp = RangeQueryStack.top();
			RangeQueryStack.pop();
			fseek(index_file, (branchp->childid-1)*sizeof(RTREENODE), 0);
			fread(&node, sizeof(RTREENODE), 1, index_file);
			index_node_access_num++;

			int flag = NodeOverlapCheck(&(branchp->mbr), qlat, qlng, qradius);
			if(flag==1) //partial overlap, then expand node (node must be an index node)
			{
				if(node.level>0) //non-leaf node
				{
					for(int i=0; i<node.count; i++)
					{
						RTREEBRANCH* childbranchp = (RTREEBRANCH*) malloc(sizeof(RTREEBRANCH));
						CopyBranch(childbranchp, &(node.branch[i]));
						RangeQueryStack.push(childbranchp);
					}
				}
				else //leaf node
				{
					for(int i=0; i<node.count; i++)
					{
						int ObjOverlapFlag = NodeOverlapCheck(&(node.branch[i].mbr), qlat, qlng, qradius);
						if(ObjOverlapFlag==1) //MBR of the aerial-FOV object partially overlap with the query
						{
							fov_object_lookup_num ++;
							if(objectOverlap(node.branch[i].childid, qlat, qlng, qradius))
							{
								printf("result: \t%d\n", node.branch[i].childid); //The fov is a result
							}
							//else printf("prune fov: %d \n", node.branch[i].childid);
							else pruned_fov_num++;
						}
						else if(ObjOverlapFlag==2) //hit
						{
							printf("result: \t%d\n", node.branch[i].childid);
						}
						//else printf("prune (mbr) fov: %d \n", node.branch[i].childid);
						else pruned_fov_num_byMBR++;
					}
				}
			}
			else if(flag==2) //total hit, then report all the objects in node to be results
			{
				ReportResults(&node);
			}
			//else printf("prune node: %d \n", node.nodeid);
			else pruned_index_node_num++;
            free(branchp);
		}//end while
		
	}//end else

	
}





int main(int argc, char* argv[])
{
	index_file = fopen(argv[1], "rb+");
	if (index_file == NULL) {
		std::cout << "Couldn't open " << argv[1] << std::endl;
		return -1;
	}
	index_root_id = std::stoi(argv[2]);
	std::cout<< "index_root_id: " << index_root_id << std::endl;
	
	std::string object_fname(argv[3]);
	object_file.open(object_fname, std::ios::in);
	if (!object_file.is_open()) {
		std::cout << "Couldn't open " << object_fname << std::endl;
		return -1;
	}
		
	REALTYPE qlat = std::stof(argv[4]);
	REALTYPE qlng = std::stof(argv[5]);
	REALTYPE qradius = std::stof(argv[6]);
	std::cout<< "qlat: " << qlat
             << "\tqlng: " << qlng
			 << "\tqradius: " << qradius
			 << std::endl;
	
	clock_t tStart = clock();
	index_node_access_num =0;
	CircleRangeQuery(qlat, qlng, qradius);
	printf("Time taken: %.2f sec\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
	printf("index_node_access_num: %d\n", index_node_access_num);
	printf("fov_object_lookup_num: %d\n", fov_object_lookup_num);
	printf("pruned_index_node_num: %d\n", pruned_index_node_num);
	printf("pruned_fov_num_byMBR: %d\n", pruned_fov_num_byMBR);
	printf("pruned_fov_num: %d\n", pruned_fov_num);

	fclose(index_file);
	object_file.close();
	return 0;
}


