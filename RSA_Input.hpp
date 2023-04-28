#ifndef RSA_INPUT_HPP
#define RSA_INPUT_HPP
#include <vector>
#include <string>

//Forward declaration
class Demand;
class Vertex;
class Edge;

class RSA_Input
{
	private:
		std::string 			file_Outputplacement_;
		std::vector<Demand*> 	requests_;
		std::vector<Vertex*>	nodes_;
		std::vector<Edge*> 		edges_;
		
	public:
		//Constructors
		RSA_Input(){}
	
		//Getters
		const std::vector<Demand*> & 	getRequests() const { return requests_;}
		const std::vector<Vertex*> &	getNodes() const { return nodes_;}
		const std::vector<Edge*> & 		getEdges() const { return edges_;}
		std::vector<Demand*> & 			getRequests() { return requests_;}
		std::vector<Vertex*> & 			getNodes() { return nodes_;}
		std::vector<Edge*> & 			getEdges()  { return edges_;}
		
		//Functions
		void 						data_load_demand(const std::string filename);
		void 						data_load_edge(const std::string filename);
		void 						data_load_node(const std::string filename);
		
		//Shows
		void 						showRequests() const;
	
		//Destructors
		~RSA_Input(){}
};

#endif // RSA_INPUT_HPP