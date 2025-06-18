
#ifndef _PARAMETERS_H
#define _PARAMETERS_H

#include <time.h>
#include <math.h>
#include <string>

class Parameters
{
	
	public:

		static char* GetInstanceFileName(){return instance_file.size() == 0?NULL:(char *)instance_file.c_str();}
		static char* GetReFileName(){ return (char *)re_file_name.c_str(); }
		static char* GetOutputFileName(){ return (char *)output_file_name.c_str(); }
		static char* GetTargetsFileName(){ return (char *)targets_file_name.c_str(); }
		static char* GetInitialCapacitiesFileName(){ return (char *)initial_capacities_file_name.c_str(); }
		
		static int AddDepotStations(){ return add_depot_stations; }
		static void SetMaxRouteDistance(int d){ max_route_distance = d;}
		static int MaxRouteDistance(){ return max_route_distance; }
		static void SetRecoursePolicy(int p){ recourse_policy = p; }
		static int RecoursePolicy(){ return recourse_policy; }
		static int GetDelta(){ return delta; }
		static void SetDelta(int d){ delta=d; }
		static int GetHardQ(){ return HardQ; }
		static void SetHardQ(int q){ HardQ = q; }
		
		static void GetCityName(const std::string& filePath, std::string & city_name);
		
		//Function to read all input parameters
		void Read(int arg, char ** argv);
		
	private:

		static std::string instance_file;
		static std::string re_file_name;
		static std::string output_file_name;
		static std::string targets_file_name;
		static std::string initial_capacities_file_name;	
		
		static int max_route_distance;
		static int add_depot_stations;
		static int recourse_policy;
		static int delta;
		static int HardQ;				

};


#endif
