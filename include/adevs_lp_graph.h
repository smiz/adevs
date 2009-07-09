/***************
Copyright (C) 2009 by James Nutaro

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Bugs, comments, and questions can be sent to nutaro@gmail.com
***************/
#ifndef adevs_lp_graph_h
#define adevs_lp_graph_h
#include <vector>
#include <map>

namespace adevs
{

/**
 * This is a graph for describing how processors in a 
 * parallel simulation are connected to each other.
 * There should be a directed edge from processor A
 * to processor B if a model assigned to A sends output
 * to a model assigned to B.
 */
class LpGraph
{
	public:
		/// Create a graph without any edges
		LpGraph(){}
		/// Create an edge from node A to node B
		void addEdge(int A, int B)
		{
			E[A].push_back(B);
			I[B].push_back(A);
		}
		/// Get the influencers of node B
		const std::vector<int>& getI(int B) { return I[B]; }
		/// Get the influencees of node A
		const std::vector<int>& getE(int A) { return E[A]; }
		/// Destructor
		~LpGraph(){}
	private:
		// Influencee graph
		std::map<int,std::vector<int> > E;
		// Complimentary influencer graph
		std::map<int,std::vector<int> > I;
};

}

#endif
