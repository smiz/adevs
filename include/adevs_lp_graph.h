#ifndef adevs_lp_graph_h
#define adevs_lp_graph_h
#include <vector>
#include <map>

namespace adevs
{

class LpGraph
{
	public:
		// Create a graph without an edges
		LpGraph(){}
		// Create an edge from node A to node B
		void addEdge(int A, int B)
		{
			E[A].push_back(B);
			I[B].push_back(A);
		}
		// Get the influencers of node B
		const std::vector<int>& getI(int B) { return I[B]; }
		// Get the influencees of node A
		const std::vector<int>& getE(int A) { return I[A]; }
		~LpGraph(){}
	private:
		// Influencee graph
		std::map<int,std::vector<int> > E;
		// Complimentary influencer graph
		std::map<int,std::vector<int> > I;
};

}
#endif
