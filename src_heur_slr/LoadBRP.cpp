#include "LoadBRP.h"

constexpr double EarthRadius = 6371.0; // in kilometers

double CalculateHarvesineDistance(Node * n1, Node * n2) {
    double lat1 = n1->lat;
    double lon1 = n1->lon;
    double lat2 = n2->lat;
    double lon2 = n2->lon;

    // Convert latitude and longitude from degrees to radians
    lat1 *= M_PI / 180.0;
    lon1 *= M_PI / 180.0;
    lat2 *= M_PI / 180.0;
    lon2 *= M_PI / 180.0;

    // Haversine formula
    double dlat = lat2 - lat1;
    double dlon = lon2 - lon1;
    double a = std::sin(dlat / 2) * std::sin(dlat / 2) +
               std::cos(lat1) * std::cos(lat2) *
               std::sin(dlon / 2) * std::sin(dlon / 2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    double distance = EarthRadius * c;

    return distance;
}

void Load::Load_coord(Prob & pr, char * filename)
{
    std::cout << "In the Load station coordinates:\n";
    std::ifstream infile(filename);
    if (!infile)
    {
        std::cerr << "Error in the input filename: " << filename << std::endl;
        exit(1);
    }
    std::cout << "Loading coordinates from: " << filename << std::endl;
	
	//Assumption for maximum route duration
	//1.-Amazon drivers drive, on average 100+ miles per day. We thus set the hard limit to <= 100 miles = 160 km
	Parameters::SetMaxRouteDistance(160);
	
    int stations, Qtot, Q;
	std::string line;
    // Read the number of stations
    std::getline(infile, line);
    stations = std::stoi(line);
    // Read Qtot
    std::getline(infile, line);
    Qtot = std::stoi(line);
    // Set Qtot for pr
    pr.SetQtot(Qtot);
	
    Q = 40; // Real Bixi cap from May, 2024
    std::cout << "stations:" << stations << " Qtot:" << Qtot << " Q:" << Q << std::endl;

    std::vector<double> lat(stations, 0.0);
    std::vector<double> lon(stations, 0.0);
    std::vector<int> cap(stations, 0);
    std::vector<std::string> bss_id_vec(stations, "");
	
    std::getline(infile, line);
    std::istringstream iss(line);
	int depot_cap = -1; double depot_lat = -1.0; double depot_lon = -1.0;
	if(!(iss >> depot_cap >> depot_lat >> depot_lon ))
	{
		std::cerr << "Error reading depot line of file " << filename << std::endl;
		std::cout << "line:" << line << std::endl;
		exit(1);		
	} else {
		cap[0] = depot_cap; lat[0] = depot_lat; lon[0] = depot_lon;
	}

    for (int i = 1; i < stations; i++)
    {
        std::getline(infile, line);
		iss.clear(); // Clear any error flags
		iss.str(line); // Set the new string to parse
		std::istringstream iss(line);
		std::string station_id_str;
		// Now you can parse the rest of the line using iss
		if (!(iss >> station_id_str >> cap[i] >> lat[i] >> lon[i])) {
			std::cerr << "Error reading line " << i+1 << " of file " << filename << std::endl;
			std::cout << "line:" << line << std::endl;
			exit(1);
		} else {
			//std::cout << "i:" << i << " station_id:" << station_id_str << " cap:" << cap[i] << " lat:" << lat[i] << " lon:" << lon[i] << std::endl;
		}
        // Assuming station_id_str is the station ID, you can do something with it.
        // For example:
        bss_id_vec[i] = station_id_str;

        //std::cout << "i:" << i << " line:" << line << std::endl;
    }
	
	//'stations' is the integer nb of stations, from the instance .txt
	for(int i = 0; i< stations-1; i++)
	{
		Node n;
		n.id = i;
		n.no = i+1;
		n.distID = i+1;
		n.type = NODE_TYPE_CUSTOMER;
		n.stationcapacity = cap[i+1];
		n.lat = lat[i+1];
		n.lon = lon[i+1];
		
		bss_id_map[ bss_id_vec[i+1] ] = i; //The maps is: station_id from the bss = the node no
		
		pr.AddNode(n);
	}
	printf("Sample node:\n");
	pr.GetNode(0)->Show();
	/*int max_station_cap = *std::max_element(cap.begin(), cap.end());
	if(Q < *std::max_element(cap.begin(), cap.end()))
	{
		printf("Max Station Cap:%d > Q:%d\n",max_station_cap,Q);
		max_station_cap = (int)(std::ceil(max_station_cap / 10.0) * 10.0); // Round up to nearest tenth
		if(Q < max_station_cap)
		{
			printf("New Q:%d\n", max_station_cap);
		}
		Q = max_station_cap;
		//getchar();
	}*/
	
	if(Parameters::AddDepotStations())
	{
		printf("Additional depot nodes:\n");
		int nb_additional_stations = 0.1 * stations; //Rouding down the integer
		printf("Nodes no from .txt:%d New nodes no:%d\n",stations,stations+nb_additional_stations);
		printf("Last regular station:%d First additional station:%d\n",stations-2, stations-2);
		
		for(int i=stations-1; i<stations+nb_additional_stations-1; i++)
		{
			Node n;
			n.id = i;
			n.no = i+1;
			n.distID = i+1;
			n.type = NODE_TYPE_CUSTOMER;
			n.stationcapacity = Q;
			n.lat = lat[0];
			n.lon = lon[0];
			n.initialcapacity = Q;
			n.demand = 0;
			n.target = 0; // demand is also set as : target - initial_capacitie = 0 - 0 = 0
			n.is_depot_duplicated = 1;
			n.w_plus = Q;
			n.w_minus = Q;
			
			n.Show();
			pr.AddNode(n);			
		}
		
		stations += nb_additional_stations;
	}	
	//OG
	for(int i = 0; i< stations-1; i++)	
		pr.AddCustomer( pr.GetNode(i) );

	//for(int i = 0 ; i < std::min( stations, 30 ) ; i++)
	for(int i = 0 ; i < stations ; i++)
	{
		Node dep1;
		dep1.id = stations - 1 + i*2;
		dep1.no = 0;
		dep1.distID = 0;
		dep1.type = NODE_TYPE_START_DEPOT;
		dep1.stationcapacity = 0;
		dep1.lat = lat[0];
		dep1.lon = lon[0];

		Node dep2(dep1);
		dep2.id = stations + i*2;
		dep2.type = NODE_TYPE_END_DEPOT;
		dep2.stationcapacity = 0;
		dep2.lat = lat[0];
		dep2.lon = lon[0];

		
		Driver d;
		d.capacity = Q;
		d.StartNodeID = dep1.id;
		d.EndNodeID = dep2.id;
		d.id = i;

		pr.AddNode(dep1);
		pr.AddNode(dep2);
		pr.AddDriver(d);
	}
	printf("Sample driver:\n");
	pr.GetDriver(0)->Show();

	int dim = stations;
	double ** d = new double*[dim];
	
	// Vector to store all distances
	std::vector<double> all_distances;
	//printf("Distance Matrix:\n");
	for(int i=0;i<dim;i++)
   {
      Node * n1 = pr.GetNode(i);
      d[n1->distID] = new double[dim];
      for(int j=0;j<dim;j++)
      {
         Node * n2 = pr.GetNode(j);
		 //printf("distID:%d distID:%d\n",n1->distID,n2->distID);
		 //n1->Show();
		 //n2->Show();
		 
		 d[n1->distID][n2->distID] = i==j ? 0 : CalculateHarvesineDistance(n1,n2);		
		//printf("distID:%d distID:%d = %d\n",n1->distID,n2->distID,(int)d[n1->distID][n2->distID]);
		all_distances.push_back(d[n1->distID][n2->distID]); // Store the distance in the vector
		if(d[n1->distID][n2->distID] > 100.0) // An intercity distance of >100km should be wrong!
		{
			printf("distance:%.3lf Nodes:\n",d[n1->distID][n2->distID]); n1->Show(); n2->Show(); getchar();
		}
		/*if(d[n1->distID][n2->distID] < 0.001 && n1->distID != n2->distID) // An intercity distance of >100km should be wrong!
		{
			printf("distance:%.3lf Nodes:\n",d[n1->distID][n2->distID]); n1->Show(); n2->Show(); getchar();
		}*/

      }
	  //printf("\n");
	  //getchar();
   }
	//exit(1);
	pr.SetMatrices(d,dim);
	
	Node * depot = pr.GetNode( stations );
	if(depot->type == NODE_TYPE_CUSTOMER)
	{
		std::cerr << "Wrong depot node. Exiting ..." << std::endl;
		exit(1);
	}
	for(int i=0; i<pr.GetCustomerCount();i++)
	{
		double depot_distance = -1.0;
		depot_distance =  pr.GetDist(depot, pr.GetCustomer(i));
		pr.GetCustomer(i)->SetDepotDistance(depot_distance);
	}
	
	// Sort the vector of distances
	std::sort(all_distances.begin(), all_distances.end());

	// Calculate quantiles (for example, 25th, 50th, and 75th percentiles)
	double quantile_25 = all_distances[static_cast<int>(0.25 * all_distances.size())];
	double quantile_50 = all_distances[static_cast<int>(0.50 * all_distances.size())];
	double quantile_75 = all_distances[static_cast<int>(0.75 * all_distances.size())];

	// Print quantiles
	printf("25th percentile: %.2f[km]\n", quantile_25);
	printf("50th percentile (median): %.2f[km]\n", quantile_50);
	printf("75th percentile: %.2f[km]\n", quantile_75);
	printf("100th percentile: %.2f[km]\n", all_distances[ all_distances.size()-1 ]);
	
	printf("Stations loaded:%d\n",pr.GetCustomerCount());
}

void Load::Load_json_station_status(Prob & pr, char * filename)
{
    printf("In the Load Json Station Status:\n");

    std::ifstream file_json(filename);
    if (!file_json)
    {
        printf("Error in the input filename: %s\n", filename);
        exit(1);
    }

    // Parse the JSON data
    nlohmann::json jsonData;
    file_json >> jsonData;
    nlohmann::json json_stations = jsonData["data"]["stations"];
    printf("Loading initial capacities from: %s NbStations: %d\n", filename, (int)json_stations.size());

    int found = 0;
    int counter = 0;
    for (const auto& json_station : json_stations)
    {
        std::string station_id = json_station["station_id"];
        counter++;
        if (counter % 50 == 0)
        {
            printf("Looped through %d stations from the json file ...\n", counter);
        }

        int bikes = 0;
        auto it = bss_id_map.find(station_id);
        if (it != bss_id_map.end())
        {
            if (json_station.contains("num_bikes_available"))
            {
                bikes = json_station["num_bikes_available"];
                pr.SetInitialCapacity(it->second, bikes);
                found++;
            }
        }
    }
    printf("Found %d/%d stations from the json station_status file!\n", found, pr.GetCustomerCount()); //getchar();

    for (int i = 0; i < pr.GetCustomerCount(); i++)
    {
        if (pr.GetCustomer(i)->initialcapacity == -1)
        {
            pr.SetInitialCapacity(i, 0);
        }
        if (pr.GetCustomer(i)->initialcapacity > pr.GetCustomer(i)->stationcapacity)
        {
            pr.SetCapacity(i, pr.GetCustomer(i)->initialcapacity);
        }
        if (pr.GetCustomer(i)->initialcapacity < 0 || pr.GetCustomer(i)->target > pr.GetCustomer(i)->stationcapacity || pr.GetCustomer(i)->initialcapacity > pr.GetCustomer(i)->stationcapacity)
        {
            printf("Something happened on the way to heaven\n");
            pr.GetCustomer(i)->Show();
            exit(1);
        }
    }
	
	/*int i=0;
	for (const auto& json_station : json_stations)
	{
		if(i>pr.GetCustomerCount()) break;
		
		//std::cout << json_station << std::endl;
		
		int ebikes = 0;
		int bikes = 0;

		if (json_station.contains("num_bikes_available"))
		{
			bikes = json_station["num_bikes_available"];
		}
		if (json_station.contains("num_ebikes_available"))
		{
			ebikes =  json_station["num_ebikes_available"];
		}
		int totalBikes = ebikes + bikes;
		int stationCapacity = pr.GetNode(i)->stationcapacity;

		if (totalBikes <= stationCapacity) {
			pr.SetInitialCapacity(i, totalBikes);
		} else if (bikes <= stationCapacity) {
			pr.SetInitialCapacity(i, bikes);
		} else {
			pr.SetInitialCapacity(i, 0);
		}
		pr.GetNode(i)->Show();
		i++;
	}
	while(i<=pr.GetCustomerCount())
	{
		pr.SetInitialCapacity(i, 0);
		i++;
	}*/
}

void Load::Load_targets(Prob & pr, char * filename)
{
	printf("In the Load Targets:\n");
	FILE * ff = fopen(filename,"r");
 	if(!ff)
	{
		printf("Error in the input filename:%s\n", filename);
		exit(1);
	}
	printf("Loading targets from:%s\n",filename);
	char line[100];
	int i=0;
	while (fgets(line, 100, ff) && i<=pr.GetCustomerCount()) 
	{
		int t;
		if(sscanf(line,"%d",&t)==1)
		{
			printf("tgt%d:%d ",i,t);
			pr.SetTarget(i,t);
			i++;
			//targets.push_back(t);
		} else{
			printf("Error parsing line:%s Load Targets\n", line);
		}		
	}
	printf("\n");
	fclose(ff); 
}

void Load::LoadInstanceEBRP(Prob & pr, char * filename)
{
    std::ifstream infile(filename);
    if (!infile)
    {
        std::cerr << "Error in the input filename: " << filename << std::endl;
        exit(1);
    }
	std::cout << "Load EBRP ===> Loading filename:" << filename << std::endl;
	Parameters::SetCityName(filename);
	printf("MaxRouteDistance in Load:%d\n",Parameters::MaxRouteDistance());
	
    // Read the meta data
	std::string line;
    std::getline(infile, line);
	std::istringstream iss(line);
	
	//output from the generator is (to be read in the same order):
	//stations, sumCap, sumInitReg, sumInitElec, sumInitU, sumqReg, sumqElec, sumAbsReg, sumAbsElec
	std::string bss_type;
	int stations, nbCharging, sumCap, sumInitReg, sumInitElec, sumInitU, sumqReg, sumqElec, sumAbsReg, sumAbsElec;
	if(!(iss >> bss_type >> stations >> nbCharging >> sumCap >> sumInitReg >> sumInitElec >> sumInitU >> sumqReg >> sumqElec >> sumAbsReg >> sumAbsElec ))
	{
		std::cout << "Error in meta data line ..." << std::endl; 
		std::cout << line << std::endl;
		exit(1);
	}
	std::cout << "BSSType:" << bss_type << " Stations:" << stations << " nbCharging:" << nbCharging  
			  << " sumCap:" << sumCap << " sumInitReg:" << sumInitReg 
			  << " sumInitElec:" << sumInitElec << " sumInitU:" << sumInitU << " sumqReg:" << sumqReg
			  << " sumqElec:" << sumqElec << " sumAbsReg:" << sumAbsReg << " sumAbsElec:" << sumAbsElec << std::endl;
			  
	Parameters::SetBSSType((char*)bss_type.c_str());
	
	printf("Bss type from load: %s MaxDist: %.1lf city_name: %s\n",
		Parameters::GetBSSType() == 1? "CS" : "SW",
		(double)Parameters::MaxRouteDistance(),
		Parameters::GetCityName());			  
	
	std::vector<Node> nodes;
	nodes.reserve(stations+1);
	
	std::getline(infile, line);
	std::istringstream idepotss(line);
	int depotInt = -1; double depotLat = 0.0; double depotLon = 0.0;
	if( !( idepotss >> depotInt >> depotLat >> depotLon ) )
	{
		std::cout << "Depot line is missing data\n" << line << std::endl; exit(1);
	}
	Node depot;
	depot.id = 0; depot.distID = 0; depot.lat = depotLat; depot.lon = depotLon; 
	printf("Depot (read from .txt):\n"); depot.Show();
	//nodes.push_back(depot);
	
	for(int i=0;i<stations-1;i++)
	{
		std::getline(infile, line);
		std::istringstream inodess(line);
		
		//output from generator is: s.cap,s.charges,s.tgtRegular,s.tgtElectric,s.initRegular,s.initElectric,s.uElectric,s.lat,s.lon
		int cap, charges, tgtRegular, tgtElectric, initRegular, initElectric, uElectric; 
		double lat, lon;
		if(!(inodess >> cap >> charges >> tgtRegular >> tgtElectric >> initRegular >> initElectric >> uElectric >> lat >> lon)) // 9 items
		{
			std::cout << "line " << i+3 << " is missing data\n" << line << std::endl; 
			std::cout << "cap charges tgtReg tgtElec initReg initElec uElec lat lon" << std::endl;
			exit(1);
		}
		
		//std::cout << "Instance line: " << line << std::endl;
		//Copy the format from the brpod to ease the ALNS implementation ...
		//n.id = i; //n.no = i+1; //n.distID = i+1;
		//nodes.emplace_back( i, //id
		//				i+1, //no
		//				i+1,  //distID
		//				NODE_TYPE_CUSTOMER,  //type
		//				initRegular - tgtRegular, // q
		//				initElectric - tgtElectric, //q_e
		//				tgtRegular, 
		//				tgtElectric, 
		//				cap, 
		//				initRegular, 
		//				initElectric, 
		//				uElectric,
		//				initRegular + initElectric + uElectric,
		//				cap - (initRegular + initElectric + uElectric),
		//				charges == 1 ? true : false,
		//				lat, 
		//				lon );

		Node n;
		n.id = i;
		n.no = i+1;
		n.distID = i+1;
		n.type = NODE_TYPE_CUSTOMER;
		n.stationcapacity = cap;
		n.lat = lat;
		n.lon = lon;
		n.target = tgtRegular + tgtElectric;
		n.initialcapacity = initRegular + initElectric + uElectric;
		
		n.UpdateDemand();
		n.UpdateW();
		//n.Show();
		nodes.push_back(n);
	}
	
	//std::cout << "Size of nodes vec:" << nodes.size() << std::endl;
	if((int)nodes.size() != stations-1)
	{
		std::cout << "Read an erroneous number of stations ... stations-1:" << stations-1 << std::endl; exit(1);
	}
	
	for(int i = 0; i< stations-1; i++)
		pr.AddNode(nodes[i]);
	
	for(int i = 0; i< stations-1; i++)	
		pr.AddCustomer( &nodes[i] );

	
	//std::cout << "Customers in Load:" << pr.GetCustomerCount() << std::endl;
	//for(int i=0; i<pr.GetCustomerCount();  i++)
		//pr.GetCustomer(i)->Show();
	
	//Add the depots : two for each customer (Not in the original Slr code ...)
	for(int i = 0 ; i < std::min(stations-1,50); i++)
	//for(int i = 0 ; i < stations-1; i++)
	{
		Node dep1;
		dep1.id = stations - 1 + i*2;
		dep1.no = 0;
		dep1.distID = 0;
		dep1.type = NODE_TYPE_START_DEPOT;
		dep1.stationcapacity = 0;
		dep1.lat = depot.lat;
		dep1.lon = depot.lon;

		Node dep2(dep1);
		dep2.id = stations + i*2;
		dep2.type = NODE_TYPE_END_DEPOT;
		dep2.stationcapacity = 0;
		dep2.lat = depot.lat;
		dep2.lon = depot.lon;
		
		Driver d;
		d.capacity = 40;
		d.StartNodeID = dep1.id;
		d.EndNodeID = dep2.id;
		d.id = i;

		pr.AddDriver(d);
		//d.Show();
		pr.AddNode(dep1);
		pr.AddNode(dep2);
	}
	//printf("Sample driver:\n");
	//pr.GetDriver(0)->Show();
	
	int dim = nodes.size()+1;
	double ** d = new double*[dim];
	
	//printf("Distance Matrix:\n");
	// Order is:  Nodes 0 ... dim-2 are customers. Node dime-1 is the first depot created
	for(int i=0;i<dim;i++)
	{
      Node * n1 = pr.GetNode(i);
      d[n1->distID] = new double[dim];
      for(int j=0;j<dim;j++)
      {
         Node * n2 = pr.GetNode(j);
		 //printf("distID:%d distID:%d\n",n1->distID,n2->distID);
		 //n1->Show();
		 //n2->Show();
		 
		 d[n1->distID][n2->distID] = i==j ? 0 : CalculateHarvesineDistance(n1,n2);		
		//printf("distID:%d distID:%d = %d\n",n1->distID,n2->distID,(int)d[n1->distID][n2->distID]);
		//all_distances.push_back(d[n1->distID][n2->distID]); // Store the distance in the vector
		if(d[n1->distID][n2->distID] > 100) // An intercity distance of >100km should be wrong!
		{
			printf("distance:%.3lf Nodes:\n",d[n1->distID][n2->distID]); n1->Show(); n2->Show(); getchar();
		}
		/*if(d[n1->distID][n2->distID] < 0.001 && n1->distID != n2->distID) // An intercity distance of >100km should be wrong!
		{
			printf("distance:%.3lf Nodes:\n",d[n1->distID][n2->distID]); n1->Show(); n2->Show(); getchar();
		}*/

      }
	  //getchar();
	}
	
	/*for(int i=0; i<dim; i++)
	{
		Node * n1 = pr.GetNode(i);
		printf("distId:%d ",n1->distID);  
		for (int j = 0; j < dim; j++) 
			printf("%2.1lf ", d[n1->distID][j]);
		printf("\n"); 
	}*/
	
	pr.SetMatrices(d,dim);	
	
}