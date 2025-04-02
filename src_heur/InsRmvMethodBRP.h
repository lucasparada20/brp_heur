#ifndef INSRMV_BRP
#define INSRMV_BRP

#include "NodeBRP.h"
#include "DriverBRP.h"
#include "Move.h"
#include "ProblemDefinition.h"
#include "Solution.h"
#include "Constants.h"

class InsRmvMethodBRP
{
	public:
		InsRmvMethodBRP(Prob & prob) { _prob = &prob;}  ;

		void InsertCost(Sol & s, Node * n, Driver * d, Move & m);
		void ApplyInsertMove(Sol & s, Move & m);
		void RemoveCost(Sol & s, Node * n, Move & m);
		void CheckMove(Sol & s, Move & m){};
		void FillInsertionList(Sol & s, std::vector<Node*> & list);

	private:
		std::vector<Node*> path;
		Prob * _prob;
};


#endif
