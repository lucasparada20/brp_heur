#ifndef INSRMV_BRP
#define INSRMV_BRP

#include "Move.h"
#include "NodeBRP.h"
#include "DriverBRP.h"
#include "ProblemDefinition.h"
#include "Solution.h"


class InsRmvMethodBRP 
{
	public:
		InsRmvMethodBRP(Prob & prob) { _prob = &prob;}  ;

		void FillInsertionList(Sol & s, std::vector<Node*> & list);		
		void InsertCost(Sol & s, Node * n, Driver * d, Move & m);
		void ApplyInsertMove(Sol & s, Move & m);
		void RemoveCost(Sol & s, Node * n, Move & m);

	private:
		std::vector<Node*> path;
		Prob * _prob;
};


#endif
