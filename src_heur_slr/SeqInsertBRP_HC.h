#ifndef SEQUENTIAL_INSERTION_BRP
#define SEQUENTIAL_INSERTION_BRP

#include "Solution.h"
#include "Move.h"
#include "InsRmvMethodBRP.h"
#include "OperatorBase.h"
#include <ctime> //clock_t
#include "RouteFeasibility.h"
#include "DriverBRP.h" 
#include "NodeBRP.h"
#include "InsRmvMethodBRP.h"
#include "Parameters.h"

#include <algorithm>


class SeqInsertBRP_HC : public InsertOperator
{
	public:
	SeqInsertBRP_HC(InsRmvMethodBRP & insrmv): _insrmv(insrmv){}
	
	void FillMoveVec_HC(Sol & s, Node * n, Driver * d, std::vector<Move> & moveVec);

	void Insert(Sol & s, bool show);
	
	private:
	InsRmvMethodBRP & _insrmv;
	std::vector<Node*> path;
};


#endif