#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <set>
#include "Vertex.hpp"
#include "Edge.hpp"
#include "Arc.hpp"
#include "Path.hpp"
#include "Graph.hpp"
#include "Demand.hpp"
#include "Clique.hpp"
#include "Obstruction.hpp"
#include "RSA_Input.hpp"
#include "MCMCF_Output.hpp"
#include "RSA_Output.hpp"
#include "RSA_Algorithms.hpp"
#include "IterationInfo.hpp"
#include <ilcplex/ilocplex.h>
#include <chrono> 
#include <math.h> 
ILOSTLBEGIN

using namespace std;
using namespace std::chrono;

RSA_Algorithms::RSA_Algorithms(RSA_Input rsa) : RSA_Input_(rsa)
{
	tConstraintsMCF_ = 0; tConstraintsEPF_ = 0; nCallsMCF_ = 0; nCallsEPF_ = 0; tTimeMCF_ = 0; tTimeEPF_ = 0;
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
	cliqueBound_ = maxSlots_ + 1;
	cout << endl << "-------------- BEGIN OF Instance Information -------------------------" << endl;
	cout << "  Graph G  " << endl;
	cout << G_ << endl;
	cout << "  Graph G prime " << endl;
	cout << GPrime_ << endl;
	cout << "  Demands  " << endl;
	rsa.showRequests();
	cout << endl << "-------------- END OF Instance Information ---------------------------" << endl << endl;
	// Preprocessing for mirror demands
	mirrorDemands_ = findMirrorDemands(RSA_Input_.getRequests());
	obstructions_.setDemands(RSA_Input_.getRequests());
	obstructions_.setDemandGroups(mirrorDemands_);
	vector<vector<Path*>*> pathPool(RSA_Input_.getRequests().size());
	possiblePathsNew_ = pathPool;
	for (int i = 0; i < mirrorDemands_.size(); i++)
	{
		vector<Path*>* pathSet = new vector<Path*>();
		for (int j = 0; j < mirrorDemands_[i].size(); j++)
		{
			for (int k = 0; k < mirrorDemands_[i][j].size(); k++)
			{
				Demand d = *mirrorDemands_[i][j][k];
				possiblePathsNew_[d.getIndex() - 1] = pathSet;
			}
		}
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

void RSA_Algorithms::framework(char *summaryFileName, bool skipEPF)
{
	// Clocks
	auto fwTime_i = high_resolution_clock::now();
	auto fwTime_f = high_resolution_clock::now();
	auto iterationTime_i = high_resolution_clock::now();
	auto iterationTime_f = high_resolution_clock::now();
	auto iterationExecTime = duration_cast<milliseconds>(iterationTime_f - iterationTime_i);

	// Output Information 
	// TODO: Extranct in a wrapper object and desacoplate processing from "presentation"
	vector<IterationInfo> iterationsInfo;

	// Framework's Flow Control Variables
	int NumOfDemands = RSA_Input_.getRequests().size();
	bool newPathsAdded = false;
	bool newCliquesAllowed = false;
	bool epfSolved = false;
	bool skipNextEPF = false;

	// Save Initialization values
	iterationsInfo.push_back(IterationInfo(-2, lowerBound_, cliqueBound_, upperBound_));

	//time markers initialization
	auto startStep = high_resolution_clock::now();
	auto endStep = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(endStep - startStep);
	
	//total times initialization
	auto totalMCMCF = duration_cast<milliseconds>(startStep - startStep);
	auto totalRSA = duration_cast<milliseconds>(startStep - startStep);
	auto totalCliques = duration_cast<milliseconds>(startStep - startStep);

	cout << "=========================================================================================" << endl;
	cout << "============== BEGIN OF FW EXECUTION ====================================================" << endl;
	cout << "=========================================================================================" << endl;
	//Initial MCMCF
	startStep = high_resolution_clock::now(); 		//Begin of MCMCF
	this->solveMinCostMultiCommodityFlow_Cplex();
	endStep = high_resolution_clock::now();			//End of MCMCF
	duration = duration_cast<milliseconds>(endStep - startStep);
	MCMCFDuration_ = duration.count();
	totalMCMCF = totalMCMCF + duration;
	cout << "TIME == MCMCF duration in ms: " << duration.count() << endl << endl;

	if ((!MCMCFSolved_) || (MCMCF_Output_.getCapacity() > maxSlots_))
	{
		cout << "============== INFEASIBLE INSTANCE ======================================================" << endl;
		iterationsInfo.push_back(IterationInfo(-1, lowerBound_, cliqueBound_, upperBound_, MCMCFSolved_, MCMCFDuration_, MCMCF_Output_.getCapacity(), nullopt, nullopt, nullopt, nullopt, nullopt, nullopt, nullopt, nullopt, MCMCFDuration_));
		return;
	}
	else
	{
		lowerBound_ = MCMCF_Output_.getCapacity();
		newPathsAdded = UpdatePossiblePathsSet();
		iterationsInfo.push_back(IterationInfo(-1, lowerBound_, cliqueBound_, upperBound_, MCMCFSolved_, MCMCFDuration_, MCMCF_Output_.getCapacity(), nullopt, nullopt, nullopt, nullopt, nullopt, newPathsAdded, nullopt, nullopt, MCMCFDuration_));
	}

	while (!isOptimal_)
	{
		iterationTime_i = high_resolution_clock::now();
		cout << "============== BEGIN ITERATION " << iterations_ << " ========================================================" << endl;
		cout << "============== LB " << lowerBound_ << endl;
		cout << "============== UB " << upperBound_ << endl;
		cout << "============== CB " << cliqueBound_ << endl;
		showPossiblePaths();
		cout << endl;

		// EPF: Edge-Path Formulation
		if (!skipNextEPF)
		{
			startStep = high_resolution_clock::now(); //Begin RSA
			this->solveEdgePathFormulation_Cplex();
			endStep = high_resolution_clock::now(); //End RSA
			duration = duration_cast<milliseconds>(endStep - startStep);
			EPFDuration_ = duration.count();
			totalRSA = totalRSA + duration;
			cout << "TIME == EPF duration in ms: " << duration.count() << endl << endl;
			epfSolved = RSASolved_;
		}
		else
		{
			cout << "-------------- EPF CPLEX SKIPPED -------------------------------------" << endl << endl;
		}

		if (epfSolved)
		{
			if (RSA_Output_.getSlots() == (NumOfDemands * lowerBound_))
			{
				cout << "============== OPTIMAL SOLUTION FOUND ===================================================" << endl;
				cout << RSA_Output_ << endl;
				cout << "============== END ITERATION   " << iterations_ << " ========================================================" << endl << endl;
				isOptimal_ = true;
				iterationTime_f = high_resolution_clock::now();
				iterationExecTime = duration_cast<milliseconds>(iterationTime_f - iterationTime_i);
				iterationsInfo.push_back(IterationInfo(iterations_, lowerBound_, cliqueBound_, upperBound_, nullopt, nullopt, nullopt, epfSolved, skipNextEPF, EPFDuration_, RSA_Output_.getSlots(), RSA_Output_.getSpan(), newPathsAdded, newCliquesAllowed, nullopt, iterationExecTime.count()));
				break;
			}
			else
			{
				if (RSA_Output_.getSpan() != upperBound_)
				{
					unsigned aux = upperBound_;
					upperBound_ = RSA_Output_.getSpan();
					cout << "/!\\ Updated UB (" << aux << " -> " << upperBound_ << ")" << endl << endl;
				}

				vector<vector<Path*>> allRoutings;
				allRoutings.push_back(MCMCF_Output_.getRouting());
				allRoutings.push_back(RSA_Output_.getRouting());

				startStep = high_resolution_clock::now(); 			//Begin search for cliques
				for (vector<vector<Path*>>::iterator rting = allRoutings.begin(); rting != allRoutings.end(); rting++)
				{
					maxWeightedClique(*rting);
				}
				endStep = high_resolution_clock::now();				//End search for cliques
				duration = duration_cast<milliseconds>(endStep - startStep);
				CliquesDuration_ = duration.count();
				totalCliques = totalCliques + duration;
				cout << "TIME == Finding cliques duration in ms: " << duration.count() << endl;
				showForbiddenCliques();
				cout << endl;
				
				if (!obstructions_.getNumberCliques() == 0)
				{
					unsigned wq = obstructions_.minForbiddenCliquesWeight();
					if (wq < cliqueBound_)
					{
						unsigned aux = cliqueBound_;
						cliqueBound_ = wq;
						cout << "/!\\ Updated CB (" << aux << " -> " << cliqueBound_ << ")" << endl << endl;
					}
				}
				obstructions_.insert(allRoutings, possiblePathsNew_);
			}
		}
		else
		{
			vector<vector<Path*>> allRoutings;
			allRoutings.push_back(MCMCF_Output_.getRouting());
			obstructions_.insert(allRoutings, possiblePathsNew_);
		}

		// MCMCF: Min-Cost Multicomodity Flow
		startStep = high_resolution_clock::now(); 		//Begin of MCMCF
		this->solveMinCostMultiCommodityFlow_Cplex();
		endStep = high_resolution_clock::now();			//End of MCMCF
		duration = duration_cast<milliseconds>(endStep - startStep);
		MCMCFDuration_ = duration.count();
		totalMCMCF = totalMCMCF + duration;
		cout << "TIME == MCMCF duration in ms: " << duration.count() << endl << endl;

		while (!MCMCFSolved_)
		{
			if (cliqueBound_ == upperBound_ && upperBound_ == maxSlots_ + 1)
			{
				cout << "============== INFEASIBLE INSTANCE ======================================================" << endl;
				cout << "============== END ITERATION   " << iterations_ << " ========================================================" << endl << endl;
				iterationTime_f = high_resolution_clock::now();
				iterationExecTime = duration_cast<milliseconds>(iterationTime_f - iterationTime_i);
				iterationsInfo.push_back(IterationInfo(iterations_, lowerBound_, cliqueBound_, upperBound_, MCMCFSolved_, MCMCFDuration_, MCMCF_Output_.getCapacity(), epfSolved, skipNextEPF, EPFDuration_, RSA_Output_.getSlots(), RSA_Output_.getSpan(), newPathsAdded, newCliquesAllowed, CliquesDuration_, iterationExecTime.count()));
				return;
			}
			else
			{
				if (upperBound_ <= cliqueBound_ && upperBound_ <= maxSlots_)
				{
					cout << "============== OPTIMAL SOLUTION FOUND ===================================================" << endl;
					cout << RSA_Output_ << endl;
					cout << "============== END ITERATION   " << iterations_ << " ========================================================" << endl << endl;
					isOptimal_ = true;
					iterationTime_f = high_resolution_clock::now();
					iterationExecTime = duration_cast<milliseconds>(iterationTime_f - iterationTime_i);
					iterationsInfo.push_back(IterationInfo(iterations_, lowerBound_, cliqueBound_, upperBound_, MCMCFSolved_, MCMCFDuration_, MCMCF_Output_.getCapacity(), epfSolved, skipNextEPF, EPFDuration_, RSA_Output_.getSlots(), RSA_Output_.getSpan(), newPathsAdded, newCliquesAllowed, CliquesDuration_, iterationExecTime.count()));
					break;
				}
				else
				{
					newCliquesAllowed = obstructions_.AllowFeasible(cliqueBound_);
					showForbiddenCliques();
					unsigned aux = cliqueBound_;
					if (obstructions_.getNumberCliques() == 0)
					{
						cliqueBound_ = maxSlots_ + 1;
					}
					else
					{
						cliqueBound_ = obstructions_.minForbiddenCliquesWeight();
					}
					cout << "/!\\ Updated CB (" << aux << " -> " << cliqueBound_ << ")" << endl << endl;
				}
			}
			startStep = high_resolution_clock::now(); 		//Begin of MCMCF
			this->solveMinCostMultiCommodityFlow_Cplex();
			endStep = high_resolution_clock::now();			//End of MCMCF
			duration = duration_cast<milliseconds>(endStep - startStep);
			MCMCFDuration_ = duration.count();
			totalMCMCF = totalMCMCF + duration;
			cout << "TIME == MCMCF duration in ms: " << duration.count() << endl << endl;
		}

		// At this point we know MCMCFSolved_ == true
		// Except if an Optimal Solution was found in the inneer While loop of MCMCF
		if (!MCMCFSolved_)
			break;
		
		unsigned cap = MCMCF_Output_.getCapacity();
		if (cliqueBound_ == upperBound_ && upperBound_ == maxSlots_ + 1 && maxSlots_ + 1 <= cap)
		{
			cout << "============== INFEASIBLE INSTANCE ======================================================" << endl;
			cout << "============== END ITERATION   " << iterations_ << " ========================================================" << endl << endl;
			iterationTime_f = high_resolution_clock::now();
			iterationExecTime = duration_cast<milliseconds>(iterationTime_f - iterationTime_i);
			iterationsInfo.push_back(IterationInfo(iterations_, lowerBound_, cliqueBound_, upperBound_, MCMCFSolved_, MCMCFDuration_, MCMCF_Output_.getCapacity(), epfSolved, skipNextEPF, EPFDuration_, RSA_Output_.getSlots(), RSA_Output_.getSpan(), newPathsAdded, newCliquesAllowed, CliquesDuration_, iterationExecTime.count()));
			return;
		}
		else
		{
			if ((cliqueBound_ == upperBound_ && upperBound_ <= maxSlots_ && upperBound_ <= cap) || (upperBound_ < cliqueBound_ && upperBound_ <= cap))
			{
				cout << "============== OPTIMAL SOLUTION FOUND ===================================================" << endl;
				cout << RSA_Output_ << endl;
				cout << "============== END ITERATION   " << iterations_ << " ========================================================" << endl << endl;
				isOptimal_ = true;
				iterationTime_f = high_resolution_clock::now();
				iterationExecTime = duration_cast<milliseconds>(iterationTime_f - iterationTime_i);
				iterationsInfo.push_back(IterationInfo(iterations_, lowerBound_, cliqueBound_, upperBound_, MCMCFSolved_, MCMCFDuration_, MCMCF_Output_.getCapacity(), epfSolved, skipNextEPF, EPFDuration_, RSA_Output_.getSlots(), RSA_Output_.getSpan(), newPathsAdded, newCliquesAllowed, CliquesDuration_, iterationExecTime.count()));
				break;
			}
			else
			{
				if (cap < cliqueBound_ && cap < upperBound_)
				{
					if (cap != lowerBound_)
					{
						unsigned aux = lowerBound_;
						lowerBound_ = cap;
						cout << "/!\\ Updated LB (" << aux << " -> " << lowerBound_ << ")" << endl << endl;
					}
				}

				if (cliqueBound_ <= cap && cliqueBound_ < upperBound_)
				{
					unsigned aux = lowerBound_;
					lowerBound_ = cliqueBound_;
					cout << "/!\\ Updated LB (" << aux << " -> " << lowerBound_ << ")" << endl << endl;

					newCliquesAllowed = obstructions_.AllowFeasible(cliqueBound_);
					showForbiddenCliques();
					aux = cliqueBound_;
					if (obstructions_.getNumberCliques() == 0)
					{
						cliqueBound_ = maxSlots_ + 1;
					}
					else
					{
						cliqueBound_ = obstructions_.minForbiddenCliquesWeight();
					}
					cout << "/!\\ Updated CB (" << aux << " -> " << cliqueBound_ << ")" << endl << endl;
				}
				
				newPathsAdded = UpdatePossiblePathsSet();
			}
		}

        cout << "============== END ITERATION   " << iterations_ << " ========================================================" << endl << endl;
		iterationTime_f = high_resolution_clock::now();
		iterationExecTime = duration_cast<milliseconds>(iterationTime_f - iterationTime_i);
		iterationsInfo.push_back(IterationInfo(iterations_, lowerBound_, cliqueBound_, upperBound_, MCMCFSolved_, MCMCFDuration_, MCMCF_Output_.getCapacity(), epfSolved, skipNextEPF, EPFDuration_, RSA_Output_.getSlots(), RSA_Output_.getSpan(), newPathsAdded, newCliquesAllowed, CliquesDuration_, iterationExecTime.count()));

		skipNextEPF = skipEPF ? !(newPathsAdded || newCliquesAllowed) : skipEPF;
		newPathsAdded = false;
		newCliquesAllowed = false;
		epfSolved = false;
		CliquesDuration_ = 0;
       	iterations_ ++;
    }

	fwTime_f = high_resolution_clock::now();
	auto fwExecTime = duration_cast<milliseconds>(fwTime_f - fwTime_i);
	cout << "=========================================================================================" << endl;
	cout << "============== END OF FW EXECUTION ======================================================" << endl;
	cout << "=========================================================================================" << endl;
    cout << "============== Total time spent in MCMCF: " << totalMCMCF.count() << endl;
	cout << "============== Average time spent in MCMCF: " << totalMCMCF.count() / nCallsMCF_ << endl;
	cout << "============== Average number of constraints in MCMCF: " << tConstraintsMCF_ / nCallsMCF_ << endl;
    cout << "============== Total time spent in RSA: " << totalRSA.count() << endl;
	cout << "============== Average time spent in RSA: " << totalRSA.count() / nCallsEPF_ << endl;
	cout << "============== Average number of constraints in RSA: " << tConstraintsEPF_ / nCallsEPF_ << endl;
    cout << "============== Total time spent finding cliques: " << totalCliques.count() <<endl;
    cout << "============== Iterations: " << iterations_ + 1 << endl;
	cout << "============== Total framework running time: " << fwExecTime.count() << endl;

	if (summaryFileName != NULL)
	{
		ofstream summary;
		summary.open(summaryFileName);
		summary << "IT; EPF; EPFt; EPFopt; EPFSpan; MCMCF; MCMCFt; MCMCFopt; Lb; Cb; Ub; newPaths?; reallowedCliques?; CQt; ITt" << endl;
		vector<IterationInfo>::iterator it = iterationsInfo.begin();
		while (it != iterationsInfo.end())
		{
			summary << *it << endl;
			it++;
		}
		summary.close();
	}
}

void RSA_Algorithms::solveMinCostMultiCommodityFlow_Cplex(){

	cout << "-------------- BEGIN OF ILP MCMCF CPLEX ------------------------------" << endl;
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
			std::string Var_Name = "f[" + to_string(k + 1) + "," + to_string(a + 1) + "]";
			f_kl[k][a].setName(Var_Name.c_str());
			ILP_MCMCF.add(f_kl[k][a]);
			variables.add(f_kl[k][a]);
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
	
	// Source constraints (2b) : In the origin the flow will go out to exactly one arc
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

	// Source contraints (2b') : In the origin there's no ingoing flow 
	for (unsigned k = 0; k < K; k++)
	{
		IloExpr Path_ingoing(MCMCF);
		for (unsigned a = 0; a < RSA_Input_.getRequests()[k]->getOrigin().getVertexInArcs().size(); a++)
		{
			Path_ingoing += f_kl[RSA_Input_.getRequests()[k]->getIndex()-1][RSA_Input_.getRequests()[k]->getOrigin().getVertexInArcs()[a]->getIndex()-1];
		}
		ILP_MCMCF.add(Path_ingoing == 0);
		Path_ingoing.end();
	}

	// Sink constraints (2c) : In the destination the flow will arrive from exactly one arc
	for (unsigned k = 0; k < K; k++)
	{
		IloExpr Path_ingoing(MCMCF);
		for (unsigned a = 0; a < RSA_Input_.getRequests()[k]->getDestination().getVertexInArcs().size(); a++)
		{
			Path_ingoing += f_kl[RSA_Input_.getRequests()[k]->getIndex() - 1][RSA_Input_.getRequests()[k]->getDestination().getVertexInArcs()[a]->getIndex() - 1];
		}
		ILP_MCMCF.add(Path_ingoing == 1);
		Path_ingoing.end();
	}

	// Sink constraints (2c') : In the destination theres no outgoing flow
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

	// Flow conservation constraints (2d)
	for (unsigned k = 0; k < K; k++)
	{
		for (unsigned v = 0; v < V; ++v)
		{
			if ((GPrime_.getVertices()[v]->getIndex() != RSA_Input_.getRequests()[k]->getDestination().getIndex()) &&
				(GPrime_.getVertices()[v]->getIndex() != RSA_Input_.getRequests()[k]->getOrigin().getIndex()))
			{
				IloExpr Flow(MCMCF);

				for (unsigned a = 0; a < GPrime_.getVertices()[v]->getVertexInArcs().size(); a++)
				{
					Flow += f_kl[RSA_Input_.getRequests()[k]->getIndex() - 1][GPrime_.getVertices()[v]->getVertexInArcs()[a]->getIndex() - 1];
				}
				for (unsigned a = 0; a < GPrime_.getVertices()[v]->getVertexOutArcs().size(); a++)
				{
					Flow -= f_kl[RSA_Input_.getRequests()[k]->getIndex() - 1][GPrime_.getVertices()[v]->getVertexOutArcs()[a]->getIndex() - 1];
				}
				ILP_MCMCF.add(Flow == 0);
				Flow.end();
			}
		}
	}

	// Degree constratints (2e) : Avoiding cycles attached to P_k
	for (unsigned k = 0; k < K; k++)
	{
		for (unsigned v = 0; v < V; ++v)
		{
			if ((GPrime_.getVertices()[v]->getIndex() != RSA_Input_.getRequests()[k]->getDestination().getIndex()) &&
				(GPrime_.getVertices()[v]->getIndex() != RSA_Input_.getRequests()[k]->getOrigin().getIndex()))
			{
				IloExpr Flow(MCMCF);

				for (unsigned a = 0; a < GPrime_.getVertices()[v]->getVertexInArcs().size(); a++)
				{
					Flow += f_kl[RSA_Input_.getRequests()[k]->getIndex() - 1][GPrime_.getVertices()[v]->getVertexInArcs()[a]->getIndex() - 1];
				}
				ILP_MCMCF.add(Flow <= 1);
				Flow.end();
			}
		}
	}

	// Capacity constraints (2f) : The sum on opposite arcs (on each edge) is bounded by Cap
	for (unsigned a = 0; a < A / 2; ++a)
	{
		IloExpr Capacity(MCMCF);
		for (unsigned k = 0; k < K; k++)
		{
			Capacity += int(RSA_Input_.getRequests()[k]->getSlots()) * f_kl[RSA_Input_.getRequests()[k]->getIndex() - 1][GPrime_.getArcs()[a]->getIndex() - 1];
			Capacity += int(RSA_Input_.getRequests()[k]->getSlots()) * f_kl[RSA_Input_.getRequests()[k]->getIndex() - 1][GPrime_.getArcs()[a]->getIndex() + A / 2 - 1];
		}
		Capacity -= Cap;
		ILP_MCMCF.add(Capacity <= 0);
		Capacity.end();
	}

	// For each edge in the original graph the flow for one demand will not goes for the 2 arcs with inverse orientation
	// for (unsigned a = 0; a < A / 2; ++a)
	// {
	// 	for (unsigned k = 0; k < K; k++)
	// 	{
	// 		IloExpr edgeconst(MCMCF);
	// 		edgeconst += f_kl[RSA_Input_.getRequests()[k]->getIndex() - 1][GPrime_.getArcs()[a]->getIndex() - 1];
	// 		edgeconst += f_kl[RSA_Input_.getRequests()[k]->getIndex() - 1][GPrime_.getArcs()[a]->getIndex() + A / 2 - 1];
	// 		ILP_MCMCF.add(edgeconst <= 1);
	// 		edgeconst.end();
	// 	}
	// }

	// Length constraints
	for (unsigned k = 0; k < K; k++)
	{
		IloExpr Length(MCMCF);
		Length += RSA_Input_.getRequests()[k]->getMaxLength();
		for (unsigned a = 0; a < GPrime_.getArcs().size(); a++)
		{
			Length -= f_kl[RSA_Input_.getRequests()[k]->getIndex() - 1][GPrime_.getArcs()[a]->getIndex() - 1] * GPrime_.getArcs()[a]->getLength();
		}
		ILP_MCMCF.add(Length >= 0);
		Length.end();
	}

	// Forbidden Cliques Constraints (4)
	// reducing cliques
	bool ainb = false, bina = false, anotb = false;
	std::vector<Clique*> unique_cliques;

	unique_cliques = obstructions_.getCircuitCliques();
    for (unsigned i = 0; i < unique_cliques.size(); ++i)
    {
        IloExpr cliqueconstraint(MCMCF);
        int counter = 0;
        for (unsigned j = 0; j < unique_cliques[i]->getForbiddenEdgesFromCliques().size(); ++j)
        {
            for (unsigned k = 0; k < unique_cliques[i]->getForbiddenEdgesFromCliques()[j].indexDemand.size(); ++k)
            {
                cliqueconstraint += f_kl[unique_cliques[i]->getForbiddenEdgesFromCliques()[j].indexDemand[k]->getIndex()-1][unique_cliques[i]->getForbiddenEdgesFromCliques()[j].edges->getIndex()-1];
                cliqueconstraint += f_kl[unique_cliques[i]->getForbiddenEdgesFromCliques()[j].indexDemand[k]->getIndex()-1][unique_cliques[i]->getForbiddenEdgesFromCliques()[j].edges->getIndex()-1 + A/2];
                counter++;
            }
        }
		//cout << cliqueconstraint << endl;
        ILP_MCMCF.add(cliqueconstraint <= counter-1);
        cliqueconstraint.end();
    }

	//Objective function
	IloExpr Ob(MCMCF);
	Ob = Cap;
	IloObjective obj(MCMCF, Ob, IloObjective::Minimize, "OBJ");
	ILP_MCMCF.add(obj);

	model.exportModel("Min_Cost_Multi_Comodity_Cplex.lp");
	cout << "Number of constraints of MCF: " << model.getNrows() << endl;
	tConstraintsMCF_ += model.getNrows();
	nCallsMCF_ += 1;
	model.setOut(MCMCF.getNullStream());
	
	MCMCFSolved_ = model.solve();
	if (!MCMCFSolved_)
    {
		MCMCF.end();
		cout << "MCMCF Instance Infeasible" << endl;
		cout << "-------------- END OF ILP MCMCF CPLEX --------------------------------" << endl;
    	return;
    }
	model.writeSolutions("MCMCF_Sol.sol");

	// Construction of the solution
	vector<Path *> routing;
	for (unsigned k = 0; k < K; k++)
	{
		Vertex nd = RSA_Input_.getRequests()[k]->getOrigin();
		vector<Edge *> path(0);
		while (nd.getIndex() != RSA_Input_.getRequests()[k]->getDestination().getIndex())
		{
			for (unsigned a = 0; a < nd.getVertexOutArcs().size(); ++a)
			{
				if (round(model.getValue(f_kl[RSA_Input_.getRequests()[k]->getIndex() - 1][nd.getVertexOutArcs()[a]->getIndex() - 1])) == 1)
				{
					path.push_back(nd.getVertexOutArcs()[a]->getEdge());
					nd = nd.getVertexOutArcs()[a]->getDestination();
					break;
				}
			}
		}
		Path *path2 = new Path(path);
		routing.push_back(path2);
	}

	// Here the solution is saved
	MCMCF_Output_ = MCMCF_Output(RSA_Input_.getRequests(), routing, round(model.getObjValue()));
	MCMCF.end();
	cout << MCMCF_Output_ << endl;
	cout << "-------------- END OF ILP MCMCF CPLEX --------------------------------" << endl;
}

void RSA_Algorithms::solveEdgePathFormulation_Cplex()
{
	cout << "-------------- BEGIN OF ILP EPF CPLEX --------------------------------" << endl;

	IloEnv RSA; 									// environnement : allows use of functions Concert Technology
	IloModel ILP_RSA(RSA); 							// model: represents the linear programm
	IloCplex model(ILP_RSA); 						// class Cplex : allows acces to optimization functiona
	// ----------------------- VARIABLES -----------------------
	unsigned K = RSA_Input_.getRequests().size();
	unsigned E = G_.getEdges().size();				//number of edges in graph G'
	unsigned V = G_.getVertices().size(); 			//number of vertices
	unsigned S = maxSlots_; 						//number of avaiable slices

	//graph and demand information
	cout << "# Demands : " << K << endl;
	cout << "# Edges   : " << E << endl;
	cout << "# Nodes   : " << V << endl;
	cout << "# Slots   : " << S << endl;

	// ----------------------- VARIABLES -----------------------
	IloNumVarArray variables(RSA);


    // Chose one path for each demand
	IloArray<IloNumVarArray> y_kp(RSA, K);
	unsigned P_k;
	for (unsigned k = 0; k < K; k++)
	{
		P_k = (*possiblePathsNew_[k]).size();
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

    // Verify which demand uses which edge
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

	// Span variable
	IloNumVar Smax = IloNumVar(RSA, 0, INFINITY, ILOINT);
	std::string Var_Name = "SMax";
	Smax.setName(Var_Name.c_str());
	ILP_RSA.add(Smax);
	variables.add(Smax);

	// End of variable cretaion
	std::cout << std::endl << "Number of variables = " << variables.getSize() << std::endl;

	// ----------------------- Constraints -----------------------
	
	//path selection (5b)
	for (unsigned k = 0; k < K; k++)
	{
		IloExpr path(RSA);
		P_k = (*possiblePathsNew_[k]).size();
		for (unsigned p = 0; p < P_k; p++)
		{
			path += y_kp[k][p];
		}
		ILP_RSA.add(path == 1);
		path.end();
	}

	//edge activation (5c)
	for (unsigned k = 0; k < K; k++)
	{
		for (unsigned e = 0; e < E; e++)
		{
			IloExpr edge_path(RSA);
			edge_path = xk_e[k][e];
			P_k = (*possiblePathsNew_[k]).size();

			for (unsigned p = 0; p < P_k; ++p)
			{
				for ( vector<Edge*>::const_iterator it = (*possiblePathsNew_[k])[p]->getEdges().begin(); it != (*possiblePathsNew_[k])[p]->getEdges().end(); ++it)
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

	// unavailable slices for each demand respecting the capacity (5d)
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

	//spectrum assignment (5e)
	for (unsigned k = 0; k < K; k++)
	{
		IloExpr modSlice(RSA);
		for (unsigned s = RSA_Input_.getRequests()[k]->getSlots() - 1; s < upperBound_ - 1; s++)
		{
			modSlice += zk_s[k][s];
		}
		ILP_RSA.add(modSlice == 1);
		modSlice.end();
	}

	//demand_edge_slice (5f)
	// tk assignment from zk (and zk - demand weight) if xk is equal to one in the edge.
	for (unsigned k = 0; k < K; k++)
	{
		for (unsigned e = 0; e < E; e++)
		{
			for (unsigned s = 0; s < upperBound_ - 1; s++)
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

	// The sum of tk in one edge that is used must be equal to weight demand (5g)
	for (unsigned k = 0; k < K; k++)
	{
		for (unsigned e = 0; e < E; e++)
		{
			IloExpr slice(RSA);
			for (unsigned s = 0; s < upperBound_ - 1; s++)
			{
				slice += tk_es[k][e][s];
			}
			slice -= int(RSA_Input_.getRequests()[k]->getSlots()) * xk_e[k][e];
			ILP_RSA.add(slice == 0);
			slice.end();
		}
	}

	//non_overlapping in edge an in one slot two different demands (5h)
	for (unsigned e = 0; e < E; e++)
	{
		for (unsigned s = 0; s < upperBound_ - 1; s++)
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

	// Demands' span constraints (5i)
	for (int k = 0; k < K; k++)
	{
		IloExpr demandSpan(RSA);
		int w_k = RSA_Input_.getRequests()[k]->getSlots();
		for (int s = w_k - 1; s < S; s++)
		{
			demandSpan += ((s + 1) * zk_s[k][s]);
		}
		demandSpan -= Smax;
		ILP_RSA.add(demandSpan <= 0);
		demandSpan.end();
	}

	// Span limit constraint (5j)
	IloExpr spanLimit(RSA);
	spanLimit += Smax;
	spanLimit -= (upperBound_ - 1);
	ILP_RSA.add(spanLimit <= 0);
	spanLimit.end();	

	// Union of forbidden routings (FULL and PARTIAL) 
	// vector<vector<Path *>> TotalForbiddenR(0);
	// // FULL from forbidden routings set
	// TotalForbiddenR.insert(TotalForbiddenR.end(), forbiddenRoutings_.begin(), forbiddenRoutings_.end());
	// // PARTIAL from forbidden qliques set
	// for (unsigned i = 0; i < forbiddenCliques_.size(); i++)
	// {
	// 	TotalForbiddenR.push_back(forbiddenCliques_[0].getPaths());
	// }

	// Constraints from forbidden PARTIAL routings i.e. Cliques (6)

	bool ainb = false, bina = false, anotb = false;
	std::vector<Clique*> unique_cliques;
	unique_cliques = obstructions_.getCircuitCliques();

	for (unsigned i = 0; i < unique_cliques.size(); i++) {
		IloExpr forbiddenRoutes(RSA);
		int size = unique_cliques[i]->getPaths().size();
		for (unsigned j = 0; j < size; ++j)
		{
			unsigned k = unique_cliques[i]->getDemands()[j]->getIndex() - 1;
			unsigned p = 0;
			while ((p < (*possiblePathsNew_[k]).size()) && !(*unique_cliques[i]->getPaths()[j] == *(*possiblePathsNew_[k])[p]))
			{
				p++;
			}
			forbiddenRoutes += y_kp[k][p];
		}
		//cout << forbiddenRoutes << endl;
		ILP_RSA.add(forbiddenRoutes <= size - 1);
		forbiddenRoutes.end();
	}

	// Mirror Demands Restrictions (Symmetry Breaking)
	for (vector<vector<vector<Demand *>>>::iterator equalOkDkGroup = mirrorDemands_.begin(); equalOkDkGroup != mirrorDemands_.end(); equalOkDkGroup++)
	{
		for (vector<vector<Demand *>>::iterator equalOkDkWkSet = (*equalOkDkGroup).begin(); equalOkDkWkSet != (*equalOkDkGroup).end(); equalOkDkWkSet++)
		{
			int L = (*possiblePathsNew_[(*equalOkDkWkSet)[0]->getIndex()-1]).size();
			int Dm = (*equalOkDkWkSet).size();
			int maxIdx = 0;
			for (vector<Demand *>::iterator dem = (*equalOkDkWkSet).begin(); dem != (*equalOkDkWkSet).end(); dem++)
			{
				if ((*dem)->getIndex() > maxIdx)
				{
					maxIdx = (*dem)->getIndex();
				}
			}

			for (vector<Demand *>::iterator dem = (*equalOkDkWkSet).begin(); dem != (*equalOkDkWkSet).end(); dem++)
			{
				int k_prim = (*dem)->getIndex();
				if (k_prim != maxIdx)
				{
					for (int l_prim = 1; l_prim < L; l_prim++)
					{
						IloExpr mirrorDem(RSA);
						for (vector<Demand *>::iterator dem2 = (*equalOkDkWkSet).begin(); dem2 != (*equalOkDkWkSet).end(); dem2++)
						{
							int k = (*dem2)->getIndex();
							if (k > k_prim)
							{
								for (int l = 0; l < l_prim; l++)
								{
									mirrorDem += y_kp[k - 1][l];
								}
							}
						}
						ILP_RSA.add(mirrorDem <= Dm * (1 - y_kp[k_prim - 1][l_prim]));
						mirrorDem.end();
					}
				}
			}
		}
	}

	// Objective function: |D|*Smax + Penalization of slots from {b_low + 1, ..., b_up}
	IloExpr Ob(RSA);
	Ob += ((int)K) * Smax;
	for (unsigned k = 0; k < K; k++)
	{
		for (int s = lowerBound_; s < upperBound_ - 1; s++)
		{
			Ob += zk_s[k][s];
		}
	}

	IloObjective obj (RSA, Ob, IloObjective::Minimize, "OBJ");
	ILP_RSA.add(obj);

	model.exportModel("Edge_Path_Formulation_Cplex.lp");
	cout << "Number of constraints of EPF: " << model.getNrows() << endl;
	tConstraintsEPF_ += model.getNrows();
	nCallsEPF_ += 1;
	model.setOut(RSA.getNullStream());

	RSASolved_ = model.solve();
	if (!RSASolved_)
	{
		gap_ = 100;
		RSA.end();
		cout << "EPF instance infeasible" << endl;
		cout << "Gap: " << gap_ << endl;
		cout << "-------------- END OF ILP EPF CPLEX ----------------------------------" << endl;
		return;
	}
	gap_ = model.getMIPRelativeGap();
	model.writeSolutions("EPF_Sol.sol");

	//Construction of the output of the model
    std::vector<Path*> routing;
    std::vector<SpectrumAssignmentPerDemand> spectrumAssignment;
    for (unsigned k = 0; k < K; k++)
	{
	    // Routing construction
		P_k = (*possiblePathsNew_[k]).size();
		for (unsigned p = 0; p < P_k; p++)
		{
			if( round(model.getValue(y_kp[k][p])) == 1)
            {
                routing.push_back((*possiblePathsNew_[k])[p]);
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
				sapd.spectrum = s + 1;
				sapd.spectrumInitial = s + 1 - RSA_Input_.getRequests()[k]->getSlots();
				break;
            }
		}
        spectrumAssignment.push_back(sapd);
	}
	RSA_Output_ = RSA_Output(RSA_Input_.getRequests(), routing, spectrumAssignment, model.getObjValue());
	RSA.end();
	cout << RSA_Output_ << endl;
	cout << "Gap: " << gap_ << endl;
	cout << "-------------- END OF ILP EPF CPLEX ----------------------------------" << endl;
}

bool RSA_Algorithms::UpdatePossiblePathsSet()
{
	bool newPath = false;
	for (unsigned i = 0; i < MCMCF_Output_.getRouting().size(); ++i)
	{
		unsigned j = 0;
		while ((j < (*possiblePathsNew_[i]).size()) && !(*(*possiblePathsNew_[i])[j] == *(MCMCF_Output_.getRouting()[i])))
		{
			j++;
		}
		if (j == (*possiblePathsNew_[i]).size())
		{
			(*possiblePathsNew_[i]).push_back(MCMCF_Output_.getRouting()[i]);
			newPath = true;
		}
	}

	return newPath;
}

void RSA_Algorithms::maxWeightedClique(vector<Path*> & routing)
{
    //--------------Begin of the construction of the intersection graph ----------------
    vector<Vertex*> nodes;
    vector<Edge*> edges;
    for (unsigned k = 0; k < routing.size(); ++k)
    {
		Vertex *vertex = new Vertex(k + 1, RSA_Input_.getRequests()[k]->getSlots());
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
							cliqueStar.addForbiddenEdgesFromCliques((*it), RSA_Input_.getRequests()[constructed_clique_star[i]], RSA_Input_.getRequests()[constructed_clique_star[j]]);
					}
				}
		  	}
		}
		for (unsigned i = 0; i < constructed_clique_star.size(); ++i)
		{
			string lbl = getPathLabel(constructed_clique_star[i], routing[constructed_clique_star[i]]);
			cliqueStar.addPath(lbl, routing[constructed_clique_star[i]]);			//this will add the path in the clique
		}
		obstructions_.insert(cliqueStar);
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
								cliqueStar.addForbiddenEdgesFromCliques((*it), RSA_Input_.getRequests()[constructed_clique_star[i]], RSA_Input_.getRequests()[constructed_clique_star[j]]);
						}
					}
		    	}
		    }
		    for (unsigned i = 0; i < constructed_clique_star.size(); ++i)
			{
				string lbl = getPathLabel(constructed_clique_star[i], routing[constructed_clique_star[i]]);
				cliqueStar.addPath(lbl, routing[constructed_clique_star[i]]);			//this will add the path in the clique
			}
			obstructions_.insert(cliqueStar);
		    if (cliqueStar.getOmega() < smallestClique_)
		    {
		    	smallestClique_ = cliqueStar.getOmega();
		    }
        }
       //Observe that, if there aren't vertex to add, one vertex will be retired and it's all in the iteration.
       // We wait in some moment there are no more option and the cycle finish in some moment.
    }
}

void RSA_Algorithms::showPossiblePaths() const
{
	cout << "-------------- BEGIN OF Possible Paths Information -------------------" << endl;
	for (unsigned i = 0; i < possiblePathsNew_.size(); ++i)
	{
		cout << "For demand: " << i + 1 << endl;
		for (unsigned j = 0; j < (*possiblePathsNew_[i]).size(); ++j)
		{
			cout << "P" << i + 1 << "_" << j + 1 << "\t: " << *((*possiblePathsNew_[i])[j]);
		}
		cout << "-----------------------" << endl;
	}
	cout << "-------------- END OF Possible Paths Information ---------------------" << endl;
}

void RSA_Algorithms::showForbiddenCliques() const
{
	cout << "-------------- BEGIN OF Circuit Obstructions Information-----------------" << endl;
	std::vector<Clique*> uniquef = obstructions_.getCircuitCliques();
	cout << "----- We have " << uniquef.size() << " circuit cliques -------" << endl;
	for (unsigned i = 0; i < uniquef.size(); ++i)
	{
		cout << *uniquef[i] << endl;
	}
	cout << "-------------- END OF Circuit Obstructions Information-------------------" << endl;
}

vector<vector<vector<Demand *>>> RSA_Algorithms::findMirrorDemands(vector<Demand *> demands)
{
	vector<vector<Demand *>> mirrorDemands(0);
	// Grouping for origin, destination and weight 
	while (!demands.empty())
	{
		vector<Demand *> group(0);
		Demand *d = demands.front();
		vector<Demand *>::iterator it = demands.begin();
		while (it != demands.end())
		{
			if (((d->getOrigin().getIndex() == (*it)->getOrigin().getIndex() && d->getDestination().getIndex() == (*it)->getDestination().getIndex()) ||
				(d->getOrigin().getIndex() == (*it)->getDestination().getIndex() && d->getDestination().getIndex() == (*it)->getOrigin().getIndex())) &&
				(d->getSlots() == (*it)->getSlots()))
			{
				group.push_back(*it);
				it = demands.erase(it);
			}
			else
			{
				++it;
			}
		}
		mirrorDemands.push_back(group);
	}

	

	// Grouping Goups by origin and destination
	vector<vector<vector<Demand*>>> demandsPartition(0);
	while (!mirrorDemands.empty())
	{
		vector<vector<Demand *>> mirrorGroups(0);
		vector<Demand*> equalWGroup = mirrorDemands.front();
		vector<vector<Demand *>>::iterator it = mirrorDemands.begin();
		while (it != mirrorDemands.end())
		{
			if ((equalWGroup.front()->getOrigin().getIndex() == (*it).front()->getOrigin().getIndex() && equalWGroup.front()->getDestination().getIndex() == (*it).front()->getDestination().getIndex()) ||
				(equalWGroup.front()->getOrigin().getIndex() == (*it).front()->getDestination().getIndex() && equalWGroup.front()->getDestination().getIndex() == (*it).front()->getOrigin().getIndex()))
			{
				mirrorGroups.push_back(*it);
				it = mirrorDemands.erase(it);
			}
			else
			{
				++it;
			}
		}
		demandsPartition.push_back(mirrorGroups);
	}

	return demandsPartition;
}

string RSA_Algorithms::getPathLabel(int demandIdx, Path* pth)
{
	int pthIdx = 0;
	while(pth != (*possiblePathsNew_[demandIdx])[pthIdx])
	{
		pthIdx++;
	}
	return "P" + to_string(demandIdx + 1) + "_" + to_string(pthIdx + 1);
}

vector<vector<Path*>> RSA_Algorithms::generateMirrorRoutings(vector<Path*> initialRouting)
{
	/*
	cout << "========== begin of mirror generation ===========" << endl;
	for (unsigned i = 0; i < initialRouting.size(); i++) {
		cout << *initialRouting[i] << endl;
	}
	*/

	vector<vector<Path *>> permutations;
	permutations.push_back(initialRouting);	
	vector<vector<vector<Demand*>>>::iterator eqOkDk = mirrorDemands_.begin();
	while(eqOkDk != mirrorDemands_.end())
	{
		vector<vector<Demand*>>::iterator eqOkDkWk = eqOkDk->begin();
		while (eqOkDkWk != eqOkDk->end())
		{
			vector<vector<Path *>> permutationsNew;
			for (int pidx = 0; pidx < permutations.size(); pidx++)
			{
				vector<vector<Path *>> permOfRouting = permute(permutations[pidx], *eqOkDkWk);
				permutationsNew.insert(permutationsNew.end(), permOfRouting.begin(), permOfRouting.end());
			}
			permutations = permutationsNew;
			eqOkDkWk++;
		}
		eqOkDk++;
	}
	// removing equal permutations
	set<vector<Path*>> s;
	unsigned size = permutations.size();
	for (unsigned i = 0; i < size; ++i)
		s.insert(permutations[i]);
	permutations.assign(s.begin(), s.end());

	/*
	cout << "======== end of mirror generation =========" << endl;
	for (unsigned i = 0; i < permutations.size(); i++) {
		cout << "routing:" << endl;
		for (unsigned j = 0; j < permutations[i].size(); j++) {
			cout << *permutations[i][j] << endl;
		}
	}
	*/

	return permutations;
}

vector<vector<Path *>> RSA_Algorithms::permute(vector<Path *> routing, vector<Demand*> demands)
{
	vector<vector<Path *>> permutations(0);
	permutations.push_back(routing);
	int n = demands.size();
	for (int i = 0; i < n - 1; i++)
	{
		Path* aux = routing[demands[0]->getIndex()-1];
		for(int idx = 0; idx < n - 1; idx++)
		{
			routing[demands[idx]->getIndex() - 1] = routing[demands[idx+1]->getIndex() - 1];
		}
		routing[demands[n - 1]->getIndex() - 1] = aux;
		permutations.push_back(routing);
	}

	return permutations;
}
