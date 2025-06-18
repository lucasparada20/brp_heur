#ifndef LOAD_BRP
#define LOAD_BRP

#include "NodeBRP.h"
#include "DriverBRP.h"
#include "ProblemDefinition.h"
#include "Parameters.h"
#include "json.hpp"

#include <map>
#include <cmath>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

class Load 
{
	public:
		void Load_coord(Prob & pr, char * filename);
		void Load_json_station_status(Prob & pr, char * filename);
		void Load_targets(Prob & pr, char * filename);
		
		std::map<std::string,int> bss_id_map;
};

#endif