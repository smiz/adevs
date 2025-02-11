/*
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

#ifndef _adevs_cellspace_h_
#define _adevs_cellspace_h_

#include <cstdlib>
#include <set>
#include "adevs/adevs.h"


namespace adevs {

/*
 * Input and output events produced by components of a CellSpace must
 * be of the type CellEvent.  A CellEvent has an event value (i.e., the actual
 * input/output value) and a target cell for the event.
 */
template <class OutputType>
class CellEvent {
  public:
    /// Default constructor. Sets x = y = z = 0.
    CellEvent() : value() { x = y = z = 0; }
    /// Copy constructor
    CellEvent(CellEvent<OutputType> const &src)
        : x(src.x), y(src.y), z(src.z), value(src.value) {}
    /// Assignment operator
    CellEvent const &operator=(CellEvent<OutputType> const &src) {
        x = src.x;
        y = src.y;
        z = src.z;
        value = src.value;
        return *this;
    }
    /// The x coordinate of the event target
    long int x;
    /// The y coordinate of the event target
    long int y;
    /// The z coordinate of the event target
    long int z;
    /// The event value
    OutputType value;
};

/*
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
template <class OutputType, class TimeType = double>
class CellSpace : public Network<CellEvent<OutputType>, TimeType> {
  public:
    using Cell = Devs<CellEvent<OutputType>, TimeType>;

    /// Create an Width x Height x Depth CellSpace with NULL entries in the cell locations.
    CellSpace(long int width, long int height = 1, long int depth = 1);

    /// Insert a model at the x,y,z position.
    void add(shared_ptr<Cell> model, long int x, long int y = 0,
             long int z = 0) {
        space[x][y][z] = model;
        model->setParent(this);
    }
    /// Get the model at location x,y,z.
    Cell const* getModel(long int x, long int y = 0, long int z = 0) const {
        return space[x][y][z].get();
    }
    /// Get a mutable version of the model at x,y,z.
    Cell* getModel(long int x, long int y = 0, long int z = 0) {
        return space[x][y][z].get();
    }

    /// Get the width of the CellSpace.
    long int getWidth() const { return w; }
    /// Get the height of the CellSpace.
    long int getHeight() const { return h; }
    /// Get the depth of the CellSpace.
    long int getDepth() const { return d; }
    /// Get the model's set of components
    void getComponents(set<Cell*> &c);

    /// Route events within the Cellspace
    void route(CellEvent<OutputType> const &event, Cell* model,
               list<Event<CellEvent<OutputType>, TimeType>> &r);
    /// Destructor; this destroys the components as well.
    ~CellSpace();

  private:
    // 3-dimensional cell space where all nodes are connected
    vector<vector<vector<shared_ptr<Cell>>>> space;
    long int w, h, d;
};

// Implementation of constructor
template <class OutputType, class TimeType>
CellSpace<OutputType, TimeType>::CellSpace(long int width, long int height,
                                           long int depth)
    : w(width),
      h(height),
      d(depth),
      Network<CellEvent<OutputType>, TimeType>() {

    // Allocate space for the cells and set the entries to nullptrs
    // ! Using a nested vector may not really be better than raw arrays.
    // ! One advantage is letting the standard library do all memory management.
    for (long int x = 0; x < width; x++) {
        vector<vector<shared_ptr<Cell>>> h_space;
        for (long int y = 0; y < height; y++) {
            h_space.push_back(vector<shared_ptr<Cell>>(depth, nullptr));
        }
        space.push_back(h_space);
    };
    cout << "Allocated vector: " << space.size() << ", " << space[0].size()
         << ", " << space[0][0].size() << endl;
}

template <class OutputType, class TimeType>
CellSpace<OutputType, TimeType>::~CellSpace() {}


template <class OutputType, class TimeType>
void CellSpace<OutputType, TimeType>::getComponents(set<Cell*> &c) {
    // Add all non-null entries to the set c
    for (long int x = 0; x < w; x++) {
        for (long int y = 0; y < h; y++) {
            for (long int z = 0; z < d; z++) {
                if (space[x][y][z] != NULL) {
                    c.insert(space[x][y][z].get());
                }
            }
        }
    }
}

// Event routing function for the net_exec
template <class OutputType, class TimeType>
void CellSpace<OutputType, TimeType>::route(
    CellEvent<OutputType> const &event, Cell* model,
    list<Event<CellEvent<OutputType>, TimeType>> &r) {
    Cell* target = nullptr;
    // If the target cell is inside of the cellspace
    if (event.x >= 0 && event.x < w &&  // check x dimension
        event.y >= 0 && event.y < h &&  // check y dimension
        event.z >= 0 && event.z < d)    // check z dimension
    {
        // Get the interior target
        target = space[event.x][event.y][event.z].get();
    } else {
        // Otherwise, the event becomes an external output from the cellspace
        target = this;
    }
    // If the target exists
    if (target != nullptr) {
        // Add an appropriate event to the receiver list
        Event<CellEvent<OutputType>, TimeType> io(target, event);
        r.push_back(io);
    }
}

}  // namespace adevs

#endif
