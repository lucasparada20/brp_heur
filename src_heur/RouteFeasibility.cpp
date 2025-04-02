#include "RouteFeasibility.h"

bool RouteFeasibility::IsFeasible(Prob* prob, std::vector<Node*> & path) 
{
	int Q = prob->GetDriver(0)->capacity;
	int min_lambda = 0;
	int sum_lambda = 0;
	int max_mu = 0;
	int sum_mu = 0;
	for (size_t i=0;i<path.size();i++)
	{
		Node* n = path[i];
		if(n->type != NODE_TYPE_CUSTOMER) continue;//ignore depots

		int lambda_i = std::max(-Q, n->demand);
		int mu_i = std::min(Q, n->demand);
		sum_lambda += lambda_i;
		min_lambda = std::min(sum_lambda, min_lambda);
		sum_mu += mu_i;
		max_mu = std::max(sum_mu, max_mu);

		int lb = sum_lambda - min_lambda;
		int ub = sum_mu + Q - max_mu;
		if(lb > ub)
			return false;
	}
	return true;
}

int RouteFeasibility::GetDriverCount(Prob* prob, std::vector<Node*>& stations)
{
	int nb_drivers=1;
	int Q = prob->GetDriver(0)->capacity;
	int dmd = 0;
	int lb_left = 0, ub_left = 0, lb_right = 0, ub_right = 0;
	for (size_t l = 0; l < stations.size(); l++)
	{
		if(stations[l]->type != NODE_TYPE_CUSTOMER) continue;
		Node * n = stations[l];
		
		//n->Show();
		dmd += n->demand;

		if(dmd > 0) { lb_left += dmd; ub_left += dmd; }

		if(dmd < 0) { lb_right -= dmd; ub_right -= dmd; }
	}

	while(ub_left+Q < lb_right || lb_left > ub_right+Q)
	{
		Q += prob->GetDriver(0)->capacity;
		nb_drivers++;
	}

	return nb_drivers;
}