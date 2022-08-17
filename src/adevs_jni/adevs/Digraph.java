/***************
Copyright (C) 2010 by James Nutaro

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
package adevs;
import java.util.HashMap;
import java.util.HashSet;
import java.util.ArrayList;
import java.util.Collection;

/**
 * This is a digraph model for connecting multiple input/multiple output
 * systems. This Digraph is a Java implementation of the SimpleDigraph
 * class that appears in the C++ simulation package.
 */
public class Digraph extends Network
{
	/**
	 * Input and output for the Digraph and its components
	 * must be PortValue objects.
	 */
	public static class PortValue
	{
		/**
		 * Create a PortValue object and assign to
		 * it the supplied port and value.
		 * @param port The port on which the value appears
		 * @param value The value on that port
		 */
		public PortValue(int port, Object value)
		{
			this.value = value;
			this.port = port;
		}
		/**
		 * Get the value object.
		 * @return The object assigned by the constructor.
		 */
		public Object getValue() { return value; }
		/**
		 * Get the port object.
		 * @return The port assigned by the constructor.
		 */
		public int getPort() { return port; }
		private Object value;
		private int port;
	}
	/**
	 * This class is used to route events inside of the network.
	 */
	private class ModelPort
	{
		public ModelPort(Devs model, int port)
		{
			this.model = model;
			this.port = port;
		}
		public Devs getModel() { return model; }
		public int getPort() { return port; }
		@Override
		public boolean equals(Object other)
		{
			if (other instanceof ModelPort)
			{
				ModelPort mp = (ModelPort)other;
				return mp.model == model && mp.port == port;
			}
			return false;
		}
		@Override
		public int hashCode()
		{
			return model.hashCode() ^ port;
		}
		private int port;
		private Devs model;
	}
	/**
	 * Construct a network without components.
	 */
	public Digraph()
	{
		super();
		nodes = new HashSet<Devs>();
		graph = new HashMap<ModelPort,ArrayList<ModelPort> >();
	}
	/**
	 * Add a model to the network.
	 * @param model The DEVS model that will be a component of the network
	 */
	public void add(Devs model)
	{
		nodes.add(model);
	}
	/**
	 * Couple the source model to the destination model.
	 * @param src The model that generates output on this link
	 * @param src_port Port on which output appears
	 * @param dst Model to receive input on this link
	 * @param dst_port Port on which input will appear
	 */
	public void couple(Devs src, int src_port, Devs dst, int dst_port)
	{
		if (src != this) add(src);
		if (dst != this) add(dst);
		ModelPort src_mp = new ModelPort(src,src_port);
		ArrayList<ModelPort> dst_list = graph.get(src_mp);
		if (dst_list == null)
		{
			dst_list = new ArrayList<ModelPort>();
			graph.put(src_mp,dst_list);
		}
		dst_list.add(new ModelPort(dst,dst_port));
	}
	/**
	 * Puts the network's set of components into c.
	 * @param c A Collection to which will be added the model's components
	 */
	public void getComponents(Collection<Devs> c)
	{
		c.addAll(nodes);
	}
	/**
	 * Route an event according to the network's couplings.
	 * @see Network
	 */
	public void route(Object x, Devs model, Collection<Event> r)
	{
		PortValue pv = (PortValue)x;
		// Find the list of target models and ports
		ArrayList<ModelPort> targets = graph.get(new ModelPort(model,pv.port));
		// If no target, just return
		if (targets == null) return;
		// Otherwise, add the targets to the event bag
		for (ModelPort dst: targets)
		{
			r.add(new Event(dst.getModel(),
						new PortValue(dst.getPort(),pv.getValue())));
		}
	}

	private HashMap<ModelPort,ArrayList<ModelPort> > graph;
	private HashSet<Devs> nodes;
};
