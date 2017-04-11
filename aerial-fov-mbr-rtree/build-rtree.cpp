
#include "rtree.h"
#include "aerialfov.h"
#include <iostream>
#include <vector>

using namespace std;

// Global variants
FILE* index_file; 
int index_root_id; //The id of root index node.
int index_node_read_num;
int index_node_write_num;
int taken_read_num;
double index_node_read_time;
double index_node_write_time;
double taken_read_time;
long long RTreeCombineRect_invoke_num;
double RTreeCombineRect_invoke_time;

int main(int argc, char* argv[])
{
	if(argc<4) {
		printf("please type input parameters described in readme.txt.\n");
		return -1;
	}
	
	FILE* object_file = fopen(argv[1], "r+");
	if (object_file == NULL) {
		std::cout << "Couldn't open " << argv[1] << std::endl;
		return -1;
	}
	
	index_file = fopen(argv[2], "rb+");
	if (index_file == NULL) {
		std::cout << "Couldn't open " << argv[2] << std::endl;
		return -1;
	}
	
	FILE* index_log_file = fopen(argv[3], "a");
	if (index_log_file == NULL) {
		printf("Couldn't open %s\n", argv[3]);
		return -1;
	}
	
	/**
	 * Construct TetraR-tree index.
	 */
	printf("Page Size: %d KB\n", PAGE_SIZE/1024);
	fprintf(index_log_file, "Page Size: %d KB\n", PAGE_SIZE/1024);
	printf("Fanout: %d\n", MAXCARD);
	fprintf(index_log_file, "Fanout: %d\n", MAXCARD);
	
	clock_t tStart = clock();
	
    build_index(object_file, index_file); //-----build indexing
	
	double index_building_time = (double)(clock() - tStart)/CLOCKS_PER_SEC;
	printf("R-tree Construct Time: %.6f sec\n", index_building_time);
	fprintf(index_log_file, "R-tree Construct Time: %.6f sec\n", index_building_time);
	
    fprintf(index_log_file, "Index Node Read Number: %d\n", index_node_read_num);
	fprintf(index_log_file, "Index Node Read Time: %.3f secs\n", index_node_read_time);
    fprintf(index_log_file, "Index Node Write Number: %d\n", index_node_write_num);
	fprintf(index_log_file, "Taken Read Number: %d\n", taken_read_num);
	fprintf(index_log_file, "Index Node Write Time: %.3f secs\n", index_node_write_time);
	fprintf(index_log_file, "Taken Read Time: %.3f secs\n", taken_read_time);
    
    fprintf(index_log_file, "RTreeCombineRect_invoke_num: %lld\n", RTreeCombineRect_invoke_num);
    fprintf(index_log_file, "RTreeCombineRect_invoke_time: %.3f secs\n", RTreeCombineRect_invoke_time);
    
	printf("Index Root Node ID: %d\n", index_root_id);
	fprintf(index_log_file, "Index Root Node ID: %d\n", index_root_id);
	
	fseek(index_file,(index_root_id-1)*sizeof(RTREENODE),0);
	RTREENODE root_node;
	fread(&root_node,sizeof(RTREENODE),1,index_file);
	int index_height = root_node.level;
	printf("Index Height: %d\n", index_height);
	fprintf(index_log_file, "Height: %d\n", index_height);
	
	fseek(index_file,0L,SEEK_END);
	long leng=ftell(index_file);
	int total_index_node_num = leng/sizeof(RTREENODE);
	double index_file_size = leng/(1024*1024); //Bytes -> MegaBytes
	printf("Total Index Node Number: %d\n", total_index_node_num);
	printf("Index File Size: %.3f MB\n", index_file_size);
	fprintf(index_log_file, "Total Index Node Number: %d\n", total_index_node_num);
	fprintf(index_log_file, "Index File Size: %.3f MB\n", index_file_size);
	
	fprintf(index_log_file, "\n\n\n");
	
	
    fclose(object_file);
	fclose(index_file);
	fclose(index_log_file);
	return 0;
}


