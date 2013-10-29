/**
 * Copyright (c) 2013, James Nutaro
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of the FreeBSD Project.
 *
 * Bugs, comments, and questions can be sent to nutaro@gmail.com
 */
#ifndef __adevs_cellspace_h_
#define __adevs_cellspace_h_
#include "adevs.h"
#include <cstdlib>

namespace adevs
{

/**
 * Input and output events produced by components of a CellSpace must
 * be of the type CellEvent.  A CellEvent has an event value (i.e., the actual
 * input/output value) and a target cell for the event.
 */
template <class X> class CellEvent
{
	public:
		/// Default constructor. Sets x = y = z = 0.
		CellEvent(){ x = y = z = 0; }
		/// Copy constructor
		CellEvent(const CellEvent<X>& src):
		x(src.x),y(src.y),z(src.z),value(src.value){}
		/// Assignment operator
		const CellEvent& operator=(const CellEvent<X>& src)
		{
			x = src.x; y = src.y; z = src.z; value = src.value;
			return *this;
		}
		/// The x coordinate of the event target
		long int x;
		/// The y coordinate of the event target
		long int y;
		/// The z coordinate of the event target
		long int z;
		/// The event value
		X value;
};

/**
 * This class describes a 3D cell space whose components accept and produce CellEvent objects.
 * This class is meant to be useful for solving PDEs, simulating
 * next event cell spaces, and for building other types of models represented as a space of 
 * discrete, interacting points. Output events produced by component models must be of type CellEvent, and
 * the CellEvent (x,y,z) coordinate indicates the
 * target cell for the event. The corresponding input event will have the same (x,y,z) value as
 * the output event. Targets that are outside of the CellSpace will become external output
 * events for the CellSpace model.  Similarly, CellEvent objects that are injected into the
 * CellSpace (i.e., external input events) will be delivered to the targeted cell.
 */
template <class X, class T = double> class CellSpace: public Network<CellEvent<X>,T>
{
	public:
		/// A component model in the CellSpace
		typedef Devs<CellEvent<X>,T> Cell;
		/// Create an Width x Height x Depth CellSpace with NULL entries in the cell locations.
		CellSpace(long int width, long int height = 1, long int depth = 1);
		/// Insert a model at the x,y,z position.
		void add(Cell* model, long int x, long int y = 0, long int z = 0) 
		{
			space[x][y][z] = model;
			model->setParent(this);
		}
		/// Get the model at location x,y,z.
		const Cell* getModel(long int x, long int y = 0, long int z = 0) const
		{
			return space[x][y][z];
		}
		/// Get a mutable version of the model at x,y,z.
		Cell* getModel(long int x, long int y = 0, long int z = 0)
		{
			return space[x][y][z];
		}
		/// Get the width of the CellSpace.
		long int getWidth() const { return w; }
		/// Get the height of the CellSpace.
		long int getHeight() const { return h; }
		/// Get the depth of the CellSpace.
		long int getDepth() const { return d; }
		/// Get the model's set of components
		void getComponents(Set<Cell*>& c);
		/// Route events within the Cellspace
		void route(const CellEvent<X>& event, Cell* model, 
		Bag<Event<CellEvent<X>,T> >& r);
		/// Destructor; this destroys the components as well.
		~CellSpace();
	private:	
		long int w, h, d;
		Cell**** space;
};

// Implementation of constructor
template <class X, class T>
CellSpace<X,T>::CellSpace(long int width, long int height, long int depth):
Network<CellEvent<X>,T>()
{
	w = width;
	h = height;
	d = depth;
	// Allocate space for the cells and set the entries to NULL
	space = new Cell***[w];
	for (long int x = 0; x < w; x++)
	{
		space[x] = new Cell**[h];
		for (long int y = 0; y < h; y++)
		{
			space[x][y] = new Cell*[h];
			for (long int z = 0; z < d; z++)
			{
				space[x][y][z] = NULL;
			}
		}
	}
}

// Implementation of destructor
template <class X, class T>
CellSpace<X,T>::~CellSpace()
{
	for (long int x = 0; x < w; x++)
	{
		for (long int y = 0; y < h; y++)
		{
			for (long int z = 0; z < d; z++)
			{
				if (space[x][y][z] != NULL)
				{
					delete space[x][y][z];
				}
			}
			delete [] space[x][y];
		}
		delete [] space[x];
	}
	delete [] space;
}

// Implementation of the getComponents() method
template <class X, class T>
void CellSpace<X,T>::getComponents(Set<Cell*>& c)
{
	// Add all non-null entries to the set c
	for (long int x = 0; x < w; x++)
	{
		for (long int y = 0; y < h; y++)
		{
			for (long int z = 0; z < d; z++)
			{
				if (space[x][y][z] != NULL)
				{
					c.insert(space[x][y][z]);
				}
			}
		}
	}
}

// Event routing function for the net_exec
template <class X, class T>
void CellSpace<X,T>::route(
const CellEvent<X>& event, Cell* model, Bag<Event<CellEvent<X>,T> >& r)
{
	Cell* target = NULL;
	// If the target cell is inside of the cellspace
	if (event.x >= 0 && event.x < w &&  // check x dimension
	event.y >= 0 && event.y < h && // check y dimension
	event.z >= 0 && event.z < d) // check z dimension
	{
		// Get the interior target
		target = space[event.x][event.y][event.z];
	}
	else
	{
		// Otherwise, the event becomes an external output from the cellspace
		target = this;
	}
	// If the target exists
	if (target != NULL)
	{
		// Add an appropriate event to the receiver bag
		Event<CellEvent<X> > io(target,event);
		r.insert(io);
	}
}

} // end of namespace

#endif
