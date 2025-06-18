#ifndef _SOLUTION_H
#define _SOLUTION_H


#include "Constants.h"
#include "ProblemDefinition.h"
#include "CostFunctionBRP.h"
#include "NodeBRP.h"
#include "DriverBRP.h"
#include "RouteFeasibility.h"

#include <vector>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

class Sol
{
	public:
		Sol():show_output(true){}
		Sol(Prob * prob, CostFunctionBRP * cost_func):
										  Next(0),Prev(0),AssignTo(0),
										  RoutesLength(prob->GetDriverCount()),
										  _prob(prob),UnassignedCount(0),
										  Unassigneds(0),UnassignedIndex(0),
										  _cost_func(cost_func),show_output(true),_last_cost(0),_is_feasible(true), _total_distances(0)

		{
			for(int i = 0 ; i < _prob->GetNodeCount() ; i++)
			{
				Next.push_back(NULL);
				Prev.push_back(NULL);
				AssignTo.push_back(NULL);
				Unassigneds.push_back(NULL);
				UnassignedIndex.push_back(-1);
			}

			for(int i = 0 ; i < _prob->GetDriverCount() ; i++)
			{
				RoutesLength[i] = 0;
				Driver * d = GetDriver(i);
				Node * n1 = GetNode(d->StartNodeID);
				Node * n2 = GetNode(d->EndNodeID);

				Next[n1->id] = n2;
				Prev[n2->id] = n1;
				AssignTo[n1->id] = d;
				AssignTo[n2->id] = d;
			}
		}

		void AddNode()
		{
			Next.push_back(NULL);
			Prev.push_back(NULL);
			AssignTo.push_back(NULL);
			Unassigneds.push_back(NULL);
			UnassignedIndex.push_back(-1);
		}

		void AddDriver()
		{
			RoutesLength.push_back(0);
		}

		void InsertAfter(Node * n, Node * prev)
		{
			RoutesLength[ AssignTo[prev->id]->id ]++;
			AssignTo[n->id] = AssignTo[prev->id];
			Next[n->id] = Next[	prev->id ];
			Prev[n->id] = prev;
			if(Next[prev->id] != NULL)
				Prev[ Next[prev->id]->id ] = n;

			Next[prev->id] = n;
		}

		void Remove(Node * n)
		{
			RoutesLength[ AssignTo[n->id]->id ]--;
			if(Next[n->id] != NULL) Prev[ Next[n->id]->id ] = Prev[n->id];
			if(Prev[n->id] != NULL) Next[ Prev[n->id]->id ] = Next[n->id];
			AssignTo[n->id] = NULL;
		}

		void AddToUnassigneds(Node * n)
		{
			UnassignedIndex[n->id] = UnassignedCount;
			Unassigneds[UnassignedCount] = n;
			UnassignedCount++;
		}

		void RemoveFromUnassigneds(Node * n)
		{
			int ind = UnassignedIndex[n->id];
			//printf("RemoveFromUnassigneds:%d index:%d count:%d\n", n->id, ind,UnassignedCount);
			if(UnassignedCount > 1)
			{
				Node * rep = Unassigneds[UnassignedCount - 1];
				Unassigneds[ind] = rep;
				UnassignedIndex[ rep->id ] = ind;
			}

			Unassigneds[UnassignedCount - 1] = NULL;
			UnassignedCount--;
			UnassignedIndex[n->id] = -1;
		}
		void RemoveAndUnassign(Node * n)
		{
			Remove(n);
			AddToUnassigneds(n);
		}

		void PutAllNodesToUnassigned()
		{
			for(int i=0;i<GetCustomerCount();i++)
				AddToUnassigneds( GetCustomer(i));
		}

		void MakePath(int driver, std::vector<Node*> & path)
		{
			Driver * d = GetDriver(driver);
			Node * prev = GetNode(d->StartNodeID);
			for(size_t j=0;j<path.size();j++)
				//if((path[j]->type & NODE_TYPE_CUSTOMER) == NODE_TYPE_CUSTOMER)
				if(path[j]->type == NODE_TYPE_CUSTOMER)
				{
					if(AssignTo[ path[j]->id ] != NULL)
						Remove( path[j] );
					if( UnassignedIndex[ path[j]->id ] != -1)
						RemoveFromUnassigneds( path[j] );
					InsertAfter(path[j], prev);
					prev = path[j];
				}
		}

		void GetPath(Driver * d, std::vector<Node*> & path)
		{
			path.clear();
			Node * n = GetNode(d->StartNodeID);
			while(n != NULL)
			{
				path.push_back(n);
				n = Next[n->id];
			}
		}

		void Show()
		{
			int nbnonempty = 0;
			for(int i=0;i<GetDriverCount();i++)
				if(RoutesLength[i] >= 1)
					nbnonempty++;
			printf("Solution non-empty routes:%d routes:%d cost:%.4lf\n", nbnonempty,GetDriverCount(), _cost_func->GetCost(*this) );
			for(int i=0;i<GetDriverCount();i++)
				if(show_output && RoutesLength[i] >= 1)
					Show(GetDriver(i));

			if(show_output && GetUnassignedCount() >= 1)
			{
				printf("Unassigneds:");
				for(int i=0;i<GetUnassignedCount();i++)
					printf("%d ", GetUnassigned(i)->no);
				printf("\n");
			}
		}

		void Show(Driver * d)
		{
			if(_cost_func != NULL)
				_cost_func->Show(this, d);
		}

		Driver* GetAssignedTo(Node * n){ return AssignTo[n->id];}
		Driver* GetAssignedTo(int node_id){ return AssignTo[node_id];}

		int GetCustomerCount(){ return _prob->GetCustomerCount();}
		Node * GetCustomer(int i){ return _prob->GetCustomer(i);}

		int GetNodeCount(){ return _prob->GetNodeCount();}
		Node * GetNode(int i){ return _prob->GetNode(i);}

		int GetDriverCount(){ return _prob->GetDriverCount();}
		Driver * GetDriver(int i){ return _prob->GetDriver(i);}

		int GetUsedDriverCount()
		{
			int nb = 0;
			for(int i=0;i<GetDriverCount();i++)
				if( RoutesLength[ GetDriver(i)->id ] >= 1)
					nb++;
			return nb;
		}
		int GetUnassignedCount(){ return UnassignedCount;}
		Node * GetUnassigned(int i){ return Unassigneds[i];}

		double GetCost(){ _last_cost = _cost_func->GetCost(*this); return _last_cost;}
		double GetCost(Driver * d){ return _cost_func->GetCost(*this,d);}
		double GetCost(int i){ return _cost_func->GetCost(*this,GetDriver(i));}
		double GetLastCalculatedCost(){ return _last_cost; }
		CostFunctionBRP * GetCostFunction(){return _cost_func;}

		bool IsFeasible(){return _is_feasible;}
		void SetIsFeasible(bool f){_is_feasible = f;}

		void Update(){ _cost_func->Update(*this);}
		void Update(Driver * d){ _cost_func->Update(*this,d);}

		double GetTotalDistances(){return _total_distances;}
		void SetTotalDistances(double d){_total_distances = d;}

		double GetTotalRecourse(){return _total_recourse;}
		void SetTotalRecourse(double d){_total_recourse = d;}

		bool IsUnassigned(Node * n){return UnassignedIndex[n->id] != -1;}

		Prob * GetProblemDefinition(){return _prob;}
		Prob * GetProb(){return _prob;}

		double ** GetDistances(){ return _prob->GetDistances();}
		double GetDist(Node * n1, Node * n2){return _prob->GetDistances()[n1->distID][n2->distID];}
		int GetRouteLength(int i){return RoutesLength[i];}
		int GetRouteLength(Driver * d){return RoutesLength[d->id];}

		std::vector<Node*> Next;
		std::vector<Node*> Prev;
		std::vector<Driver*> AssignTo;
		std::vector<int> RoutesLength;//# of customers in each route

		Node * GetNext(Node * n){return Next[n->id];}
		Node * GetPrev(Node * n){return Prev[n->id];}

		/*=================================================*/
		/*========= Methods for the SLR instances =========*/
		/*=================================================*/
		
		//Sequential search for paths to merge
		void MergeAllPaths()
		{
			//This method requires a previous call to Update() to compute the distances of the drivers
			int initial_drvs = GetUsedDriverCount();
			std::vector<Driver*> drivers; drivers.reserve( GetUsedDriverCount() );
			for(int i = 0; i < GetDriverCount(); i++)
			{
				Driver* d = _prob->GetDriver(i);
				if(RoutesLength[i] && d->curDistance < Parameters::MaxRouteDistance())
					drivers.push_back( d );
					//d->Show();
			}					
			//printf("MergeAllPaths drvs to merge:%d\n",(int)drivers.size());
			for(size_t i = 0; i < drivers.size()-1; i+=2)
			{
				if (i + 1 >= drivers.size()) break; // Ensure there is a next driver available

				Driver* d1 = drivers[i];
				Driver* d2 = drivers[i+1];
				
				if(Parameters::RecoursePolicy() == 1  &&  d1->curDistance + d2->curDistance > Parameters::MaxRouteDistance())
					continue;
				if(Parameters::RecoursePolicy() == 2  &&  d1->curDistance + d1->curRecourse + d2->curDistance + d2->curRecourse > Parameters::MaxRouteDistance()) 	
					continue;
				
				//printf("Merging d%d dist:%.1lf with d%d dist:%.1lf ...\n",d1->id,d1->curDistance,d2->id,d2->curDistance);
				//Show(d1); Show(d2);
				Node* last_d1_node = Prev[ GetNode(d1->EndNodeID)->id ];
				
				// Traverse nodes of d2
				Node* n = GetNode(d2->StartNodeID);
				n = Next[n->id]; // Move to the first actual node in d2
				while(n->type != NODE_TYPE_END_DEPOT)
				{
					Node* next_in_d2 = Next[n->id];
					//Remove first
					Remove(n);
					// Insert n after last_d1_node
					InsertAfter(n, last_d1_node);
					// Update last_d1_node to n
					last_d1_node = n;
					// Move to the next node in d2
					n = next_in_d2;
				}
				
				Update(d1); Update(d2);
				if(d1->curRecourse > 9990)
					_is_feasible = false;
				/*printf("New d%d\n",d1->id);
				Show(d1);
				printf("New d%d\n",d2->id);
				Show(d2);
				getchar();*/
			}
			//printf("Initial Drvs:%d New Drivers:%d\n",initial_drvs, GetUsedDriverCount());
		}

		//Make the solution feasible by checking every route and removing infeasible customers until the route is feasible
		void MakeFeasible()
		{
			//printf("MakeFeasible() Drivers:%d\n",GetDriverCount());
			int cntr=0;
			for(int i=0;i<GetDriverCount();i++)
			{
				//RemoveInfeasibleCustomers(GetDriver(i));
				_cost_func->Update(*this,GetDriver(i));
				if(GetDriver(i)->nb_customers == 0) continue;
				//printf("Drv in MakeFeasible():\n");
				//GetDriver(i)->Show(); //getchar();
				//if(!GetDriver(i)->is_feasible && Parameters::RecoursePolicy()==1)
				//{
					RemoveInfeasibleCustomers_HC(GetDriver(i)); cntr++;
				//}else if( Parameters::RecoursePolicy()==2 )
					//RemoveInfeasibleCustomers_HR(GetDriver(i));
			}
			//printf("Infeasible Drivers:%d\n",cntr);
		}
		void RemoveInfeasibleCustomers_HR(Driver * d)
		{
			std::vector<Node*> path;
			Node * prev = GetNode(d->StartNodeID);
			path.push_back(prev);
			double travelledD = 0.0;

			// Calculate the total distance travelled and construct the path
			while (prev->type != NODE_TYPE_END_DEPOT)
			{
				Node * next = Next[prev->id];
				double dist = _prob->GetDist(prev, next);
				travelledD += dist;
				path.push_back(next);
				prev = next;
			}

			// Construct a vector of pairs (node, delta_distance) for infeasible customers
			std::vector<std::pair<Node*, double>> vec;
			for (size_t i = 1; i < path.size() - 1; i++) // No depots
			{
				double delta_distance = _prob->GetDist(path[i], path[0]) +
										_prob->GetDist(path[0], path[i + 1]) -
										_prob->GetDist(path[i], path[i + 1]);
				vec.push_back(std::make_pair( path[i], delta_distance ));
			}

			// Custom comparison function to sort by the second element of the pair (the double value)
			auto compare = [](const std::pair<Node*, double> &a, const std::pair<Node*, double> &b) {
				return a.second < b.second;
			};
			// Sort the vector based on the double value
			std::sort(vec.begin(), vec.end(), compare);

			double remaining_distance = Parameters::MaxRouteDistance() - travelledD;
			int min_vehicles_required = RouteFeasibility::GetDriverCount(_prob, path);

			// Remove infeasible customers until the remaining distance allows it and there are customers left
			std::vector<int> RmvCust; int cntr=0;
			while (min_vehicles_required > 1 && !vec.empty() && remaining_distance < vec.back().second)
			{
				int node_no = vec.back().first->no;
				RmvCust.push_back(node_no);
				RemoveAndUnassign(vec.back().first);
				vec.pop_back();

				// Recalculate path, travelled distance, and minimum vehicles required
				path.clear();
				prev = GetNode(d->StartNodeID);
				path.push_back(prev);
				travelledD = 0.0;
				while (prev->type != NODE_TYPE_END_DEPOT)
				{
					Node * next = Next[prev->id];
					if (next->no == node_no)
					{
						prev = next;
						continue;
					}
					double dist = _prob->GetDist(prev, next);
					travelledD += dist;
					path.push_back(next);
					prev = next;
				}
				min_vehicles_required = RouteFeasibility::GetDriverCount(_prob, path);
				remaining_distance = Parameters::MaxRouteDistance() - travelledD;
			}

			// Print removed customers
			/*if (!RmvCust.empty())
			{
				printf("RestockingRec recourse_policy:%d CustRmv:%d Customers:\n", Parameters::RecoursePolicy(), cntr);
				for (int i = 0; i < RmvCust.size(); i++)
				{
					printf("%d ", RmvCust[i]);
				}
				printf("\n");
			}*/
		}

		
		void RemoveInfeasibleCustomers_HC(Driver * d)
		{
			int Q = d->capacity;
			int sumLambda = 0; int minLambda = 0; int prevMinLambda = 0; // Store previous minLambda
			int sumMu = 0; int maxMu = 0; int prevMaxMu = 0; // Store previous maxMu
	 
			int cntr=0; std::vector<int> RmvCust;
			Node * prev = GetNode(d->StartNodeID);
			//printf("RmvInfCust d_id:%d Nodes:%d-",d->id,prev->no);
			while (prev->type != NODE_TYPE_END_DEPOT)
			{
				Node * next = Next[prev->id];
				//printf("%d-",next->no);
				int lambda_i = std::max(-Q, next->demand - next->w_plus);
				int mu_i = std::min(Q, next->demand + next->w_minus);
				
				// End load feasibility
				sumLambda += lambda_i;
				minLambda = std::min(sumLambda, minLambda);
				sumMu += mu_i;
				maxMu = std::max(sumMu, maxMu);
				int lb = sumLambda - minLambda;
				int ub = sumMu + Q - maxMu;
				if (lb > ub)
				{
					RemoveAndUnassign(next);
					RmvCust.push_back(next->no);
					sumLambda -= lambda_i;
					sumMu -= mu_i;
					// Restore minLambda and maxMu
					minLambda = prevMinLambda;
					maxMu = prevMaxMu;
					cntr++;
				}
				else
				{
					// Update prevMinLambda and prevMaxMu
					prevMinLambda = minLambda;
					prevMaxMu = maxMu;
				}
				prev = next;
			}
			if(RmvCust.size())
			{
				/*printf("Continue-to-Next RecoursePolicy:%d CustRmv:%d Customers:\n",Parameters::RecoursePolicy(),cntr);
				for(int i=0;i<RmvCust.size();i++)
				{
					printf("%d ",RmvCust[i]);
				}
				printf("\n");*/
				d->nb_customers -= RmvCust.size();
			}
			//getchar();
		}		
    
	private:
		Prob * _prob;
		int UnassignedCount;
		std::vector<Node*> Unassigneds;
		std::vector<int> UnassignedIndex;
		CostFunctionBRP * _cost_func;
		
		bool show_output;
		double _last_cost;
		bool _is_feasible;
		double _total_distances;
		double _total_recourse;
};

#endif
