#include "SeqInsertBRP_HC.h"

void SeqInsertBRP_HC::FillMoveVec_HC(Sol & s, Node * n, Driver * d, std::vector<Move> & moveVec)
{	
	int Q = d->capacity;
	n->ResetFeasibilityQuantities();
	int lambda_pos = std::max(-Q, n->demand - n->w_plus);
	int mu_pos = std::min(Q, n->demand + n->w_minus);
	int gamma_pos = std::min(Q, n->demand + n->w_minus);
	int zeta_pos = std::max(-Q, n->demand - n->w_plus);
	n->sumLambda = lambda_pos; n->minLambda = lambda_pos; n->sumMu = mu_pos; n->maxMu = mu_pos;
	n->sumGamma = -1*lambda_pos; n->minGamma = -1*lambda_pos; n->sumZeta = -1*zeta_pos; n->maxZeta = -1*zeta_pos;

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

		std::vector<Node*> path_until_prev; std::vector<Node*> path_after_insertion;
		for (int i = 0; i < path.size(); i++)
		{
			if (n->no == path[i]->no) continue;
			if (i < pos + 1) path_until_prev.push_back(path[i]);
			else path_after_insertion.push_back(path[i]);
		}
		//1.- Check if the route is feasible up to prev. This check is O(1)
		if (prev->type == NODE_TYPE_CUSTOMER && prev->IsForwardFeasible == 0)
		{
			prev = next; pos++; continue;
		}
		//2.- Check if the route is BACKWARDS feasible from the next node after the insertion (Compute StartLoadWindow for path Insert+1-....-d). This check is O(n).
		//IMPROVED: In the Update() function the lb, ubs where compute for both forward and backward feasibility -> The check becomes O(1).
		if (path_after_insertion[0]->type == NODE_TYPE_CUSTOMER && path_after_insertion[0]->IsBackwardFeasible == 0)
		{
			prev = next; pos++; continue;
		}

		int sumLambda = prev->sumLambda + lambda_pos;
		int minLambda = std::min(sumLambda, (int)prev->minLambda);
		int sumMu = prev->sumMu + mu_pos;
		int maxMu = std::max(sumMu, (int)prev->maxMu);

		int lb3 = sumLambda - minLambda;
		int ub3 = sumMu + Q - maxMu;
		int lb4 = next->sumGamma - next->minGamma;
		int ub4 = next->sumZeta + Q - next->maxZeta;
		//3.- Check if the insertion is feasible. This check is O(1)
		if (!(lb3 <= ub4 && lb4 <= ub3))
		{
			prev = next; pos++; continue;
		}

		bool hasZeroRec = RouteFeasibility::HasZeroHC(s.GetProb(),path);

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
		
		//if(mo.DeltaRec>0) 
			//mo.Show();
		
		moveVec.push_back(mo);
		
		/*printf("Feas insertion d:%d n:%d Path:\n",d->id,n->no);
		for(int k=0;k<path.size();k++)
			printf("%d-",path[k]->no);
		printf("\n");*/
		
		prev = next; pos++;
	}
}

void SeqInsertBRP_HC::Insert(Sol & s, bool show)
{
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
			printf("SeqInsetion with Continue-to-Next Recourse Inserting Node:%d ElapsedSeconds:%.1lf\n", n->id, elapsedSeconds);
		}
		std::vector<Move> moveVec;
		bool has_seen_empty_driver = false;
		for(int j=0;j<s.GetDriverCount();j++)
		{				
			Driver * d = s.GetDriver(j);
			int Q = d->capacity;
			int sumPosDmd = d->sumPosDmd;
			int sumNegDmd = d->sumNegDmd;
			
			if(n->demand > 0) sumPosDmd += n->demand;
			else sumNegDmd += n->demand;
			
			if(sumPosDmd > Q + d->sumWp + std::abs( sumNegDmd ) || std::abs( sumNegDmd ) > Q + d->sumWp + std::abs( sumNegDmd )) //Eqs (17)-(18)
				continue;				
			
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
			FillMoveVec_HC(s,n,d,moveVec);
			
			if(s.GetRouteLength( s.GetDriver(j)) == 0)
				has_seen_empty_driver = true;
		}
		
		std::sort(moveVec.begin(), moveVec.end());
		int bestRec = 9999;
		Move best;
		best.IsFeasible = true;
		best.DeltaCost = INFINITE;
		
		//Find the best delta Recourse of the moves
		for(int j=0;j<std::min(50,(int)moveVec.size());j++)
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
			
			int rec = mo.DeltaRec <= 0 ? 0 : RouteFeasibility::RecourseCost(s.GetProb(),path);
			
			mo.DeltaRec = mo.DeltaRec <= 0 ? mo.DeltaRec : rec - d->curRecourse; //DeltaRec can be negative due to a helpful insertion of a station in the middle of a route
			
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

				int nextRec = nextMo.DeltaRec <= 0 ? 0 : RouteFeasibility::RecourseCost(s.GetProb(), path);	
				
				nextMo.DeltaRec = nextMo.DeltaRec <= 0 ? nextMo.DeltaRec : rec - nextD->curRecourse; //DeltaRec can be negative due to a helpful insertion of a station in the middle of a route
				
				if(nextMo.DeltaDistance + nextMo.DeltaRec > best.DeltaCost)
					break;
			}
		}			
		
		best.from = NULL;
		//_insrmv.ApplyInsertMove(s, best);
		//s.Update(best.to);
		
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
		
		/*printf("Best move:\n");
		best.Show(); 
		printf("NodeInserted:%d Cost:%.2lf Unassigneds:%d\n",n->no,s.GetCost(),s.GetUnassignedCount());
		getchar();*/
	}
}