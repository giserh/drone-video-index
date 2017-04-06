#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>
#include <cmath> 

#include "drone.h"

using namespace std;


/***
 Calculate the surface distance between two points A and B.
 @para[in] point A(Alat, Alng) and point B(Blat, Blng)
 @para[out] the distance between A and B (In km unit).
***/
REALTYPE EarthDistance(REALTYPE Alat, REALTYPE Alng, REALTYPE Blat, REALTYPE Blng) 
{
	REALTYPE distpq = 6371 * 2 * asin(sqrt(pow(sin((Alat - Blat)*PI / 180 / 2), 2) 
	                                    + cos(Alat*PI / 180) 
										  * cos(Blat*PI / 180) 
										  * pow(sin((Alng - Blng) * PI / 180 / 2), 2)
										));
	return distpq; //kilometers
}



/*** 
 Calculat the area of a quadrilateral (pointA, pointB, pointC, pointD).
 Each point is represented as <lat, lng>
***/
REALTYPE calc_quadrilateral_area(std::vector<REALTYPE>& quadrilateral)
{
	// Calculate the distance from pointB to pointD
	REALTYPE segmentBD_length = EarthDistance(quadrilateral[2], quadrilateral[3], 
	                                       quadrilateral[6], quadrilateral[7]);
	
	/***
	 * resources: https://knowledge.safe.com/articles/725/calculating-accurate-length-in-meters-for-lat-long.html
	 * resources: http://physics.wooster.edu/Manz/sandcollection/swaplist/Latitude%20and%20Longitude.pdf
	 ***/
	REALTYPE lat = quadrilateral[2];
	REALTYPE KMperDegreeLat = 111.13292 - 0.55982 * cos(2* lat/180*PI) + 0.001175*cos(4*lat/180*PI);
	REALTYPE KMperDegreeLng = 111.41284 * cos(lat/180*PI) - 0.0935 * cos(3*lat/180*PI);
	REALTYPE avgKMperDegree = (KMperDegreeLat + KMperDegreeLng)/2.0;
	//std::cout<<KMperDegreeLat <<", " << KMperDegreeLng << ": " << avgKMperDegree << '\n';
	
	// Calculate the height of the triangle ABD, i.e., the distance from pointA to lineBD
	REALTYPE AA = quadrilateral[2] - quadrilateral[6]; //YB - YD
	REALTYPE BB = quadrilateral[7] - quadrilateral[3]; //XD - XB
	REALTYPE CC = quadrilateral[3]*quadrilateral[6] - quadrilateral[7]*quadrilateral[2]; //XBYD - XDYB
	REALTYPE height_A_BD = abs(AA*quadrilateral[1] + BB*quadrilateral[0] + CC) / sqrt(AA*AA + BB*BB);
	REALTYPE height_C_BD = abs(AA*quadrilateral[5] + BB*quadrilateral[4] + CC) / sqrt(AA*AA + BB*BB);
	REALTYPE area_triangleABD = height_A_BD * avgKMperDegree * segmentBD_length * 0.5;
	REALTYPE area_triangleCBD = height_C_BD * avgKMperDegree * segmentBD_length * 0.5;
	REALTYPE area = area_triangleABD + area_triangleCBD;
	return area;
}


/***
 Calculat the area of the MBR of a quadrilateral (pointA, pointB, pointC, pointD).
 Each point is represented as <lat, lng>
***/
REALTYPE calc_quadrilateral_mbr_area(std::vector<REALTYPE>& quadrilateral)
{
	REALTYPE minLat = std::min(std::min(std::min(quadrilateral[0], quadrilateral[2]), quadrilateral[4]), quadrilateral[6]);
	REALTYPE minLng = std::min(std::min(std::min(quadrilateral[1], quadrilateral[3]), quadrilateral[5]), quadrilateral[7]);
	REALTYPE maxLat = std::max(std::max(std::max(quadrilateral[0], quadrilateral[2]), quadrilateral[4]), quadrilateral[6]);
	REALTYPE maxLng = std::max(std::max(std::max(quadrilateral[1], quadrilateral[3]), quadrilateral[5]), quadrilateral[7]);
	REALTYPE dist_lat = EarthDistance(minLat, minLng, maxLat, minLng);
	REALTYPE dist_lng = EarthDistance(minLat, minLng, minLat, maxLng);
	REALTYPE area = dist_lat * dist_lng;
	return area;
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
	out_file.open(out_fname);
	if (!out_file.is_open()) {
		std::cout << "Couldn't open " << out_fname << std::endl;
		return -1;
	}
	
	
	std::vector<REALTYPE> quadrilateral(8);
	REALTYPE timestamp;
	REALTYPE azimuth;
	char comma;
	std::string line;
	int linenum = 0;
	REALTYPE avg_mbr_area=0.0, avg_quadrilateral_area=0.0, avg_area_diff=0.0;
	
	while(!data_file.eof())
	{
		std::getline(data_file, line);
		std::istringstream iss(line);
		iss >> quadrilateral[0] >> comma >> quadrilateral[1] >> comma 
		    >> quadrilateral[2] >> comma >> quadrilateral[3] >> comma
			>> quadrilateral[4] >> comma >> quadrilateral[5] >> comma
			>> quadrilateral[6] >> comma >> quadrilateral[7] >> comma
			>> azimuth >> comma
			>> timestamp;
		
		// Calculate the area of the quadrilateral.
		REALTYPE quadrilateral_area = calc_quadrilateral_area(quadrilateral);
		avg_quadrilateral_area += quadrilateral_area;
		
		// Calculate the area of the MBR of the quadrilateral.
		REALTYPE mbr_area = calc_quadrilateral_mbr_area(quadrilateral);
		avg_mbr_area += mbr_area;
		
		REALTYPE area_diff = mbr_area - quadrilateral_area;
		avg_area_diff += area_diff;
		
		out_file << quadrilateral_area << ','
		         << mbr_area << ','
				 << azimuth << ','
		         << area_diff/mbr_area*100.0 << '\n';
		linenum ++;
	}
	
	avg_quadrilateral_area /= linenum;
	avg_mbr_area /= linenum;
	avg_area_diff /= linenum;
	
	std::cout << "avg_quadrilateral_area: " << avg_quadrilateral_area <<'\n'
	          << "avg_mbr_area: " << avg_mbr_area << '\n'
			  << "avg_area_diff: " << avg_area_diff << '\n'
			  << "dead space ratio: " << avg_area_diff/avg_mbr_area*100.0 << '\n';
	return 0;
}
	
