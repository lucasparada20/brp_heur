

#ifndef MOVE_H_
#define MOVE_H_
#include <stdlib.h>
#include "NodeBRP.h"
#include "DriverBRP.h"

class MoveBRP
{
	public:
	Node * prev;
};

class Move
{
	public:
		Node * n;
		Driver * to;
		Driver * from;
		double DeltaCost;
		double DeltaDistance;
		double DeltaRec;
		bool IsFeasible;
		int pos;
		MoveBRP move;

		Move(): n(NULL), to(NULL),from(NULL), DeltaCost(INFINITE), DeltaRec(INFINITE), IsFeasible(false), pos(-1){}

		void Show(){printf("Mo n:%d DriverTo:%d DriverFrom:%d IsFeas:%d dCost:%.1lf dDist:%.1lf dRec:%.1lf prev:%d\n",n==NULL?-1:n->no,to==NULL?-1:to->id,from==NULL?-1:from->id,IsFeasible,DeltaCost,DeltaDistance,DeltaRec,move.prev == NULL ? -1: move.prev->no);}

		bool operator < (const Move & m) const
		{
			return (DeltaCost < m.DeltaCost);
		}

		bool operator < (const Move * m) const
		{
			return (DeltaCost < m->DeltaCost);
		}
};



#endif
