#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>

#include "drone.h"

using namespace std;

/***
 @para data_file: input file name
 @para out_file: output file name
 @para batch_size: the number of snaptshots / FOVs are processed each time.
                   suggest setting 130. 
***/
void distribute_every_one_min(ifstream& data_file, 
                              ofstream& out_file, 
							  int batch_size)
{
	REALTYPE lat = 0.0;
	REALTYPE lng = 0.0;
	REALTYPE hgt = 0.0; //camera height (distance from ground, meter)
	REALTYPE azimuth = 0.0; //w.r.t. the north, in degree.
	REALTYPE pitch = 0.0; //in degree
	REALTYPE roll = 0.0; //in degree
	std::string line;
	std::getline(data_file, line);
	char comma;
	std::string timestr;
	int linenum = 0;
	
	double minLat = 47.313494;
	double minLng = 7.999397;
    double maxLat = 48.014538;
	double maxLng = 9.100777;
	
	std::random_device rd; // Random seed
    std::mt19937 gen(rd()); //Init Mersenne Twister pseudo-random number generator
    std::uniform_int_distribution<> dis(0, 645436/batch_size-1); //645436: data number.
    
	double vector_centerlat, vector_centerlng;
	while(!data_file.eof())
	{
		std::getline(data_file, line);
		std::istringstream iss(line);
		iss>>lat>>comma>>lng>>comma>>hgt>>comma>>azimuth>>comma>>pitch>>comma>>roll>>comma>>timestr;
		
		double newlat, newlng;
		if(linenum % batch_size == 0)
		{
			// Another one minute. Re-assign the vectors.
			double percentage_lat = dis(gen) / (double)(645436/batch_size-1);
			vector_centerlat = percentage_lat * (maxLat - minLat) + minLat - lat;
			double percentage_lng = dis(gen) / (double)(645436/batch_size-1);
			vector_centerlng = percentage_lng * (maxLng - minLng) + minLng - lng;
		}
		newlat = vector_centerlat + lat;
		newlng = vector_centerlng + lng;
		
		//print new extended data
		out_file << std::fixed << std::setprecision(6)
				 << newlat << "," << newlng << "," << hgt << "," 
				 << azimuth << "," << pitch << "," << roll << ","
				 << timestr << "\n";
		linenum ++;
	}
}





int main(int argc, char* argv[])
{
	std::string data_fname(argv[1]);
	ifstream data_file;
	data_file.open(data_fname);
	if (!data_file.is_open()) {
		std::cout << "Couldn't open " << data_fname << std::endl;
		return -1;
	}
	
	std::string out_fname(argv[2]);
	ofstream out_file;
	out_file.open(out_fname, std::ofstream::out | std::ofstream::app);
	if (!out_file.is_open()) {
		std::cout << "Couldn't open " << out_fname << std::endl;
		return -1;
	}
	
	int batch_size = atoi(argv[3]);
	
	//method 1: scaling
	
	//method 2: distribute_every_one_min
	distribute_every_one_min(data_file, out_file, batch_size);
	
	return 0;
}
	
