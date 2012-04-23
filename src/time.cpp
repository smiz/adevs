#include "adevs_time.h"

std::ostream& operator<<(std::ostream& strm, const adevs::Time<>& t)
{
	strm << "(" << t.t << "," << t.c << ")";
	return strm;
}
