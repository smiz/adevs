#include <iostream>
#include <memory>
#include "node.h"

using namespace std;

using Simulator = adevs::Simulator<shared_ptr<token_t>>;
using Graph = adevs::Graph<shared_ptr<token_t>>;

int main() {

    shared_ptr<Graph> model = make_shared<Graph>();

    shared_ptr<token_t> token0 = make_shared<token_t>(0);
    shared_ptr<token_t> token1 = make_shared<token_t>(1);

    shared_ptr<Node> n1 = make_shared<Node>(0, 1, token0);
    shared_ptr<Node> n2 = make_shared<Node>(1, 1, token1);
    shared_ptr<Node> n3 = make_shared<Node>(2, 3, nullptr);

    model->add_atomic(n1);
    model->add_atomic(n2);
    model->add_atomic(n3);
    model->connect(n1->in, n1);
    model->connect(n2->in, n2);
    model->connect(n3->in, n3);
    model->connect(n1->out, n2->in);
    model->connect(n2->out, n3->in);
    model->connect(n3->out, n1->in);

    shared_ptr<Simulator> sim = make_shared<Simulator>(model);

    for (int i = 0; i < 10 && sim->nextEventTime() < adevs_inf<double>(); i++) {
        cout << endl;
        sim->execNextEvent();
    }

    cout << endl;
    cout << "End of run!" << endl;

    return 0;
}
