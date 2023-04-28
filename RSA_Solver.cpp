#include <ilcplex/ilocplex.h>
ILOSTLBEGIN
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <cstdlib>
#include "Vertex.hpp"
#include "Edge.hpp"
#include "Arc.hpp"
#include "Demand.hpp"
#include "Graph.hpp"
#include "Path.hpp"
#include "Clique.hpp"
#include "RSA_Input.hpp"
#include "RSA_Output.hpp"
#include "MCMCF_Output.hpp"
#include "RSA_Algorithms.hpp"
#include <stdlib.h>
#include <exception>
#include <chrono> 

using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[])
{
	try
	{
		cout << endl << "====================================================== "<< endl;
		cout << endl << "Number of arguments : " << argc << endl;
		cout << endl << "Instances name : " << argv[1] << endl;
		cout << endl << "File outputs name : " << argv[2] << endl;
		freopen(argv[2], "w", stdout); 									//this line saves the output from the terminal to a file, comment to see output in terminal
		cout << endl << "FRAMEWORK FOR COLUMN GENERATION FOR AN EDGE PATH FORMULATION"<< endl;
		RSA_Input RSA;
		string instance_= argv[1];
		cout << endl << "==================BEGIN READING DATA============== "<< endl;
		RSA.data_load_node(instance_+"/Node.csv");
		RSA.data_load_edge(instance_+"/Link.csv");
		RSA.data_load_demand(instance_+"/Demand.csv");
		cout << endl << "==================END READING DATA============== "<< endl;
		
		RSA_Algorithms rsa_algo(RSA);

		auto startStep = high_resolution_clock::now();
        rsa_algo.framework_1();
        auto endStep = high_resolution_clock::now();
        auto duration = duration_cast<seconds>(endStep - startStep);
        cout << "TOTAL FRAMEWORK TIME in seconds: " << duration.count() << endl; 
	}
	catch(exception& e)
	{
		cout << endl << e.what() << endl;
	}
}
