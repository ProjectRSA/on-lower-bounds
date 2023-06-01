#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include "Vertex.hpp"
#include "Edge.hpp"
#include "Arc.hpp"
#include "Path.hpp"
#include "Graph.hpp"
#include "Demand.hpp"
#include "Clique.hpp"
#include "RSA_Input.hpp"
#include "MCMCF_Output.hpp"
#include "RSA_Output.hpp"
#include "RSA_Algorithms.hpp"
#include <ilcplex/ilocplex.h>
#include <chrono> 
#include <math.h> 
ILOSTLBEGIN

using namespace std;
using namespace std::chrono;

RSA_Algorithms::RSA_Algorithms(RSA_Input rsa) : RSA_Input_(rsa)
{
	iterations_ = 0;
	Construct_G_Prime();
	G_ = Graph(rsa.getNodes(), rsa.getEdges());
	maxSlots_ = 0;
	for (unsigned i = 0; i < G_.getEdges().size(); ++i)
	{
		if (G_.getEdges()[i]->getNumberSlices() > maxSlots_)
			maxSlots_ = G_.getEdges()[i]->getNumberSlices();
	}
	upperBound_ = maxSlots_ + 1;
	smallestClique_ = maxSlots_+ 1;
	std::cout << std::endl << "---------------BEGIN OF Data Information------------------" << std::endl;
	std::cout << "  Graph G  " << std::endl;
	std::cout << G_ << std::endl;
	std::cout << "  Graph G prime " << std::endl;
	std::cout << GPrime_ << std::endl;
	std::cout << "  Demands  " << std::endl;
	rsa.showRequests();
	std::cout << std::endl << "----------------END OF Data Information-------------------" << std::endl;
	vector<Path*> path(0);
	for(unsigned i = 0; i < RSA_Input_.getRequests().size(); ++i)
	{
		possiblePaths_.push_back(path);
	}
	lowerBound_ = 0;
	isOptimal_ = false;
}

void RSA_Algorithms::Construct_G_Prime()
{
	vector<Arc*> arcs;
	arcs.resize(RSA_Input_.getEdges().size() * 2);

	for (unsigned i = 1; i <= RSA_Input_.getEdges().size(); ++i)
	{
		Arc * arc1 = new Arc(*RSA_Input_.getEdges()[i-1]);
		Arc * arc2 = new Arc(*arc1);
		arc2->InvertOrientation();
		arc2->setIndex(RSA_Input_.getEdges().size() + arc1->getIndex());
		arcs[i-1] = arc1;
		arcs[i - 1 + RSA_Input_.getEdges().size()] = arc2;
	}

	for (vector<Arc*>::iterator it = arcs.begin(); it != arcs.end(); ++it)
	{
		(*it)->getOrigin().addVertexOutArcs(**it);
		(*it)->getDestination().addVertexInArcs(**it);
	}

	GPrime_ = Graph(RSA_Input_.getNodes(), arcs);
}

void RSA_Algorithms::framework_1()
{
	//time markers initialization
	auto startStep = high_resolution_clock::now();
	auto endStep = high_resolution_clock::now();
	auto duration = duration_cast<seconds>(endStep - startStep);
	//total time control
	std::chrono::duration<long int> limit = (std::chrono::duration<long int>) 7100;

	//total times initialization
	auto totalKShortest = duration_cast<seconds>(startStep - startStep);
	auto totalRSA = duration_cast<seconds>(startStep - startStep);

    startStep = high_resolution_clock::now(); 		//Begin of MCMCF
    //this->solveMinCostMultiCommodityFlow_Cplex();
	this->SolveKShortest();
    endStep = high_resolution_clock::now();			//End of MCMCF
    duration = duration_cast<seconds>(endStep - startStep);
    totalKShortest = totalKShortest + duration;
    std::cout << "TIME == MCMCF duration in seconds: " << duration.count() << std::endl; 
	addNewRoutesToRSA();
	// Here, we begin tha SA part of the problem
    showPossiblePaths();
    startStep = high_resolution_clock::now();			//Begin RSA
    this->solveEdgePathFormulation_Cplex();
    endStep = high_resolution_clock::now();				//End RSA
    duration = duration_cast<seconds>(endStep - startStep);
    totalRSA = totalRSA + duration;
    std::cout << "TIME == RSA duration in seconds: " << duration.count() << std::endl; 
    // If gap is not 0, it means that we interrupted the last EPF by time limit
    if (gap_ > 0){
		std::cout << "RSA TOOK TOO LONG" << std::endl;
		std::cout << "Solution found but there is a gap of: " << gap_;
    }
    // If RSA solution is optimal we found our solution and we can end the program
    if(isOptimal_ == true){
		std::cout << "===== SOLUTION =====" << std::endl;
		std::cout << RSA_Output_ << std::endl;
	}
    std::cout << "---------------------------------------------------: " << std::endl;
    std::cout << "Total time spent in k-shortest paths: " << totalKShortest.count() << std::endl;
    std::cout << "Total time spent in RSA: " << totalRSA.count() << std::endl;
    std::cout << "----------------END OF EXECUTION-------------------: "  << std::endl;
}

void  RSA_Algorithms::solveKShortest(){}

void RSA_Algorithms::solveMinCostMultiCommodityFlow_Cplex(){

	std::cout << std::endl << "---------------BEGIN OF ILP Min Cost Multi Commodity Cplex------------------" << std::endl;
	unsigned K = RSA_Input_.getRequests().size();
	unsigned A = GPrime_.getArcs().size(); 				//number of arcs in the auxilar graph G'
	unsigned V = GPrime_.getVertices().size(); 			//number of vertices

	//graph and demand information
	std::cout << "Number of demands : " << K << std::endl;
	std::cout << "Number of arcs : " << A << std::endl;
	std::cout << "Number of nodes : " << V << std::endl;

	IloEnv MCMCF; 										// environnement : allows use of functions in Concert Technology
	IloModel ILP_MCMCF(MCMCF);							// model: represents the linear program
	IloCplex model(ILP_MCMCF); 							// class Cplex : allows acces to optimization functions

	// ----------------------- VARIABLES -----------------------
	IloNumVarArray variables(MCMCF);

	IloArray<IloNumVarArray> f_kl(MCMCF, K);
	for (unsigned k = 0; k < K; k++)
	{
		f_kl[k] = IloNumVarArray(MCMCF, A, 0, 1, ILOINT);
		for (unsigned a = 0; a < A; a++)
		{
			std::string Var_Name = "f[" + to_string(k+1) + "," + to_string(a + 1) + "]";
			f_kl[k][a].setName(Var_Name.c_str());
			ILP_MCMCF.add(f_kl[k][a]);
			variables.add(f_kl[k][a]);
			//std::std::cout << "Variable : " << k << "\t||" << a << "\t||" <<  Var_Name << std::std::endl;
		}
	}

	IloNumVar Cap = IloNumVar(MCMCF, 0, INFINITY, ILOINT);
	std::string Var_Name = "Cap";
	Cap.setName(Var_Name.c_str());
	ILP_MCMCF.add(Cap);
	variables.add(Cap);

	// End of variable cretaion
	std::cout << std::endl << "Number of variables = " << variables.getSize() << std::endl;

	// ----------------------- Constraints -----------------------
	//In the origin the flow will go out to exactly one arc
	for (unsigned k = 0; k < K; k++)
	{
		IloExpr Path_outgoing(MCMCF);
		for (unsigned a = 0; a < RSA_Input_.getRequests()[k]->getOrigin().getVertexOutArcs().size(); a++)
		{
			Path_outgoing += f_kl[RSA_Input_.getRequests()[k]->getIndex()-1][RSA_Input_.getRequests()[k]->getOrigin().getVertexOutArcs()[a]->getIndex()-1];
		}
		ILP_MCMCF.add(Path_outgoing == 1);
		Path_outgoing.end();
	}

	//In the destination the flow will not go out
	for (unsigned k = 0; k < K; k++)
	{
		IloExpr Path_outgoing(MCMCF);
		for (unsigned a = 0; a < RSA_Input_.getRequests()[k]->getDestination().getVertexOutArcs().size(); a++)
		{
			Path_outgoing += f_kl[RSA_Input_.getRequests()[k]->getIndex()-1][RSA_Input_.getRequests()[k]->getDestination().getVertexOutArcs()[a]->getIndex()-1];
		}
		ILP_MCMCF.add(Path_outgoing == 0);
		Path_outgoing.end();
	}

	//In the destination the flow will arrive from exactly one arc
	for (unsigned k = 0; k < K; k++)
	{
		IloExpr Path_ingoing(MCMCF);
		for (unsigned a = 0; a < RSA_Input_.getRequests()[k]->getDestination().getVertexInArcs().size(); a++)
		{
			Path_ingoing += f_kl[RSA_Input_.getRequests()[k]->getIndex()-1][RSA_Input_.getRequests()[k]->getDestination().getVertexInArcs()[a]->getIndex()-1];
		}
		ILP_MCMCF.add(Path_ingoing == 1);
		Path_ingoing.end();
	}


	//Flow constraint
	for (unsigned k = 0; k < K; k++)
	{
		for (unsigned v = 0; v < V; ++v)
		{
			if ((GPrime_.getVertices()[v]->getIndex() != RSA_Input_.getRequests()[k]->getDestination().getIndex()) && (GPrime_.getVertices()[v]->getIndex() != RSA_Input_.getRequests()[k]->getOrigin().getIndex()))
			{
				IloExpr Flow(MCMCF);

				for (unsigned a = 0; a < GPrime_.getVertices()[v]->getVertexInArcs().size();a++)
				{
						Flow += f_kl[RSA_Input_.getRequests()[k]->getIndex()-1][GPrime_.getVertices()[v]->getVertexInArcs()[a]->getIndex()-1];
				}

				for (unsigned a = 0; a < GPrime_.getVertices()[v]->getVertexOutArcs().size();a++)
				{
						Flow -= f_kl[RSA_Input_.getRequests()[k]->getIndex()-1][GPrime_.getVertices()[v]->getVertexOutArcs()[a]->getIndex()-1];
				}

				ILP_MCMCF.add(Flow == 0);
				Flow.end();
			}

		}
	}

	// The flow ingoing one vertex will be just from one other vertex
	for (unsigned k = 0; k < K; k++)
	{
		for (unsigned v = 0; v < V; ++v)
		{
			IloExpr Flow(MCMCF);

			for (unsigned a = 0; a < GPrime_.getVertices()[v]->getVertexInArcs().size();a++)
			{
					Flow += f_kl[RSA_Input_.getRequests()[k]->getIndex()-1][GPrime_.getVertices()[v]->getVertexInArcs()[a]->getIndex()-1];
			}
			ILP_MCMCF.add(Flow <=1);
			Flow.end();


		}
	}

    // The flow outgoing one vertex will be just to one other vertex
	for (unsigned k = 0; k < K; k++)
	{
		for (unsigned v = 0; v < V; ++v)
		{
			IloExpr Flow(MCMCF);

			for (unsigned a = 0; a < GPrime_.getVertices()[v]->getVertexOutArcs().size();a++)
				{
						Flow += f_kl[RSA_Input_.getRequests()[k]->getIndex()-1][GPrime_.getVertices()[v]->getVertexOutArcs()[a]->getIndex()-1];
				}
			ILP_MCMCF.add(Flow <=1);
			Flow.end();


		}
	}

    // Length constraint
    for (unsigned k = 0; k < K; k++)
	{
		IloExpr Length(MCMCF);
		Length += RSA_Input_.getRequests()[k]->getMaxLength();
		for (unsigned a = 0; a < GPrime_.getArcs().size(); a++)
		{
			Length -= f_kl[RSA_Input_.getRequests()[k]->getIndex()-1][GPrime_.getArcs()[a]->getIndex()-1] * GPrime_.getArcs()[a]->getLength();
		}
		ILP_MCMCF.add(Length >= 0);
		Length.end();
	}
    // The capacity is equal to the biggest flow in one edge in the original RSA problem
    // Equal to the sum of the two arcs in inverse orientation
	for (unsigned a = 0; a < A/2; ++a)
	{
		IloExpr Capacity(MCMCF);
		for (unsigned k = 0; k < K; k++)
		{
			Capacity += int(RSA_Input_.getRequests()[k]->getSlots()) * f_kl[RSA_Input_.getRequests()[k]->getIndex()-1][GPrime_.getArcs()[a]->getIndex()-1] ;
			Capacity +=	int(RSA_Input_.getRequests()[k]->getSlots()) * f_kl[RSA_Input_.getRequests()[k]->getIndex()-1][GPrime_.getArcs()[a]->getIndex() + A/2 -1] ;
		}
		Capacity -= Cap;
		ILP_MCMCF.add(Capacity <= 0);
		Capacity.end();

	}

    // For each edge in the original graph the flow for one demand will not goes for the 2 arcs with inverse orientation
	for (unsigned a = 0; a < A/2; ++a)
	{

		for (unsigned k = 0; k < K; k++)
		{
			IloExpr edgeconst(MCMCF);
			edgeconst += f_kl[RSA_Input_.getRequests()[k]->getIndex()-1][GPrime_.getArcs()[a]->getIndex()-1] ;
			edgeconst += f_kl[RSA_Input_.getRequests()[k]->getIndex()-1][GPrime_.getArcs()[a]->getIndex() + A/2 -1] ;
			ILP_MCMCF.add(edgeconst <= 1);
			edgeconst.end();
		}
	}

    // Constraints from RSA resolution
    // For each forbidden routing with no spectrum assignment less or equal to the lower bound
    // If there are not forbidden routing, the program will not execute this part
    for (unsigned i = 0; i < impossibleRoutings_.size(); ++i)
    {
        IloExpr rsaconst(MCMCF);
        int counter = 0;
        // for each demand attend
        for (unsigned k = 0; k < impossibleRoutings_[i].getRouting().size(); ++k)
        {
            // Start in the vertex of the origin of the demand
            unsigned vertex_index = RSA_Input_.getRequests()[k]->getOrigin().getIndex();
            // Add the part of the constraint for all the edges in the path choose
            for (unsigned j = 0; j < impossibleRoutings_[i].getRouting()[k]->getEdges().size(); ++j)
            {
                // if the arc is in the same orientation that the edge or not
                if(vertex_index == impossibleRoutings_[i].getRouting()[k]->getEdges()[j]->getV1().getIndex())
                {
                    rsaconst += f_kl[RSA_Input_.getRequests()[k]->getIndex()-1][impossibleRoutings_[i].getRouting()[k]->getEdges()[j]->getIndex()-1];
                    vertex_index = impossibleRoutings_[i].getRouting()[k]->getEdges()[j]->getV2().getIndex();
                    counter ++;
                }
                else
                {
                    rsaconst += f_kl[RSA_Input_.getRequests()[k]->getIndex()-1][impossibleRoutings_[i].getRouting()[k]->getEdges()[j]->getIndex()-1+(A/2)];
                    vertex_index = impossibleRoutings_[i].getRouting()[k]->getEdges()[j]->getV1().getIndex();
                    counter ++;
                }

            }
        }
        ILP_MCMCF.add(rsaconst <= counter-1);
        rsaconst.end();
    }

    // Constraints from MCMCF resolution
    // For each forbidden routing with no spectrum assignment less or equal to the lower bound
    // If there are not forbidden routing, the program will not execute this part
    for (unsigned i = 0; i < impossibleRoutings_MCMCF.size(); ++i)
    {
        IloExpr mcmcfconstraint(MCMCF);
        int counter = 0;
        // for each demand attend
        for (unsigned k = 0; k < impossibleRoutings_MCMCF[i].getRouting().size(); ++k)
        {
            // Start in the vertex of the origin of the demand
            unsigned vertex_index = RSA_Input_.getRequests()[k]->getOrigin().getIndex();
            // Add the part of the constraint for all the edges in the path choose
            for (unsigned j = 0; j < impossibleRoutings_MCMCF[i].getRouting()[k]->getEdges().size(); ++j)
            {
                // if the arc is in the same orientation that the edge or not
                if(vertex_index == impossibleRoutings_MCMCF[i].getRouting()[k]->getEdges()[j]->getV1().getIndex())
                {
                    mcmcfconstraint += f_kl[RSA_Input_.getRequests()[k]->getIndex()-1][impossibleRoutings_MCMCF[i].getRouting()[k]->getEdges()[j]->getIndex()-1];
                    vertex_index = impossibleRoutings_MCMCF[i].getRouting()[k]->getEdges()[j]->getV2().getIndex();
                    counter ++;
                }
                else
                {
                    mcmcfconstraint += f_kl[RSA_Input_.getRequests()[k]->getIndex()-1][impossibleRoutings_MCMCF[i].getRouting()[k]->getEdges()[j]->getIndex()-1+(A/2)];
                    vertex_index = impossibleRoutings_MCMCF[i].getRouting()[k]->getEdges()[j]->getV1().getIndex();
                    counter ++;
                }

            }
        }
        ILP_MCMCF.add(mcmcfconstraint <= counter-1);
        mcmcfconstraint.end();
    }

    //Clique constraints
    for (unsigned i = 0; i < forbiddenCliques_.size(); ++i)
    {
        IloExpr cliqueconstraint(MCMCF);
        int counter = 0;
        for (unsigned j = 0; j < forbiddenCliques_[i].getForbiddenEdgesFromCliques().size(); ++j)
        {
            for (unsigned k = 0; k < forbiddenCliques_[i].getForbiddenEdgesFromCliques()[j].indexDemand.size(); ++k)
            {
                cliqueconstraint += f_kl[forbiddenCliques_[i].getForbiddenEdgesFromCliques()[j].indexDemand[k]->getIndex()-1][forbiddenCliques_[i].getForbiddenEdgesFromCliques()[j].edges->getIndex()-1];
                cliqueconstraint += f_kl[forbiddenCliques_[i].getForbiddenEdgesFromCliques()[j].indexDemand[k]->getIndex()-1][forbiddenCliques_[i].getForbiddenEdgesFromCliques()[j].edges->getIndex()-1 + A/2];
                counter++;
            }
        }
        ILP_MCMCF.add(cliqueconstraint <= counter-1);
        cliqueconstraint.end();
    }

	//Objective function
	IloExpr Ob(MCMCF);
	Ob = Cap;
	IloObjective obj(MCMCF, Ob, IloObjective::Minimize, "OBJ");
	ILP_MCMCF.add(obj);

	model.exportModel("Min_Cost_Multi_Comodity_Cplex.lp");
	model.setOut(MCMCF.getNullStream());
	if (model.solve() == false)
    {
 
    	MCMCFSolved_ = false;
    	return;
    }
    MCMCFSolved_ = true;
	std::cout << std::endl << "Capacity :" << model.getValue(Cap) << std::endl;

	// Construction of the solution
	vector<Path*> routing;
	for (unsigned k = 0; k < K; k++)
	{
		Vertex nd =  RSA_Input_.getRequests()[k]->getOrigin();
		vector<Edge*> path(0);
		while(nd.getIndex() != RSA_Input_.getRequests()[k]->getDestination().getIndex())
		{
			for (unsigned a = 0; a < nd.getVertexOutArcs().size();++a)
			{
				if (round(model.getValue(f_kl[RSA_Input_.getRequests()[k]->getIndex()-1][nd.getVertexOutArcs()[a]->getIndex()-1])) == 1)
				{
					path.push_back(nd.getVertexOutArcs()[a]->getEdge());
					nd = nd.getVertexOutArcs()[a]->getDestination() ;
					break;
				}
			}
		}
		Path * path2 = new Path(path,RSA_Input_.getRequests()[k]);
		routing.push_back(path2);
	}
	// Here the solution is saved
	MCMCF_Output_ = MCMCF_Output(routing,model.getValue(Cap));
	// We save the solution to the vector of solutions
	impossibleRoutings_MCMCF.push_back(MCMCF_Output_);
	MCMCF.end();
	std::cout << std::endl << "---------------END OF ILP Min Cost Multi Commodity Cplex------------------" << std::endl;

}

void RSA_Algorithms::solveEdgePathFormulation_Cplex()
{
	std::cout << std::endl << "---------------BEGIN OF ILP Edge Path Formulation Cplex------------------" << std::endl;

	IloEnv RSA; 									// environnement : allows use of functions Concert Technology
	IloModel ILP_RSA(RSA); 							// model: represents the linear programm
	IloCplex model(ILP_RSA); 						// class Cplex : allows acces to optimization functiona
	// ----------------------- VARIABLES -----------------------
	unsigned K = RSA_Input_.getRequests().size();
	unsigned E = G_.getEdges().size();				//number of edges in graph G'
	unsigned V = G_.getVertices().size(); 			//number of vertices
	unsigned S = maxSlots_; 						//number of avaiable slices

	//graph and demand information
	std::cout << "Number of demands : " << K << std::endl;
	std::cout << "Number of Edges : " << E << std::endl;
	std::cout << "Number of nodes : " << V << std::endl;

	// ----------------------- VARIABLES -----------------------
	IloNumVarArray variables(RSA);


    // Chose one path for each demand
	IloArray<IloNumVarArray> y_kp(RSA, K);

	unsigned P_k;
	for (unsigned k = 0; k < K; k++)
	{
		P_k = possiblePaths_[k].size();
		y_kp[k] = IloNumVarArray(RSA, P_k, 0, 1, ILOINT);
		for (unsigned p = 0; p < P_k; ++p){
			std::string Var_Name = "y[" + to_string(k + 1) + "," + to_string(p + 1) + "]";
			y_kp[k][p].setName(Var_Name.c_str());
			ILP_RSA.add(y_kp[k][p]);
			variables.add(y_kp[k][p]);
		}
	}

    //Chose one spectrum assignment for each demand. Save the right spectrum
	IloArray<IloNumVarArray> zk_s(RSA, K);
	for (unsigned k = 0; k < K; k++)
	{
		zk_s[k] = IloNumVarArray(RSA, S, 0, 1, ILOINT);
		for (unsigned s = 0; s < S; s++)
		{
			std::string Var_Name = "z[" + to_string(k + 1) + "," + to_string(s + 1) + "]";
			zk_s[k][s].setName(Var_Name.c_str());
			ILP_RSA.add(zk_s[k][s]);
			variables.add(zk_s[k][s]);
		}
	}

    // Verify which demand use which edge
	IloArray<IloNumVarArray> xk_e(RSA, K);
	for (unsigned k = 0; k < K; k++)
	{
		xk_e[k] = IloNumVarArray(RSA, E, 0, 1, ILOINT);
		for (unsigned e = 0; e < E; e++)
		{
			std::string Var_Name = "x[" + to_string(k + 1) + "," + to_string(e + 1) + "]";
			xk_e[k][e].setName(Var_Name.c_str());
			ILP_RSA.add(xk_e[k][e]);
			variables.add(xk_e[k][e]);
		}
	}

    // If the demand use one edge one slot
	IloArray <IloArray< IloNumVarArray>> tk_es(RSA, K);
	for (unsigned k = 0; k < K; k++)
	{
		tk_es[k] = IloArray<IloNumVarArray>(RSA, E);
		for (unsigned e = 0; e < E; e++)
		{
			tk_es[k][e] = IloNumVarArray(RSA, S, 0, 1, ILOINT);
			for (unsigned s = 0; s < S; s++)
			{
				std::string Var_Name = "t[" + to_string(k + 1) + "," + to_string(e + 1) + "," + to_string(s + 1) + "]";
				tk_es[k][e][s].setName(Var_Name.c_str());
				ILP_RSA.add(tk_es[k][e][s]);
				variables.add(tk_es[k][e][s]);
			}
		}
	}

	// Variable for the interval chromatic number
	IloNumVar xI = IloNumVar(RSA, 0, INFINITY, ILOINT);
	std::string Var_Name = "xI";
	xI.setName(Var_Name.c_str());
	ILP_RSA.add(xI);
	variables.add(xI);

	// End of variable cretaion
	std::cout << std::endl << "Number of variables = " << variables.getSize() << std::endl;

	// ----------------------- Constraints -----------------------
	//path selection
	for (unsigned k = 0; k < K; k++)
	{
		IloExpr path(RSA);
		P_k = possiblePaths_[k].size();
		for (unsigned p = 0; p < P_k; p++)
		{
			path += y_kp[k][p];
		}
		ILP_RSA.add(path == 1);
		path.end();
	}
	//edge activation
	for (unsigned k = 0; k < K; k++)
	{
		for (unsigned e = 0; e < E; e++)
		{
			IloExpr edge_path(RSA);
			edge_path = xk_e[k][e];
			P_k = possiblePaths_[k].size();

			for (unsigned p = 0; p < P_k; ++p)
			{
				for ( vector<Edge*>::const_iterator it = possiblePaths_[k][p]->getEdges().begin(); it != possiblePaths_[k][p]->getEdges().end(); ++it)
				{
					if (((*it)->getV1().getIndex() == G_.getEdges()[e]->getV1().getIndex()) && ((*it)->getV2().getIndex() == G_.getEdges()[e]->getV2().getIndex()))
					{
						edge_path -= y_kp[k][p];
					}
				}
			}
			ILP_RSA.add(edge_path == 0);
			edge_path.end();
		}
	}
	// length constraint (total length moode)
	for (unsigned k = 0; k < K; k++)
	{
		IloExpr length(RSA);
		for (unsigned e = 0; e < E; e++)
		{
			length += (G_.getEdges()[e]->getLength()) * xk_e[k][e];
		}
		ILP_RSA.add(length <= RSA_Input_.getRequests()[k]->getMaxLength());
		length.end();
	}

	//spectrum assignment
	for (unsigned k = 0; k < K; k++)
	{
		IloExpr modSlice(RSA);
		for (unsigned s = RSA_Input_.getRequests()[k]->getSlots() - 1; s < S; s++)
		{
			modSlice += zk_s[k][s];
		}
		ILP_RSA.add(modSlice == 1);
		modSlice.end();
	}

	//demand_edge_slice
	// tk assignment from zk (and zk - demand weight) if xk is equal to one in the edge.
	for (unsigned k = 0; k < K; k++)
	{
		for (unsigned e = 0; e < E; e++)
		{
			for (unsigned s = 0; s < S; s++)
			{
				IloExpr slice(RSA);
				slice += xk_e[k][e];
				for (unsigned s2 = s; s2 < s + RSA_Input_.getRequests()[k]->getSlots(); s2++)
				{
					if (s2 < S)
					{
						slice += zk_s[k][s2];
					}
				}
				slice -= tk_es[k][e][s];
				ILP_RSA.add(slice <= 1);
				slice.end();
			}
		}
	}

	// The sum of tk in one edge that is used must be equal to weight demand
	for (unsigned k = 0; k < K; k++)
	{
		for (unsigned e = 0; e < E; e++)
		{
			IloExpr slice(RSA);
			for (unsigned s = 0; s < S; s++)
			{
				slice += tk_es[k][e][s];
			}
			slice -= int(RSA_Input_.getRequests()[k]->getSlots()) * xk_e[k][e];
			ILP_RSA.add(slice == 0);
			slice.end();
		}
	}
	//non_overlapping in edge an in one slot two different demands
	for (unsigned e = 0; e < E; e++)
	{
		for (unsigned s = 0; s < S; s++)
		{
			IloExpr slice(RSA);
			for (unsigned k = 0; k < K; k++)
			{
				slice += tk_es[k][e][s];
			}
			ILP_RSA.add(slice <= 1);
			slice.end();
		}
	}
	// unavailable slices for each demand respecting the capacity
	for (unsigned k = 0; k < K; k++)
	{
		IloExpr Mod_Slice(RSA);
		for (unsigned s = 0; s < RSA_Input_.getRequests()[k]->getSlots() - 1; s++)
		{
			Mod_Slice += zk_s[k][s];
		}
		ILP_RSA.add(Mod_Slice == 0);
		Mod_Slice.end();
	}

	// constraints for interval chromatic number version assignement
	for (unsigned k = 0; k < K; k++)
	{
		IloExpr xINumber(RSA);
		xINumber += xI;
		for (unsigned s = 0; s < S; s++)
		{
			xINumber -= IloNum(s + 1) * zk_s[k][s];
		}
		ILP_RSA.add(xINumber >= 0);
		xINumber.end();
	}
    IloExpr xINumber(RSA);
    xINumber += xI;
    ILP_RSA.add(xINumber >= IloNum(lowerBound_));
    xINumber.end();

	//Objective Function
	IloExpr Ob(RSA);
	// Objective function: interval chromatic number
	Ob = xI;

	IloObjective obj (RSA, Ob, IloObjective::Minimize, "OBJ");
	ILP_RSA.add(obj);


	model.exportModel("Edge_Path_Formulation_Cplex.lp");
	model.setOut(RSA.getNullStream());
	model.setParam(IloCplex::TiLim, 3600); // Execution time limited

	if (model.solve() == false)
    {
    	RSASolved_ = false;
    	gap_ = 100;
    	return;
    }
    RSASolved_ = true;

	std::cout << std::endl << "Objective Value: " << model.getObjValue() << std::endl;
	std::cout << std::endl << "Gap: " << model.getMIPRelativeGap() << std::endl;
	gap_ = model.getMIPRelativeGap();

	//Construction of the output of the model
    std::vector<Path*> routing;
    std::vector<SpectrumAssignmentPerDemand> spectrumAssignment;
    for (unsigned k = 0; k < K; k++)
	{
	    // Routing construction
		P_k = possiblePaths_[k].size();
		for (unsigned p = 0; p < P_k; p++)
		{
			if( round(model.getValue(y_kp[k][p])) == 1)
            {
                routing.push_back(possiblePaths_[k][p]);
                break;
            }
		}

        // Spectrum assignment construction
        SpectrumAssignmentPerDemand sapd;
        sapd.indexDemand = k+1;
		for (unsigned s = 0; s < S; s++)
		{
			if(round(model.getValue(zk_s[k][s])) == 1)
            {
                sapd.spectrum = s+1;
                break;
            }
		}
        spectrumAssignment.push_back(sapd);
	}
    RSA_Output_ = RSA_Output(routing,spectrumAssignment,model.getObjValue());
    if (model.getObjValue() <= lowerBound_)
    {
        isOptimal_ = true;
    }
    else
    {
        impossibleRoutings_.push_back(RSA_Output_);
    }
	std::cout << std::endl << "---------------END OF ILP Edge Path Formulation Cplex------------------" << std::endl;
}

void RSA_Algorithms::addNewRoutesToRSA()
{
	bool alrealdyExistantPath;
	for (unsigned i = 0; i < MCMCF_Output_.getRouting().size(); ++i)
	{
		alrealdyExistantPath = false;
		for(unsigned j = 0; j < possiblePaths_[i].size(); ++j)
        {
            if (*(possiblePaths_[i][j]) == *(MCMCF_Output_.getRouting()[i]))
            {
                alrealdyExistantPath = true;
                break;
            }
        }
        if (alrealdyExistantPath == false)
            possiblePaths_[i].push_back(MCMCF_Output_.getRouting()[i]);
	}
}

void 	RSA_Algorithms::showPossiblePaths() const {
	std::cout << std::endl << "---------------BEGIN OF Possible paths Information------------------" << std::endl;
	for (unsigned i = 0; i < possiblePaths_.size(); ++i){
		std::cout << "For demand: " << i+1 << std::endl;
		for (unsigned j = 0; j < possiblePaths_[i].size(); ++j){
			std::cout << *(possiblePaths_[i][j]);
		}
		std::cout << "***********************: " << std::endl;
	}
	std::cout << std::endl << "---------------END OF Possible paths Information------------------" << std::endl;
}

