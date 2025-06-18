#ifndef ROUTE_FEASIBILITY_H
#define ROUTE_FEASIBILITY_H

#include "ProblemDefinition.h"
#include "NodeBRP.h"
#include "DriverBRP.h"
#include <iostream>
#include <algorithm>


class RouteFeasibility{
public:
	
	static bool HasZeroHC(Prob* prob, std::vector<Node*>& path);
	static bool IsFeasible(Prob* prob, std::vector<Node*>& path, bool show);
	static bool IsRestockFeasible(Prob* prob, std::vector<Node*>& path, bool show);
	static bool StartLoadIsFeasible(Prob* prob, std::vector<Node*> & path, bool show);
	
	static int ComputeRecourseFromStartingCost(Prob* prob, std::vector<Node*>& path, std::vector<int> & input_costs);
	static double RecourseCost(Prob* prob, std::vector<Node*>& path);
	static double RecourseCostAndStore(Prob* prob, std::vector<Node*>& path);
	static double LowerBound(Prob* prob, std::vector<Node*>& stations);
	
	static double RestockingRecourseCostWithMaxDistance(Prob*prob, std::vector<Node*>& path);
	static double GetCostRecursive(int k, int x_k, double d_k, Prob* prob, std::vector<std::vector<std::vector<double>>> &m, std::vector<Node*> path);
	static double GetCostRecursive(int k, int x_k, double d_k, double & UB, double cumul_cost, Prob* prob, std::vector<std::vector<std::vector<double>>> & m, std::vector<Node*> path);

	static int GetDriverCount(Prob * prob, std::vector<Node*> & path);
	//static double RestockingRecourseCostWithMaxDistance(Prob*prob, std::vector<Node*>& path, int current_distance);
};

#endif