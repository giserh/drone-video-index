
/**
 * @file Implements drone functions.
 *
 * @author Ying Lu <ylu720@usc.edu>
 */
 
#include <math.h>
#include <iostream>
#include <iomanip>
#include <cmath> 

#include "drone.h"



using namespace std;



void Drone::matrix_prod(std::vector<REALTYPE>& X,
                 std::vector<std::vector<REALTYPE>> rotation_matrix)
{
	std::vector<REALTYPE> copyX = X;
	int dim = 0;
	//std::cout<<"matrix: "<<std::endl;
	for(auto row : rotation_matrix)
	{
		REALTYPE sum = 0;
		for(size_t i=0; i<row.size() && i<copyX.size(); i++)
		{
			sum += copyX[i] * row[i];
			//std::cout<<row[i]<<" ";
		}
		//std::cout<<"\n";
		X[dim] = sum;
		dim++;
	}
}


void Drone::calc_rotatedPoint(std::vector<REALTYPE>& X, 
	                   REALTYPE alpha, REALTYPE beta, REALTYPE gama)
{
	//Convert from degree to radian.
	alpha = alpha;
	beta  = beta;
	gama  = gama;
	
	std::vector<std::vector<REALTYPE>> rotation_matrix(3, std::vector<REALTYPE>(3));
	rotation_matrix[0][0] = cos(alpha)*cos(beta);
	rotation_matrix[0][1] = cos(alpha)*sin(beta)*sin(gama) -
	                        sin(alpha)*cos(gama);
	rotation_matrix[0][2] = cos(alpha)*sin(beta)*cos(gama) +
	                        sin(alpha)*sin(gama);
							
	rotation_matrix[1][0] = sin(alpha)*cos(beta);
	rotation_matrix[1][1] = sin(alpha)*sin(beta)*sin(gama) + 
	                        cos(alpha)*cos(gama);
	rotation_matrix[1][2] = sin(alpha)*sin(beta)*cos(gama) -
	                        cos(alpha)*sin(gama);
							
	rotation_matrix[2][0] = 0 - sin(beta);
	rotation_matrix[2][1] = cos(beta)*sin(gama);
	rotation_matrix[2][2] = cos(beta)*cos(gama);
	
	matrix_prod(X, rotation_matrix);
}






void Drone::Calc_projected_points(std::vector<REALTYPE>& X)
{
	for(int i=0; i<2; i++)
	{
		X[i] = X[i] * (0-this->hgt)/X[2];
	}
}


void Drone::Calc_actual_points(std::vector<REALTYPE> X)
{
	X[0] = X[0] / KMPERDEGREE + this->lat;
	X[1] = X[1] / KMPERDEGREE + this->lng;
	this->quadrilateral.push_back(X[0]);
	this->quadrilateral.push_back(X[1]);
}



void Drone::calc_coverage()
{
	REALTYPE alpha = ((int)this->azimuth_angle + 360)%360 * PI / 180.0;
	REALTYPE beta = ((int)this->pitch_angle + 360)%360 * PI / 180.0;
	REALTYPE gama = ((int)this->roll_angle + 360)%360 * PI / 180.0;
	
	//Calculate rotated (relative) pointA
	std::vector<REALTYPE> XA(3);
	XA[0] = this->hgt * tan(this->view_angle/2.0 * PI / 180.0);
	XA[1] = 0 - this->hgt * tan(this->view_angle/2.0 * PI / 180.0);
	XA[2] = 0 - this->hgt;
	/*for(auto x : XA) 
		std::cout << std::fixed << std::setprecision(6) << x << " ";
	std::cout << '\n';*/
	calc_rotatedPoint(XA, alpha, beta, gama);
	
	
	//Calculate rotated (relative) pointB
	std::vector<REALTYPE> XB(3);
	XB[0] = this->hgt * tan(this->view_angle/2.0 * PI / 180.0);
	XB[1] = this->hgt * tan(this->view_angle/2.0 * PI / 180.0);
	XB[2] = 0 - this->hgt;
	/*for(auto x : XB) 
		std::cout << std::fixed << std::setprecision(6) << x << " ";
	std::cout << '\n';*/
	calc_rotatedPoint(XB, alpha, beta, gama);
	
	
	//Calculate rotated (relative) pointC
	std::vector<REALTYPE> XC(3);
	XC[0] = 0 - this->hgt * tan(this->view_angle/2.0 * PI / 180.0);
	XC[1] = this->hgt * tan(this->view_angle/2.0 * PI / 180.0);
	XC[2] = 0 - this->hgt;
	/*for(auto x : XC) 
		std::cout << std::fixed << std::setprecision(6) << x << " ";
	std::cout << '\n';*/
	calc_rotatedPoint(XC, alpha, beta, gama);
	
	
	//Calculate rotated (relative) pointD
	std::vector<REALTYPE> XD(3);
	XD[0] = 0 - this->hgt * tan(this->view_angle/2.0 * PI / 180.0);
	XD[1] = 0 - this->hgt * tan(this->view_angle/2.0 * PI / 180.0);
	XD[2] = 0 - this->hgt;
	/*for(auto x : XD) 
		std::cout << std::fixed << std::setprecision(6) << x << " ";
	std::cout << '\n';*/
	calc_rotatedPoint(XD, alpha, beta, gama);
	
	
	/*std::cout << "\n\n";
	for(auto x : XA) 
		std::cout << std::fixed << std::setprecision(6) << x << " ";
	std::cout << "\n";
	for(auto x : XB) 
		std::cout << std::fixed << std::setprecision(6) << x << " ";
	std::cout << "\n";
	for(auto x : XC) 
		std::cout << std::fixed << std::setprecision(6) << x << " ";
	std::cout << "\n";
	for(auto x : XD) 
		std::cout << std::fixed << std::setprecision(6) << x << " ";
	std::cout << "\n\n";*/
	
	//Calculate the projected (relative) points on the ground
	Calc_projected_points(XA);
	Calc_projected_points(XB);
	Calc_projected_points(XC);
	Calc_projected_points(XD);
	
	/*std::cout << "\n\n";
	for(auto x : XA) 
		std::cout << std::fixed << std::setprecision(6) << x << " ";
	std::cout << "\n";
	for(auto x : XB) 
		std::cout << std::fixed << std::setprecision(6) << x << " ";
	std::cout << "\n";
	for(auto x : XC) 
		std::cout << std::fixed << std::setprecision(6) << x << " ";
	std::cout << "\n";
	for(auto x : XD) 
		std::cout << std::fixed << std::setprecision(6) << x << " ";
	std::cout << "\n\n";*/
	
	//Calculate the actual (latitude, longitude) points (i.e., four corners of the quadrilateral)
	Calc_actual_points(XA);
	Calc_actual_points(XB);
	Calc_actual_points(XC);
	Calc_actual_points(XD);
}










/***
 * Calculate the surface distance between two points A and B.
 * @para[in] point A(Alat, Alng) and point B(Blat, Blng)
 * @para[out] the distance between A and B (In km unit).
 */
REALTYPE Drone::EarthDistance(REALTYPE Alat, REALTYPE Alng, REALTYPE Blat, REALTYPE Blng) 
{
	REALTYPE distpq = 6371 * 2 * asin(sqrt(pow(sin((Alat - Blat)*PI / 180 / 2), 2) 
	                                    + cos(Alat*PI / 180) 
										  * cos(Blat*PI / 180) 
										  * pow(sin((Alng - Blng) * PI / 180 / 2), 2)
										));
	return distpq; //kilometers
}



/*** 
 * Calculat the area of a quadrilateral (pointA, pointB, pointC, pointD).
 * Each point is represented as <lat, lng>
 */
REALTYPE Drone::calc_quadrilateral_area()
{
	// Calculate the distance from pointB to pointD
	REALTYPE segmentBD_length = EarthDistance(this->quadrilateral[2], this->quadrilateral[3], 
	                                          this->quadrilateral[6], this->quadrilateral[7]);
	
	/***
	 * resources: https://knowledge.safe.com/articles/725/calculating-accurate-length-in-meters-for-lat-long.html
	 * resources: http://physics.wooster.edu/Manz/sandcollection/swaplist/Latitude%20and%20Longitude.pdf
	 ***/
	REALTYPE lat = this->quadrilateral[2];
	REALTYPE KMperDegreeLat = 111.13292 - 0.55982 * cos(2* lat/180*PI) + 0.001175*cos(4*lat/180*PI);
	REALTYPE KMperDegreeLng = 111.41284 * cos(lat/180*PI) - 0.0935 * cos(3*lat/180*PI);
	REALTYPE avgKMperDegree = (KMperDegreeLat + KMperDegreeLng)/2.0;
	//std::cout<<KMperDegreeLat <<", " << KMperDegreeLng << ": " << avgKMperDegree << '\n';
	
	// Calculate the height of the triangle ABD, i.e., the distance from pointA to lineBD
	REALTYPE AA = this->quadrilateral[2] - this->quadrilateral[6]; //YB - YD
	REALTYPE BB = this->quadrilateral[7] - this->quadrilateral[3]; //XD - XB
	REALTYPE CC = this->quadrilateral[3]*this->quadrilateral[6] - this->quadrilateral[7]*this->quadrilateral[2]; //XBYD - XDYB
	REALTYPE height_A_BD = abs(AA*this->quadrilateral[1] + BB*this->quadrilateral[0] + CC) / sqrt(AA*AA + BB*BB);
	REALTYPE height_C_BD = abs(AA*this->quadrilateral[5] + BB*this->quadrilateral[4] + CC) / sqrt(AA*AA + BB*BB);
	REALTYPE area_triangleABD = height_A_BD * avgKMperDegree * segmentBD_length * 0.5;
	REALTYPE area_triangleCBD = height_C_BD * avgKMperDegree * segmentBD_length * 0.5;
	REALTYPE area = area_triangleABD + area_triangleCBD;
	return area;
}


/***
 * Calculat the area of the MBR of a quadrilateral (pointA, pointB, pointC, pointD).
 * Each point is represented as <lat, lng>
 */
REALTYPE Drone::calc_quadrilateral_mbr_area(std::vector<REALTYPE>& mbr)
{
	REALTYPE minLat = std::min(std::min(std::min(this->quadrilateral[0], this->quadrilateral[2]), this->quadrilateral[4]), this->quadrilateral[6]);
	REALTYPE minLng = std::min(std::min(std::min(this->quadrilateral[1], this->quadrilateral[3]), this->quadrilateral[5]), this->quadrilateral[7]);
	REALTYPE maxLat = std::max(std::max(std::max(this->quadrilateral[0], this->quadrilateral[2]), this->quadrilateral[4]), this->quadrilateral[6]);
	REALTYPE maxLng = std::max(std::max(std::max(this->quadrilateral[1], this->quadrilateral[3]), this->quadrilateral[5]), this->quadrilateral[7]);
	REALTYPE dist_lat = EarthDistance(minLat, minLng, maxLat, minLng);
	REALTYPE dist_lng = EarthDistance(minLat, minLng, minLat, maxLng);
	REALTYPE area = dist_lat * dist_lng;
	
	if(mbr.size()>=4)
	{
		// [minx, maxx, miny, maxy], i.e., [minLng, maxLng, maxLat, maxLat]
		mbr[0] = minLng;
		mbr[1] = maxLng;
		mbr[2] = minLat;
		mbr[3] = maxLat;
	}
	
	return area;
}



/*
REALTYPE lat; //camera latitude
REALTYPE lng; //camera longitude
REALTYPE hgt; //camera height (distance from ground, kilometer)
REALTYPE azimuth_angle; //w.r.t. the north, in degree.
REALTYPE pitch_angle; //in degree
REALTYPE roll_angle; //in degree
REALTYPE view_angle; //camera viewable angle (in degree)
*/

/*** 
 * Check whether a point inside of a pyramid (a 3d-aerial-FOV object).
 *
 * @param[in] the latitude of the point.
 * @param[in] the longitude of the point.
 *
 * @return true if inside; false otherwise. 
 */
bool Drone::point_in_pyramid(REALTYPE plat, REALTYPE plng)
{
	REALTYPE vecx_oq = plng - this->lng;
	REALTYPE vecy_oq = plat - this->lat;
	REALTYPE vecz_oq = (0 - this->hgt) - 0;
	
	return true;
}













