
#include "query-basic-functions.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stack>
#include <math.h>
#include <cmath> 
#include <time.h>
#include<limits>

using namespace std;

//Global variants
extern FILE* index_file;
extern FILE* object_file;
extern int index_node_access_num;
extern int result_fov_num;
extern double index_node_access_time;
extern double fov_object_lookup_time;



/***
 * Read an fov object from a (binary) data file.
 *
 * @para[in] objectid: the id (in object_file) of the fov object
 * @para[in] fov: an aerial-FOV.
 *
 */
void read_aerial_fov_from_file(int objectid, AerialFOV& fov)
{
	clock_t tStart = clock();
	
	fseek(object_file, (objectid-1)*sizeof(AerialFOV), 0);
	fread(&fov, sizeof(AerialFOV), 1, object_file);
	
	fov_object_lookup_time += (double)(clock() - tStart)/CLOCKS_PER_SEC;
}




/***
 * Report all the objects in the subtree of an index node.
 *
 * @para[in] node: an R-tree node
 *
 */
void total_hit_report(RTREENODE* node)
{
	RTREENODE childnode;
	if(node->level>0) //non-leaf node
	{
		for(int i=0; i<node->count; i++)
		{
			clock_t tStart = clock();
			fseek(index_file, (node->branch[i].childid-1)*sizeof(RTREENODE), 0);
			fread(&childnode, sizeof(RTREENODE), 1, index_file);
			index_node_access_num++;
			index_node_access_time += (double)(clock() - tStart)/CLOCKS_PER_SEC;
			total_hit_report(&childnode);
		}
	}
	else //leaf node
	{
		for(int i=0; i<node->count; i++)
		{
			result_fov_num++;
		}
	}
}




/***
 * Calculate the surface distance between two points A and B.
 *
 * @para[in] point A(Alat, Alng) and point B(Blat, Blng)
 * 
 * Return the distance between A and B (In km unit).
 */
REALTYPE earth_surface_distance(REALTYPE Alat, REALTYPE Alng, 
                                REALTYPE Blat, REALTYPE Blng) 
{
	REALTYPE distpq = EARTH_RADIUS * 2 
					  * asin(sqrt(pow(sin((Alat - Blat)*PI / 180 / 2), 2) 
	                               + cos(Alat*PI / 180) 
									  * cos(Blat*PI / 180) 
									  * pow(sin((Alng - Blng) * PI / 180 / 2), 2)
							      )
						    );
	return distpq; //kilometers
}




/***
 * Calculate the Euclidean distance between a point to an MBR.
 *
 * @para[in] mbrp: an MBR in form of (minLng,minLat,maxLng,maxLat)
 * @para[in] (qlat, qlng): the latitude and longitude of a point.
 * 
 * Return the Euclidean distance between them (in km unit). 
 */
REALTYPE calc_min_dist(RTREEMBR* mbrp, REALTYPE qlat, REALTYPE qlng)
{
	if(qlat < mbrp->bound[3] 
	   && qlat > mbrp->bound[1] 
	   && qlng < mbrp->bound[2] 
	   && qlng > mbrp->bound[0]) return 0;
	   
	REALTYPE plat, plng;
	if(qlat > (mbrp->bound[1]+mbrp->bound[3])/2) plat=mbrp->bound[3];
	else plat = mbrp->bound[1];
	if(qlng > (mbrp->bound[0]+mbrp->bound[2])/2) plng=mbrp->bound[2];
	else plng = mbrp->bound[0];
	REALTYPE distpq = earth_surface_distance(plat, plng, qlat, qlng);
	return distpq;
}




/***
 * Check whether a circle is contained inside of an MBR or not.
 *
 * @para[in] mbrp: an MBR in form of (minLng,minLat,maxLng,maxLat)
 * @para[in] (qlat, qlng, qradius): the center (latitude and longitude) and radius of a circle.
 * 
 * Return true if contain; false otherwise. 
 */
bool mbr_contain_circle(RTREEMBR* mbrp, REALTYPE qlat, REALTYPE qlng, REALTYPE qradius)
{
	if(earth_surface_distance(qlat, mbrp->bound[0], qlat, qlng) < qradius && 
	   earth_surface_distance(qlat, mbrp->bound[2], qlat, qlng) < qradius &&
	   earth_surface_distance(mbrp->bound[1], qlng, qlat, qlng) < qradius &&
	   earth_surface_distance(mbrp->bound[3], qlng, qlat, qlng) < qradius) return true;
	else return false;
}




/***
 * Check whether a circle overlaps with an MBR or not.
 *
 * @para[in] mbr: an MBR in form of (minLng,minLat,maxLng,maxLat)
 * @para[in] (qlat, qlng, qradius): the center (latitude and longitude) and radius of a circle.
 * 
 * Return 1 if partially overlap; 
 *        2 if totally overlap (the circle is contained in the MBR);
 *        3 if disjoint (no overlap).
 */
int circle_overlap_mbr(RTREEMBR* mbr, REALTYPE qlat, REALTYPE qlng, REALTYPE qradius)
{
	//------check whether can be reported as results.
	if(mbr_contain_circle(mbr, qlat, qlng, qradius)) return 2; //total overlap
	
	//-----check whether can be pruned
	REALTYPE mindist = calc_min_dist(mbr, qlat, qlng);
	if(mindist > qradius) return 3; //disjoint

	return 1; //partially overlap
}



/***
 * Check whether a circle overlaps with a quadrilateral or not.
 *
 * @para[in] objectid: the id (in object_file) of an aerial-FOV (a quadrilateral with orientation)
 * @para[in] (qlat, qlng, qradius): the center (latitude and longitude) and radius of a circle.
 * 
 * Return true if overlap; false otherwise. 
 */
bool circle_overlap_quadrilateral(int objectid, REALTYPE qlat, REALTYPE qlng, REALTYPE qradius)
{
	return false;
}





/***
 * Check whether a point inside of an MBR or not.
 *
 * @para[in] mbr: an MBR in form of (minLng,minLat,maxLng,maxLat)
 * @para[in] (qlat, qlng): the point (latitude and longitude).
 * 
 * Return true if inside; false otherwise. 
 */
bool point_in_mbr(RTREEMBR* mbr, REALTYPE qlat, REALTYPE qlng)
{
	if (!mbr) 
	{
		printf("Error in function point_in_mbr: mbr is null. "); 
		return false;
	}
	if(qlng >= mbr->bound[0] &&
	   qlng <= mbr->bound[2] &&
	   qlat >= mbr->bound[1] &&
	   qlat <= mbr->bound[3])
	{
		return true;
	}
	return false;
}



/***
 * Check whether a point inside of a quadrilateral or not.
 *
 * @para[in] fov: the aerial-FOV (a quadrilateral with orientation)
 * @para[in] (qlat, qlng): the point (latitude and longitude).
 * 
 * Return true if inside; false otherwise. 
 */
bool point_in_quadrilateral(AerialFOV& fov, REALTYPE qlat, REALTYPE qlng)
{	
	/*if(objectid==576226)
	{
		for(auto p : fov.quadrilateral) std::cout << p << "\t";
		std::cout << "\n";
	}*/
		
	if(fov.pointInPolygon(qlat, qlng)) //point query
	{
		/*if(objectid==576226)
		{
			std::cout << "after pointInPolygon, true " << "\n";
		}*/
		return true;
	}
	/*if(objectid==576226)
	{
		std::cout << "after pointInPolygon, false " << "\n";
	}*/
	return false;
}







