
#ifndef __AERIALFOV_H__
#define __AERIALFOV_H__

#include "basics.h"
#include "polygon.h"

//#define QUADRILATERAL_DIMS 8

using namespace std;

/***
 Here an aerial-FOV (drone-FOV) is represented as a quadrilateral 
 with a viewing orientation.
 **/
 
class AerialFOV : public Polygon{
public:
	/**
	 * Spatial coverage of the drone, a quadrilateral.
	 * <Ay, Ax, By, Bx, Cy, Cx, Dy, Dx>, i.e., 
	 * <upperleft_lat, upperleft_lng, upperright_lat, upperright_lng,
	 * lowerright_lat, lowerright_lng, lowerleft_lat, lowerleft_lng>
     */	
	//REALTYPE quadrilateral[QUADRILATERAL_DIMS]; 
	
	REALTYPE azimuth; // Viewing orientation [-180, 180] (in degree)
	REALTYPE timestamp;
	
	
	
public:
    //Constructors
    
	AerialFOV()
	{
		/*for(int i=0; i<QUADRILATERAL_DIMS; i++)
		{
			this->quadrilateral[i] = 0.0; //Initialize with zeros.
		}*/
		
		this->azimuth = 0.0;
		this->timestamp = 0.0;
	}
	
	AerialFOV(int size)
	{
		this->azimuth = 0.0;
		this->timestamp = 0.0;
	}
	

	AerialFOV(std::vector<Point>& points, REALTYPE orientation, REALTYPE time) 
	: Polygon(points)
	{
		this->azimuth = orientation;
		this->timestamp = time;
	}
	
    //Destructor
	~AerialFOV()
	{}
	
	
};




#endif // __AERIALFOV_H__