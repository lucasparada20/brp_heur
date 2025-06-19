#include "LoadBRP.h"
#include "ProblemDefinition.h"
#include "Solution.h"
#include "CostFunctionBRP.h"
#include "InsRmvMethodBRP.h"
#include "AlnsBRP.h"
#include <fstream>
#include <iomanip>
#include "RouteFeasibility.h" // For testing ...

int main(int argc, char ** argv)
{
	Parameters param;
	param.Read(argc,argv);
	
	bool printing_output = false;
	//printing_output = param.SetCityName(Parameters::GetInstanceFileName()); //Comment this line to avoid saving the output in files ...
	
	Prob pr;
	LoadBRP load;
	
	load.Load_brp_instance(pr,Parameters::GetInstanceFileName());	
	
	CostFunctionBRP cost_func;
	
	Sol sol(&pr,&cost_func);
	sol.PutAllNodesToUnassigned();

	//Testing the optimal sol in ReggioEmilia10 and ReggioEmilia20
	/*std::vector<Node*> r0;
	r0.push_back(pr.GetCustomer(1)); r0.push_back(pr.GetCustomer(5)); r0.push_back(pr.GetCustomer(12)); r0.push_back(pr.GetCustomer(4)); r0.push_back(pr.GetCustomer(7)); r0.push_back(pr.GetCustomer(6));
	r0.insert(r0.begin(), pr.GetNode( pr.GetCustomerCount() ));
	r0.push_back( pr.GetNode( pr.GetCustomerCount()+1 ) );
	
	std::vector<Node*> r1;
	r1.push_back(pr.GetCustomer(0)); r1.push_back(pr.GetCustomer(2)); r1.push_back(pr.GetCustomer(9)); r1.push_back(pr.GetCustomer(3)); r1.push_back(pr.GetCustomer(8)); r1.push_back(pr.GetCustomer(10));
	r1.insert(r1.begin(), pr.GetNode( pr.GetCustomerCount() ));
	r1.push_back( pr.GetNode( pr.GetCustomerCount()+1 ) );
	
	std::vector<Node*> r2;
	r2.push_back(pr.GetCustomer(11));
	r2.insert(r2.begin(), pr.GetNode( pr.GetCustomerCount() ));
	r2.push_back( pr.GetNode( pr.GetCustomerCount()+1 ) );
	
	std::vector<Node*> r3;
	r3.push_back(pr.GetCustomer(12)); r3.push_back(pr.GetCustomer(1)); r3.push_back(pr.GetCustomer(9)); r3.push_back(pr.GetCustomer(3)); r3.push_back(pr.GetCustomer(5)); r3.push_back(pr.GetCustomer(2)); r3.push_back(pr.GetCustomer(0)); r3.push_back(pr.GetCustomer(4)); r3.push_back(pr.GetCustomer(7)); r3.push_back(pr.GetCustomer(6));
	r3.insert(r3.begin(), pr.GetNode( pr.GetCustomerCount() ));
	r3.push_back( pr.GetNode( pr.GetCustomerCount()+1 ) );
	
	std::vector<Node*> r4;
	r4.push_back(pr.GetCustomer(8)); r4.push_back(pr.GetCustomer(10)); r4.push_back(pr.GetCustomer(11));
	r4.insert(r4.begin(), pr.GetNode( pr.GetCustomerCount() ));
	r4.push_back( pr.GetNode( pr.GetCustomerCount()+1 ) );
	
	Sol solReggio10(&pr,&cost_func);
	solReggio10.PutAllNodesToUnassigned();
	
	Sol solReggio20(&pr,&cost_func);
	solReggio20.PutAllNodesToUnassigned();

	solReggio10.MakePath(0,r0);
	solReggio10.MakePath(1,r1);
	solReggio10.MakePath(2,r2);
	
	solReggio20.MakePath(0,r3);
	solReggio20.MakePath(1,r4);
	
	printf("Reggio10 optimal solution:\n");
	cost_func.Update(solReggio10);
	solReggio10.Show();
	
	printf("Reggio20 optimal solution:\n");
	cost_func.Update(solReggio20);
	solReggio20.Show();
	
	printf("r0 feas?%d drvCount:%d\n",RouteFeasibility::IsFeasible(&pr,r0),RouteFeasibility::GetDriverCount(&pr,r0));
	printf("r1 feas?%d drvCount:%d\n",RouteFeasibility::IsFeasible(&pr,r1),RouteFeasibility::GetDriverCount(&pr,r1));
	printf("r2 feas?%d drvCount:%d\n",RouteFeasibility::IsFeasible(&pr,r2),RouteFeasibility::GetDriverCount(&pr,r2));
	printf("r3 feas?%d drvCount:%d\n",RouteFeasibility::IsFeasible(&pr,r3),RouteFeasibility::GetDriverCount(&pr,r3));
	printf("r4 feas?%d drvCount:%d\n",RouteFeasibility::IsFeasible(&pr,r4),RouteFeasibility::GetDriverCount(&pr,r4));
	
	exit(1);*/
	
	InsRmvMethodBRP method(pr);
	SequentialInsertionBRP seq(method);
	//----------------------------------
	RemoveRandomBRP random_remove;
	RelatednessRemoveBRP related_remove(pr.GetDistances());
	
	seq.Insert(sol); //Sequential insertion of nodes to build an initial solution and store in sol 

	ALNS alns;
	//----------------------------------
	alns.AddInsertOperator(&seq);
	//----------------------------------
	alns.AddRemoveOperator(&random_remove);
	alns.AddRemoveOperator(&related_remove);
	//----------------------------------
	
	//Optimize
	alns.SetTemperatureIterInit(0);
	alns.SetTemperature(0.9995);
	alns.SetIterationCount( Parameters::GetIterations() );
	alns.SetAcceptationGap( 1.1 );
	
	clock_t start_time = clock();
	for(int i=0;i<10;i++)
	{
		Sol s = sol;
		seq.Insert(s);
		alns.Optimize(sol);

		if(sol.GetCost() > s.GetCost())
			sol = s;
	}
	//BestSolutionList best_sol_list(&pr,100);
	//alns.Optimize(sol,&best_sol_list);
	//for(int k=0;k<best_sol_list.GetSolutionCount();k++)
	//{
		//printf("Alns solution:%d\n",k);
		//Sol& s1 = *best_sol_list.GetSolution(k);
		//s1.Show();
	//}	
	
	clock_t end_time = clock();
	
	double elapsed_seconds = (double) (end_time - start_time) / CLOCKS_PER_SEC;
	
	sol.Update();
	sol.Show();
	
	double heur_ub = sol.GetTotalDistances();
	int heur_nb_drivers = sol.GetUsedDriverCount();
	printf("time:%.2lf\n", elapsed_seconds);
	printf("UB Heur:%.3lf drv:%d\n", heur_ub, heur_nb_drivers);	
	
	if(printing_output)
	{
		std::ofstream re_file_name(Parameters::GetReFileName()); std::ofstream out_file_name(Parameters::GetOutputFileName());
		if(!re_file_name.is_open() || !out_file_name.is_open())
		{
			std::cout << Parameters::GetReFileName() << " " << Parameters::GetOutputFileName() << std::endl;
			std::cout << "Could not open re or out file. Exiting ..." << std::endl; 
			exit(1);
		}				
		
		//re file
		re_file_name << std::string(Parameters::GetCityName()) << "," << Parameters::GetNbStations() << "," << heur_ub << "," << heur_nb_drivers << ",";
		re_file_name << std::fixed << std::setprecision(2) << elapsed_seconds << ",\n";
		printf("Re file written to:%s\n",Parameters::GetReFileName());
		re_file_name.close();
		
		//out file
		out_file_name << pr.GetCustomerCount() << "," << sol.GetUsedDriverCount() << ",\n";	
		int routeCounter=0;
		for(int i=0;i<sol.GetDriverCount();i++)
		{
			Driver * d = sol.GetDriver(i);
			if(sol.GetRouteLength(i)<2) continue;

			//distances.push_back(d->curDistance);
			
			Node * curr = sol.GetNode( d->StartNodeID );
			Node * prev = sol.GetNode( d->StartNodeID ); // Keep track of the previous node
			
			int consecutiveNonCustomers = 0;
			while( curr != NULL)
			{
				out_file_name << curr->id << "-";
				//printf("%d-",curr->id);
				
				curr = sol.Next[ curr->id ];
				
				prev = curr;
			
			}
			//printf(" length:%d\n",sol.GetRouteLength(i));
			out_file_name << ",\n";
			out_file_name << routeCounter << "," << sol.GetRouteLength(d) << "," << d->sum_demand << "," << d->curDistance << ",\n";
			routeCounter++;
		}
		printf("Out file written to:%s\n",Parameters::GetOutputFileName());
		out_file_name.close();		
	}
	
	return 0;
}
