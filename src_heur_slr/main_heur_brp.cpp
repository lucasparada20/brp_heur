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
	
	std::string city_name; 
	Parameters::GetCityName(std::string(Parameters::GetInstanceFileName()),city_name);
	std::string file_name_HC_str = "results/re_HC_" + std::to_string(Parameters::GetDelta()) + "_" + city_name + ".txt";
	std::string file_name_HR_str = "results/re_HR_" + std::to_string(Parameters::GetDelta()) + "_" + city_name + ".txt";
	std::string output_file_name_str = "results/out_heur_" + std::to_string(Parameters::GetDelta()) + "_" + city_name + ".txt";
	
	std::cout << "HC file name: " << file_name_HC_str << "\n"
			  << "HR file name: " << file_name_HR_str << std::endl;
			  
	if(arg == 4) // Slr instances
	{
		load.Load_coord(pr, Parameters::GetInstanceFileName());
		load.Load_targets(pr, Parameters::GetTargetsFileName());
		load.Load_json_station_status(pr, Parameters::GetInitialCapacitiesFileName());
	} else if(arg == 2) // Ebrp instances
	{
		load.LoadInstanceEBRP(pr, Parameters::GetInstanceFileName());
	}		
	else
	{
		printf("Phil Collins (1989). Didn't Load anything ... exiting."); exit(1);
	}

  
	int max_abs_dmd = 0; int max_station_cap = 0;
	for(int i=0;i<pr.GetCustomerCount();i++)
	{
		if(pr.GetNode(i)->IsDuplicatedDepot()) continue;
		
		pr.GetNode(i)->UpdateDemand(); 
		pr.GetNode(i)->UpdateW();
		max_abs_dmd = std::max( std::abs(pr.GetCustomer(i)->demand), max_abs_dmd );
		max_station_cap =  std::max( pr.GetCustomer(i)->demand, max_station_cap );
	}
	pr.SetVehicleCapacity( std::max( max_abs_dmd, max_station_cap ) );
	printf("In main: Delta:%d Vehicle Capacity Q:%d max_|q|:%d max_station_cap:%d drvCount:%d\n",Parameters::GetDelta(),pr.GetDriver(0)->capacity,max_abs_dmd,max_station_cap,pr.GetDriverCount()); //getchar();
	
	//Remove zero demand stations
	pr.RemoveZeroDemandNodes();
	//pr.ShowNodes();
	
	//Setting a hard max vehicle cap Q = 40
	Parameters::SetHardQ(40);
	std::vector<Node*> node_vec; int nbBigStationCap = 0;
	for(int i=0;i<pr.GetCustomerCount();i++)
	{
		Node * n = pr.GetCustomer(i);
		if(n->demand > Parameters::GetHardQ() || n->demand < -1*Parameters::GetHardQ())
		{
			//n->Show();
			node_vec.push_back( n );
		}
		if( n->stationcapacity > Parameters::GetHardQ() ) nbBigStationCap++;
	}
	printf("Changing Q:%d to HardQ:%d NodesAboveHardQ:%d nbBigStationCap:%d\n",pr.GetDriver(0)->capacity,Parameters::GetHardQ(),(int)node_vec.size(),nbBigStationCap);
	for(int i=0;i<node_vec.size();i++)
		pr.DuplicateAndStore( node_vec[i]->id );
	for(int i=0;i<pr.GetDriverCount();i++)
		pr.GetDriver(i)->SetCapacity( Parameters::GetHardQ() );	

	
	//Feasibility test
	printf("Checking feasibility of all stations ...\n");
	for(int i=0;i<pr.GetCustomerCount();i++)
	{
		std::vector<Node*> path(1,pr.GetCustomer(i));
		bool IsFeas = RouteFeasibility::IsFeasible(&pr,path,false);
		//printf("Bool:%d ",IsFeas);
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
	printf("Instance:%s SumDmd:%d SumTgt:%d SumInitCap:%d SumCap:%d SumAbsDiff:%d SumPick:%d SumDel:%d\n",Parameters::GetInstanceFileName(),sum_demands,sum_targets,sum_init_cap,sum_cap,sum_abs_diff,sum_pickups,sum_deliveries);	
	
	
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
	alns.SetIterationCount(10000);//Remember to set a lot of iterations
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
	
	std::ofstream file_name_HC(file_name_HC_str);
	
	file_name_HC << Parameters::GetDelta() << ";" << pr.GetDriver(0)->capacity << ";" << pr.GetCustomerCount() << ";" << heur_ub << ";" << heur_ub_distance << ";" << heur_ub_recourse << ";" << heur_nb_drivers << ";";
	file_name_HC << std::fixed << std::setprecision(2) << elapsed_time << ";";
	printf("HC re file written to:%s\n",file_name_HC_str.c_str());
	file_name_HC.close();
	
	
	//==== 2ND ALNS RUN ====//	
	ALNS alnsHR;	
	SeqInsertBRP_HR seqHR(method);
	alnsHR.AddInsertOperator( &seqHR );
	alnsHR.AddRemoveOperator(&random_remove);
	alnsHR.AddRemoveOperator(&related_remove);
	
	//Optimize HR with 10K iterations
	alnsHR.SetTemperatureIterInit(0);
	//Test with toronto because is really slow ...
	alnsHR.SetTemperature(0.9980); //Lower than HC
	alnsHR.SetMaxTime(18000);//~5hrs
	alnsHR.SetIterationCount(200);
	alnsHR.SetAcceptationGap( 1.1 );
	//if (city_name == "newyork")
		//alnsHR.SetIterationCount(2000);
	clock_t begin_2nd_alns = clock();
	alnsHR.SetRecoursePolicy(2); // To modify Alns printing output every 1000 or 100 iterations
	Parameters::SetRecoursePolicy(2); //It is called in CostFunction methods
	//alnsHR.Optimize(solHR);
	//solHR.Update();
	alnsHR.Optimize(sol);
	sol.Update();
	
	//solHR.Show();
	
	sol.Show();
	printf("HR Manual count of nb_unassigned:%d\n",nb_unassigned);
	heur_ub_distance = sol.GetTotalDistances();
	heur_ub_recourse = sol.GetTotalRecourse();
	heur_ub = heur_ub_distance + heur_ub_recourse;
	heur_nb_drivers = sol.GetUsedDriverCount();
	elapsed_time = (double)(clock() - begin_2nd_alns) / CLOCKS_PER_SEC;
	printf("HR UB Heur:%.3lf dist:%.3lf rec:%.3lf drv:%d time:%.3lf\n", heur_ub, heur_ub_distance, heur_ub_recourse,heur_nb_drivers,elapsed_time);
	
	std::ofstream file_name_HR(file_name_HR_str);
	if(!file_name_HR.is_open())
	{
		std::cerr << "Could not open HR re file" << std::endl;
		exit(1);
	}

	file_name_HR << Parameters::GetDelta() << ";" << pr.GetDriver(0)->capacity << ";" << pr.GetCustomerCount() << ";" << heur_ub << ";" << heur_ub_distance << ";" << heur_ub_recourse << ";" << heur_nb_drivers << ";";
	file_name_HR << std::fixed << std::setprecision(2) << elapsed_time << ";";
	printf("HR re file written to:%s\n",file_name_HR_str.c_str());
	file_name_HR.close();
	
	//out file
	std::ofstream outputFile(output_file_name_str);
	if(!outputFile.is_open())
	{
		printf("Could not open output file:%s\n",Parameters::GetOutputFileName()); 
		exit(1);
	}	
	outputFile << pr.GetCustomerCount() << "," << sol.GetUsedDriverCount() << ",\n";	
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
			outputFile << curr->id << "-";
			//printf("%d-",curr->id);
			
			curr = sol.Next[ curr->id ];
			
			prev = curr;
		
		}
		//printf(" length:%d\n",sol.GetRouteLength(i));
		outputFile << ",\n";
		outputFile << routeCounter << "," << sol.GetRouteLength(d) << "," << d->sum_demand << "," << d->curDistance << ",\n";
		routeCounter++;
	}
	printf("Out file written to:%s\n",output_file_name_str.c_str());
	outputFile.close();		
	
	return 0;
}
