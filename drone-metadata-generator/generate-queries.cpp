#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>

#include "drone.h"

using namespace std;

/**
 * @para query_out_file: the output file name including generated queries.
 * @para minLat: the minimum latitude of the query point.
 * @para minLng: the minimum longitude of the query point.
 * @para maxLat: the maximum latitude of the query point.
 * @para maxLng: the maximum longitude of the query point.
 * @para query_num: the total number of queries to be generated.
 */
void generate_point_queries(ofstream& query_out_file, 
					  double minLat,
					  double minLng,
					  double maxLat,
					  double maxLng,
					  int query_num)
{
	std::random_device rd; // Random seed
    std::mt19937 gen(rd()); //Init Mersenne Twister pseudo-random number generator
    std::uniform_int_distribution<> dis(0, 1000);
    
	for(int count = 0; count<query_num; count++)
	{
		REALTYPE percentage_lat = dis(gen) / (REALTYPE)(1000.0);
		REALTYPE qlat = percentage_lat * (maxLat - minLat) + minLat;
		REALTYPE percentage_lng = dis(gen) / (REALTYPE)(1000.0);
		REALTYPE qlng = percentage_lng * (maxLng - minLng) + minLng;
		
		//print the generated query
		query_out_file << std::fixed << std::setprecision(6)
				       << qlat << "," << qlng << "\n";
	}
}





/**
 * @para query_out_file: the output file name including generated queries.
 * @para minLat: the minimum latitude of the query point.
 * @para minLng: the minimum longitude of the query point.
 * @para maxLat: the maximum latitude of the query point.
 * @para maxLng: the maximum longitude of the query point.
 * @para minR_Dir: the maximum query radius in km (or direction in degree).
 * @para maxR_Dir: the maximum query radius in km (or direction in degree).
 * @para query_num: the total number of queries to be generated.
 */
void generate_range_directional_queries(ofstream& query_out_file, 
					  double minLat,
					  double minLng,
					  double maxLat,
					  double maxLng,
					  double minR_Dir,
					  double maxR_Dir,
					  int query_num)
{	
	std::random_device rd; // Random seed
    std::mt19937 gen(rd()); //Init Mersenne Twister pseudo-random number generator
    std::uniform_int_distribution<> dis(0, 1000);
    
	for(int count = 0; count<query_num; count++)
	{
		REALTYPE percentage_lat = dis(gen) / (REALTYPE)(1000.0);
		REALTYPE qlat = percentage_lat * (maxLat - minLat) + minLat;
		
		REALTYPE percentage_lng = dis(gen) / (REALTYPE)(1000.0);
		REALTYPE qlng = percentage_lng * (maxLng - minLng) + minLng;
		
		REALTYPE percentage_radius = dis(gen) / (REALTYPE)(1000.0);
		REALTYPE qradius_direction = percentage_radius * (maxR_Dir - minR_Dir) + minR_Dir;
		
		//print the generated query
		query_out_file << std::fixed << std::setprecision(6)
				       << qlat << "," << qlng << "," << qradius_direction << "\n";
	}
}




/**
 *
 * @para query_type: the type of queries 
                   (point_query, range_query, directional_query).
 */

int main(int argc, char* argv[])
{
	if(argc<8) {
		printf("please type input parameters described in readme.txt.\n");
		return -1;
	}
	
	std::string out_fname(argv[1]);
	ofstream out_file;
	out_file.open(out_fname, std::ofstream::out | std::ofstream::app);
	if (!out_file.is_open()) {
		std::cout << "Couldn't open " << out_fname << std::endl;
		return -1;
	}
	
	std::string query_type(argv[2]);
	double minLat = atof(argv[3]);
	double minLng = atof(argv[4]);
    double maxLat = atof(argv[5]);
	double maxLng = atof(argv[6]);
	int query_num = atoi(argv[7]);
	
	if(query_type == "point_query")
	{
		generate_point_queries(out_file, minLat, minLng, maxLat, maxLng, query_num);
	}
	else //if(query_type == "range_query" or query_type == "directional_query")
	{
		double minR_Dir = atof(argv[8]);
		double maxR_Dir = atof(argv[9]);
		generate_range_directional_queries(out_file, 
		                                   minLat, minLng, 
										   maxLat, maxLng, 
										   minR_Dir, maxR_Dir, 
										   query_num);
	}
	
	out_file.close();
	return 0;
}
	
