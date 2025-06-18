#include "Parameters.h"
#include "Constants.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <string>
#include <string.h>
#include <vector>
#include <limits>
#include <algorithm>

std::string Parameters::instance_file;
std::string Parameters::re_file_name;
std::string Parameters::output_file_name;
std::string Parameters::targets_file_name;
std::string Parameters::initial_capacities_file_name;
int Parameters::add_depot_stations=0;
int Parameters::max_route_distance=0;
int Parameters::recourse_policy=1;
int Parameters::delta = 1;
int Parameters::HardQ = 100;

void Parameters::GetCityName(const std::string& filePath, std::string& city_name) {

    // Variables to hold positions of slashes and underscores
    size_t firstSlashPos = std::string::npos;
    size_t cityStart = std::string::npos;
    size_t cityEnd = std::string::npos;

	firstSlashPos = filePath.find('/');
	if (firstSlashPos == std::string::npos) 
	{
		printf("GetCityName: No slashes found...\n");
		exit(1);
	}	
	
	cityStart = firstSlashPos + 1;
	cityEnd = firstSlashPos + 1;
    while (cityEnd < filePath.size() && std::isalpha(filePath[ cityEnd ])) {
        ++cityEnd;
    }

    city_name = filePath.substr(cityStart, cityEnd - cityStart);
	
	printf("In GetCityName:%s\n",city_name.c_str());
}


void Parameters::Read(int arg, char ** argv)
{
	printf("Reading parameters\n");
	for(int i=0;i<arg;i++)
	{
		char * first = strtok (argv[i]," ;=");
		char * second = strtok (NULL, " ;=");
		printf ("Parameter:%s value:%s\n",first,second);
		if(second == NULL) continue;
		
		if(strcmp(first, "instance_file") == 0)
		{
			instance_file = std::string(second);
		}
		else if(strcmp(first, "re_file")==0)
		{
			re_file_name = std::string(second);
		}
		else if(strcmp(first, "output_file")==0)
		{
			output_file_name = std::string(second);
		}
		else if(strcmp(first,"targets_file_name")==0)
		{
			targets_file_name = std::string(second);
		}
		else if(strcmp(first,"initial_capacities_file_name")==0)
		{
			initial_capacities_file_name = std::string(second);
		}
		else if(strcmp(first,"add_depot_stations")==0)
		{
			add_depot_stations = std::stoi(second);
		}
		else if(strcmp(first,"delta")==0)
		{
			delta = std::atoi( second );
			SetDelta( delta );
			//printf("Parsed delta:%d value:%d\n",delta,GetDelta());
		}
		else if(strcmp(first,"HardQ")==0)
		{
			HardQ = std::atoi( second );
			SetHardQ( HardQ );
		}
	}
}