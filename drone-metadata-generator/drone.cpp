
/**
 * @file Implements drone functions.
 *
 * @author Ying Lu <ylu720@usc.edu>
 */
 
#include <math.h>
#include <iostream>
#include <iomanip>
 
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



