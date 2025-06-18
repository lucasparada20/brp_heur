#include "RouteFeasibility.h"

double RouteFeasibility::LowerBound(Prob* prob, std::vector<Node*>& stations)
{
	int nb_iter = 1;
	int Q = prob->GetDriver(0)->capacity;
	int dmds = 0;
	int lb_left = 0, ub_left = 0, lb_right = 0, ub_right = 0;
	for (int l = 0; l < stations.size(); l++)
	{
		if(stations[l]->type != NODE_TYPE_CUSTOMER) continue;

		Node * n = stations[l];
		int dmd = n->demand;
		dmds += dmd;

		if(dmd > 0) { lb_left += dmd; ub_left += dmd; }
		ub_left += n->w_minus;

		if(dmd < 0) { lb_right -= dmd; ub_right -= dmd; }
		ub_right += n->w_plus;
	}

	//printf("ub_left:%d < lb_right:%d || lb_left:%d > ub_right:%d\n", ub_left, lb_right,lb_left , ub_right);
	if(ub_left+Q  < lb_right || lb_left > ub_right+Q)
		return 9999999999;
	else
		return std::max(0, std::abs(dmds) - Q);

}

bool RouteFeasibility::HasZeroHC(Prob* prob, std::vector<Node*>& path)
{
	int Q = prob->GetDriver(0)->capacity;
	bool hasZeroRec = true;
	int L = 0; int q = 0;
	for(int i=0;i<path.size();i++)
	{
		Node* n = path[i];
		if(n->type != NODE_TYPE_CUSTOMER) continue;
		q += n->demand;
		if( q < 0 )
		{
			L += std::abs(q);
			q = 0;
		}
		if( q > Q || L > Q)
		{
			hasZeroRec = false; break;
		}
	}
	return hasZeroRec;
}

bool RouteFeasibility::StartLoadIsFeasible(Prob* prob, std::vector<Node*> & path, bool show) 
{
	int Q = prob->GetDriver(0)->capacity;
	int min_gamma = 0;
	int sum_gamma = 0;
	int max_zeta = 0;
	int sum_zeta = 0;
	//printf("Path in RouteFeasibility:\n");
	for (size_t i=path.size()-1;i>0;i--)
	{
		Node* n = path[i];
		if(n->type != NODE_TYPE_CUSTOMER) continue;//ignore depots

		int gamma_i = std::min( Q, path[i]->demand + path[i]->w_minus );
		int zeta_i = std::max( -Q, path[i]->demand - path[i]->w_plus );
		
		//Start load feasibility
		sum_gamma -= gamma_i;
		min_gamma = std::min( sum_gamma, min_gamma );
		sum_zeta -= zeta_i;
		max_zeta = std::max( sum_zeta, max_zeta );

		int lb = sum_gamma - min_gamma;
		int ub = sum_zeta + Q - max_zeta;
		
		if(show) printf("StartLoad Lb>Ub?%d no:%d lb:%d ub:%d dmd:%d wp:%d wm:%d gamma:%d zeta:%d sum_gamma:%d sum_zeta:%d min_gamma:%d max_zeta:%d\n",lb>ub,n->no,lb,ub,n->demand,n->w_plus,n->w_minus,gamma_i,zeta_i,sum_gamma,sum_zeta, min_gamma, max_zeta);
		if(lb > ub)
			return false;
	}
	return true;
}

//There are infeasible routes that are still being missed by this procedure. Maybe intersect with StartLoadIsFeasible() ???
bool RouteFeasibility::IsRestockFeasible(Prob* prob, std::vector<Node*> & path, bool show)
{
	int Q = prob->GetDriver(0)->capacity;
	int min_lambda = 0; int sum_lambda = 0; 
	int max_mu = 0; int sum_mu = 0;
	if(show)
		printf("Path in IsRestockFeasible?\n");
	double travelledD = 0.0; double remain_dist = Parameters::MaxRouteDistance(); double restock_dist = 0.0;
	for (size_t i=0;i<path.size();i++)
	{
		Node* n = path[i];
		if(n->type != NODE_TYPE_CUSTOMER) continue;//ignore depots
		
		travelledD += prob->GetDist(path[i-1],path[i]);
		remain_dist -= prob->GetDist(path[i-1],path[i]);
		if(i>1)
			restock_dist = prob->GetDist(path[i-1], path[0]) + prob->GetDist(path[0], path[i]) - prob->GetDist(path[i-1], path[i]);
		
		int lambda_i = std::max(-Q, n->demand - n->w_plus);
		int mu_i = std::min(Q, n->demand + n->w_minus);
		sum_lambda += lambda_i;
		min_lambda = std::min(sum_lambda, min_lambda);
		sum_mu += mu_i;
		max_mu = std::max(sum_mu, max_mu);

		int lb = sum_lambda - min_lambda;
		int ub = sum_mu + Q - max_mu;
		
		if(show && i>1) printf("EndLoad+Restock Lb>Ub?%d remainDist-Restock<0?%d remainDist:%.1lf restockDist:%.1lf no:%d lb:%d ub:%d dmd:%d wp:%d wm:%d lambda:%d mu:%d sum_lambda:%d sum_mu:%d min_lambda:%d max_mu:%d\n",lb>ub,remain_dist-restock_dist < 0, remain_dist,restock_dist,n->no,lb,ub,n->demand,n->w_plus,n->w_minus,lambda_i,mu_i,sum_lambda,sum_mu, min_lambda, max_mu);
		
		if(lb > ub && remain_dist + restock_dist >= Parameters::MaxRouteDistance())
			return false;
			
	}
	return true;	
}

bool RouteFeasibility::IsFeasible(Prob* prob, std::vector<Node*> & path, bool show) 
{
	int Q = prob->GetDriver(0)->capacity;
	int min_lambda = 0; int sum_lambda = 0; 
	int max_mu = 0; int sum_mu = 0;
	if(show)
		printf("Path in IsFeasible?\n");
	for (size_t i=0;i<path.size();i++)
	{
		Node* n = path[i];
		if(n->type != NODE_TYPE_CUSTOMER) continue;//ignore depots

		int lambda_i = std::max(-Q, n->demand - n->w_plus);
		int mu_i = std::min(Q, n->demand + n->w_minus);
		sum_lambda += lambda_i;
		min_lambda = std::min(sum_lambda, min_lambda);
		sum_mu += mu_i;
		max_mu = std::max(sum_mu, max_mu);

		int lb = sum_lambda - min_lambda;
		int ub = sum_mu + Q - max_mu;
		
		if(show) printf("EndLoad Lb>Ub?%d no:%d lb:%d ub:%d dmd:%d wp:%d wm:%d lambda:%d mu:%d sum_lambda:%d sum_mu:%d min_lambda:%d max_mu:%d\n",lb>ub,n->no,lb,ub,n->demand,n->w_plus,n->w_minus,lambda_i,mu_i,sum_lambda,sum_mu, min_lambda, max_mu);
		if(lb > ub)
			return false;
	}
	return true;
}

int RouteFeasibility::ComputeRecourseFromStartingCost(Prob* prob, std::vector<Node*>& path, std::vector<int> & input_costs)
{
    double cost = 0;
    int Q = prob->GetDriver(0)->capacity;
    int BigM = 9999;
    int* prev = new int[Q + 1];
    int* next = new int[Q + 1];

    // Initialize prev with input_costs
	//printf("size of input_costs:%d Q+1:%d\n",(int)input_costs.size(),Q+1);
    std::copy(input_costs.begin(), input_costs.end(), prev);

    for (int q = 0; q <= Q; q++)
        next[q] = 0;

    for (int i = path.size() - 1; i >= 1; i--)
    {
        if (path[i]->type != NODE_TYPE_CUSTOMER) continue;

        Node* n = path[i];
        n->costs.clear(); // Clear previous costs
        for (int q = 0; q <= Q; q++)
        {
            int best = BigM;
            for (int u = 0; u <= n->w_plus; u++)
                if (q + n->demand - u >= 0 && q + n->demand - u <= Q)
                    best = std::min(best, u + prev[q + n->demand - u]);

            for (int u = 0; u <= n->w_minus; u++)
                if (q + n->demand + u >= 0 && q + n->demand + u <= Q)
                    best = std::min(best, u + prev[q + n->demand + u]);

            n->costs.push_back(best); // Store the cost in the node's costs vector
            next[q] = best;
        }

        int* temp = next;
        next = prev;
        prev = temp;
    }

    int cost2 = BigM;
    for (int q = 0; q <= Q; q++)
        cost2 = std::min(cost2, prev[q]);
    cost += cost2;

    delete[] prev;
    delete[] next;
    return cost;
}

double RouteFeasibility::RecourseCostAndStore(Prob* prob, std::vector<Node*>& path)
{
    double cost = 0;
    int Q = prob->GetDriver(0)->capacity;
    int BigM = 9999;
    int * prev = new int[Q+1];
    int * next = new int[Q+1];

    for(int q=0;q<=Q;q++)
        next[q] = prev[q] = 0;

    for(int i=path.size()-2;i>=1;i--)
    {
        Node * n = path[i];
        n->costs.clear(); // Clear previous costs
        for(int q=0;q<=Q;q++)
        {
            int best = BigM;
            for(int u=0;u<=n->w_plus;u++)
                if(q+n->demand-u >= 0 && q+n->demand-u<=Q)
                    best = std::min(best, u+prev[q+n->demand-u]);
                    
            for(int u=0;u<=n->w_minus;u++)
                if(q+n->demand+u >= 0 && q+n->demand+u<=Q)
                    best = std::min(best, u+prev[q+n->demand+u]);
                    
            n->costs.push_back(best); // Store the cost in the node's costs vector
            next[q] = best;
        }
        
        int * temp = next;
        next = prev;
        prev = temp;
    }

    int cost2 = BigM;
    for(int q=0;q<=Q;q++)
        cost2 = std::min(cost2 , prev[q]);
    cost += cost2;

    delete [] prev;
    delete [] next;
    return cost;
}

double RouteFeasibility::RecourseCost(Prob* prob, std::vector<Node*>& path)
{
	double cost = 0;
	int Q = prob->GetDriver(0)->capacity;
	int BigM = 9999;
	int * prev = new int[Q+1];
	int * next = new int[Q+1];
	
	for(int q=0;q<=Q;q++)
		next[q] = prev[q] = 0;

	for(int i=path.size()-2;i>=1;i--)
	{
		Node * n = path[i];
		for(int q=0;q<=Q;q++)
		{
			int best = BigM;
			for(int u=0;u<=n->w_plus;u++)
				if(q+n->demand-u >= 0 && q+n->demand-u<=Q)
					best = std::min(best, u+prev[q+n->demand-u]);
					
			for(int u=0;u<=n->w_minus;u++)
				if(q+n->demand+u >= 0 && q+n->demand+u<=Q)
					best = std::min(best, u+prev[q+n->demand+u]);
					
			next[q] = best;
			
			//printf("ContinueRec Node:%d q:%d Best Cost:%d\n", n->no, best, q);
			//getchar();
		}
		
		int * temp = next;
		next = prev;
		prev = temp;		
	}

	int cost2 = BigM;
	for(int q=0;q<=Q;q++)
		cost2 = std::min(cost2 , prev[q]);
	cost += cost2;
	//printf("ContinueRec cost:%d\n", cost2);
	//getchar();	
	
	delete [] prev;
	delete [] next;
	return Parameters::GetDelta() * cost;
}

//OG
double RouteFeasibility::RestockingRecourseCostWithMaxDistance(Prob* prob, std::vector<Node*>& path)
{
    int Q = prob->GetDriver(0)->capacity;
    double D = Parameters::MaxRouteDistance();
    std::vector<std::vector<std::vector<double>>> matrix3D; 
    matrix3D.resize(path.size());

	double travelledD = 0;
	for (int i = 1; i < path.size(); i++)
		travelledD += prob->GetDist(path[i-1],path[i]);
	
    for (int i = 0; i < path.size(); i++) {
		matrix3D[i].resize(Q + 1);
        for (int q = 0; q <= Q; q++)
			matrix3D[i][q].resize(path.size()+1, -1);
    }
	
    double cost = 9999.0; 
	/*printf("\nBeginning DP Computation Path:\n");
	for(int i=0;i<path.size();i++)
		printf("%d->",path[i]->no);
    printf("\n");*/
	double UB = 9999.0; double cumul_cost = 0.0;
	for (int q = 0; q <= Q; q++) 
    {
        //cost = std::min(cost, GetCostRecursive(1, q, D - travelledD, UB, cumul_cost, prob, matrix3D, path));
		cost = std::min(cost, GetCostRecursive(1, q, D - travelledD, prob, matrix3D, path));
    }
    return cost;
}

//With UB cost to check
double RouteFeasibility::GetCostRecursive(int k, int x_k, double d_k, double & UB, double cumul_cost, Prob* prob, std::vector<std::vector<std::vector<double>>> & m, std::vector<Node*> path)
{
	if(cumul_cost >= UB) return 9999.0;

	int Q = prob->GetDriver(0)->capacity;

	//Base cases
	if (k == path.size() - 1) //Put a 0 once you arrive to the ending depot
	{
		UB = std::min(UB, cumul_cost);
		return 0.0;
	}

	std::vector<double> distances;
	for(int i=k;i<path.size()-1;i++)
		distances.push_back( prob->GetDist(path[i], path[0]) + prob->GetDist(path[0], path[i+1]) - prob->GetDist(path[i], path[i + 1]) );
	std::sort(distances.begin(),distances.end()); // Sorting from smallest to largest

	double sum_d = 0.0; int kp_obj = 0;
	for(int i=0;i<distances.size();i++)
		if(sum_d + distances[i] <= d_k)
		{
			sum_d += distances[i]; kp_obj++;
		}
		else
			break;

	if ( m[k][x_k][kp_obj] >= 0 )
		return m[k][x_k][kp_obj];

	double HC = 9999.0;
	Node * n = path[k];

	bool hasZeroRec = true;
	int L = 0; int q = 0;
	for(int i=k;i<path.size();i++)
	{
		Node* another_n = path[i];
		if(another_n->type != NODE_TYPE_CUSTOMER) continue;
		q += another_n->demand;
		if( q < 0 )
		{
			L += std::abs(q);
			q = 0;
		}
		if( q > Q || L > Q)
		{
			hasZeroRec = false; break;
		}
	}
	if(hasZeroRec)
		HC = 0.0;
	else
	{
		for (int wp = 0; wp <= n->w_plus; wp++)
			if (x_k + n->demand - wp >= 0 && x_k + n->demand - wp <= Q)
			{
				double cost = Parameters::GetDelta() * wp;
				if(cumul_cost + cost >= UB) continue;

				cost += GetCostRecursive(k+1, x_k + n->demand - wp, d_k, UB, cumul_cost + cost, prob, m, path );
				HC = std::min(HC, cost);
			}

		for (int wm = 0; wm <= n->w_minus; wm++)
			if (x_k + n->demand + wm >= 0 && x_k + n->demand + wm <= Q)
			{
				double cost = Parameters::GetDelta() * wm;
				if(cumul_cost + cost >= UB) continue;

				cost += GetCostRecursive(k+1, x_k + n->demand + wm, d_k, UB, cumul_cost + cost, prob, m, path );
				HC = std::min(HC, cost);
			}
	}

	double HR1 = 9999.0; double HR2 = 9999.0;
	double r_k = prob->GetDist(path[k], path[0]) + prob->GetDist(path[0], path[k+1]) - prob->GetDist(path[k], path[k + 1]);

	if(d_k < r_k || HC < r_k)
	{
		m[k][x_k][kp_obj] = HC;
		return HC;
	}

  for (int wp = 0; wp <= n->w_plus; wp++)
		if (x_k + n->demand - wp >= 0 && x_k + n->demand - wp <= Q)
			HR1 = std::min(HR1, (double)Parameters::GetDelta() * wp);

	for (int wm = 0; wm <= n->w_minus; wm++)
		if (x_k + n->demand + wm >= 0 && x_k + n->demand + wm <= Q)
			HR1 = std::min(HR1, (double)Parameters::GetDelta() * wm);

	if(HC < r_k + HR1)
	{
		m[k][x_k][kp_obj] = HC;
		return HC;
	}
	if(cumul_cost + r_k + HR1 > UB) return 9999.0;

	for(int y=0; y<=Q; y++)
		HR2 = std::min( HR2, GetCostRecursive( k+1, y, d_k - r_k,UB, cumul_cost + r_k, prob, m, path ) );

	m[k][x_k][kp_obj] = std::min( HC, r_k + HR1 + HR2 );
	return m[k][x_k][kp_obj];
}


//OG
double RouteFeasibility::GetCostRecursive(int k, int x_k, double d_k, Prob* prob, std::vector<std::vector<std::vector<double>>> & m, std::vector<Node*> path)
{
	int Q = prob->GetDriver(0)->capacity;

	//Base cases
	if (k == path.size() - 1) //Put a 0 once you arrive to the ending depot
		return 0.0; 
         
	std::vector<double> distances;
	for(int i=k;i<path.size()-1;i++)
		distances.push_back( prob->GetDist(path[i], path[0]) + prob->GetDist(path[0], path[i+1]) - prob->GetDist(path[i], path[i + 1]) ); 
	std::sort(distances.begin(),distances.end()); // Sorting from smallest to largest
	
	double sum_d = 0.0; int kp_obj = 0;
	for(int i=0;i<distances.size();i++)
	{
		if(sum_d + distances[i] <= d_k)
		{
			sum_d += distances[i]; kp_obj++;
		} else {
			break;
		}
	}
	
	if ( m[k][x_k][kp_obj] >= 0 )
		return m[k][x_k][kp_obj];
    
	double HC = 9999.0;
	Node * n = path[k]; 

	bool hasZeroRec = true;
	int L = 0; int q = 0;
	for(int i=k;i<path.size();i++)
	{
		Node* another_n = path[i];
		if(another_n->type != NODE_TYPE_CUSTOMER) continue;
		q += another_n->demand;
		if( q < 0 )
		{
			L += std::abs(q);
			q = 0;
		}
		if( q > Q || L > Q)
		{
			hasZeroRec = false; break;
		}
	}
	if(hasZeroRec)
		HC = 0.0;
	else {
		for (int wp = 0; wp <= n->w_plus; wp++) {
			if (x_k + n->demand - wp >= 0 && x_k + n->demand - wp <= Q) {
				double cost = Parameters::GetDelta() * wp + GetCostRecursive(k+1, x_k + n->demand - wp, d_k, prob, m, path );
				if (cost <  HC) {
					HC = cost;
					//printf("k:%d x_k:%d d_k:%.1lf wp:%d HC:%.1lf\n",no,q,d,wp,HC);
				}
			}
		}
		for (int wm = 0; wm <= n->w_minus; wm++) {
			if (x_k + n->demand + wm >= 0 && x_k + n->demand + wm <= Q) {
				double cost = Parameters::GetDelta() * wm + GetCostRecursive(k+1, x_k + n->demand + wm, d_k, prob, m, path );
				if (cost < HC) {
					HC = cost;
					//printf("k:%d x_k:%d d_k:%.1lf wm:%d HC:%.1lf\n",no,q,d,wm,HC);
				}
			}
		}		
	}
		
	double HR1 = 9999.0; double HR2 = 9999.0;
	double r_k = prob->GetDist(path[k], path[0]) + prob->GetDist(path[0], path[k+1]) - prob->GetDist(path[k], path[k + 1]);
	
	if(d_k < r_k || HC < r_k)
	{
		m[k][x_k][kp_obj] = HC;
		return HC;
	} 
	
    for (int wp = 0; wp <= n->w_plus; wp++) 
		if (x_k + n->demand - wp >= 0 && x_k + n->demand - wp <= Q) 
			if (Parameters::GetDelta() * wp <  HR1) 
				HR1 = Parameters::GetDelta() * wp;
			//See if a brake can be placed here
				//printf("k:%d x_k:%d d_k:%.1lf wp:%d HC:%.1lf\n",no,q,d,wp,HR);
		
	for (int wm = 0; wm <= n->w_minus; wm++) 
		if (x_k + n->demand + wm >= 0 && x_k + n->demand + wm <= Q) 
			if (Parameters::GetDelta() * wm < HR1) 
				HR1 = Parameters::GetDelta() * wm;
				//printf("k:%d x_k:%d d_k:%.1lf wm:%d HC:%.1lf\n",no,q,d,wm,HR);
	
	if( HC < r_k + HR1)
	{
		m[k][x_k][kp_obj] = HC;
		return HC;
	}
	
	for(int y=0; y<=Q; y++)
		HR2 = std::min( HR2, GetCostRecursive( k+1, y, d_k - r_k, prob, m, path ) );
	
	m[k][x_k][kp_obj] = std::min( HC, r_k + HR1 + HR2 );
	//m[k][x_k][kp_obj] = HC;

	return m[k][x_k][kp_obj];
}


int RouteFeasibility::GetDriverCount(Prob* prob, std::vector<Node*>& path)
{
	int Q = prob->GetDriver(0)->capacity;
	//Quantities to compute ...
	int nb_drivers = 1;
	int sum_dmd = 0; 
	int lb_left = 0; int ub_left = 0;
	int lb_right = 0; int ub_right = 0;
	for (int i = 0; i < path.size(); i++)
	{
		if(path[i]->type != NODE_TYPE_CUSTOMER) continue;

		Node * n = path[i];
		sum_dmd += n->demand;
		
		if(n->demand > 0) { lb_left += n->demand; ub_left += n->demand; }
		ub_left += n->w_minus;

		if(n->demand < 0) { lb_right -= n->demand; ub_right -= n->demand; }
		ub_right += n->w_plus;
	}
	//get the minimum number of vehicles
	while(ub_left + Q*nb_drivers < lb_right || lb_left > ub_right + Q*nb_drivers)
		nb_drivers++;
		
	return nb_drivers;
}





