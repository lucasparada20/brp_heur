#include "LoadBRP.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <climits>

void LoadBRP::Load_brp_instance(Prob & pr, const char * filename)
{
	std::cout << "In the Load brp instances:\n";
    std::ifstream infile(filename);
    if (!infile)
    {
        std::cerr << "Error in the input filename: " << filename << std::endl;
        exit(1);
    }
		
    int stations, Q;
	std::string line;
    // Read the number of stations
    std::getline(infile, line);
    stations = std::stoi(line);
    // Read demands
    std::getline(infile, line);
    std::vector<int> demands; demands.reserve( stations );
	std::istringstream istr(line);
	
	//std::cout << "Dmds:" << line << std::endl;
	for(int i=0;i<stations;i++)
	{
		int no; istr >> no;
		demands.push_back( no );
	}
		
    // Read Q
    std::getline(infile, line);
	Q = std::stoi(line);
	
    std::cout << "stations:" << stations << " Q:" << Q << std::endl;

	std::vector< std::vector<int> > distance_matrix(stations, std::vector<int>(stations, 9999));
	for (int i = 0; i < stations; i++)
	{
		std::getline(infile, line);
		std::istringstream istr1(line);
		for (int j = 0; j < stations; j++)
		{
			double no; istr1 >> no;
			//printf("%.1lf ",no);
			distance_matrix[i][j] = no > 1e+008 ? 99999999 : no;
		}
		//printf("\n");
	}
	
	//for(size_t i=0;i<distance_matrix.size();i++)
	//{
	//	for(size_t j=0;j<distance_matrix[i].size();j++)
	//		printf("%d ",distance_matrix[i][j]);
	//	printf("\n");
	//}

	//'stations' is the integer nb of stations, from the instance .txt
	for(int i = 0; i< stations-1; i++)
	{
		Node n;
		n.id = i;
		n.no = i+1;
		n.distID = i+1;
		n.demand = demands[i+1];
		n.type = NODE_TYPE_CUSTOMER;
		
		pr.AddNode(n);
		n.Show();
	}
	//printf("Sample node:\n");
	//pr.GetNode(0)->Show();
	
	//OG
	for(int i = 0; i< stations-1; i++)	
		pr.AddCustomer( pr.GetNode(i) );
	
	for(int i = 0 ; i < stations ; i++)
	{
		Node dep1;
		dep1.id = stations - 1 + i*2;
		dep1.no = 0;
		dep1.distID = 0;
		dep1.type = NODE_TYPE_START_DEPOT;

		Node dep2(dep1);
		dep2.id = stations + i*2;
		dep2.type = NODE_TYPE_END_DEPOT;
		
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
	
	printf("Distance Matrix:\n");
	double min_val = 1e9;
	double max_val = -1e9;
	double sum = 0.0;
	int count = 0;

	for(int i = 0; i < dim; i++)
	{
		Node* n1 = pr.GetNode(i);
		d[n1->distID] = new double[dim];
		
		for(int j = 0; j < dim; j++)
		{
			Node* n2 = pr.GetNode(j);
			double val = (i == j) ? 0.0 : distance_matrix[n1->distID][n2->distID]; // original logic
			d[n1->distID][n2->distID] = val;

			if (i != j) 
			{
				if (val < min_val) min_val = val;
				if (val > max_val) max_val = val;
				sum += val;
				count++;
			}

			printf("%.1lf ", val);
		}
		printf("\n");
	}

	double avg_val = (count > 0) ? sum / count : 0.0;

	printf("Min: %.2lf, Avg: %.2lf, Max: %.2lf\n", min_val, avg_val, max_val); //getchar();
	pr.SetMatrices(d,dim);
	
	Node * depot = pr.GetNode( stations );
	if(depot->type == NODE_TYPE_CUSTOMER)
	{
		std::cerr << "Wrong depot node. Exiting ..." << std::endl;
		exit(1);
	}
}