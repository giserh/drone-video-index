
#include "tetrartree.h"
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
void total_hit_report(TETRARTREENODE* node);




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
REALTYPE calc_min_dist(MBR* mbrp, REALTYPE qlat, REALTYPE qlng);




/***
 * Check whether a circle is contained inside of an MBR or not.
 *
 * @para[in] mbrp: an MBR in form of (minLng,minLat,maxLng,maxLat)
 * @para[in] (qlat, qlng, qradius): the center (latitude and longitude) and radius of a circle.
 * 
 * Return true if contain; false otherwise. 
 */
bool mbr_contain_circle(MBR* mbrp, REALTYPE qlat, REALTYPE qlng, REALTYPE qradius);




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
int circle_overlap_mbr(MBR* mbr, REALTYPE qlat, REALTYPE qlng, REALTYPE qradius);



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
bool point_in_mbr(MBR* mbr, REALTYPE qlat, REALTYPE qlng);




/** 
 * Calculate 2D cross product of OA and OB vectors, 
 * i.e. z-component of their 3D cross product.
 * @para point O, point A, point B.
 * 
 * @Return a positive value, if OAB makes a counter-clockwise turn
 *         (i.e., left turn. OB is on the left side of the direction of OA);
 *         a negative value, if making a clockwise turn;
 *         zero if the points are collinear.
 */
REALTYPE cross(const Point &O, const Point &A, const Point &B);


REALTYPE cross(const Edge &e1, const Edge &e2);

/**
 * Calculate the convex hull for a set of (four) MBRs.
 * 
 * @para four MBRs.
 * 
 * @Return a list of points on the convex hull in counter-clockwise order.
 */
Polygon calc_outer_convex(MBR* mbrs);



/**
 * Calculate the convex hull for a set of given points.
 * 
 * @para a set of points
 * 
 * @Return a list of points on the convex hull in counter-clockwise order.
 * 
 * Reference: https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain
 * Time complexity: O(nlogn), where n is the number of points.
 */
std::vector<Point> calc_convex_hull_points(std::vector<Point> P);



double det( const double a, const double b, const double c, const double d);

Point calc_intersection_point(Edge &e1, Edge &e2);

std::vector<Point> calc_inner_convex_points(std::vector<Edge> sortedE);

Polygon calc_inner_convex(MBR* mbrs);

double get_clockwise_angle(const Point& p, Point& center);


class sorter 
{
	public:
	Point center;
	sorter(Point &c)
	{
		this->center.longitude = c.longitude;
		this->center.latitude = c.latitude;
	}
	bool operator()(const Edge& a, const Edge& b)
	{
		return (get_clockwise_angle(a.a, this->center) 
		        < get_clockwise_angle(b.a, this->center));
	}
};


Polygon calc_inner_convex(MBR* mbrs);

bool is_overlap_two_mbrs(MBR &r1, MBR &r2);

/**
 * Check whether the four mbrs overlap or not.
 * Return true if there exist two mbrs overlap.
 */
bool is_overlap_among_mbrs(MBR* mbrs);