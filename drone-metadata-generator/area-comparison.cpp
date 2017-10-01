#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>
#include <cmath>

#include "drone.h"

using namespace std;




int main(int argc, char* argv[])
{
	/***
	 * Calculate the deadspace ratio for aerial-FOVs in the data file.
	 */
	/*std::string data_fname(argv[1]);
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
			  << "dead space ratio: " << avg_area_diff/avg_mbr_area*100.0 << '\n';*/


	/***
	 * Analyze the deadspace ratio affected by azimuth, pitch and roll angles.
	 */
	std::string out_fname(argv[1]);
	ofstream out_file;
	out_file.open(out_fname);
	if (!out_file.is_open()) {
		std::cout << "Couldn't open " << out_fname << std::endl;
		return -1;
	}

	/* Tommy Trajon */
	//REALTYPE lat = 34.020578; //34.020551;
	//REALTYPE lng = -118.285416; //-118.290452;
	
	/* LA Memorial Coliseum */
	//REALTYPE lat = 34.014012; 
	//REALTYPE lng = -118.287927; 
	
	REALTYPE lat = 34.020112; 
	REALTYPE lng = -118.286535; 
	
	REALTYPE hgt = 0.2; //camera height (distance from ground, kilometer)
	REALTYPE azimuth = 0.0; //w.r.t. the north, in degree.
	REALTYPE pitch = 0.0; //in degree
	REALTYPE roll = 0.0; //in degree
	REALTYPE viewable_angle = 100.0; //camera viewable angle (in degree)


	for(azimuth = 0.0; azimuth<360; azimuth = azimuth+30)
		for(roll = 0.0; roll<30.0; roll = roll+17)
			for(pitch=0.0; pitch<30.0; pitch = pitch+5)
	{
		Drone drone(lat, lng, hgt, azimuth, pitch, roll, viewable_angle);
		drone.calc_coverage();

		// Calculate the area of the quadrilateral.
		//REALTYPE quadrilateral_area = drone.calc_quadrilateral_area();

		// Calculate the area of the MBR of the quadrilateral.
		/*std::vector<REALTYPE> MBR(4);
		REALTYPE mbr_area = drone.calc_quadrilateral_mbr_area(MBR);
		REALTYPE area_diff = mbr_area - quadrilateral_area;*/

		// Print information.
		char seprator = ',';
		/*out_file << azimuth << seprator << roll << seprator << pitch << seprator
		         << quadrilateral_area << seprator
		         << mbr_area << seprator
				 << azimuth << seprator
		         << area_diff/mbr_area*100.0 << seprator;*/
		out_file << azimuth << seprator << roll << seprator << pitch << seprator;
		
		for(size_t qi=0; qi+1<drone.quadrilateral.size(); qi = qi + 2)
		{
			/*out_file << std::fixed << std::setprecision(6)
		             << "{lat: " << drone.quadrilateral[qi] << ", lng: " << drone.quadrilateral[qi+1] << "}, ";*/
			 out_file << std::fixed << std::setprecision(6)
 		             << drone.quadrilateral[qi+1] << "," << drone.quadrilateral[qi] << ", ";
		}

        //print middle point.
		out_file << std::fixed << std::setprecision(6)
		         << lng << "," << lat << ",";

	    /*for(auto r : MBR)
			out_file << std::fixed << std::setprecision(6) << r << seprator;*/

		out_file << '\n';
	}

	out_file.close();
	return 0;
}
