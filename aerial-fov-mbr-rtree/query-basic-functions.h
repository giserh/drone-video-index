
#include "rtree.h"
#include "aerialfov.h"
#include "basics.h"
#include <stdio.h>

using namespace std;



/***
 * Read an fov object from a (binary) data file.
 *
 * @para[in] objectid: the id (in object_file) of the fov object
 * @para[in] fov: an aerial-FOV.
 *
 */
void read_aerial_fov_from_file(int objectid, AerialFOV& fov);


/***
 * Report all the objects in the subtree of an index node.
 *
 * @para[in] node: an R-tree node
 *
 */
void total_hit_report(RTREENODE* node);




/***
 * Calculate the surface distance between two points A and B.
 *
 * @para[in] point A(Alat, Alng) and point B(Blat, Blng)
 * 
 * Return the distance between A and B (In km unit).
 */
REALTYPE earth_surface_distance(REALTYPE Alat, REALTYPE Alng, 
                                REALTYPE Blat, REALTYPE Blng);




/***
 * Calculate the Euclidean distance between a point to an MBR.
 *
 * @para[in] mbrp: an MBR in form of (minLng,minLat,maxLng,maxLat)
 * @para[in] (qlat, qlng): the latitude and longitude of a point.
 * 
 * Return the Euclidean distance between them (in km unit). 
 */
REALTYPE calc_min_dist(RTREEMBR* mbrp, REALTYPE qlat, REALTYPE qlng);




/***
 * Check whether a circle is contained inside of an MBR or not.
 *
 * @para[in] mbrp: an MBR in form of (minLng,minLat,maxLng,maxLat)
 * @para[in] (qlat, qlng, qradius): the center (latitude and longitude) and radius of a circle.
 * 
 * Return true if contain; false otherwise. 
 */
bool mbr_contain_circle(RTREEMBR* mbrp, REALTYPE qlat, REALTYPE qlng, REALTYPE qradius);




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
int circle_overlap_mbr(RTREEMBR* mbr, REALTYPE qlat, REALTYPE qlng, REALTYPE qradius);



/***
 * Check whether a circle overlaps with a quadrilateral or not.
 *
 * @para[in] objectid: the id (in object_file) of an aerial-FOV (a quadrilateral with orientation)
 * @para[in] (qlat, qlng, qradius): the center (latitude and longitude) and radius of a circle.
 * 
 * Return true if overlap; false otherwise. 
 */
bool circle_overlap_quadrilateral(int objectid, REALTYPE qlat, REALTYPE qlng, REALTYPE qradius);





/***
 * Check whether a point inside of an MBR or not.
 *
 * @para[in] mbr: an MBR in form of (minLng,minLat,maxLng,maxLat)
 * @para[in] (qlat, qlng): the point (latitude and longitude).
 * 
 * Return true if inside; false otherwise. 
 */
bool point_in_mbr(RTREEMBR* mbr, REALTYPE qlat, REALTYPE qlng);



/***
 * Check whether a point inside of a quadrilateral or not.
 *
 * @para[in] fov: the aerial-FOV (a quadrilateral with orientation)
 * @para[in] (qlat, qlng): the point (latitude and longitude).
 * 
 * Return true if inside; false otherwise. 
 */
bool point_in_quadrilateral(AerialFOV& fov, REALTYPE qlat, REALTYPE qlng);







