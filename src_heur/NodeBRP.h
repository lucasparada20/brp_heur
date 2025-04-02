#ifndef NODE_BRP
#define NODE_BRP

#include "Constants.h"
#include "Parameters.h"
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

class Node
{
	public:
		Node() : id(-1), origin_id(-1), distID(-1), no(0), type(0), demand(0) {}

		int id;			//from 0 to n-1
		int origin_id;  //ID at the begining, it is never modified, only customers have a valid unique ID
		int stationcapacity;   // Station capacity
		int distID;		//indicate which line and column that contains the distance/time in the matrices
		int no;			//personnal identifier, put what you want
		char type;		//type of the node
		int demand; 
    
		void Show()
		{
			if(type == NODE_TYPE_CUSTOMER)
			{
				printf("Node:%d type:Cust dmd:%d\n", id, demand);
			}
			else if(type == NODE_TYPE_START_DEPOT)
			{
				printf("Node:%d type:stadepot\n", id);
			}
			else if(type == NODE_TYPE_END_DEPOT)
			{
				printf("Node:%d type:enddepot\n", id);
			}
		}

		bool IsCustomer()
		{
			return (type & NODE_TYPE_CUSTOMER) == NODE_TYPE_CUSTOMER;
		}
};

#endif
