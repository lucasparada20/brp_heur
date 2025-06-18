#include "SeqInsertBRP_HR.h"
#include <numeric> //std::accumulate

void SeqInsertBRP_HR::FillMoveVec_HR(Sol & s, Node * n, Driver * d, std::vector<Move> & moveVec)
{	
	Prob* prob = s.GetProb();
	int Q = d->capacity;
	int pos = 0;
	Node * prev = s.GetNode(d->StartNodeID);
	while (prev->type != NODE_TYPE_END_DEPOT)
	{
		Node * next = s.Next[prev->id];
		if (prev->type != NODE_TYPE_START_DEPOT)
			iter_swap(path.begin() + pos, path.begin() + pos + 1);

		double deltaDist = s.GetDist(prev, n) + s.GetDist(n, next) - s.GetDist(prev, next);
		if (d->curDistance + deltaDist > Parameters::MaxRouteDistance())
		{
			prev = next; pos++; continue;
		}
		
		std::vector<double> restocking_distances;
		for(int i=0;i<path.size()-1;i++)
			restocking_distances.push_back( prob->GetDist(path[i], path[0]) + prob->GetDist(path[0], path[i+1]) - prob->GetDist(path[i], path[i + 1]) ); 
			
		bool hasZeroRec = RouteFeasibility::HasZeroHC(prob,path);

		//The move is feasible! store it.
		Move mo;
		mo.DeltaDistance = deltaDist;
		mo.DeltaRec = (hasZeroRec && d->curRecourse > 0) ? (-1 * d->curRecourse) : 
		  (hasZeroRec && d->curRecourse == 0) ? 0 : 9999;  
		mo.DeltaCost = mo.DeltaDistance + mo.DeltaRec; // if mo.DeltaRec > 0, then DeltaCost = mo.DeltaDistance + INFINITE 
		mo.IsFeasible = true;
		mo.n = n;
		mo.to = d;
		mo.pos = pos;
		mo.move.prev = prev;
				
		moveVec.push_back(mo);		
			
		prev = next; pos++;
	}
}

void SeqInsertBRP_HR::Insert(Sol& s, bool show)
{
	Prob * prob = s.GetProb();
	s.Update();
	std::vector<Node*> nodes; //list of customers to insert
	std::vector<Node*> refused; //list of infeasible customers
	_insrmv.FillInsertionList(s, nodes); //fills nodes with the unassigned customers
	
	clock_t startTime = clock();
	for(size_t i=0;i<nodes.size();i++)
	{
		Node * n = nodes[i];
		if (show && i % 50 == 0)
		{
			clock_t endTime = clock();
			double elapsedSeconds = (double)(endTime - startTime) / CLOCKS_PER_SEC;
			printf("SeqInsertion with Restocking Recourse Inserting Node:%d ElapsedSeconds:%.1lf\n", n->id, elapsedSeconds);
		}
		std::vector<Move> moveVec;
		bool has_seen_empty_driver = false;
		for(int j=0;j<s.GetDriverCount();j++)
		{				
			Driver * d = s.GetDriver(j);		
			
			path.clear(); path.reserve( s.GetRouteLength( s.GetDriver(j)) );
			Node * prev = s.GetNode( d->StartNodeID );
			path.push_back(prev); 
			path.push_back(n);
			while(prev->type != NODE_TYPE_END_DEPOT)
			{
				Node * next = s.Next[ prev->id ];
				path.push_back(next);
				prev = next;
			}
			
			if(s.GetRouteLength( s.GetDriver(j)) == 0 && has_seen_empty_driver) continue;
			
			//Fills moveVec with all feasible insertions of node 'n' in the path. DeltaDistance is stored for each move to sort.
			//The recourse is not computed for the moves ...
			
			FillMoveVec_HR(s,n,d,moveVec);
			
			if(s.GetRouteLength( s.GetDriver(j)) == 0)
				has_seen_empty_driver = true;			
		}
		
		std::sort(moveVec.begin(), moveVec.end());
		Move best;
		best.IsFeasible = true;
		best.DeltaCost = INFINITE;
		
		//Find the best delta Recourse of the moves
		for(int j=0;j<std::min(10,(int)moveVec.size());j++)
		{
			Move mo = moveVec[j];
			Driver * d = mo.to;
			path.clear(); path.reserve( s.GetRouteLength( s.GetDriver(j)) );
			
			//insert the node 'n' in path at position mo.pos
			Node * prev = s.GetNode( d->StartNodeID );
			path.push_back(prev); 
			int cntr = 0;
			if(mo.pos == 0) path.push_back(n);
			while(prev->type != NODE_TYPE_END_DEPOT)
			{
				Node * next = s.Next[ prev->id ];
				path.push_back(next);
				cntr++;
				if(mo.pos == cntr) path.push_back(n);
				prev = next;
			}	
				
			double rec = mo.DeltaRec <= 0 ? 0.0 : RouteFeasibility::RestockingRecourseCostWithMaxDistance(s.GetProb(),path);
			if(rec > 9990)
				continue;
				
				
			mo.DeltaRec = mo.DeltaRec <= 0 ? mo.DeltaRec : rec - d->curRecourse; //DeltaRec can be negative due to a helpful insertion of a station in the middle of a route
			//mo.Show();
			
			if(mo.DeltaDistance + mo.DeltaRec < best.DeltaCost) // Just if its feasible and then store in an std::vector<Move> and sort
			{
				mo.DeltaCost = mo.DeltaRec + mo.DeltaDistance; 
				best = mo;
			}
			
			//Test the next move in the vector to check if is not better. If its indeed not better, then break!
			if( j+1 < moveVec.size() )
			{
				Move nextMo = moveVec[j+1];
				Driver * nextD = nextMo.to;
				path.clear(); path.reserve( s.GetRouteLength( s.GetDriver(j)) );
				
				//insert the node 'n' in path at position mo.pos
				Node * prev = s.GetNode( nextD->StartNodeID );
				path.push_back(prev); 
				int cntr = 0;
				if(nextMo.pos == 0) path.push_back(n);
				while(prev->type != NODE_TYPE_END_DEPOT)
				{
					Node * next = s.Next[ prev->id ];
					path.push_back(next);
					cntr++;
					if(nextMo.pos == cntr) path.push_back(n);
					prev = next;
				}

				double nextRec = nextMo.DeltaRec <= 0 ? 0 : RouteFeasibility::RestockingRecourseCostWithMaxDistance(s.GetProb(), path);
				if(nextRec > 9990) break;
				
				nextMo.DeltaRec = nextMo.DeltaRec <= 0 ? nextMo.DeltaRec : rec - nextD->curRecourse; //DeltaRec can be negative due to a helpful insertion of a station in the middle of a route
				
				if(nextMo.DeltaDistance + nextMo.DeltaRec > best.DeltaCost)
					break;
				
			}
		}			
		//If the node could not be inserted ...
		if(best.DeltaCost > 9990)
		{
			refused.push_back(n);
			s.RemoveFromUnassigneds(n);
			printf("Could not insert Best move:\n");
			best.Show(); 
			printf("NodeInserted:%d Cost:%.2lf Unassigneds:%d\n",n->no,s.GetCost(),s.GetUnassignedCount());
			//getchar();
		}
		else{
			best.from = NULL;
			_insrmv.ApplyInsertMove(s, best);
			s.Update(best.to);
		}
		for(size_t i = 0 ; i < refused.size() ; i++)
			s.AddToUnassigneds( refused[i] );
		/*printf("Best move:\n");
		best.Show(); 
		printf("NodeInserted:%d Cost:%.2lf Unassigneds:%d\n",n->no,s.GetCost(),s.GetUnassignedCount());
		getchar();*/
	}
}

