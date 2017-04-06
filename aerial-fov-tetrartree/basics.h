#ifndef __BASICS_H__
#define __BASICS_H__

#include <limits>
#include <cmath>
#include <iomanip>

using namespace std;


#ifndef  TRUE
#define  TRUE  1
#endif

#ifndef  FALSE
#define  FALSE  0
#endif

#define  PI 3.1415926
#define	 EARTH_RADIUS 6371.00 //km



#ifndef MIN
 #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
 #define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

const double EPSILON = 1e-4;
const double DOUBLE_MIN = numeric_limits<double>::min();
const double DOUBLE_MAX = numeric_limits<double>::max();

typedef float REALTYPE;





#endif // __BASICS_H__
