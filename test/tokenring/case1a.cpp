#include <iostream>
#include <list>
#include <memory>
#include "node.h"

using namespace std;


/**
  * This is the token ring case 1 test but it initializes the
  * simulator using a list of atomic components instead of
  * providing the coupled model.
  */

using Simulator = adevs::Simulator<shared_ptr<token_t>>;
using Graph = adevs::Graph<shared_ptr<token_t>>;

int main() {

    shared_ptr<Graph> model = make_shared<Graph>();
    shared_ptr<token_t> token = make_shared<token_t>();
    shared_ptr<Node> n1 = make_shared<Node>(0, 1, token);
    shared_ptr<Node> n2 = make_shared<Node>(1, 1, nullptr);

    model->add_atomic(n1);
    model->add_atomic(n2);
    model->connect(n1->in, n1);
    model->connect(n2->in, n2);
    model->connect(n1->out, n2->in);
    model->connect(n2->out, n1->in);

    shared_ptr<Simulator> sim = make_shared<Simulator>(model);

    for (int i = 0; i < 10 && sim->nextEventTime() < adevs_inf<double>(); i++) {
        cout << endl;
        sim->execNextEvent();
    }
    cout << endl;
    cout << "End of run!" << endl;

    return 0;
}
