
#include "query-basic-functions.h"
#include "tetrartree.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stack>
#include <math.h>
#include <cmath> 
#include <time.h>
#include <limits>
#include <algorithm>

using namespace std;

//Global variants
extern FILE* index_file;
extern int index_node_access_num;
extern int result_fov_num;
extern double index_node_access_time;
extern int total_hit_node_num;




/***
 * Report all the objects in the subtree of an index node.
 *
 * @para[in] node: an R-tree node
 *
 */
void total_hit_report(TETRARTREENODE* node)
{
	FILE* orientation_info_file = fopen("./orientation-info.log", "a");
	if (orientation_info_file == NULL) {
		printf("Couldn't open ./orientation-info.log \n");
		return;
	}
	
	TETRARTREENODE childnode;
	if(node->level>0) //non-leaf node
	{
		for(int i=0; i<node->count; i++)
		{
			clock_t tStart = clock();
			fseek(index_file, (node->branch[i].childid-1)*sizeof(TETRARTREENODE), 0);
			fread(&childnode, sizeof(TETRARTREENODE), 1, index_file);
			index_node_access_num++;
			index_node_access_time += (double)(clock() - tStart)/CLOCKS_PER_SEC;
			
			/**
			 * Calculate the number of nodes whose four mbrs do not overlap.
			 */
			/*if(!is_overlap_among_mbrs(node->branch[i].mbrs))
			{
				//total_hit_node_num += node.level*node.count + 1;
				total_hit_node_num += 1;
			}*/
			
			/**
			 * Print direction range for each index node.
			 */
			REALTYPE angle = CalculateAngleClockwise(node->branch[i].orientation.min, 
			                                         node->branch[i].orientation.max);
			angle = angle * 180.0/PI;
			
			fprintf(orientation_info_file, "%d\t%.6f\n", childnode.level, angle);
	
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
	fclose(orientation_info_file);
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
REALTYPE calc_min_dist(MBR* mbrp, REALTYPE qlat, REALTYPE qlng)
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
bool mbr_contain_circle(MBR* mbrp, REALTYPE qlat, REALTYPE qlng, REALTYPE qradius)
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
int circle_overlap_mbr(MBR* mbr, REALTYPE qlat, REALTYPE qlng, REALTYPE qradius)
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
bool point_in_mbr(MBR* mbr, REALTYPE qlat, REALTYPE qlng)
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
REALTYPE cross(const Point &O, const Point &A, const Point &B)
{
	return (A.longitude - O.longitude) * (B.latitude - O.latitude) 
	        - (A.latitude - O.latitude) * (B.longitude - O.longitude);
}

// OA -> (e1.a, e1.b);  OB -> (e2.a, e2.b)

REALTYPE cross(const Edge &e1, const Edge &e2)
{
	return (e1.b.longitude - e1.a.longitude) * (e2.b.latitude - e2.a.latitude) 
	        - (e1.b.latitude - e1.a.latitude) * (e2.b.longitude - e2.a.longitude);
}




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
std::vector<Point> calc_convex_hull_points(std::vector<Point> P)
{
	int n = P.size(), k = 0;
	std::vector<Point> H(2*n);

	// Sort points by their x-coordinates
	std::sort(P.begin(), P.end());

	// Build lower hull
	for (int i = 0; i < n; ++i) { //From right to left
		while (k >= 2 && cross(H[k-2], H[k-1], P[i]) <= 0) k--; //Remove H[k] from H
		//Always keep right turn
		H[k++] = P[i];
	}

	// Build upper hull
	for (int i = n-2, t = k+1; i >= 0; i--) { //From left to right
		while (k >= t && cross(H[k-2], H[k-1], P[i]) <= 0) k--; //Remove H[k] from H
		//Always keep right turn
		H[k++] = P[i];
	}

	H.resize(k-1);
	//Until now the last point in the returned list is the same as the first one.
	
	return H;
}




/**
 * Calculate the convex hull for a set of (four) MBRs.
 * 
 * @para the pointer of four MBRs.
 * 
 * @Return a list of points on the convex hull in counter-clockwise order.
 */
Polygon calc_outer_convex(MBR* mbrs)
{
	/**
	 * Select points from the input MBRs that may contribute the boundary points 
	 * for the convex hull.
	 */
	std::vector<Point> points;
	/*REALTYPE minX = MIN(MIN(MIN(mbrs[0].bound[0], 
	                            mbrs[1].bound[0]), 
	                        mbrs[2].bound[0]),
						mbrs[3].bound[0]);*/
	std::vector<REALTYPE> minx_vec;
	for(int i=0; i<MBR_NUMB; i++) minx_vec.push_back(mbrs[i].bound[0]);
	std::vector<REALTYPE>::iterator minx_iter = std::min_element(minx_vec.begin(), 
	                                                             minx_vec.end());
	int minx_idx = std::distance(minx_vec.begin(), minx_iter);
	points.push_back(Point(mbrs[minx_idx].bound[1], minx_vec[minx_idx]));
	points.push_back(Point(mbrs[minx_idx].bound[3], minx_vec[minx_idx]));
	
	/*REALTYPE minY = MIN(MIN(MIN(mbrs[0].bound[1], 
	                            mbrs[1].bound[1]), 
	                        mbrs[2].bound[1]),
						mbrs[3].bound[1]);*/
	std::vector<REALTYPE> miny_vec;
	for(int i=0; i<MBR_NUMB; i++) miny_vec.push_back(mbrs[i].bound[1]);
	std::vector<REALTYPE>::iterator miny_iter = std::min_element(miny_vec.begin(), 
	                                                             miny_vec.end());
	int miny_idx = std::distance(miny_vec.begin(), miny_iter);
	points.push_back(Point(miny_vec[miny_idx], mbrs[miny_idx].bound[0]));
	points.push_back(Point(miny_vec[miny_idx], mbrs[miny_idx].bound[2]));
						
	/*REALTYPE maxX = MAX(MAX(MAX(mbrs[0].bound[2], 
	                            mbrs[1].bound[2]), 
	                        mbrs[2].bound[2]),
						mbrs[3].bound[2]);*/
	std::vector<REALTYPE> maxx_vec;
	for(int i=0; i<MBR_NUMB; i++) maxx_vec.push_back(mbrs[i].bound[2]);
	std::vector<REALTYPE>::iterator maxx_iter = std::max_element(maxx_vec.begin(), 
	                                                             maxx_vec.end());
	int maxx_idx = std::distance(maxx_vec.begin(), maxx_iter);
	points.push_back(Point(mbrs[maxx_idx].bound[1], maxx_vec[maxx_idx]));
	points.push_back(Point(mbrs[maxx_idx].bound[3], maxx_vec[maxx_idx]));					
						
	/*REALTYPE maxY = MAX(MAX(MAX(mbrs[0].bound[3], 
	                            mbrs[1].bound[3]), 
	                        mbrs[2].bound[3]),
						mbrs[3].bound[3]);*/
	std::vector<REALTYPE> maxy_vec;
	for(int i=0; i<MBR_NUMB; i++) maxy_vec.push_back(mbrs[i].bound[3]);
	std::vector<REALTYPE>::iterator maxy_iter = std::max_element(maxy_vec.begin(), 
	                                                             maxy_vec.end());
	int maxy_idx = std::distance(maxy_vec.begin(), maxy_iter);
	points.push_back(Point(maxy_vec[maxy_idx], mbrs[maxy_idx].bound[0]));
	points.push_back(Point(maxy_vec[maxy_idx], mbrs[maxy_idx].bound[2]));
	
	/*printf("points: \n");
	for(auto p : points) printf("%.6f\t%0.6f\n", p.longitude, p.latitude);*/
	
	std::vector<Point> outer_convex_points = calc_convex_hull_points(points);
	
	/*printf("polygon: \n");
	for(auto p : outer_convex_points) printf("%.6f\t%0.6f\n", p.longitude, p.latitude);*/
	
	Polygon polygon(outer_convex_points);
	return polygon;
}



double det( const double a, const double b, const double c, const double d)
{
    return a * d - b * c;
}


/** 
 * Reference: http://www.cplusplus.com/forum/beginner/193529/
 */
Point calc_intersection_point(Edge &e1, Edge &e2)
{
	//x1 <- e1.a.longitude; y1 <- e1.a.latitude
    //x2 <- e1.b.longitude; y2 <- e1.b.latitude
    //x3 <- e2.a.longitude; y3 <- e2.a.latitude
    //x4 <- e2.b.longitude; y4 <- e2.b.latitude
    double denominator = det(e1.a.longitude - e1.b.longitude, 
	                         e1.a.latitude - e1.b.latitude, 
							 e2.a.longitude - e2.b.longitude, 
							 e2.a.latitude - e2.b.latitude);
	REALTYPE x = 0, y =0;
    if(denominator != 0)
    {
        x = det( det(e1.a.longitude, e1.a.latitude, e1.b.longitude, e1.b.latitude), 
		         e1.a.longitude - e1.b.longitude, 
				 det(e2.a.longitude, e2.a.latitude, e2.b.longitude, e2.b.latitude), 
				 e2.a.longitude - e2.b.longitude ) / denominator;
        y = det( det(e1.a.longitude, e1.a.latitude, e1.b.longitude, e1.b.latitude), 
		         e1.a.latitude - e1.b.latitude, 
				 det(e2.a.longitude, e2.a.latitude, e2.b.longitude, e2.b.latitude), 
				 e2.a.latitude - e2.b.latitude ) / denominator;
    }
    else 
	{
		/*std::cout << "Line((" << e1.a.latitude << ", " << e1.a.longitude << "), ("
		                      << e1.b.latitude << ", " << e1.b.latitude  << ")) " << "and "
				  << "Line((" << e2.a.latitude << ", " << e2.a.longitude << "), ("
		                      << e2.b.latitude << ", " << e2.b.latitude  << ")) " 
				  << "don't intersect.\n";*/
	}
	
	return Point(y, x);
}




std::vector<Point> calc_inner_convex_points(std::vector<Edge> sortedE)
{
	/** 
	 * First, calculate the four line segments that compose 
	 * the inner convex.
	 */
	std::vector<Edge> convex_lines;
	for(size_t i=0; i<sortedE.size(); i++)
	{
		size_t j = (i+1)%sortedE.size();
		Edge e1, e2;
		if(cross(sortedE[i].a, sortedE[j].a, sortedE[j].b) <= 0)
		{
			e1.setEdge(sortedE[i].a, sortedE[j].b);
		}
		else e1.setEdge(sortedE[i].a, sortedE[j].a);
		
		if(cross(sortedE[i].b, sortedE[j].a, sortedE[j].b) <= 0)
		{
			e2.setEdge(sortedE[i].b, sortedE[j].b);
		}
		else e2.setEdge(sortedE[i].b, sortedE[j].a);
		
		if(cross(e1, e2) <= 0)
		{
			convex_lines.push_back(e1);
		}
		else convex_lines.push_back(e2);
	}
	
	/**
	 * Then, calculate the intersection points of the four 
	 * line segments to compute the inner convex.
	 */
	std::vector<Point> intersection_points;
	for(size_t i=0; i<convex_lines.size(); i++)
	{
		size_t j = (i+1)%convex_lines.size();
		/**
		 * Calculate intersection point of 
		 * line convex_lines[i] and convex_lines[j].
		 */
		Point p = calc_intersection_point(convex_lines[i], convex_lines[j]);
		intersection_points.push_back(p);
	}
	return intersection_points;
}



double get_clockwise_angle(const Point& p, Point& center)
{   
    double angle = 0.0;

	Point tmpp;
	tmpp.longitude = p.longitude - center.longitude;
	tmpp.latitude = p.latitude - center.latitude;
	
    /*calculate angle and return it*/
	angle = -std::atan2(tmpp.longitude, -tmpp.latitude);
    return angle;
}


bool is_overlap_two_mbrs(MBR &r1, MBR &r2)
{
	// x-axis
	if( fabs((r1.bound[2] + r1.bound[0])/2.0 - (r2.bound[2] + r2.bound[0])/2.0) 
		> (r1.bound[2] - r1.bound[0])/2.0 + (r2.bound[2] - r2.bound[0])/2.0 )
		return false;
	// y-axis
	if( fabs((r1.bound[3] + r1.bound[1])/2.0 - (r2.bound[3] + r2.bound[1])/2.0) 
		> (r1.bound[3] - r1.bound[1])/2.0 + (r2.bound[3] - r2.bound[1])/2.0 )
		return false;
	return true;
}


/**
 * Check whether the four mbrs overlap or not.
 * Return true if there exist two mbrs overlap.
 */
bool is_overlap_among_mbrs(MBR* mbrs)
{
	for(int i=0; i<MBR_NUMB; i++)
	{
		int j = (i+1)%MBR_NUMB;
		if(is_overlap_two_mbrs(mbrs[i], mbrs[j]))
		{
			return true; 
		}
	}
	return false;
}



Polygon calc_inner_convex(MBR* mbrs)
{
	/**
	 * Check whether the four mbrs overlap or not.
	 * Return null polygon if there are two mbrs overlap.
	 */
	if(is_overlap_among_mbrs(mbrs))
	{
		return Polygon(); 
	}
	return Polygon(2);
	
	/**
	 * Select points from the input MBRs that may contribute the boundary points 
	 * for the convex hull.
	 */
	std::vector<Edge> edges;
	std::vector<REALTYPE> maxx_vec;
	for(int i=0; i<MBR_NUMB; i++) maxx_vec.push_back(mbrs[i].bound[0]);
	std::vector<REALTYPE>::iterator maxx_iter = std::max_element(maxx_vec.begin(), 
	                                                             maxx_vec.end());
	int maxx_idx = std::distance(maxx_vec.begin(), maxx_iter);
	Edge edge1(Point(mbrs[maxx_idx].bound[1], maxx_vec[maxx_idx]),
	           Point(mbrs[maxx_idx].bound[3], maxx_vec[maxx_idx]));
	edges.push_back(edge1);
	
	std::vector<REALTYPE> maxy_vec;
	for(int i=0; i<MBR_NUMB; i++) maxy_vec.push_back(mbrs[i].bound[1]);
	std::vector<REALTYPE>::iterator maxy_iter = std::max_element(maxy_vec.begin(), 
	                                                             maxy_vec.end());
	int maxy_idx = std::distance(maxy_vec.begin(), maxy_iter);
	Edge edge2(Point(maxy_vec[maxy_idx], mbrs[maxy_idx].bound[0]),
	           Point(maxy_vec[maxy_idx], mbrs[maxy_idx].bound[2]));
	edges.push_back(edge2);
	
	std::vector<REALTYPE> minx_vec;
	for(int i=0; i<MBR_NUMB; i++) minx_vec.push_back(mbrs[i].bound[2]);
	std::vector<REALTYPE>::iterator minx_iter = std::min_element(minx_vec.begin(), 
	                                                             minx_vec.end());
	int minx_idx = std::distance(minx_vec.begin(), minx_iter);
	Edge edge3(Point(mbrs[minx_idx].bound[1], minx_vec[minx_idx]),
	           Point(mbrs[minx_idx].bound[3], minx_vec[minx_idx]));					
	edges.push_back(edge3);
	
	std::vector<REALTYPE> miny_vec;
	for(int i=0; i<MBR_NUMB; i++) miny_vec.push_back(mbrs[i].bound[3]);
	std::vector<REALTYPE>::iterator miny_iter = std::min_element(miny_vec.begin(), 
	                                                             miny_vec.end());
	int miny_idx = std::distance(miny_vec.begin(), miny_iter);
	Edge edge4(Point(miny_vec[miny_idx], mbrs[miny_idx].bound[0]),
	           Point(miny_vec[miny_idx], mbrs[miny_idx].bound[2]));
	edges.push_back(edge4);
	
	/*printf("points: \n");
	for(auto p : points) printf("%.6f\t%0.6f\n", p.longitude, p.latitude);*/
	
	
	Point center;
	center.longitude = (maxx_vec[maxx_idx] + minx_vec[minx_idx])/2.0;
	center.latitude = (maxy_vec[maxy_idx] + miny_vec[miny_idx])/2.0;
	//Sort points in clockwise direction.
	std::sort(edges.begin(), edges.end(),sorter(center));
	
	
	std::vector<Point> outer_convex_points = calc_inner_convex_points(edges);
	
	
	/*printf("polygon: \n");
	for(auto p : outer_convex_points) printf("%.6f\t%0.6f\n", p.longitude, p.latitude);*/
	
	Polygon polygon(outer_convex_points);
	return polygon;
}






