
#ifndef __AERIALFOV_H__
#define __AERIALFOV_H__

#include "basics.h"

#define QUADRILATERAL_DIMS 8

using namespace std;

/***
 Here an aerial-FOV (drone-FOV) is represented as a quadrilateral 
 with a viewing orientation.
 **/
 
class AerialFOV{
public:
	/**
	 * Spatial coverage of the drone, a quadrilateral.
	 * <Ay, Ax, By, Bx, Cy, Cx, Dy, Dx>, i.e., 
	 * <upperleft_lat, upperleft_lng, upperright_lat, upperright_lng,
	 * lowerright_lat, lowerright_lng, lowerleft_lat, lowerleft_lng>
     */	
	REALTYPE quadrilateral[QUADRILATERAL_DIMS]; 
	
	REALTYPE azimuth; // Viewing orientation [-180, 180] (in degree)
	REALTYPE timestamp;
	
	
	
public:
    //Constructors
    
	AerialFOV()
	{
		for(int i=0; i<8; i++)
		{
			this->quadrilateral[i] = 0.0; //Initialize with zeros.
		}
		this->azimuth = 0.0;
		this->timestamp = 0.0;
	}

	
    //Destructor
	~AerialFOV()
	{}
	
	
	/** 
	 * Check whether a point inside of a quadrilateral (polygon).
	 * Apply the ray casting algorithm, reference:
	 * [M. Shimrat. Algorithm 112: Position of point relative to polygon. Commun. ACM 1962]
	 *
	 * @param[in] the latitude of the point.
	 * @param[in] the longitude of the point.
	 *
	 * @return true if inside; false otherwise. 
	 */
	bool pointInPolygon(REALTYPE plat, REALTYPE plng);
	
	
	/** 
	 * Check whether a point intersects a segment (i.e., a side of polygon) or not.
	 *
	 * @param[in] (px, py): the X-axis (longitude) and Y-axis (latitude) of the point.
	 * @param[in] (Ax, Ay, Bx, By): the segment AB. 
	 *
	 * @return true if inside; false otherwise. 
	 */
	bool ray_intersects_segment(REALTYPE px, REALTYPE py, 
		                        REALTYPE Ax, REALTYPE Ay, 
							    REALTYPE Bx, REALTYPE By);
};




#endif // __AERIALFOV_H__