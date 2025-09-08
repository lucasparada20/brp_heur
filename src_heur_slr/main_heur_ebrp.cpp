#include <stdio.h>
#include <stdlib.h>
#include <string.h> //strcmp
#include <time.h>
#include <csignal>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <iostream>

//==== General headers ====//

#include "Parameters.h"
#include "ProblemDefinition.h"
#include "NodeBRP.h"
#include "DriverBRP.h"
#include "LoadBRP.h"
#include "Solution.h"
#include "RouteFeasibility.h"


//==== ALNS headers ====//

#include "RandomRemoveBRP.h"
#include "RelatednessRemoveBRP.h"
#include "InsRmvMethodBRP.h"
#include "CostFunctionBRP.h"
#include "CostFunctionBRP.h"
#include "SeqInsertBRP_HC.h"
#include "SeqInsertBRP_HR.h"
#include "AlnsBRP.h"

//=====================//

int main(int arg, char ** argv)
{
	srand(0);

	Prob pr;
	Load load;

	Parameters param;
	param.Read(arg,argv);
		
	load.LoadInstanceEBRP(pr, Parameters::GetInstanceFileName());
	
	//Feasibility test
	printf("Checking feasibility of all stations ...\n");
	for(int i=0;i<pr.GetCustomerCount();i++)
	{
		std::vector<Node*> path(1,pr.GetCustomer(i));
		bool IsFeas = RouteFeasibility::IsFeasible(&pr,path,false);
		//printf("Bool:%d ",IsFeas);
		if(std::abs(pr.GetCustomer(i)->demand) > 30) pr.GetCustomer(i)->Show();
		if(std::abs(pr.GetCustomer(i)->demand) > 40) IsFeas = false;
		if(IsFeas == false)
		{
			printf("Infeasible node:");
			pr.GetCustomer(i)->Show();
			exit(1);
		}
	}	
	printf("All customers feasible! Beginning ALNS ...\n");
	int sum_demands = 0; int sum_targets = 0; int sum_init_cap = 0; int sum_cap = 0; int sum_pickups = 0; int sum_deliveries = 0; int sum_abs_diff = 0;
	
	for(int i = 0;i<pr.GetCustomerCount();i++)
	{
		Node * n = pr.GetCustomer(i);
		sum_demands += n->demand; sum_targets += n->target; sum_init_cap += n->initialcapacity; sum_cap += n->stationcapacity;
		sum_abs_diff += std::abs(n->target - n->initialcapacity);
		if( n->demand > 0 ) 
			sum_pickups += n->demand;
		else if(n->demand < 0)
			sum_deliveries += n->demand; 
	}
	printf("Instance:%s CustomerCount():%d SumDmd:%d SumTgt:%d SumInitCap:%d SumCap:%d SumAbsDiff:%d SumPick:%d SumDel:%d\n",Parameters::GetInstanceFileName(),pr.GetCustomerCount(),sum_demands,sum_targets,sum_init_cap,sum_cap,sum_abs_diff,sum_pickups,sum_deliveries);	
	
	
	clock_t begin = clock();
	CostFunctionBRP cost_func;
	InsRmvMethodBRP method(pr);

	Sol sol(&pr,&cost_func);
	sol.PutAllNodesToUnassigned();
	
	SeqInsertBRP_HC seqHC(method);
	
	RemoveRandomBRP random_remove;
	RelatednessRemoveBRP related_remove(pr.GetDistances());

	ALNS alns;
	alns.AddInsertOperator( &seqHC );
	alns.AddRemoveOperator(&related_remove);
	alns.AddRemoveOperator(&random_remove);

	seqHC.Insert(sol,true);

	//Optimize HC with 50K iterations
	alns.SetTemperatureIterInit(0);
	alns.SetTemperature(0.9995);
	alns.SetIterationCount(50000);//Remember to set a lot of iterations
	alns.SetAcceptationGap( 1.1 );
	
	alns.SetRecoursePolicy(1); // To modify Alns printing output every 1000 or 100 iterations
	Parameters::SetRecoursePolicy(1);
	alns.Optimize(sol);
	sol.Update();
	
	int nb_unassigned = 0;
	for(int i=0;i<pr.GetCustomerCount();i++)
	{
		Node * n = pr.GetCustomer(i);
		if(sol.IsUnassigned(n))
		{
			nb_unassigned++;
		}
	}
	sol.Show();
	printf("Manual count of nb_unassigned:%d\n",nb_unassigned);
	
	//==== ALNS output ====//
	double heur_ub_distance = sol.GetTotalDistances();
	double heur_ub_recourse = sol.GetTotalRecourse();
	double heur_ub = heur_ub_distance + heur_ub_recourse;
	int heur_nb_drivers = sol.GetUsedDriverCount();
	double elapsed_time = (double)(clock() - begin) / CLOCKS_PER_SEC;
	printf("HC UB Heur:%.3lf dist:%.3lf rec:%.3lf drv:%d time:%.3lf\n", heur_ub, heur_ub_distance, heur_ub_recourse,heur_nb_drivers, elapsed_time);
	
	int total_charges = 0; int total_init_reg = 0; int total_init_elec = 0; int total_init_u = 0;
	std::string cons_heur_str = "SEQ";
	std::string bss_type_str = Parameters::GetBSSType() == CS ? "_CS" : "_SW";
	std::string re_continue_file_name = std::string("results/re_CN_") + cons_heur_str + "_" + std::to_string((int)Parameters::MaxRouteDistance()) + "_" + Parameters::GetCityName() + "_" + std::to_string(Parameters::GetUValue()) + bss_type_str + ".txt";
	std::ofstream re_file_CN(re_continue_file_name);
	
	re_file_CN << std::string(Parameters::GetCityName()) << "," << Parameters::GetNbStations() << "," << Parameters::GetUValue() << "," << "CN" << "," << total_init_reg << "," << total_init_elec << "," << total_init_u << "," << heur_ub << "," << heur_ub_distance << "," << heur_ub_recourse << "," << heur_nb_drivers << ",";
	re_file_CN << std::fixed << std::setprecision(2) << elapsed_time << "\n";
	printf("ReCN file written to:%s\n",re_continue_file_name.c_str());
	re_file_CN.close();	
		
	//route file
	std::string solution_file_name_str = std::string("results/test_solution_CN_") + cons_heur_str + "_" + std::to_string((int)Parameters::MaxRouteDistance()) + "_" + Parameters::GetCityName() + "_" + std::to_string(Parameters::GetUValue()) + bss_type_str + ".txt";	
	std::ofstream solutionFile(solution_file_name_str);
	
	if(!solutionFile.is_open())
	{
		printf("Could not open solutionFile file:%s\n",solution_file_name_str.c_str()); 
		exit(1);
	}
	
	solutionFile << pr.GetCustomerCount() << "," << sol.GetDriverCount() << "\n";	
	int routeCounter=0; 
	int total_stations=0;
	for(int i=0;i<sol.GetDriverCount();i++) // Printing all routes to avoid I/O bugs later when loading
	{
		Driver * d = sol.GetDriver(i);
		
		Node * curr = sol.GetNode( d->StartNodeID );
		solutionFile << routeCounter << "," << sol.RoutesLength[d->id] << "," << d->sum_demand << "," << 0 << "," << d->curDistance << "\n";
		while( curr != NULL)
		{
			solutionFile << curr->id << "-";
			//printf("%d-",curr->id);
			curr = sol.Next[ curr->id ];
		}
		//printf(" length:%d\n",sol.GetRouteLength(i));
		solutionFile << "\n";
		
		routeCounter++;
		total_stations += sol.RoutesLength[d->id];
	}
	printf("Solution file written to:%s\n",solution_file_name_str.c_str());
	solutionFile.close();
	printf("total stations routed:%d\n",total_stations);
	
	return 0;
}