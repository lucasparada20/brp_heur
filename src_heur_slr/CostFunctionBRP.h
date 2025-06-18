#ifndef COST_FUNCTION_BRP	
#define COST_FUNCTION_BRP

#include "NodeBRP.h"
#include "DriverBRP.h"
#include "ProblemDefinition.h"
#include "Parameters.h"

class Sol; //Forward declaration here and not including the header of Solution.h

class CostFunctionBRP
{
	public:
		CostFunctionBRP(){}
		~CostFunctionBRP(){}

		double GetCost(Sol & s);
		double GetCost(Sol & s, Driver * d);
		void Update(Sol & s);
		void Update(Sol & s, Driver * d);
		void Show(Sol * s, Driver * d);
		void MakeFeasible(Sol * s, Driver * d);
		
	private:
		std::vector<Node*> path;
};


#endif
