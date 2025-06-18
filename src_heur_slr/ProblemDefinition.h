#ifndef PROBLEM_DEF_H
#define PROBLEM_DEF_H

#include <stddef.h>
#include <vector>
#include <cstring>
#include "Constants.h"
#include "NodeBRP.h"
#include "DriverBRP.h"


class Prob
{
	public:
		Prob(): _nodes(0),_customers(0),_drivers(0),_distances(NULL),_dimension(0),_driver_count_lb(1), _upper_bound(9999999999.9),_delete_matrices(true)
		{
			_nodes.reserve(4000);		
			_customers.reserve(4000);
			_drivers.reserve(300);
		}

		~Prob()
		{
			if(_delete_matrices)
			{
				if(	_distances != NULL)
				{
					for(int i = 0 ; i < _dimension ; i++)
						delete [] _distances[i];
					delete [] _distances;
				}
				_distances = NULL;
			}
		}

		void DuplicateAndStore(int i) 
		{
			Node* originalNode = &_nodes[_customers[i]]; // Retrieve the original node
				
			while (std::abs(originalNode->target - originalNode->initialcapacity) > Parameters::GetHardQ()) 
			{
				Node* newNode = new Node(*originalNode);

				int excess = std::abs(originalNode->target - originalNode->initialcapacity) - Parameters::GetHardQ();
				double excessPercentage = (double)Parameters::GetHardQ() / std::abs(originalNode->target - originalNode->initialcapacity);

				int new_stationcapacity = std::ceil(originalNode->stationcapacity * excessPercentage);
				int new_initialcapacity = std::ceil(originalNode->initialcapacity * excessPercentage);
				int new_target = std::ceil(originalNode->target * excessPercentage);

				originalNode->stationcapacity -= new_stationcapacity;
				newNode->stationcapacity = new_stationcapacity;
				originalNode->initialcapacity -= new_initialcapacity;
				newNode->initialcapacity = new_initialcapacity;
				originalNode->target -= new_target;
				newNode->target = new_target;

				originalNode->UpdateDemand();
				originalNode->UpdateW();
				newNode->UpdateDemand();
				newNode->UpdateW();

				newNode->id = _nodes.size();
				newNode->no = _nodes.size() + 1;

				// Add the new node to _nodes and _customers
				AddNode(*newNode);
				AddCustomer(newNode);

				//printf("Modified node:\n");
				//originalNode->Show();
				//printf("New node:\n");
				//newNode->Show();
				//printf("Excess:%d percentage:%.2lfx100 New cap:%d initCap:%d tgt:%d\n", 
					   //excess, excessPercentage * 100, new_stationcapacity, new_initialcapacity, new_target);
			}
		}

		void RemoveZeroDemandNodes() 
		{
			int initial_size = _nodes.size(); 
			int initial_custs = _customers.size(); 
			int initial_drvs = _drivers.size();
			Node depot = _nodes[ (int)_nodes.size()-1 ];
			Driver driver0 = _drivers[0];
			
			std::vector<Node> aux_customers;
			for(Node & node : _nodes)
				if(node.demand != 0 && node.type == NODE_TYPE_CUSTOMER)
					aux_customers.push_back( node );
			int stations = (int)aux_customers.size()+1;
			int nb_rmv = initial_custs - stations;
			
			_customers.clear(); _nodes.clear(); _drivers.clear();
			for(int i=0;i<aux_customers.size();i++)
			{
				aux_customers[i].id = i;
				aux_customers[i].no = i+1;
				aux_customers[i].distID = i+1;
				AddCustomer( &aux_customers[i] );
				AddNode( aux_customers[i] );
			}
			for(int i = 0 ; i < stations ; i++)
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
				d.capacity = driver0.capacity;
				d.StartNodeID = dep1.id;
				d.EndNodeID = dep2.id;
				d.id = i;

				AddNode(dep1);
				AddNode(dep2);
				AddDriver(d);
			}	
			
			printf("RmvZeroDmd InitNodes:%d InitCust:%d InitDrvs:%d Rmv:%d NewNodes:%d NewCust:%d NewDrvs:%d\n",
				   initial_size, initial_custs, initial_drvs, nb_rmv, (int)_nodes.size(), (int)_customers.size(), (int)_drivers.size());
		}		

		void AddCustomer(Node * n){ _customers.push_back(n->id); }
		void AddNode(Node & n){ _nodes.push_back(n); }

		int GetCustomerCount(){ return (int)_customers.size();}
		int GetNodeCount(){ return (int)_nodes.size();}

		Node* GetCustomer(int i){ return &_nodes[ _customers[i] ]; }
		Node* GetNode(int i){ return &_nodes[i]; }

		void AddDriver(Driver & d){ _drivers.push_back(d);}
		int GetDriverCount(){ return (int)_drivers.size();}
		Driver* GetDriver(int i){ return &_drivers[i];}

		void SetMatrices(double ** d, int dim){ _distances = d; _dimension = dim;}
		double ** GetDistances(){ return _distances;}
		double GetDistance(Node * i, Node * j){ return _distances[i->distID][j->distID];}
		double GetDist(Node * i, Node * j){ return _distances[i->distID][j->distID];}

		void ShowNodes()
		{
			for(size_t i=0;i<_nodes.size();i++)
				_nodes[i].Show();
		}

		double GetUpperBound(){return _upper_bound;}
		void SetUpperBound(double ub){_upper_bound = ub;}

		int GetDriverCountLB(){return _driver_count_lb;}
		void SetDriverCountLB(int d){_driver_count_lb = d;}

		void SetQtot(int q){Qtot=q;}
		int GetQtot(){return Qtot;}
		void SetInitialCapacity(int i, int cap){ _nodes[ _customers[i] ].SetInitialCapacity(cap); }
		void SetCapacity(int i, int cap){ _nodes[ _customers[i] ].SetCapacity(cap); }
		void SetTarget(int i, int t){ _nodes[ _customers[i] ].SetTarget(t) ; }
		void SetVehicleCapacity( int Q )
		{
			for( Driver & d : _drivers)
				d.capacity = Q;
		}
	
	private:
		std::vector<Node> _nodes;			//list of nodes
		std::vector<int> _customers;		//list of nodes that are customers
		std::vector<Driver> _drivers;		//list of drivers

		double ** _distances;
		int _dimension;
		int Qtot;

		int _driver_count_lb;
		double _upper_bound;
		bool _delete_matrices;		//if the problem definition comes from a copy it is false,
											//if it is original it is true
  
};

#endif
