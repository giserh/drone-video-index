
#ifndef __POLYGON_H__
#define __POLYGON_H__

#include "basics.h"
#include <vector>

using namespace std;


struct Point
{
	REALTYPE latitude; // Y-axis
	REALTYPE longitude; // X-axis
	
	Point()
	{
		this->latitude = 0.0;
		this->longitude = 0.0;
	}
	
	Point(REALTYPE lat, REALTYPE lng)
	{
		this->latitude = lat;
		this->longitude = lng;
	}
	
	/*Point(Point A)
	{
		this->latitude = A.latitude;
		this->longitude = A.longitude;
	}*/
	
	//Sorted in increasing order by longitude first, then latitude
	bool operator <(const Point &p) const { 
		return this->longitude < p.longitude || 
		       (this->longitude == p.longitude && this->latitude < p.latitude);
	}
};

 
struct Edge {
    Point a, b;
	
	Edge(){}
	
	Edge(Point A, Point B)
	{
		this->a.latitude = A.latitude;
		this->a.longitude = A.longitude;
		this->b.latitude = B.latitude;
		this->b.longitude = B.longitude;
	}
	
	void setEdgeA(Point A)
	{
		this->a.latitude = A.latitude;
		this->a.longitude = A.longitude;
	}
	
	void setEdgeB(Point B)
	{
		this->b.latitude = B.latitude;
		this->b.longitude = B.longitude;
	}
	
	void setEdge(Point A, Point B)
	{
		this->a.latitude = A.latitude;
		this->a.longitude = A.longitude;
		this->b.latitude = B.latitude;
		this->b.longitude = B.longitude;
	}
	
	/**
	 * Check whether a point intersects the segment (i.e., edge) or not.
	 *
	 * @param[in] Point p: the X-axis (longitude) and Y-axis (latitude) of the point.
	 *
	 * @return true if inside; false otherwise.
	 */
	bool ray_intersects_segment(Point& p)
	{
		if (a.latitude > b.latitude) 
		{
			Edge ba(b, a); // Change from Edge(ab) to Edge(ba)
			return ba.ray_intersects_segment(p); 
		}
			
		if (fabs(p.latitude-a.latitude) <= DOUBLE_MIN 
		    || fabs(p.latitude-b.latitude) <= DOUBLE_MIN) 
		{
			// Change checking point.
			p.latitude = p.latitude + EPSILON;
			return ray_intersects_segment(p); 
		}

		if (p.latitude > b.latitude 
		    || p.latitude < a.latitude 
			|| p.longitude > max(a.longitude, b.longitude)) return false;
		
		if (p.longitude < min(a.longitude, b.longitude)) return true;
		
		auto blue = abs(a.longitude - p.longitude) > DOUBLE_MIN 
		            ? (p.latitude - a.latitude) / (p.longitude - a.longitude) 
					: DOUBLE_MAX;
		
		auto red = abs(a.longitude - b.longitude) > DOUBLE_MIN 
		           ? (b.latitude - a.latitude) / (b.longitude - a.longitude) 
				   : DOUBLE_MAX;
		return blue >= red;
	}
};





/***
 This is the polygon structure.
 **/
 
class Polygon{
public:
	std::vector<Edge> edges;
	
public:
    //Constructors
    
	Polygon(){}
	
	Polygon(int size)
	{
		Edge e = {{0.0, 0.0}, {0.0, 0.0}};
		for(int i=0; i<size; i++)
		{
			this->edges.push_back(e);
		}
	}
	
	Polygon(std::vector<Point> &points)
	{
		for(size_t i=0; i+1<points.size(); i++)
		{
			Edge e(points[i], points[i+1]);
			this->edges.push_back(e);
		}
		Edge e(points[points.size()-1], points[0]);
		this->edges.push_back(e);
		//for(auto p : points) printf("%.6f\t%.6f\t", p.latitude, p.longitude);
		//printf("\n");
	}
	
	
    //Destructor
	~Polygon()
	{}
	
	
	/** 
	 * Check whether a point inside of a quadrilateral (polygon).
	 * Apply the ray casting algorithm, references:
	 * [M. Shimrat. Algorithm 112: Position of point relative to polygon. Commun. ACM 1962]
	 * https://rosettacode.org/wiki/Ray-casting_algorithm#C.2B.2B
	 *
	 * @param[in] the latitude of the point.
	 * @param[in] the longitude of the point.
	 *
	 * @return true if inside; false otherwise. 
	 */
	bool point_in_polygon(REALTYPE plat, REALTYPE plng)
	{
		Point p(plat, plng);
		auto c = 0;
        for (auto e : this->edges) if (e.ray_intersects_segment(p)) c++;
        return c % 2 != 0; //return true if odd; false if even.
	}
	
	
};




#endif // __POLYGON_H__