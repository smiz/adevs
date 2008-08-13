#include "adevs.h"
#include <iostream>
#include <algorithm>
using namespace std;
using namespace adevs;

int main () {

rv r;
double sum = 0.0;
double _min = 999999.9;
double _max = -999999.9;
int count = 1000000;

cout << "triangular (0, 1, 2)" << endl; 
for (int i = 0; i < count; i++) {
   double x = r.triangular (0, 1, 2);
   sum += x;
   _min = min (x, _min);
   _max = max (x, _max);
   }
   sum /= (double)count;
   cout << "m = " << sum << " ,min = " << _min << " ,max =" << _max << endl;

cout << "triangular (-10, 1, 3)" << endl; 
sum = 0.0;
_min = 9999999.9;
_max = -9999999.9;
for (int i = 0; i < count; i++) {
   double x = r.triangular (-10, 1, 3);
   sum += x;
   _min = min (x, _min);
   _max = max (x, _max);
   }
   sum /= (double)count;
   cout << "m = " << sum << " ,min = " << _min << " ,max =" << _max << endl;

cout << "uniform (-5, 1)" << endl;
sum = 0.0;
_min = 9999999.9;
_max = -9999999.9;
for (int i = 0; i < count; i++) {
   double x = r.uniform (-5, 1);
   sum += x;
   _min = min (x, _min);
   _max = max (x, _max);
   }
   sum /= (double)count;
   cout << "m = " << sum << " ,min = " << _min << " ,max =" << _max << endl;

cout << "exp (1)" << endl;
sum = 0.0;
_min = 9999999.9;
_max = -9999999.9;
for (int i = 0; i < count; i++) {
   double x = r.exp (1);
   sum += x;
   _min = min (x, _min);
   _max = max (x, _max);
   }
   sum /= (double)count;
   cout << "m = " << sum << " ,min = " << _min << " ,max =" << _max << endl;

cout << "normal (0,1)" << endl;
sum = 0.0;
_min = 9999999.9;
_max = -9999999.9;
for (int i = 0; i < count; i++) {
   double x = r.normal (0,1);
   sum += x;
   _min = min (x, _min);
   _max = max (x, _max);
   }
   sum /= (double)count;
   cout << "m = " << sum << " ,min = " << _min << " ,max =" << _max << endl;

return 0;

}
