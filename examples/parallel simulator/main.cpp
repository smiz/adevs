#include "Transcribe.h"
#include "Genr.h"
#include "adevs.h"
#include <iostream>
using namespace adevs;
using namespace std;

/**
 * Extend the SimpleDigraph class to allow its lookahead
 * to be set manually.
 */
class SimpleDigraphWithLookahead:
	public SimpleDigraph<char>
{
	public:
		SimpleDigraphWithLookahead():
			SimpleDigraph<char>(),
			look_ahead(0.0)
			{
			}
		void setLookahead(double look_ahead)
		{
			this->look_ahead = look_ahead;
		}
		double lookahead() { return look_ahead; }
	private:
		double look_ahead;
};

/**
 * Listener to record the output and state trajectories of the
 * component models.
 */
Genr* A_g;
Transcribe *A_t, *B, *C1, *C2;
SimpleDigraphWithLookahead *A, *C;

class Listener:
	public EventListener<char>
{
	public:
		Listener(){}
		void outputEvent(Event<char> y, double t)
		{
			string which = which_model(y.model);
			#pragma omp critical
			cout << which << " @ t = " << t << ", y(t)= " << y.value << endl;  
		}
		void stateChange(Atomic<char>* model, double t)
		{
			if (model == A_g)
				#pragma omp critical
				cout << which_model(A_g) << " @ t = " << t << ", running= "
					<< A_g->isRunning() << ", next output= " <<
					A_g->getNextOutput() << endl;
			else if (model == A_t)
				#pragma omp critical
				cout << which_model(A_t) << " @ t = " << t << ", memory= "
					<< A_t->getMemory() << ", ta()= " <<
					A_t->ta() << endl;
			else if (model == C1)
				#pragma omp critical
				cout << which_model(C1) << " @ t = " << t << ", memory= "
					<< C1->getMemory() << ", ta()= " <<
					C1->ta() << endl;
			else if (model == C2)
				#pragma omp critical
				cout << which_model(C2) << " @ t = " << t << ", memory= "
					<< C2->getMemory() << ", ta()= " <<
					C2->ta() << endl;
			else if (model == B)
				#pragma omp critical
				cout << which_model(B) << " @ t = " << t << ", memory= "
					<< B->getMemory() << ", ta()= " <<
					B->ta() << endl;
			else assert(false);
		}
	private:
		string which_model(Devs<char>* model)
		{
			if (model == A_g) return "A.A_g";
			if (model == A_t) return "A.A_t";
			if (model == A) return "A";
			if (model == B) return "B";
			if (model == C1) return "C.C1";
			if (model == C2) return "C.C2";
			if (model == C) return "C";
			assert(false);
			return "";
		}
};

int main(int argc, char** argv)
{
	// Component A
	A_g = new Genr();
	A_t = new Transcribe();
	A = new SimpleDigraphWithLookahead();
	A->setLookahead(A_t->lookahead()+A_g->lookahead());
	A->add(A_g);
	A->add(A_t);
	A->couple(A,A_g); // A -> A_g
	A->couple(A_g,A_t); // A_g -> A_t
	A->couple(A_t,A); // A_t -> A
	A->setProc(0); // Assign to thread zero
	// Component B
	B = new Transcribe();
	B->setProc(1); // Assign to thread one
	// Component C
	C1 = new Transcribe();
	C2 = new Transcribe();
	C = new SimpleDigraphWithLookahead();
	C->setLookahead(C1->lookahead()+C2->lookahead());
	C->add(C1);
	C->add(C2);
	C->couple(C,C1); // C -> C1
	C->couple(C2,C); // C2 -> C
	C->couple(C1,C2); // C1 -> C2
	C->couple(C2,C1); // C2 -> C1
	C->setProc(2); // Assign to thread two
	// Create the overarching model
	SimpleDigraph<char>* model  = new SimpleDigraph<char>();
	model->add(A);
	model->add(B);
	model->add(C);
	model->couple(A,B);
	model->couple(B,C);
	model->couple(C,A);
	// Create the corresponding LPGraph
	LpGraph lpg;
	lpg.addEdge(0,1);
	lpg.addEdge(1,2);
	lpg.addEdge(2,0);
	// Create the simulator
	ParSimulator<char>* sim = new ParSimulator<char>(model,lpg);
	// Register the listener
	Listener* listener = new Listener();
	sim->addEventListener(listener);
	// Run the simulation until t=10
	sim->execUntil(10.0);
	// Cleanup and exit
	delete sim;
	delete listener;
	delete model;
	return 0;
}
