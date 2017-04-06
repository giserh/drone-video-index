#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "drone.h"

using namespace std;

int main(int argc, char* argv[])
{
	REALTYPE lat = 0.0;
	REALTYPE lng = 0.0;
	REALTYPE hgt = 0.0; //camera height (distance from ground, meter)
	REALTYPE azimuth = 0.0; //w.r.t. the north, in degree.
	REALTYPE pitch = 0.0; //in degree
	REALTYPE roll = 0.0; //in degree
	REALTYPE viewable_angle = 0.0; //camera viewable angle (in degree)
	if(argc > 3)
	{
		/****** One FOV ******/
		lat = atof(argv[1]);
		lng = atof(argv[2]);
		hgt = atof(argv[3]); //camera height (distance from ground, kilometer)
		azimuth = atof(argv[4]); //w.r.t. the north, in degree.
		pitch = atof(argv[5]); //in degree
		roll = atof(argv[6]); //in degree
		viewable_angle = atof(argv[7]); //camera viewable angle (in degree)
		Drone drone(lat, lng, hgt, azimuth, pitch, roll, viewable_angle); 
		
		drone.calc_coverage();
		//print quadrilateral
		std::cout << std::fixed << std::setprecision(6)
				  << "{lat: " << drone.quadrilateral[0] << ", lng: " << drone.quadrilateral[1] << "},\n"
				  << "{lat: " << drone.quadrilateral[2] << ", lng: " << drone.quadrilateral[3] << "},\n"
				  << "{lat: " << drone.quadrilateral[4] << ", lng: " << drone.quadrilateral[5] << "},\n"
				  << "{lat: " << drone.quadrilateral[6] << ", lng: " << drone.quadrilateral[7] << "}"
				  << std::endl;
	}
	else
	{
		/****** Process a set of FOVs in a file *******/
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
		
		viewable_angle = 80.0;
		std::string line;
		std::getline(data_file, line);
		char comma;
		std::string timestr;
		while(!data_file.eof())
		{
			std::getline(data_file, line);
			std::istringstream iss(line);
		    iss>>lat>>comma>>lng>>comma>>hgt>>comma>>azimuth>>comma>>pitch>>comma>>roll>>comma>>timestr;
			hgt = hgt / 1000.0; //meter -> kilometer
			azimuth = azimuth * 180.0 / PI;
			pitch = pitch * 180.0 / PI;
			roll = roll * 180.0 / PI;
			Drone drone(lat, lng, hgt, azimuth, pitch, roll, viewable_angle); 
			drone.calc_coverage();
			//print quadrilateral
			out_file << std::fixed << std::setprecision(6)
					 << drone.quadrilateral[0] << "," << drone.quadrilateral[1] << ","
					 << drone.quadrilateral[2] << "," << drone.quadrilateral[3] << ","
					 << drone.quadrilateral[4] << "," << drone.quadrilateral[5] << ","
					 << drone.quadrilateral[6] << "," << drone.quadrilateral[7] << ","
					 << azimuth << ","
					 << timestr << "\n";
		}
	}
	return 0;
}
	