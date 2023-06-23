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

using namespace std;

int findParam(int argc, char *argv[], string pName);

int main(int argc, char *argv[])
{
	try
	{
		// Required Params validation
		int instanceIdx = findParam(argc, argv, "-I");
		if(instanceIdx == 0)
		{
			cout << "Please enter the path of the instance and retry." << endl;
			return 1;
		}

		// Required Params values
		string instance_ = argv[instanceIdx];

		// Optional Params
		int outputFileIdx = findParam(argc, argv, "-O");
		int summaryFileIdx = findParam(argc, argv, "-S");
		int skipEPFIdx = findParam(argc, argv, "-SkEPF");

		cout << "====================================================== " << endl;
		cout << "Number of arguments : " << argc << endl;
		cout << "Instance path : " << instance_ << endl;
		bool skipEPF = false;
		if (skipEPFIdx != 0)
		{
			skipEPF = true;
			cout << "Skip EPF Formulations : " << skipEPF << endl;
		}
		char *summaryFile_ = NULL; 
		if (summaryFileIdx != 0)
		{
			summaryFile_ = argv[summaryFileIdx];
			cout << "Summary file : " << summaryFile_ << endl;
		}
		if (outputFileIdx != 0)
		{
			char *outputFile_ = argv[outputFileIdx];
			cout << "Output file : " << outputFile_ << endl;
			freopen(outputFile_, "w", stdout);
		}		

		RSA_Input RSA;
		cout << "FRAMEWORK FOR COLUMN GENERATION FOR AN EDGE PATH FORMULATION"<< endl;
		cout << "==================BEGIN READING DATA============== "<< endl;
		RSA.data_load_node(instance_ + "/Node.csv");
		RSA.data_load_edge(instance_ + "/Link.csv");
		RSA.data_load_demand(instance_ + "/Demand.csv");
		cout << "==================END READING DATA============== "<< endl;

		RSA_Algorithms rsa_algo(RSA);
		rsa_algo.framework(summaryFile_, skipEPF);
	}
	catch(exception& e)
	{
		cout << endl << e.what() << endl;
	}
}

int findParam(int argc, char *argv[], string pName)
{
	int i = 0;
	while ((i < argc) && (argv[i] != pName))
		i++;

	if (i == argc)
	{
		return 0;
	}
	else
	{
		return i + 1;
	}
}