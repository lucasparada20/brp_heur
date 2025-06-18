#include "InsRmvMethodBRP.h"
#include "Constants.h"
#include "RouteFeasibility.h"
#include "Parameters.h"
#include <algorithm>

void InsRmvMethodBRP::FillInsertionList(Sol & s, std::vector<Node*> & list)
{
	list.clear(); list.reserve( _prob->GetCustomerCount() );
	for(int i=0;i<s.GetUnassignedCount();i++)
		list.push_back(s.GetUnassigned(i));
}

void InsRmvMethodBRP::InsertCost(Sol & s, Node * n, Driver * d, Move & mo)
{
	//printf("InsertCost n:%d d:%d cap:%d\n", n->id, d->id, d->capacity);
	int Q = d->capacity;
	int sumPosDmd = d->sumPosDmd;
	int sumNegDmd = d->sumNegDmd;
	
	if(n->demand > 0) sumPosDmd += n->demand;
	else sumNegDmd += n->demand;
	
	if(sumPosDmd > Q + d->sumWp + std::abs( sumNegDmd ) || std::abs( sumNegDmd ) > Q + d->sumWp + std::abs( sumNegDmd )) //Eqs (17)-(18)
		return;
	
	mo.IsFeasible = false;
	mo.DeltaCost = INFINITE;
	mo.n = n;
	mo.to = d;
	
	std::vector<Node*> path_without_insertion;
	path.clear();
	Node * prev = s.GetNode( d->StartNodeID );
	path.push_back(prev); 
	path_without_insertion.push_back(prev);
	path.push_back(n);
	while(prev->type != NODE_TYPE_END_DEPOT)
	{
		Node * next = s.Next[ prev->id ];
		path.push_back(next);
		path_without_insertion.push_back(next);
		prev = next;
	}
	
	int rec_without_insertion = path_without_insertion[path_without_insertion.size() - 2]->IsForwardFeasible ? RouteFeasibility::RecourseCostAndStore(s.GetProb(), path_without_insertion) : 9999;
	int rec_to_compare = 9999;
			
	//INSERT COST FOR THE SBRP
	//Looping to search for the insertion move that respects distance constraint, is feasible, and has the better cost. The following 3 feasibility checks are made.
	//Route is of the form: d-1-2-...-prev-InsertNode-Insert+1-Insert+2-...-d
	//Input: In the Update() function, ALL forward and backward feasibility lb, ubs are computed and stored for all nodes in the route.
	//Check 1.- Check if the route is feasible up to prev (d-1-2-...-prev). This check is O(1).
	//Check 2.- Check if the route is BACKWARDS feasible from the next node after the insertion up to the end depot (Compute StartLoadWindow for path Insert+1-....-d). This check is O(1).
	//Check 3.- Check if the insertion is feasible (InsertNode). This check is O(1)
	// ---------------------------------------------------------------------------------------------------------------------------------------------------
	//TOTAL COMPLEXITY: 3 checks made, each one is O(1).
	
	n->ResetFeasibilityQuantities();
	int lambda_pos = std::max(-Q, n->demand - n->w_plus);
	int mu_pos = std::min(Q, n->demand + n->w_minus);
	int gamma_pos = std::min(Q, n->demand + n->w_minus);
	int zeta_pos = std::max(-Q, n->demand - n->w_plus);
	n->sumLambda = lambda_pos; n->minLambda = lambda_pos; n->sumMu = mu_pos; n->maxMu = mu_pos;
	n->sumGamma = -1*lambda_pos; n->minGamma = -1*lambda_pos; n->sumZeta = -1*zeta_pos; n->maxZeta = -1*zeta_pos;
	
	int pos = 0;
	prev = s.GetNode( d->StartNodeID );
	while(prev->type != NODE_TYPE_END_DEPOT)
	{
		Node * next = s.Next[ prev->id ];
		//printf("Prev:%d next:%d\n", prev->id, next->id);
		if(prev->type != NODE_TYPE_START_DEPOT)
			iter_swap(path.begin() + pos, path.begin() + pos + 1);
		
		double deltaDist = s.GetDist(prev,n) + s.GetDist(n,next) - s.GetDist(prev,next);
		if(d->curDistance + deltaDist > Parameters::MaxRouteDistance()) 
		{
			prev = next; pos++; continue;
		}
		
		std::vector<Node*> path_until_prev; std::vector<Node*> path_after_insertion;
		for(int i=0;i<path.size();i++)
		{
			if(n->no == path[i]->no) continue;
			if(i<pos+1) path_until_prev.push_back( path[i] );
			else path_after_insertion.push_back( path[i] );
		}	
		
		//1.- Check if the route is feasible up to prev. This check is O(1)
		if( prev->type ==NODE_TYPE_CUSTOMER && prev->IsForwardFeasible == 0)
		{
			prev = next; pos++; continue;
		}		
		
		//2.- Check if the route is BACKWARDS feasible from the next node after the insertion (Compute StartLoadWindow for path Insert+1-....-d). This check is O(n).
		//IMPROVED: In the Update() function the lb, ubs where compute for both forward and backward feasibility -> The check becomes O(1).
		if(path_after_insertion[0]->type == NODE_TYPE_CUSTOMER && path_after_insertion[0]->IsBackwardFeasible == 0)
		{	
			prev = next; pos++; continue;
		}
		
		int sumLambda = prev->sumLambda + lambda_pos;
		int minLambda = std::min( sumLambda, (int)prev->minLambda );
		int sumMu = prev->sumMu + mu_pos;
		int maxMu = std::max( sumMu, (int)prev->maxMu );

		int lb3 = sumLambda - minLambda;
		int ub3 = sumMu + Q - maxMu;
		int lb4 = next->sumGamma - next->minGamma;
		int ub4 = next->sumZeta + Q - next->maxZeta;
		
		//3.- Check if the insertion is feasible. This check is O(1)
		if(!(lb3 <= ub4 && lb4 <= ub3))
		{
			prev = next; pos++; continue;
		}
		double newcost = deltaDist;
		
		if(newcost < mo.DeltaCost)
		{
			int BigM = 9999;
			int rec = BigM;
			int insertionCost = BigM;
			
			if(path.size() == 3)
				rec = RouteFeasibility::RecourseCost(s.GetProb(),path); 
			else {
				Node * prev1 = path_without_insertion[ pos+1 ];
				//prev1->Show();
				if(prev1->type != NODE_TYPE_CUSTOMER)
					prev1->costs.resize(Q+1,0);
				//n->costs is std::vector<int>
				n->costs.clear(); n->costs.resize(Q+1,0);
				for(int q=0;q<=Q;q++)
				{
					int best = BigM;
					for(int u=0;u<=n->w_plus;u++)
						if(q+n->demand-u >= 0 && q+n->demand-u<=Q)
							best = std::min(best, u+prev1->costs[q+n->demand-u]);
							
					for(int u=0;u<=n->w_minus;u++)
						if(q+n->demand+u >= 0 && q+n->demand+u<=Q)
							best = std::min(best, u+prev1->costs[q+n->demand+u]);			
					n->costs[q] = best; // Store the cost in the node's costs vector
				}
				
				for(int q=0;q<=Q;q++)
					insertionCost = std::min(insertionCost , n->costs[q]);
				
				if(pos>0 && *std::min_element(n->costs.begin(),n->costs.end()) > rec_to_compare )
				{
					prev = next; pos++; continue;
				}
				
				std::vector<Node*> path1{path.begin(),path.begin()+pos+1};
				rec = RouteFeasibility::ComputeRecourseFromStartingCost(s.GetProb(),path1,n->costs);
				//To Dbg .....
				/*int rec1 = RouteFeasibility::RecourseCost(s.GetProb(),path);
				if( rec != rec1 )
				{
					printf("Rec of Path:%d PathW/oInsertion:%d pos:%d insertNode:%d\n",rec,rec1,pos,n->no);
					printf("Path:\n");
					for(int i=0;i<path.size();i++)
						printf("%d-",path[i]->no);
					printf("\n");
					printf("PathW/oInsertion:\n");
					for(int i=0;i<path_without_insertion.size();i++)
						printf("%d-",path_without_insertion[i]->no);
					printf("\n");
					printf("Path1:\n");
					for(int i=0;i<path1.size();i++)
						printf("%d-",path1[i]->no);
					printf("\n");				
					//getchar();
					//exit(1);
				}*/				
			}

			
			if(newcost + rec < mo.DeltaCost)
			{
				mo.DeltaDistance = deltaDist;
				mo.DeltaCost = newcost + rec;
				mo.IsFeasible = true;
				mo.move.prev = prev;
				
				/*rec_to_compare = rec;
				if(path.size()>3)
				{
					printf("Path with better move in InsRmvMethodBRP: n:%d Dist:%.1lf Rec:%d RecToCompare:%d\n",n->no,d->curDistance+deltaDist,rec,rec_to_compare);
					for(size_t i=0;i<path.size();i++)
						printf("%d-", path[i]->no);
					printf("\n");
					getchar();					
				}*/
			}
		}

		prev = next;
		pos++;
	}
	n->ResetFeasibilityQuantities();
}

void InsRmvMethodBRP::ApplyInsertMove(Sol & s, Move & m)
{
	if(m.from != NULL)
	{
		s.Remove(m.n);
		m.from->sum_demand -=  m.n->demand;

	}
	else if(s.IsUnassigned(m.n))
		s.RemoveFromUnassigneds(m.n);

	s.InsertAfter(m.n, m.move.prev);
	//opt2.Optimize(s, m.to);
}

void InsRmvMethodBRP::RemoveCost(Sol & s, Node * n, Move & m)
{
	m.IsFeasible = true;
	m.DeltaDistance = 0;
	m.n = n;
	m.to = NULL;
	m.move.prev = NULL;
	m.from = s.GetAssignedTo(n);
	if(m.from != NULL)
	{
		Node * prev = s.Prev[n->id];
		Node * next = s.Next[n->id];
		m.DeltaDistance = 	s.GetDist( prev,next) -
							s.GetDist( prev,n) -
		 					s.GetDist( n,next);
		m.move.prev = prev;
	}

	m.DeltaCost = m.DeltaDistance;
}
