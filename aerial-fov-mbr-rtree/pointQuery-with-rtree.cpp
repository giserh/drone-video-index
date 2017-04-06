
#include "rtree.h"
#include "aerialfov.h"
#include "query-basic-functions.h"
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
FILE* object_file;
FILE* query_log_file;

/** 
 * The number of pruned index nodes (leaf and non-leaf nodes).
 */
int index_node_access_num = 0;
double index_node_access_time = 0.0;

/** 
 * The number of fovs that need to look up the object file.
 */
int fov_object_lookup_num = 0;
double fov_object_lookup_time = 0.0;

/** 
 * The number of fovs that are results.
 */
int result_fov_num = 0;


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




void point_query(REALTYPE qlat, REALTYPE qlng)
{
	RTREEROOT root; //root node
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
			
			clock_t tStart = clock();
			fseek(index_file, (branchp->childid-1)*sizeof(RTREENODE), 0);
			if(!index_file) printf("index_file is null. %d\n", branchp->childid);
			fread(&node, sizeof(RTREENODE), 1, index_file);
			index_node_access_num++;
			index_node_access_time += (double)(clock() - tStart)/CLOCKS_PER_SEC;
						
			if(point_in_mbr(&(branchp->mbr), qlat, qlng)) 
			{
				//inside the MBR, then expand node (node must be an index node)
				if(node.level>0) //non-leaf node
				{
					for(int i=0; i<node.count; i++)
					{
						RTREEBRANCH* childbranchp = (RTREEBRANCH*) malloc(sizeof(RTREEBRANCH));
						CopyBranch(childbranchp, &(node.branch[i]));
						RangeQueryStack.push(childbranchp);
						/*if(childbranchp->childid == 3404 or 
						   childbranchp->childid == 3248 or 
						   childbranchp->childid == 3007)
						   std::cout << "nodeid: \t" << node.nodeid << std::endl;*/
					}
				}
				else //leaf node
				{
					for(int i=0; i<node.count; i++)
					{
						if(point_in_mbr(&(node.branch[i].mbr), qlat, qlng)) 
						{
							//Inside the MBR of the aerial-FOV, but still need to be checked.
							fov_object_lookup_num ++;
							AerialFOV fov;
							read_aerial_fov_from_file(node.branch[i].childid, fov);
							if(point_in_quadrilateral(fov, qlat, qlng))
							{
								//std::cout << "nodeid: \t" << node.nodeid << std::endl;
								result_fov_num++;
							}
							else 
							{
								pruned_fov_num++;
							}
						}
						else 
						{
							pruned_fov_num_byMBR++;
						}
					}
				}
			}
			else 
			{
				pruned_index_node_num++;
			}
            if(branchp) free(branchp);
		}//end while
		
	}//end else

}



void point_query_with_print_info(REALTYPE qlat, REALTYPE qlng)
{
	
	/**
	 * Initialize the glable variants
	 */
	index_node_access_num = 0;
	index_node_access_time = 0.0;
	fov_object_lookup_time = 0;
	result_fov_num = 0;
    pruned_index_node_num = 0;
	pruned_fov_num_byMBR = 0;
	fov_object_lookup_num = 0;
    pruned_fov_num = 0; 
	
	fprintf(query_log_file, "point query with R-tree:\n");
	fprintf(query_log_file, "point query: (%.6f, %.6f)\n", qlat, qlng);
	
	clock_t tStart = clock();
	
	point_query(qlat, qlng); // Point query
	
	double query_time = (double)(clock() - tStart)/CLOCKS_PER_SEC;
	//printf("Total query time: %.6f sec\n", query_time);
	fprintf(query_log_file, "Total query time: %.6f sec\n", query_time);
	
	//printf("index_node_access_time: %.6f sec\n", index_node_access_time);
	fprintf(query_log_file, "index_node_access_time: %.6f sec\n", index_node_access_time);
	
	
	//printf("fov_object_lookup_time: %.6f sec\n", fov_object_lookup_time);
	fprintf(query_log_file, "fov_object_lookup_time: %.6f sec\n", fov_object_lookup_time);
	
	//printf("index_node_access_num: %d\n", index_node_access_num);
	fprintf(query_log_file, "index_node_access_num: %d\n", index_node_access_num);
	
	//printf("pruned_index_node_num: %d\n", pruned_index_node_num);
	fprintf(query_log_file, "pruned_index_node_num: %d\n", pruned_index_node_num);
	
	//printf("pruned_fov_num_byMBR: %d\n", pruned_fov_num_byMBR);
	fprintf(query_log_file, "pruned_fov_num_byMBR: %d\n", pruned_fov_num_byMBR);
	
	//printf("fov_object_lookup_num: %d\n", fov_object_lookup_num);
	fprintf(query_log_file, "fov_object_lookup_num: %d\n", fov_object_lookup_num);
	
	//printf("result_fov_num: %d\n", result_fov_num);
	fprintf(query_log_file, "result_fov_num: %d\n", result_fov_num);
	
	//printf("pruned_fov_num: %d\n", pruned_fov_num);
	fprintf(query_log_file, "pruned_fov_num: %d\n", pruned_fov_num);
	
	fprintf(query_log_file, "\n\n\n");
}





int main(int argc, char* argv[])
{
	if(argc<6) {
		printf("please type input parameters described in readme.txt.\n");
		return -1;
	}
	
	index_file = fopen(argv[1], "rb+");
	if (index_file == NULL) {
		std::cout << "Couldn't open " << argv[1] << std::endl;
		return -1;
	}
	index_root_id = std::stoi(argv[2]);
	
	object_file = fopen(argv[3], "rb");
	if (object_file == NULL) {
		std::cout << "Couldn't open " << argv[3] << std::endl;
		return -1;
	}
	
	query_log_file = fopen(argv[4], "a");
	if (query_log_file == NULL) {
		printf("Couldn't open %s\n", argv[4]);
		return -1;
	}
	
	
	if(argc>=7)
	{
		REALTYPE qlat = std::stof(argv[5]);
		REALTYPE qlng = std::stof(argv[6]);
		point_query_with_print_info(qlat, qlng); 
	}
	else
	{
		// argc = 6
		FILE* query_file = fopen(argv[5], "r+");
		if (query_file == NULL) {
			printf("Couldn't open %s\n", argv[5]);
			return -1;
		}
	
		REALTYPE qlat, qlng;
		while(!feof(query_file))
		{
			fscanf(query_file, "%f,%f\n", &qlat, &qlng);
			point_query_with_print_info(qlat, qlng);
		}
		fclose(query_file);
	}
	
	
	fclose(index_file);
	fclose(object_file);
	fclose(query_log_file);
	return 0;
}


