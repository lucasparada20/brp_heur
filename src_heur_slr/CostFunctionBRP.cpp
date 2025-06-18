#include "CostFunctionBRP.h"
#include "RouteFeasibility.h"
#include "Solution.h"

double CostFunctionBRP::GetCost(Sol & s)
{
	double cost = 0;
	for(int i = 0 ; i < s.GetDriverCount() ; i++)
		cost += GetCost(s, s.GetDriver(i));
	return cost + s.GetUnassignedCount()*UNASSIGNED_COST;
}

double CostFunctionBRP::GetCost(Sol & s,Driver * d)
{
	double dist = 0;
	Node * prev = s.GetNode( d->StartNodeID );
	path.clear();
	while(prev->type != NODE_TYPE_END_DEPOT)
	{
		Node * next = s.Next[ prev->id ];
		dist += s.GetDist(prev,next);
		path.push_back(prev);
		prev = next;
	}
	path.push_back(prev);
	
	if (dist > Parameters::MaxRouteDistance())
		return INF_ROUTE_COST;

	if (Parameters::RecoursePolicy() == 1) {
		if (!RouteFeasibility::IsFeasible(s.GetProb(), path, false))
			return INF_ROUTE_COST;
		return dist + RouteFeasibility::RecourseCost(s.GetProb(), path);
	}
	
	bool isFeasible = RouteFeasibility::IsFeasible(s.GetProb(), path, false);
    bool hasZeroHC = RouteFeasibility::HasZeroHC(s.GetProb(), path);

    if (isFeasible && hasZeroHC)
        return dist;

    double DP = RouteFeasibility::RestockingRecourseCostWithMaxDistance(s.GetProb(), path);
    double HR = (DP < 9990) ? dist + DP : INF_ROUTE_COST;

    return HR;
}

void CostFunctionBRP::Update(Sol & s)
{
	double sumd = 0, sumr = 0;
	for(int i = 0 ; i < s.GetDriverCount() ; i++)
	{
		Driver * d = s.GetDriver(i);
		Update(s, d);
		sumd += d->curDistance;
		sumr += d->curRecourse;
	}
	s.SetTotalDistances(sumd);
	s.SetTotalRecourse(sumr);
}

void CostFunctionBRP::Update(Sol & s, Driver * d)
{
	d->curDistance = 0;
	d->sum_demand = 0;
	d->ResetFeasibilityQuantities();
	int Q = d->capacity; int nb_customers = 0;
	
	/*Keeping track for the feasibility check*/
	int sumLambda = 0; int sumMu = 0; int sumGamma = 0; int minGamma = 0;
	int minLambda = 0; int maxMu = 0; int sumZeta = 0; int maxZeta = 0;
	
	Node * prev = s.GetNode( d->StartNodeID );
	prev->ResetFeasibilityQuantities();
	path.clear();
	bool IsForwardFeasible = true; bool IsBackwardFeasible = true;
	//printf("Update() d:%d Nodes:%d-",d->id,prev->no);
	while(prev->type != NODE_TYPE_END_DEPOT)
	{
		Node * next = s.Next[ prev->id ];
		//printf("%d-",next->no);
		path.push_back(prev);
		d->curDistance += s.GetDist(prev,next);
		prev = next;	
		d->sum_demand += next->demand;
		
		/*-------Quantities for the stations-----*/
		if(next->type != NODE_TYPE_CUSTOMER) 
		{ 	
			//prev = next; 
			continue; 
		}
		next->ResetFeasibilityQuantities();
		nb_customers++;
		
		if(next->demand>0) d->sumPosDmd += next->demand;
		else d->sumNegDmd += next->demand;
		d->sumWp += next->w_plus; d->sumWm += next->w_minus;
		
		int lambda_i = std::max( -Q, next->demand - next->w_plus );
		int mu_i = std::min( Q, next->demand + next->w_minus );
		
		//End load feasibility
		sumLambda += lambda_i;
		minLambda = std::min( sumLambda, minLambda );
		sumMu += mu_i;
		maxMu = std::max( sumMu, maxMu );	
		next->sumLambda = sumLambda; next->sumMu = sumMu; 
		next->minLambda = minLambda; next->maxMu = maxMu; 
		
		int lb = sumLambda - minLambda;
		int ub = sumMu + Q - maxMu;
		if(IsForwardFeasible && lb <= ub) next->IsForwardFeasible = 1;
		else
		{
			next->IsForwardFeasible = 0; IsForwardFeasible = false;
		}
		/*--------------------------------------------------*/		
	}
	path.push_back(prev);
	//printf("\n");
	d->nb_customers = nb_customers;
		
	for(int i=path.size()-1;i>=0;i--)
	{
		if(path[i]->type != NODE_TYPE_CUSTOMER) continue;
		int gamma_i = std::min( Q, path[i]->demand + path[i]->w_minus );
		int zeta_i = std::max( -Q, path[i]->demand - path[i]->w_plus );
		//Start load feasibility
		sumGamma -= gamma_i;
		minGamma = std::min( sumGamma, minGamma );
		sumZeta -= zeta_i;
		maxZeta = std::max( sumZeta, maxZeta );
		path[i]->sumGamma = sumGamma; path[i]->minGamma = minGamma;
		path[i]->sumZeta = sumZeta; path[i]->maxZeta = maxZeta;

		int lb = sumGamma - minGamma;
		int ub = sumZeta + Q - maxZeta;
		if(IsBackwardFeasible && lb <= ub) path[i]->IsBackwardFeasible = 1;
		else
		{
			path[i]->IsBackwardFeasible = 0; IsBackwardFeasible = false;
		}			
		path[i]->Q = Q; // Doesn't do anything. Is just for the printf
		//path[i]->ShowFeasibilityQuantities(); getchar();
	}
	
	if(Parameters::RecoursePolicy() == 1)
		d->curRecourse = RouteFeasibility::RecourseCost(s.GetProb(), path);
	else if(Parameters::RecoursePolicy() == 2)
		d->curRecourse = RouteFeasibility::RestockingRecourseCostWithMaxDistance(s.GetProb(), path);
	
	if( path.size() > 2 && path[ path.size()-2 ]->IsForwardFeasible && path[ 1 ]->IsBackwardFeasible)
	{
		d->is_feasible = true;
		//path[ path.size()-2 ]->ShowFeasibilityQuantities(); getchar();
	} 
	if( path.size() > 2 && !path[ path.size()-2 ]->IsForwardFeasible && !path[ 1 ]->IsBackwardFeasible)
	{
		d->is_feasible = false; 
		/*for(int i=0;i<path.size();i++)
			path[i]->ShowFeasibilityQuantities();
		getchar();*/
	}	
	//printf("Drv in Update():\n");
	//d->Show();
}

void CostFunctionBRP::Show(Sol * s, Driver * d)
{
	double cost = GetCost(*s,d);
	printf("Route:%d Q:%d sumDmd:%d cost:%.2lf rec:%.2lf len:%d Stations:", d->id, d->capacity, d->sum_demand, d->curDistance, d->curRecourse, s->RoutesLength[d->id]);
	Node * cur = s->GetNode( d->StartNodeID );
	while(cur != NULL)
	{
		printf("%d-", cur->id);
		cur = s->Next[ cur->id ];
	}
	printf("\n");
}

void CostFunctionBRP::MakeFeasible(Sol* s, Driver * d)
{
	Parameters::RecoursePolicy() == 1 ? 
		s->RemoveInfeasibleCustomers_HC(d) : 
		s->RemoveInfeasibleCustomers_HR(d) ;
}
