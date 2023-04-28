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
	cout << endl << "---------------BEGIN OF Data Information------------------" << endl;
	cout << "  Graph G  " << endl;
	cout << G_ << endl;
	cout << "  Graph G prime " << endl;
	cout << GPrime_ << endl;
	cout << "  Demands  " << endl;
	rsa.showRequests();
	cout << endl << "----------------END OF Data Information-------------------" << endl;
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
	auto totalMCMCF = duration_cast<seconds>(startStep - startStep);
	auto totalRSA = duration_cast<seconds>(startStep - startStep);
	auto totalCliques = duration_cast<seconds>(startStep - startStep);

    while(isOptimal_ == false)
    {
    	startStep = high_resolution_clock::now(); 		//Begin of MCMCF
        this->solveMinCostMultiCommodityFlow_Cplex();
        endStep = high_resolution_clock::now();			//End of MCMCF
        duration = duration_cast<seconds>(endStep - startStep);
        totalMCMCF = totalMCMCF + duration;
        cout << "TIME == MCMCF duration in seconds: " << duration.count() << endl; 

        // If MCMCF was infeasible we can end the program if it is the first iteration or upgrade the lower bound and look for a previous feasible solution
        if ((MCMCFSolved_ == false) || (MCMCF_Output_.getCapacity() > maxSlots_)) 
    	{
		   	if (iterations_ == 0)
		   	{
		   		cout << "The first iteration of MCMCF is impossible for this instance" << endl;
		   		cout << "Problem is infeasible" << endl;
		   		return;
		   	}
		   	//If there are no cliques to reallow, this means that all possible routes were already sugested and a feasible solution was not found
		   	if (forbiddenCliques_.size() == 0)
		   	{
		   		cout << "This iteration of MCMCF is infeasible, and there are no cliques to reallow " << endl;
		   		cout << "No new routing can be proposed" << endl;
		   		cout << "Lower bound update to upper bound + 1 so we can look one last time for a solution" << endl;
		   		lowerBound_ = maxSlots_+ 1;
		    	cout << "Lower Bound UPDATED: " << lowerBound_ << endl;
		    	cout << "Upper Bound: " << upperBound_ << endl;
		   	}
		   	cout << "This iteration of MCMCF is infeasible, updating lower bound to the smallest clique blocked " << endl;
		   	//This will make the program look for a previous solution		
		   	lowerBound_ = smallestClique_;
		   	cout << "Lower Bound UPDATED: " << smallestClique_ << endl;
		   	cout << "Upper Bound: " << upperBound_ << endl;
		}
		// If MCMCF was feasible we print the solution and verify if the lower bound got higher
		// If the solution is smallest than the smallest forbidden clique, this solution is the new lb, else, the weight of the smallest clique
		else
		{
	    	cout << MCMCF_Output_ << endl;
	    	// If the last MCMCF was feasible and the solution is feasible to the rsa, we should add the proposed routing to the pool
	    	addNewRoutesToRSA();
			if (MCMCF_Output_.getCapacity() > lowerBound_)
		    {
		    	if (MCMCF_Output_.getCapacity() <= smallestClique_)
		    	{
		    		lowerBound_ = MCMCF_Output_.getCapacity();
		    		cout << "Lower Bound UPDATED: " << lowerBound_ << endl;
		    		cout << "Upper Bound: " << upperBound_ << endl;
		    	}
		    	else
		    	{
		    		lowerBound_ = smallestClique_;
		    		cout << "There is at lest one clique of omega between the lower bound and the solution found" << endl;
		    		cout << "This clique has weight: " << smallestClique_ << endl;
		    		cout << "Lower Bound UPDATED: " << lowerBound_ << endl;
		    		cout << "Upper Bound: " << upperBound_ << endl;
		    	}		    		
		    }
		 }
		
		// If the lower bound grew, maybe an ancient solution becomes feasible, so we verify if we already have this solution if 
		// lower bound matches upper bound, there are only 2 possible situations
		// 1 - lower bound match a solution (as the upper bound is the value of a solution), so we find it
		// 2 - lower bound match the maximal number of slots + 1 (upper bound was never changed), if the lower bound matchs the number of maximal slots we could find a solotion
		// but if the lower bound matches the number of maximal slots + 1 and we do not have a solution yet, the problem is infeasible
	   	if (lowerBound_ >= upperBound_)
		{
			cout << "Lower bound matches upper bound" << endl;
			lookForFeasibleSolution();
		    // if we found an optimal previous solution in any of the two situations above, we show it in the screen and stop the execution
		    if(isOptimal_ == true)
			{
				cout << "===== SOLUTION =====" << endl;
				cout << "This solution becomes feasible and this iteration will be stopped" << endl;
				cout << RSA_Output_ << endl;
				break;
			}
			else
			{
				cout << "Problem is infeasible" << endl;
			}
		}

		// If there's not a solution yet, the iteration will continue with the steps below, if there is it will stop 		
		// As there's not a feasible solution yet, we can allow some cliques that respect the new lower bound to appear again
	    // Also, if we reallow at least one clique, we should not go to the RSA step. We can solve the MCMCF with this clique reallowed
	    if (allowFeasibleCliques() == true)
	    {
	    	cout << "One or more cliques were allowed, solving one extra MCMCF" << endl;
	    	startStep = high_resolution_clock::now(); 		//Begin of MCMCF
       		this->solveMinCostMultiCommodityFlow_Cplex();
       		endStep = high_resolution_clock::now();			//End of MCMCF
       		duration = duration_cast<seconds>(endStep - startStep);
       		totalMCMCF = totalMCMCF + duration;
       		cout << "TIME == MCMCF duration in seconds: " << duration.count() << endl; 
       		//If this second MCMCF had a solution, we can add this solution to the pool
       		if ((MCMCFSolved_ == true) && (MCMCF_Output_.getCapacity() <= maxSlots_)) 
       		{
       			cout << "The following solution will become a possible routing" << endl;
       			cout << MCMCF_Output_ << endl;
       			addNewRoutesToRSA();
       		}
       		else
       		{
       			cout << "The extra MCMCF is infeasible, nothing will be done" << endl;
       		}
	    }

		// Here, we begin tha SA part of the problem
        showPossiblePaths();
        startStep = high_resolution_clock::now();			//Begin RSA
        this->solveEdgePathFormulation_Cplex();
        endStep = high_resolution_clock::now();				//End RSA
        duration = duration_cast<seconds>(endStep - startStep);
        totalRSA = totalRSA + duration;
        cout << "TIME == RSA duration in seconds: " << duration.count() << endl; 

        // If gap is not 0, it means that we interrupted the last EPF by time limit
        if (gap_ > 0)
        {
		   	cout << "RSA TOOK TOO LONG" << endl;
		   	cout << "Solution found but there is a gap of: " << gap_;
        }
        // If RSA solution is optimal we found our solution and we can end the program
        if(isOptimal_ == true)
		{
			cout << "===== SOLUTION =====" << endl;
			cout << "This RSA solution matches the lowerBound_" << endl;
			cout << RSA_Output_ << endl;
			break;
		}
		// After solving the RSA we shall look for cliques in the solution
		if (RSASolved_ == true) 
		{
	       	cout << RSA_Output_ << endl;
	       	//For instances with a big number of avaible slot and a big number of demands we can use the first solution to reduce the number of variables that will be created
	       	//With this, the cplex will run faster.
	       	if ((iterations_ == 0) && (RSA_Input_.getRequests().size() >= 50))
			{
				cout << "The first iteration of RSA will reduce the number of variables for the problem" << endl;
				cout << "The actual value is: " << maxSlots_ << " and will become: " << RSA_Output_.getSlots() << endl;
				maxSlots_ = RSA_Output_.getSlots();
			}

	       	if (RSA_Output_.getSlots() < upperBound_)
	       	{
	       		upperBound_ = RSA_Output_.getSlots();
		    	cout << "Upper Bound UPDATED: " << upperBound_ << endl;
		    	cout << "Lower Bound: " << lowerBound_ << endl;	
	       	}
	       	startStep = high_resolution_clock::now(); 			//Begin search for cliques
	      	maxWeightedClique(RSA_Output_.getRouting());
	       	endStep = high_resolution_clock::now();				//End search for cliques
	       	duration = duration_cast<seconds>(endStep - startStep);
	       	totalCliques = totalCliques + duration;
	       	cout << "TIME == Finding cliques duration in seconds: " << duration.count() << endl;
	    }
	    // If this RSA iteration was infeasible, we can just finish the iteration and go to MCMCF again
	    else
	    {
	    	cout << "This RSA iteration was infeasible" << endl;
	    } 
        showForbiddenCliques();
    
        cout << endl << "ITERATION FINISHED" << endl;
        // TIME CONTROL
       	if (totalRSA.count() > limit.count())
       	{
       		cout << "Limit of " << limit.count() << " passed" << endl;
       		cout << "LB: " << lowerBound_ << endl;
       		cout << "UB: " << upperBound_ << endl;
       		return; 
       	}

       	iterations_ ++;
    }
    cout << "---------------------------------------------------: " << endl;
    cout << "Total time spent in MCMCF: " << totalMCMCF.count() << endl;
    cout << "Total time spent in RSA: " << totalRSA.count() << endl;
    cout << "Total time spent finding cliques: " << totalCliques.count() <<endl;
    cout << "Iterations: " << iterations_ + 1 << endl;
    cout << "----------------END OF EXECUTION-------------------: "  << endl;
}



void RSA_Algorithms::solveMinCostMultiCommodityFlow_Cplex(){

	cout << endl << "---------------BEGIN OF ILP Min Cost Multi Commodity Cplex------------------" << endl;
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
			//std::cout << "Variable : " << k << "\t||" << a << "\t||" <<  Var_Name << std::endl;
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
	cout << endl << "Capacity :" << model.getValue(Cap) << endl;

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
	cout << endl << "---------------END OF ILP Min Cost Multi Commodity Cplex------------------" << endl;

}

void RSA_Algorithms::solveEdgePathFormulation_Cplex()
{
	cout << endl << "---------------BEGIN OF ILP Edge Path Formulation Cplex------------------" << endl;

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
	std::cout << "Number of slices from MCMCF : " << S << endl;

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
	// Constraints from previous RSA resolution
    // For each forbidden routing that already has been a solution for a previous RSA, we shall forbid it appearing again
    // If there are not forbidden routing, the program will not execute this part
    for (unsigned i = 0; i < impossibleRoutings_.size(); ++i)
    {
        IloExpr forbiddenRoutes(RSA);
        int counter = 0;
        //unsigned vertexIndex;
        // for each demand attend
        for (unsigned k = 0; k < impossibleRoutings_[i].getRouting().size(); ++k)
        {
            // Start in the vertex of the origin of the demand
            //vertexIndex = RSA_Input_.getRequests()[k]->getOrigin().getIndex();
            // Add the part of the constraint for all the edges in the path choose
            for (unsigned j = 0; j < impossibleRoutings_[i].getRouting()[k]->getEdges().size(); ++j)
            {
	            forbiddenRoutes += xk_e[RSA_Input_.getRequests()[k]->getIndex()-1][impossibleRoutings_[i].getRouting()[k]->getEdges()[j]->getIndex()-1];
    	        //vertexIndex = impossibleRoutings_[i].getRouting()[k]->getEdges()[j]->getV2().getIndex();
        	    counter ++;
            }
        }
        ILP_RSA.add(forbiddenRoutes <= counter-1);
        forbiddenRoutes.end();
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
	model.setParam(IloCplex::TiLim, 1800); // Execution time limited

	if (model.solve() == false)
    {
    	RSASolved_ = false;
    	gap_ = 100;
    	return;
    }
    RSASolved_ = true;

	cout << endl << "Objective Value: " << model.getObjValue() << endl;
	cout << endl << "Gap: " << model.getMIPRelativeGap() << endl;
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
	cout << endl << "---------------END OF ILP Edge Path Formulation Cplex------------------" << endl;
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

void RSA_Algorithms::lookForFeasibleSolution()
{
	for (unsigned i = 0; i < impossibleRoutings_.size();++i)
	{ 
		if (lowerBound_ >= impossibleRoutings_[i].getSlots())
		{
			isOptimal_ = true;
			lowerBound_  = impossibleRoutings_[i].getSlots();
			RSA_Output_ = impossibleRoutings_[i];
			for (unsigned j = i+1; j < impossibleRoutings_.size();++j)
			{
				if (lowerBound_ > impossibleRoutings_[j].getSlots())
				{
					lowerBound_  = impossibleRoutings_[j].getSlots();
					RSA_Output_ = impossibleRoutings_[j];
				}
			}
			break;
		}
	}
}

bool RSA_Algorithms::allowFeasibleCliques()
{
	bool allowed = false;
	vector<Clique>::iterator it = forbiddenCliques_.begin();
	while (it != forbiddenCliques_.end())
	{
	   	if (it->getOmega() <= lowerBound_)
	    {
	    	cout << "The clique: " << endl << *it << "Becomes feasible" << endl;
	     	forbiddenCliques_.erase(it);
	     	allowed = true;
	    }
	    else
	    {
	        ++it;
	    }
	}
	it = forbiddenCliques_.begin();
	smallestClique_ = upperBound_+ 1;
	while (it != forbiddenCliques_.end())
	{
	   	if (it->getOmega() < smallestClique_)
	    {
	    	smallestClique_ = it->getOmega();
	    }
	    ++it;
	}
	if (allowed == true)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void RSA_Algorithms::maxWeightedClique(vector<Path*> & routing)
{
    //--------------Begin of the construction of the intersection graph ----------------
    vector<Vertex*> nodes;
    vector<Edge*> edges;
    for (unsigned k = 0; k < routing.size(); ++k)
    {
        Vertex* vertex  = new Vertex(routing[k]->getIndexDemand(),routing[k]->getDemand()->getSlots());
        nodes.push_back(vertex);
    }

    //edge intersection
    bool pathIntersected;
    unsigned index_edges = 1;
    //Comparation between two different paths if they share at least one edge
    for (unsigned k = 0; k < routing.size() - 1; ++k)
    {
        for (unsigned l = k+1; l < routing.size(); ++l)
        {
            // Suppose that the two different paths do not share at least edge
            pathIntersected = false;
            for(unsigned ke = 0; ke <routing[k]->getEdges().size(); ++ke)
            {
                if(pathIntersected == true)
                    break;
                for(unsigned le = 0; le <routing[l]->getEdges().size(); ++le)
                {
                    //If it is found one edge in common, update the value of path intersection
                    if (routing[k]->getEdges()[ke]->getIndex() == routing[l]->getEdges()[le]->getIndex())
                    {
                        pathIntersected = true;
                        break;
                    }
                }
            }
            //If there are at least one edge in common, add a edge in the graph.
            //Update the neighborhood of each vertex
            if (pathIntersected == true)
            {
                Edge* edge = new Edge(nodes[k],nodes[l],index_edges);
                edges.push_back(edge);
                index_edges ++;
                nodes[k]->addNeighborhood(*(nodes[l]));
                nodes[l]->addNeighborhood(*(nodes[k]));
                nodes[k]->addVertexEdges(*edge);
                nodes[l]->addVertexEdges(*edge);
            }
        }
    }

    //--------------End of the construction of the intersection graph ----------------
    //--------------Begin of the algorithm to find the max weighted clique number ----
    //Best clique already found
    //Construction of the first clique
    Clique tests,
    cliqueStar;
    vector<unsigned> constructed_clique(0);
    vector<unsigned> constructed_clique_star(0); // The aim of this vector is to know the position in the routing parameter in this function which path was selectioned to be in the clique star
    bool inClique;
    for (unsigned i = 0; i < nodes.size(); ++i)
    {
        inClique = tests.tryAddVertex(nodes[i]);
        if (inClique == true)
        {
            constructed_clique.push_back(i);
        }
    }
    cliqueStar = tests;
    constructed_clique_star = constructed_clique;
    if (tests.getOmega() > lowerBound_)
   	{
        cliqueStar = tests;
		//--------------Begin of the algorithm to find forbidden edges ----
		//Verify the idea
		for (unsigned i = 0; i < cliqueStar.getVertices().size() - 1; ++i)
		{
		    for (unsigned j = i+1; j < cliqueStar.getVertices().size(); ++j)
		    {
		    	for (vector<Edge*>::iterator it = routing[constructed_clique_star[i]]->getEdges().begin(); it != routing[constructed_clique_star[i]]->getEdges().end(); ++it)
					{
					for (vector<Edge*>::iterator it2 = routing[constructed_clique_star[j]]->getEdges().begin(); it2 != routing[constructed_clique_star[j]]->getEdges().end(); ++it2)
					{
						if ((*it)->getIndex() == (*it2)->getIndex())
							cliqueStar.addForbiddenEdgesFromCliques((*it), routing[constructed_clique_star[i]]->getDemand(), routing[constructed_clique_star[j]]->getDemand());
					}
				}
		  	}
		}
		for (unsigned i = 0; i < constructed_clique_star.size(); ++i)
		{
			cliqueStar.addPath(routing[constructed_clique_star[i]]);			//this will add the path in the clique
		}
		forbiddenCliques_.push_back(cliqueStar);
		if (cliqueStar.getOmega() < smallestClique_)
		{
		   	smallestClique_ = cliqueStar.getOmega();
		}
    }
    //Backtrack in the clique
    //This while verify if there are other possibles cliques to try
    while(constructed_clique[0] < nodes.size()-1 )
    {
        // Find the last vertex in the clique
        unsigned last_vertex = constructed_clique[constructed_clique.size()-1];
        //Remove the las tvertex inth test clique
        tests.removeTheLastVertex();
        constructed_clique.pop_back();
        //Try to add other vertex, maybe impossible because of the last vertex
        for (unsigned i = last_vertex+1; i < nodes.size(); ++i)
        {
            inClique = tests.tryAddVertex(nodes[i]);
            if (inClique == true)
            {
                constructed_clique.push_back(i);
        	}
        }
        //If the new clique has weighted greater omega than the best clique already found, update
        if (tests.getOmega() > lowerBound_)
        {
        	cliqueStar = tests;
        	constructed_clique_star = constructed_clique;
		    //--------------Begin of the algorithm to find forbidden edges ----
		    //Verify the idea
		    for (unsigned i = 0; i < cliqueStar.getVertices().size() - 1; ++i)
		    {
		    	for (unsigned j = i+1; j < cliqueStar.getVertices().size(); ++j)
		    	{
		    		for (vector<Edge*>::iterator it = routing[constructed_clique_star[i]]->getEdges().begin(); it != routing[constructed_clique_star[i]]->getEdges().end(); ++it)
						{
						for (vector<Edge*>::iterator it2 = routing[constructed_clique_star[j]]->getEdges().begin(); it2 != routing[constructed_clique_star[j]]->getEdges().end(); ++it2)
						{
							if ((*it)->getIndex() == (*it2)->getIndex())
								cliqueStar.addForbiddenEdgesFromCliques((*it), routing[constructed_clique_star[i]]->getDemand(), routing[constructed_clique_star[j]]->getDemand());
						}
					}
		    	}
		    }
		    for (unsigned i = 0; i < constructed_clique_star.size(); ++i)
			{
				cliqueStar.addPath(routing[constructed_clique_star[i]]);			//this will add the path in the clique
			}
		    forbiddenCliques_.push_back(cliqueStar);
		    if (cliqueStar.getOmega() < smallestClique_)
		    {
		    	smallestClique_ = cliqueStar.getOmega();
		    }
        }
       //Observe that, if there aren't vertex to add, one vertex will be retired and it's all in the iteration.
       // We wait in some moment there are no more option and the cycle finish in some moment.
    }
}

void 	RSA_Algorithms::showPossiblePaths() const {
	cout << endl << "---------------BEGIN OF Possible paths Information------------------" << endl;
	for (unsigned i = 0; i < possiblePaths_.size(); ++i){
		cout << "For demand: " << i+1 << endl;
		for (unsigned j = 0; j < possiblePaths_[i].size(); ++j){
			cout << *(possiblePaths_[i][j]);
		}
		cout << "***********************: " << endl;
	}
	cout << endl << "---------------END OF Possible paths Information------------------" << endl;
}

void 	RSA_Algorithms::showForbiddenCliques() const
{
	cout << endl << "---------------BEGIN OF Forbidden Cliques Information------------------" << endl;
	for (unsigned i = 0; i < forbiddenCliques_.size(); ++i)
	{
		cout << forbiddenCliques_[i] << endl;
	}
	cout << endl << "---------------END OF Forbidden Cliques Information------------------" << endl;
}
