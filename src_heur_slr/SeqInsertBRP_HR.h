#ifndef SEQ_INSERT_BRP_HC
#define SEQ_INSERT_BRP_HC

#include "Solution.h"
#include "Move.h"
#include "InsRmvMethodBRP.h"
#include "OperatorBase.h"
#include "RouteFeasibility.h"
#include "DriverBRP.h" 
#include "NodeBRP.h"
#include "InsRmvMethodBRP.h"
#include "Parameters.h"

#include <ctime> //clock_t
#include <algorithm>

class SeqInsertBRP_HR : public InsertOperator
{
	public:
	SeqInsertBRP_HR(InsRmvMethodBRP & insrmv): _insrmv(insrmv){}
	
	void FillMoveVec_HR(Sol & s, Node * n, Driver * d, std::vector<Move> & moveVec);

	void Insert(Sol & s, bool show);
	
	private:
	InsRmvMethodBRP & _insrmv;
	std::vector<Node*> path;
};


#endif