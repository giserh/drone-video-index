
#ifndef __DRONE_H__
#define __DRONE_H__

#include<vector>
//#include <boost_1_54_0/boost/numeric/ublas/matrix.hpp>
//#include <boost_1_54_0/boost/numeric/ublas/io.hpp>

#define PI 3.1415926
#define KMPERDEGREE 111.3200

using namespace std;

typedef float REALTYPE;

class Drone{
public:
	REALTYPE lat; //camera latitude
	REALTYPE lng; //camera longitude
	REALTYPE hgt; //camera height (distance from ground, kilometer)
	REALTYPE azimuth_angle; //w.r.t. the north, in degree.
	REALTYPE pitch_angle; //in degree
	REALTYPE roll_angle; //in degree
	REALTYPE view_angle; //camera viewable angle (in degree)
	
	/**
	  Spatial coverage of the drone, a quadrilateral.
	  <Ay, Ax, By, Bx, Cy, Cx, Dy, Dx>, i.e., 
	  <upperleft_lat, upperleft_lng, upperright_lat, upperright_lng,
	   lowerright_lat, lowerright_lng, lowerleft_lat, lowerleft_lng>
	  //Initialize with zeros.
     */	
	std::vector<REALTYPE> quadrilateral; 
	
	
	
	
public:
    //Constructors
    
	Drone()
	{
		this->lat = 0.0;
		this->lng = 0.0;
		this->hgt = 0.0;
		this->azimuth_angle = 0.0;
		this->pitch_angle = 0.0;
		this->roll_angle = 0.0;
		this->view_angle = 0.0;
	}


	Drone(REALTYPE latitude, REALTYPE longitude, REALTYPE height,
		  REALTYPE azimuth, REALTYPE pitch, REALTYPE roll, REALTYPE viewable_angle)
	{
		this->lat = latitude;
		this->lng = longitude;
		this->hgt = height;
		this->azimuth_angle = azimuth;
		this->pitch_angle = pitch;
		this->roll_angle = roll;
		this->view_angle = viewable_angle;
	}
		  
	// TODO: DeConstructor
	
    void calc_coverage();
	
	
	/***
     * Calculate the surface distance between two points A and B.
     * @para[in] point A(Alat, Alng) and point B(Blat, Blng)
     * @para[out] the distance between A and B (In km unit).
     */
    REALTYPE EarthDistance(REALTYPE Alat, REALTYPE Alng, REALTYPE Blat, REALTYPE Blng);

    /*** 
     * Calculat the area of a quadrilateral (pointA, pointB, pointC, pointD).
     * Each point is represented as <lat, lng>
     */
    REALTYPE calc_quadrilateral_area();
	
	/***
     * Calculat the area of the MBR of a quadrilateral (pointA, pointB, pointC, pointD).
     * Each point is represented as <lat, lng>
     */
	REALTYPE calc_quadrilateral_mbr_area(std::vector<REALTYPE>& mbr);
	
	
	
private:
    /**
	 * Calculate the new rotated point.
     * @param X[in, out] point(x, y, z, 1) 
     * @param alpha[in] rotation angle around x-axis, i.e., pitch
     * @param beta[in] rotation angle around y-axis, i.e., roll
     * @param gama[in] rotation angle around z-axis, i.e., azimuth 
	 * Referece: http://inside.mines.edu/fs_home/gmurray/ArbitraryAxisRotation/
	 */
    void calc_rotatedPoint(std::vector<REALTYPE>& X, 
	                       REALTYPE alpha, REALTYPE beta, REALTYPE gama);
						   
						   
	/**
	 * Calculate the product of two matrixes.
	 */
	void matrix_prod(std::vector<REALTYPE>& X,
                     std::vector<std::vector<REALTYPE>> rotation_matrix);
					 
	/**
	 * Calculate the projected point for a rotated point
	 * @param X[in, out] point(x, y, z, 1)
	 */
	void Calc_projected_points(std::vector<REALTYPE>& X);
	
	/**
	 * Calculate the actual (latitude, longitude) points from the relative point / position
	 * Assign the four corners of the quadrilateral.
	 * @param X[in] point(x, y, z, 1)
	 * 
	 **/
	void Calc_actual_points(std::vector<REALTYPE> X);
};



#endif // __DRONE_H__