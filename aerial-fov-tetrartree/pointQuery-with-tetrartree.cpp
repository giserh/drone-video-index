
#include "tetrartree.h"
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

FILE* query_log_file;

/** 
 * The number of pruned index nodes (leaf and non-leaf nodes).
 */
int index_node_access_num = 0;
double index_node_access_time = 0.0;


/** 
 * The number of fovs that are results.
 */
int result_fov_num = 0;


int pruned_index_node_num = 0;


/** 
 * Pruned fov number being checked by ray-casting algorithm.
 * Need to look up the object file.
 */
int pruned_fov_num = 0; 


int total_hit_node_num = 0;


void point_query(REALTYPE qlat, REALTYPE qlng)
{
	TETRARTREEROOT root; //root node
	root.root_node=(TETRARTREENODE*)malloc(sizeof(TETRARTREENODE));
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
		TETRARTREEBRANCH* RootBranch = (TETRARTREEBRANCH*)malloc(sizeof(TETRARTREEBRANCH));
		TetraRTreeNodeCover(RootBranch, root.root_node);
		
		total_hit_report(root.root_node);
		
		stack<TETRARTREEBRANCH*> RangeQueryStack;
		RangeQueryStack.push(RootBranch);
		TETRARTREENODE node;
		while(!RangeQueryStack.empty())
		{
			TETRARTREEBRANCH* branchp = RangeQueryStack.top();
			RangeQueryStack.pop();
			
			clock_t tStart = clock();
			fseek(index_file, (branchp->childid-1)*sizeof(TETRARTREENODE), 0);
			if(!index_file) printf("index_file is null. %d\n", branchp->childid);
			fread(&node, sizeof(TETRARTREENODE), 1, index_file);
			index_node_access_num++;
			index_node_access_time += (double)(clock() - tStart)/CLOCKS_PER_SEC;
			
			/**
			 * MBR-based pruning
			 */
			/*MBR mbr01 = CombineMBR(&(branchp->mbrs[0]), &(branchp->mbrs[1]));
			MBR mbr012 = CombineMBR(&(mbr01), &(branchp->mbrs[2]));
			MBR mbr0123 = CombineMBR(&(mbr012), &(branchp->mbrs[3]));
			if(!point_in_mbr(&mbr0123, qlat, qlng))*/
			
			/**
			 * Outer-convex-based pruning
			 */
			Polygon outer_convex = calc_outer_convex(branchp->mbrs);
			if(outer_convex.edges.size()>0 && 
			   !outer_convex.point_in_polygon(qlat, qlng))  
			{
				// Not inside the outer convex. Prune the branch.
				pruned_index_node_num++;
			}
			else 
			{
				/*for(int j=0; j<MBR_NUMB; j++) 
				{
					for(int b=0; b<SIDES_NUMB; b++)
					{
						std::cout << branchp->mbrs[j].bound[b] << "\t";
					}
					std::cout << "\n";
				}*/
				
				/*Polygon inner_convex = calc_inner_convex(branchp->mbrs);
				if(inner_convex.point_in_polygon(qlat, qlng))
				{
					// Inside the inner convex. It's a "total hit".
				    //total_hit_report(&node);
				}
				else */
				{
					/**  
					 * Otherwise, refer it as a candidate.
					 * Expand the node to check its child nodes / objects.
					 */				 
					if(node.level>0) //non-leaf node
					{
						for(int i=0; i<node.count; i++)
						{
							TETRARTREEBRANCH* childbranchp = (TETRARTREEBRANCH*) malloc(sizeof(TETRARTREEBRANCH));
							CopyBranch(childbranchp, &(node.branch[i]));
							RangeQueryStack.push(childbranchp);
							/*if(childbranchp->childid == 3379 or 
						       childbranchp->childid == 3232 or 
						       childbranchp->childid == 2994)
						       std::cout << "nodeid: \t" << node.nodeid << std::endl;*/
						}
					}
					else //leaf node
					{
						std::vector<Point> qud_corner_points(MBR_NUMB);
						for(int i=0; i<node.count; i++)
						{
							// fov <- node.branch[i]
							for(int j=0; j<MBR_NUMB; j++)
							{
								qud_corner_points[j] = Point(node.branch[i].mbrs[j].bound[1], 
														     node.branch[i].mbrs[j].bound[0]);
							}
							AerialFOV fov(qud_corner_points, node.branch[i].orientation.min, 0.0);
							
							if(fov.point_in_polygon(qlat, qlng)) 
							{
								//std::cout << "nodeid: \t" << node.nodeid << std::endl;
								result_fov_num++;
							}
							else 
							{
								/*printf("nodeid: %d\n", node.nodeid);
								printf("points: \n");
								for(auto p : qud_corner_points) 
									printf("%.6f\t%0.6f\n", p.longitude, p.latitude);*/
								pruned_fov_num++;
							}
						}
					}
				}
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
	result_fov_num = 0;
    pruned_index_node_num = 0;
    pruned_fov_num = 0; 
	total_hit_node_num = 0;
	
	fprintf(query_log_file, "point query with TetraR-tree:\n");
	fprintf(query_log_file, "point query: (%.6f, %.6f)\n", qlat, qlng);
	
	clock_t tStart = clock();
	
	point_query(qlat, qlng); // Point query
	
	double query_time = (double)(clock() - tStart)/CLOCKS_PER_SEC;
	//printf("Total query time: %.6f sec\n", query_time);
	fprintf(query_log_file, "Total query time: %.6f sec\n", query_time);
	
	//printf("index_node_access_time: %.6f sec\n", index_node_access_time);
	fprintf(query_log_file, "index_node_access_time: %.6f sec\n", index_node_access_time);
	
	//printf("index_node_access_num: %d\n", index_node_access_num);
	fprintf(query_log_file, "index_node_access_num: %d\n", index_node_access_num);
	
	//printf("pruned_index_node_num: %d\n", pruned_index_node_num);
	fprintf(query_log_file, "pruned_index_node_num: %d\n", pruned_index_node_num);
	
	//printf("result_fov_num: %d\n", result_fov_num);
	fprintf(query_log_file, "result_fov_num: %d\n", result_fov_num);
	
	//printf("pruned_fov_num: %d\n", pruned_fov_num);
	fprintf(query_log_file, "pruned_fov_num: %d\n", pruned_fov_num);
	
	fprintf(query_log_file, "\n\n\n");
	
	//printf("total_hit_node_num: %d\n", total_hit_node_num);
}


int main(int argc, char* argv[])
{
	if(argc<5) {
		printf("please type input parameters described in readme.txt.\n");
		return -1;
	}
	
	index_file = fopen(argv[1], "rb+");
	if (index_file == NULL) {
		std::cout << "Couldn't open " << argv[1] << std::endl;
		return -1;
	}
	index_root_id = std::stoi(argv[2]);
	
	query_log_file = fopen(argv[3], "a");
	if (query_log_file == NULL) {
		printf("Couldn't open %s\n", argv[3]);
		return -1;
	}
	
	if(argc>=6)
	{
		REALTYPE qlat = std::stof(argv[4]);
		REALTYPE qlng = std::stof(argv[5]);
		point_query_with_print_info(qlat, qlng); 
	}
	else
	{
		// argc = 5
		FILE* query_file = fopen(argv[4], "r+");
		if (query_file == NULL) {
			printf("Couldn't open %s\n", argv[4]);
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
	fclose(query_log_file);
	return 0;
}


