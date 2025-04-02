
#ifndef ROUTE_FEASIBILITY
#define ROUTE_FEASIBILITY

#include <vector>
#include "NodeBRP.h"
#include "DriverBRP.h"
#include "ProblemDefinition.h"


class RouteFeasibility
{
public:

	static bool IsFeasible(Prob* prob, std::vector<Node*>& path);
	static int GetDriverCount(Prob* prob, std::vector<Node*>& path);

private:
	
};

#endif
