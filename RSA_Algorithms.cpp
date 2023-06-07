#include <iostream>
#include <algorithm>
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
	auto duration = duration_cast<milliseconds>(endStep - startStep);
	//total time control
	std::chrono::duration<long int> limit = (std::chrono::duration<long int>) 7100;

	//total times initialization
	auto totalKShortest = duration_cast<milliseconds>(startStep - startStep);
	auto totalRSA = duration_cast<milliseconds>(startStep - startStep);

    startStep = high_resolution_clock::now(); 		//Begin of MCMCF
    //this->solveMinCostMultiCommodityFlow_Cplex();
	this->solveKShortest();
    endStep = high_resolution_clock::now();			//End of MCMCF
    duration = duration_cast<milliseconds>(endStep - startStep);
    totalKShortest = totalKShortest + duration;
    std::cout << "TIME == K shortest duration in milliseconds: " << duration.count() << std::endl; 
	addNewRoutesToRSA();
	// Here, we begin tha SA part of the problem
    showPossiblePaths();
    startStep = high_resolution_clock::now();			//Begin RSA
    this->solveEdgePathFormulation_Cplex();
    endStep = high_resolution_clock::now();				//End RSA
    duration = duration_cast<milliseconds>(endStep - startStep);
    totalRSA = totalRSA + duration;
    std::cout << "TIME == RSA duration in milliseconds: " << duration.count() << std::endl; 
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

int RSA_Algorithms::minDistance(std::vector<int> dist, std::vector<bool> sptSet)
{
  	// Initialize min value
    int min = INT_MAX, min_index;

    for (int v = 0; v < RSA_Input_.getNodes().size(); v++)
        if (sptSet[v] == false && dist[v] <= min)
            min = dist[v], min_index = v;

    return min_index;
}
  
std::vector<int> RSA_Algorithms::dijkstra(std::vector<std::vector<int> > graph, int src, int dest, double & pathdistance)
{
    std::vector<int> dist; 	// The output array.  dist[i] will hold the shortest distance from src to i
	
	std::vector<std::vector<int> > pathNodes;
  
    std::vector<bool> sptSet; // sptSet[i] will be true if vertex i is included in shortest path tree or shortest distance from src to i is finalized
  
    // Initialize all distances as INFINITE and stpSet[] as
    // false
    for (int i = 0; i < RSA_Input_.getNodes().size(); i++){
        dist.push_back(INT_MAX);
		sptSet.push_back(false);
	}

	std::vector<int> auxpathNodes;
	for (int i = 0; i < RSA_Input_.getNodes().size(); i++){
        pathNodes.push_back(auxpathNodes);
	}
	  
    // Distance of source vertex from itself is always 0
    dist[src] = 0;
    // Find shortest path for all vertices
    for (int count = 0; count < RSA_Input_.getNodes().size(); count++) {
        // Pick the minimum distance vertex from the set of
        // vertices not yet processed. u is always equal to
        // src in the first iteration.
        int u = minDistance(dist, sptSet);
        // Mark the picked vertex as processed
        sptSet[u] = true;
  
        // Update dist value of the adjacent vertices of the
        // picked vertex.
        for (int v = 0; v < RSA_Input_.getNodes().size(); v++){
  
            // Update dist[v] only if is not in sptSet,
            // there is an edge from u to v, and total
            // weight of path from src to  v through u is
            // smaller than current value of dist[v]
			
            if (!sptSet[v] && graph[u][v] && dist[u] != INT_MAX && dist[u] + graph[u][v] < dist[v]){
				//std::cout << "avaliando " << v+1 <<std::endl; ;
				if (pathNodes[v].size() > 0){
					//std::cout << "path existente a ser substituido" << std::endl;
					pathNodes[v].clear();
				}
                dist[v] = dist[u] + graph[u][v];
				pathNodes[v].push_back(u);
				//std::cout << "no path do " << v+1 << " adicionar " << u+1 ;
				//std::cout << " e tambem ";
				for(int i = 0; i < pathNodes[u].size(); i++){
					pathNodes[v].push_back(pathNodes[u][i]);
					//std::cout << pathNodes[u][i] + 1 << " "; 
				}
				/*
				std::cout<< std::endl;
				std::cout << "novo path do " << v+1 << " : " ;
				for(int i = 0; i < pathNodes[v].size(); i++){
					std::cout << pathNodes[v][i] + 1 << " "; 
				}
				std::cout<< std::endl;
				std::cout << "dist " << v+1 << " : " << dist[v] ;
				std::cout<< std::endl;
				*/
			}
		}
    }
	/*
	std::cout << "Vertex Path from Source" << endl;
	for (int i = 0; i < pathNodes.size(); i++){
		std::cout << i+1 << " ";
		for (int j = 0; j < pathNodes[i].size(); j++){
			std::cout <<  pathNodes[i][j] + 1<< " "; 
		}
		std::cout<< std::endl;
	}
	*/
	/*
	std::cout << "Chosen path" << endl;
	for (int i = pathNodes[dest].size() -1 ; i >= 0; i--){
		std::cout <<  pathNodes[dest][i] + 1 << " "; 
	}
	std::cout << dest + 1;
	std::cout<< std::endl;
	*/
	double chosendistance = dist[dest];
	/*
	std::cout << "Chosen distance " << chosendistance << endl;
    // print the constructed distance array
    std::cout << "Vertex Distance from Source" << endl;
    for (int i = 0; i < RSA_Input_.getNodes().size(); i++){
        std::cout << i+1 << " " << dist[i] << std::endl;
	}
	*/
	std::vector<int> sol;
	for (int i = pathNodes[dest].size() -1 ; i >= 0; i--){
		sol.push_back(pathNodes[dest][i] + 1); 
	}
	sol.push_back(dest+ 1);
	
	return sol;
}

std::vector<std::vector<Edge*> > RSA_Algorithms::kdijkstra(std::vector<std::vector<int> > graph, int src, int dest, int K){
	std::vector<std::vector<int> > modifiablegraph = graph;
	std::vector<std::vector<int> > A; //chosen paths
	std::vector<std::vector<int> > B; //candidate paths 
	std::vector<int> djikistraSolution;
	int spurNode;
	std::vector<int> spurPath;
	double dist;
	djikistraSolution = dijkstra(graph,src,dest,dist);
	//std::cout << "1 djikitra: ";
	//for (int zz = 0; zz < djikistraSolution.size(); zz++){
	//		std::cout << djikistraSolution[zz] << " ";
	//}
	//std::cout << std::endl;
	A.push_back(djikistraSolution);
	for (int k = 1; k<K; k++){
		for (int i = 0; i < A[k-1].size()-1; i++ ){
			//std::cout << "olha o i " << i << std::endl;
			spurNode = A[k-1][i];
			//std::cout << "spur node " << spurNode << std::endl;
			std::vector<int> rootpath;
			for (int j = 0; j <= i; j++){
				rootpath.push_back(A[k-1][j]);
			}
			//std::cout << "root path: ";
			//for (int w = 0; w < rootpath.size(); w++){
			//	std::cout << rootpath[w] << " ";
			//}
			//std::cout << std::endl;
			for (int path = 0; path < A.size() ; path ++){
				std::vector<int> auxiliarpath;
				//definindo se vai ate o i ou ate o fim do path ai
				int stop = i;
				if (A[path].size()<i){
					//std::cout << "A nao é grande o suficiente pro i" << std::endl;
					stop = A[path].size() -1;
				}
				for (int node = 0; node <= stop; node++){
					auxiliarpath.push_back(A[path][node]);
				}
				//std::cout << "auxiliar path from A: ";
				//	for (int w = 0; w < auxiliarpath.size(); w++){
				//		std::cout << auxiliarpath[w] << " ";
				//	}
				//std::cout << std::endl;
				//verificar se o roothpath é igual
				bool auxigualroot = true;
				for (int v = 0; v < auxiliarpath.size(); v++){
					if (auxiliarpath[v]!=rootpath[v]){
						auxigualroot = false;
					}
				}
				if (auxigualroot == true){
					//std::cout << "remover " << A[path][i] << " e "  << A[path][i+1] << std::endl;
					modifiablegraph[A[path][i]-1][A[path][i+1]-1] = 0;
					modifiablegraph[A[path][i+1]-1][A[path][i]-1] = 0;
				}
			}
			//std::cout << "removendo do spur path" << std::endl;
			if (rootpath.size() > 1){
				for (int w = 0; w < rootpath.size()-1; w++){
					//std::cout << "remover " << rootpath[w] << " e "  << rootpath[w+1] << std::endl;
					modifiablegraph[rootpath[w]-1][rootpath[w+1]-1] = 0;
					modifiablegraph[rootpath[w+1]-1][rootpath[w]-1] = 0;
				}
			}
			//std::cout << "removendo causa de ciclos path" << std::endl;
			for (int w = 0; w < rootpath.size()-1; w++){
				for (int adj = 0; adj < modifiablegraph[rootpath[w]-1].size(); adj++){
					//std::cout << "remover " << rootpath[w] << " e "  <<  adj+1<< std::endl;
					modifiablegraph[rootpath[w]-1][adj] = 0;
					modifiablegraph[adj][rootpath[w]-1] = 0;
				}
			}
			//std::cout << "printing adjmatrix" <<std::endl; 
			//for (int i = 0; i < modifiablegraph.size(); i++){
			//	for (int j = 0; j < modifiablegraph[i].size(); j++){
			//		std::cout << modifiablegraph[i][j] << " ";
			//	}
			//	std::cout << std::endl;
			//}
			spurPath = dijkstra(modifiablegraph,spurNode-1,dest,dist);
			if (spurPath.size() == 1){
				modifiablegraph = graph;
				//std::cout << "breakou o pau comeu " << std::endl;
			}
			else{
			//std::cout << "pedaço do novo candidato: ";
			//for (int zz = 0; zz < spurPath.size(); zz++){
			//	std::cout << spurPath[zz] << " ";
			//}
			//std::cout << std::endl;
			std::vector<int> totalpath;
			//std::cout << "adicionando root"<< std::endl;
			for (int nn = 0; nn < rootpath.size()-1; nn++){
				totalpath.push_back(rootpath[nn]);
			}
			//std::cout << "adicionando pedaço"<< std::endl;
			for (int nn = 0; nn < spurPath.size(); nn++){
				totalpath.push_back(spurPath[nn]);
			}
			//std::cout << "novo candidato: ";
			//for (int zz = 0; zz < totalpath.size(); zz++){
			//	std::cout << totalpath[zz] << " ";
			//}
			bool ehigual = false;
			for (int bzin=0; bzin < B.size(); bzin++){
				if (B[bzin].size() == totalpath.size()){
					bool auxtotaligualbzin = true;
					for (int v = 0; v < totalpath.size(); v++){
						if (B[bzin][v]!=totalpath[v]){
							//std::cout << "elemento diferente caraio"<<std::endl;
							auxtotaligualbzin = false;
						}
					}
					if (auxtotaligualbzin == true){
						//std::cout << "essa porra é ingual adiciona n"<<std::endl;
						ehigual = true;
						break;
					}
				}
			}
			if (ehigual == false){
				//std::cout << "adicione ";
				//for (int zz = 0; zz < totalpath.size(); zz++){
				//	std::cout << totalpath[zz] << " ";
				//}
				//std::cout << std::endl;
				B.push_back(totalpath);
			}
			modifiablegraph = graph;
			}
		}
		if (B.size()==0){
			break;
		}
		//std::cout << "paths em b: " << B.size()<<  std::endl;
		//for (int bzin = 0; bzin < B.size(); bzin++){
		//	std::cout << "paths " << bzin + 1<< " : ";
		//	for (int bzin2 = 0; bzin2 < B[bzin].size(); bzin2++){
		//		std::cout << B[bzin][bzin2] << " ";
		//	}
		//	std::cout << std::endl;
		//}
		//FALTA O SORT
		std::vector<double> distB;
		for (int bzin = 0; bzin < B.size(); bzin++){
			double distatual = 0;
			for (int bzin2 = 0; bzin2 < B[bzin].size()-1; bzin2++){
				//std::cout << "Somando edge " << B[bzin][bzin2] <<" " << B[bzin][bzin2+1] << " dist: " << graph[B[bzin][bzin2]-1][B[bzin][bzin2+1]-1] <<std::endl;
				distatual = distatual + graph[B[bzin][bzin2]-1][B[bzin][bzin2+1]-1];
			}
			distB.push_back(distatual);
			//std::cout << std::endl;
		}
		//for (int bzin = 0; bzin < distB.size(); bzin++){
		//	std::cout << "b " << bzin + 1  << " dist: " << distB[bzin] <<std::endl;
		//
		//}
		double minelement = *min_element(distB.begin(),distB.end());
		//std::cout << "el minimo: " << minelement <<std::endl;
		int index;
		for (int bzin = 0; bzin < distB.size(); bzin++){
			if(distB[bzin]==minelement){
				index = bzin;
				break;
			}
		}
		//std::cout << "index del minimo: " << index <<std::endl;
		//FAZER SORT
		//std::cout << "paths em A: " << A.size()<<  std::endl;
		//for (int bzin = 0; bzin < A.size(); bzin++){
		//	std::cout << "paths " << bzin + 1<< " : ";
		//	for (int bzin2 = 0; bzin2 < A[bzin].size(); bzin2++){
		//		std::cout << A[bzin][bzin2] << " ";
		//	}
		//	std::cout << std::endl;
		//}

		//std::cout << "adicionando ";
		//for (int zz = 0; zz < B[index].size(); zz++){
		//	std::cout << B[index][zz] << " ";
		//}
		A.push_back(B[index]);
		//aqui fazer swap de quem ta no index com o begin
		//std::cout << "trocando ";
		//for (int zz = 0; zz < B[0].size(); zz++){
		//	std::cout << B[0][zz] << " ";
		//}
		//std::cout << "por ";
		//for (int zz = 0; zz < B[index].size(); zz++){
		//	std::cout << B[index][zz] << " ";
		//}
		B[0].swap(B[index]);
		B.erase(B.begin());
		//std::cout << "paths em b: " << B.size()<<  std::endl;
		//for (int bzin = 0; bzin < B.size(); bzin++){
		//	std::cout << "paths " << bzin + 1<< " : ";
		//	for (int bzin2 = 0; bzin2 < B[bzin].size(); bzin2++){
		//		std::cout << B[bzin][bzin2] << " ";
		//	}
		//	std::cout << std::endl;
		//}
		/*
		std::cout << "paths em A: " << A.size()<<  std::endl;
		for (int bzin = 0; bzin < A.size(); bzin++){
			std::cout << "paths " << bzin + 1<< " : ";
			for (int bzin2 = 0; bzin2 < A[bzin].size(); bzin2++){
				std::cout << A[bzin][bzin2] << " ";
			}
			std::cout << std::endl;
		}
		std::cout << "paths em b: " << B.size()<<  std::endl;
		for (int bzin = 0; bzin < B.size(); bzin++){
			std::cout << "paths " << bzin +1<< " : ";
			for (int bzin2 = 0; bzin2 < B[bzin].size(); bzin2++){
				std::cout << B[bzin][bzin2] << " ";
			}
			std::cout << std::endl;
		}*/
	}

	std::vector<std::vector<Edge*> > solution;
	std::vector<Edge*> auxsolution;
	for (int a = 0; a <A.size(); a++){
		for (int i = 0; i < A[a].size() -1 ; i++){
			int origindege = A[a][i]  ;
			int destinationedge = A[a][i+1] ;  
			for (int j = 0; j < RSA_Input_.getEdges().size(); j++){
				if (RSA_Input_.getEdges()[j]->getV1().getIndex() == origindege && RSA_Input_.getEdges()[j]->getV2().getIndex() == destinationedge){
					auxsolution.push_back(RSA_Input_.getEdges()[j]);
				}
				if (RSA_Input_.getEdges()[j]->getV2().getIndex() == origindege && RSA_Input_.getEdges()[j]->getV1().getIndex() == destinationedge){
					auxsolution.push_back(RSA_Input_.getEdges()[j]);
				}
			}
		}
		solution.push_back(auxsolution);
		auxsolution.clear();
	}
	return solution;

}

void  RSA_Algorithms::solveKShortest(){

	//DJIKISTRA MODULE     
	std::vector<int> djikistraDemandsId;
	std::vector<std::vector<Edge*> > djikistraPathsEdges;
    int originDjikistra;
    int destinationDijikistra;
	vector<Path*> routing;
	int countroutings=0;

    for (int i = 0; i < RSA_Input_.getRequests().size(); ++i){
	//for (int i = 0; i < 1; ++i){	
		//std::cout << std::endl <<"===========demanda: "<< i+1 << std::endl;
        originDjikistra = RSA_Input_.getRequests()[i]->getOrigin().getIndex();
        destinationDijikistra =  RSA_Input_.getRequests()[i]->getDestination().getIndex();
		//std::cout << originDjikistra << " " << destinationDijikistra << std::endl;

		//creating adj matrix
		std::vector<std::vector<int> > adjmatrix;
		std::vector<int> auxadj;
		for (int i = 0; i < RSA_Input_.getNodes().size(); ++i){
			for (int j = 0; j < RSA_Input_.getNodes().size(); ++j){
				auxadj.push_back(0);
			}
			adjmatrix.push_back(auxadj);
			auxadj.clear();
		}
		//filling adj matrix
		for (int i = 0; i < RSA_Input_.getNodes().size(); ++i){
			int demandorigin = i+1;
			for (int j = 0; j < RSA_Input_.getEdges().size(); ++j){
				int edgeorigin = RSA_Input_.getEdges()[j]->getV1().getIndex();
				int edgedestination = RSA_Input_.getEdges()[j]->getV2().getIndex();
				if (edgedestination == demandorigin){
					adjmatrix[i][edgeorigin-1] = RSA_Input_.getEdges()[j]->getLength();
				}
				else{
					if (edgeorigin == demandorigin){
						adjmatrix[i][edgedestination-1] = RSA_Input_.getEdges()[j]->getLength();
					}
				}
			}
		}
		//std::cout << "printing matrix" <<std::endl; 
		/*for (int i = 0; i < adjmatrix.size(); i++){
			for (int j = 0; j < adjmatrix[i].size(); j++){
				std::cout << adjmatrix[i][j] << " ";
			}
			std::cout << std::endl;
		}*/

		//for kdjikistra
		std::vector<std::vector<Edge*> > kpathsedges;
		//DEFINE K SHORTEST
		int k = 5;
		kpathsedges = kdijkstra(adjmatrix,originDjikistra-1, destinationDijikistra-1, k);
		for (int k = 0; k < kpathsedges.size(); k++){
			//std::cout << std:: endl<< "Path from " << originDjikistra << " to " << destinationDijikistra << " is: " ; 
			//for (int p = 0; p != kpathsedges[k].size(); ++p)
			//	std::cout << kpathsedges[k][p]->getV1().getIndex() << " " << kpathsedges[k][p]->getV2().getIndex() << " " ;
			djikistraDemandsId.push_back(RSA_Input_.getRequests()[i]->getIndex());
			djikistraPathsEdges.push_back(kpathsedges[k]);
		}
		//std::cout << std::endl << "===========demanda: "<< i+1 << std::endl;
	}
		
	// Construction of the solution
	for (unsigned a = 0; a < djikistraPathsEdges.size(); a++)
	{
		vector<Edge*> path(0);
		for (unsigned b = 0; b < djikistraPathsEdges[a].size(); b++)
		{
			path.push_back(djikistraPathsEdges[a][b]);
		}
		Path * path2 = new Path(path,RSA_Input_.getRequests()[djikistraDemandsId[countroutings]-1]);
		//AQUI SO DAR PUSHBACK SE RESPEITAR O MAX LENGTH	
		// URGENTE
		if (path2->getLengthPath() <= path2->getDemand()->getMaxLength()){
			//std::cout << "k shortest respects max length: "<< path2->getLengthPath()<< " < "<< path2->getDemand()->getMaxLength() <<std::endl;
			routing.push_back(path2);
		}
		countroutings = countroutings + 1;
	}
	/*
	std::cout << "Current routing" <<std::endl;
	for (int i = 0; i < routing.size(); i ++){
		std::cout << " routing: " << routing[i]->getIndex() << " demand: " << routing[i]->getDemand()->getIndex() << " edges: ";  
		for (int j = 0; j < routing[i]->getEdges().size(); j ++){
			std::cout << " "  << routing[i]->getEdges()[j]->getV1().getIndex() << " " << routing[i]->getEdges()[j]->getV2().getIndex();
		}
		std::cout << std::endl;
	}*/
    
	// Here the solution is saved
	MCMCF_Output_ = MCMCF_Output(routing,0);
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


	//model.exportModel("Edge_Path_Formulation_Cplex.lp");
	model.setOut(RSA.getNullStream());
	model.setParam(IloCplex::TiLim, 3600); // Execution time limited

	if (model.solve() == false)
    {
    	RSASolved_ = false;
    	gap_ = 100;
    	return;
    }
    RSASolved_ = true;
	isOptimal_ = true;

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
	std::cout << std::endl << "---------------END OF ILP Edge Path Formulation Cplex------------------" << std::endl;
}

void RSA_Algorithms::addNewRoutesToRSA()
{
	for (unsigned i = 0; i < MCMCF_Output_.getRouting().size(); ++i)
	{
        possiblePaths_[MCMCF_Output_.getRouting()[i]->getDemand()->getIndex()-1].push_back(MCMCF_Output_.getRouting()[i]);
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

