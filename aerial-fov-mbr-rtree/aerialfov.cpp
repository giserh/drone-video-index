// aerialfov.cpp : implementation file


#include "aerialfov.h"
#include "basics.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <assert.h>
#include <cmath>
#include <limits>
#include <iomanip>


bool AerialFOV::ray_intersects_segment(REALTYPE px, REALTYPE py, 
		                               REALTYPE Ax, REALTYPE Ay, 
							           REALTYPE Bx, REALTYPE By)
{
	if (Ay > By) //then change from Edge(AB) to Edge(BA)
	{
		return ray_intersects_segment(px, py, Bx, By, Ax, Ay); 
	}
	    
	if (fabs(py-Ay) <= DOUBLE_MIN || fabs(py-By) <= DOUBLE_MIN) //then change checking point.
	{
		return ray_intersects_segment(px, py + EPSILON, Ax, Ay, Bx, By); 
	}

	if (py > By || py < Ay || px > max(Ax, Bx)) return false;
	
	if (px < min(Ax, Bx)) return true;
	auto blue = abs(Ax - px) > DOUBLE_MIN ? (py - Ay) / (px - Ax) : DOUBLE_MAX;
	auto red = abs(Ax - Bx) > DOUBLE_MIN ? (By - Ay) / (Bx - Ax) : DOUBLE_MAX;
	return blue >= red;
}


bool AerialFOV::pointInPolygon(REALTYPE plat, REALTYPE plng)
{
	REALTYPE sideAlat, sideAlng, sideBlat, sideBlng;
	int count = 0;
	for(int i=0; i<=6; i=i+2)
	{
		sideAlat = this->quadrilateral[i%8];
		sideAlng = this->quadrilateral[(i+1)%8];
		sideBlat = this->quadrilateral[(i+2)%8];
		sideBlng = this->quadrilateral[(i+3)%8];
				
		if(ray_intersects_segment(plng, plat, 
		                          sideAlng, sideAlat, 
								  sideBlng, sideBlat))
			count++;
	}
	if(count%2 !=0) return true; //count is an odd
	else return false; //cout is an even
}