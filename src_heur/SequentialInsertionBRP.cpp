#include "SequentialInsertionBRP.h"

void SequentialInsertionBRP::Insert(Sol & s)
{
	s.Update();
	std::vector<Node*> nodes; //list of customers to insert
	std::vector<Node*> refused; //list of infeasible customers
	_insrmv.FillInsertionList(s, nodes);
	//printf("SeqInsertion %d\n", s.GetUnassignedCount());

	for(size_t i=0;i<nodes.size();i++)
	{
		Node * n = nodes[i];
		//printf("Node:%d\n", n->id);

		Move best;
		best.IsFeasible = false;
		best.DeltaCost = INFINITE;
		bool has_seen_empty_driver = false;
		for(int j = 0 ; j < s.GetDriverCount() ; j++)
		{
			Move m;
			if(s.GetRouteLength( s.GetDriver(j)) == 0 && has_seen_empty_driver) continue;

			_insrmv.InsertCost(s, n, s.GetDriver(j), m);

			if(s.GetRouteLength( s.GetDriver(j)) == 0)
				has_seen_empty_driver = true;
			//printf("Insert cost:%.2lf driver:%d Feasible:%d\n", m.DeltaCost, s.GetDriver(j)->id, m.IsFeasible );

			if(m.IsFeasible && m.DeltaCost < best.DeltaCost)
				best = m;
		}

		best.from = NULL;
		if(best.IsFeasible)
		{
			_insrmv.ApplyInsertMove(s, best);
			s.Update(best.to);
		}
		else
		{
			refused.push_back(n);
			s.RemoveFromUnassigneds(n);
		}
	}//end while


	for(size_t i = 0 ; i < refused.size() ; i++)
		s.AddToUnassigneds( refused[i] );
}
