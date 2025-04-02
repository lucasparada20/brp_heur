#include "LoadBRP.h"
#include "ProblemDefinition.h"
#include "Solution.h"
#include "CostFunctionBRP.h"
#include "InsRmvMethodBRP.h"
#include "AlnsBRP.h"
#include <fstream>
#include <iomanip>

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
	alns.SetTemperature(0.95);
	alns.SetIterationCount( Parameters::GetIterations() );
	alns.SetAcceptationGap( 1.1 );
	
	clock_t start_time = clock();
	
	
	alns.Optimize(sol);
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