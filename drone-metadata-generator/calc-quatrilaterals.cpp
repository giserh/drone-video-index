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
	if(argc > 1 and argv[1]==std::string("process-one-fov"))
	{
		/****** One FOV ******/
		lat = atof(argv[2]);
		lng = atof(argv[3]);
		hgt = atof(argv[4]); //camera height (distance from ground, kilometer)
		azimuth = atof(argv[5]); //w.r.t. the north, in degree.
		pitch = atof(argv[6]); //in degree
		roll = atof(argv[7]); //in degree
		viewable_angle = atof(argv[8]); //camera viewable angle (in degree)
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
	else if(argc > 1 and argv[1]==std::string("atlantik-drone-video-file"))
	{
		/****** Process a set of FOVs in a file (atlantik-drone-video-file) *******/
		std::string data_fname(argv[2]);
		ifstream data_file;
		data_file.open(data_fname);
	    if (!data_file.is_open()) {
	    	std::cout << "Couldn't open " << data_fname << std::endl;
		    return -1;
	    }

		std::string out_fname(argv[3]);
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
	else if(argc > 1 and argv[1]==std::string("nii-drone-video-file"))
	{
		/****** Process a set of FOVs in a file (nii-drone-video-file) *******/
		std::string data_fname(argv[2]);
		ifstream data_file;
		data_file.open(data_fname);
	    if (!data_file.is_open()) {
	    	std::cout << "Couldn't open " << data_fname << std::endl;
		    return -1;
	    }

		std::string out_fname(argv[3]);
		ofstream out_file;
		out_file.open(out_fname, std::ofstream::out | std::ofstream::app);
		if (!out_file.is_open()) {
	    	std::cout << "Couldn't open " << out_fname << std::endl;
		    return -1;
	    }

		viewable_angle = 40.0;
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
			pitch = pitch + 90.0;
			roll = 0.0;
			Drone drone(lat, lng, hgt, azimuth, pitch, roll, viewable_angle);
			drone.calc_coverage();
			//print quadrilateral
			out_file << std::fixed << std::setprecision(6)
					 << drone.quadrilateral[1] << "," << drone.quadrilateral[0] << ",0,"
					 << drone.quadrilateral[3] << "," << drone.quadrilateral[2] << ",0,"
					 << drone.quadrilateral[5] << "," << drone.quadrilateral[4] << ",0,"
					 << drone.quadrilateral[7] << "," << drone.quadrilateral[6] << ",0,"
					 << lng << "," << lat << "," << hgt*1000.0 << "\n";
		}
	}
	else if(argc > 1 and argv[1]==std::string("nii-generate-kml"))
	{
		/****** Process a set of FOVs in a file (nii-drone-video-file) and generate kml file*******/
		std::string data_fname(argv[2]);
		ifstream data_file;
		data_file.open(data_fname);
	    if (!data_file.is_open()) {
	    	std::cout << "Couldn't open " << data_fname << std::endl;
		    return -1;
	    }

		std::string out_fname(argv[3]);
		ofstream out_file;
		out_file.open(out_fname, std::ofstream::out | std::ofstream::app);
		if (!out_file.is_open()) {
	    	std::cout << "Couldn't open " << out_fname << std::endl;
		    return -1;
	    }

		viewable_angle = 40.0;
		std::string line;
		std::getline(data_file, line);
		char comma;
		time_t epochtime;

		/**
		 * print kml (head)
		 */
		out_file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \n"
		         << "<kml xmlns=\"http://www.opengis.net/kml/2.2\"> \n"
                 << " <Document> \n"
                 << "  <name>drone-video-play</name> \n"
                 << "  <Style id=\"transBluePoly\"> \n"
                 << "   <LineStyle> \n"
                 << "    <width>1.5</width> \n"
                 << "   </LineStyle> \n"
                 << "   <PolyStyle> \n"
                 << "    <color>7dff0000</color> \n"
                 << "   </PolyStyle> \n"
                 << "  </Style> \n";

		while(!data_file.eof())
		{
			std::getline(data_file, line);
			std::istringstream iss(line);
		    iss>>lat>>comma>>lng>>comma>>hgt>>comma>>azimuth>>comma>>pitch>>comma>>roll>>comma>>epochtime;
			hgt = hgt / 1000.0; //meter -> kilometer
			pitch = pitch + 90.0;
			roll = 0.0;
			Drone drone(lat, lng, hgt, azimuth, pitch, roll, viewable_angle);
			drone.calc_coverage();

			/**
			 * print kml body. One quadrilateral is corresponding to a placemark
			 */

			//for(int millisecond=0; millisecond<800; millisecond=millisecond+200)
			//{
				/*out_file << "  <Placemark> \n"
                     << "   <name>Absolute Extruded</name> \n"
                     << "   <visibility>1</visibility> \n"
                     << "   <styleUrl>#transBluePoly</styleUrl> \n";*/

				struct tm * timeinfo;
				timeinfo = localtime (&epochtime);
				char begin_time[100], end_time[100];
				std::strftime (begin_time, sizeof(begin_time),"%Y-%m-%dT%H:%M:%S",timeinfo);
				epochtime = epochtime +1;
				timeinfo = localtime (&(epochtime));
				std::strftime (end_time, sizeof(end_time),"%Y-%m-%dT%H:%M:%S",timeinfo);

				/*out_file << "   <TimeSpan> \n"
						 << "   <begin>" << begin_time << "</begin>\n"
						 //<< "   <begin>" << begin_time << "." << millisecond << "</begin>\n"
						 << "   <end>" << end_time << "</end>\n"
						 //<< "   <end>" << end_time << "." << millisecond+199 << "</end>\n"
						 << "   </TimeSpan>\n"
						 << "   <MultiGeometry>\n";*/
				out_file << epochtime << ": "; 
				for(int poly=0; poly<4; poly++)
				{
					/*out_file << "   <Polygon>\n"
							 << "    <extrude>0</extrude>\n"
							 << "    <tessellate>1</tessellate>\n"
							 << "    <altitudeMode>absolute</altitudeMode>\n"
							 << "    <outerBoundaryIs>\n"
							 << "     <LinearRing>\n"
							 << "      <coordinates>\n"
							 << std::fixed << std::setprecision(6)
							 << lng << "," << lat << "," << hgt*1000.0 << "\n"
							 << drone.quadrilateral[2*poly+1] << "," << drone.quadrilateral[2*poly] << ",0\n"         //(1, 0), (3, 2), (5, 4), (7, 6)
							 << drone.quadrilateral[(2*(poly+1)+1)%8] << "," << drone.quadrilateral[(2*(poly+1))%8] << ",0\n" //(3, 2), (5, 4), (7, 6), (9, 8)
							 << lng << "," << lat << "," << hgt*1000.0 << "\n"
							 << lng << "," << lat << "," << hgt*1000.0 << "\n"
							 << "      </coordinates>\n"
							 << "     </LinearRing>\n"
							 << "    </outerBoundaryIs>\n"
							 << "    </Polygon>\n";*/
							 out_file
		 							 << std::fixed << std::setprecision(6)
		 							 << lng << "," << lat << "," << hgt*1000.0 << "; "
		 							 << drone.quadrilateral[2*poly+1] << "," << drone.quadrilateral[2*poly] << ",0; "         //(1, 0), (3, 2), (5, 4), (7, 6)
		 							 << drone.quadrilateral[(2*(poly+1)+1)%8] << "," << drone.quadrilateral[(2*(poly+1))%8] << ",0 | "; //(3, 2), (5, 4), (7, 6), (9, 8)
				}
				out_file << "\n";
				/*out_file << "   </MultiGeometry>\n"
						 << "   </Placemark>\n";*/
			}

		//}
		/**
		 * print kml (tail)
		 */
		/*out_file << " </Document>\n"
		         << " </kml>\n";*/
	}

	return 0;
}
