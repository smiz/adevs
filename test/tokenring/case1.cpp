#include <iostream>
#include <memory>
#include "node.h"

using namespace std;


int main() {
    shared_ptr<Network> model = make_shared<Network>();
    shared_ptr<token_t> token = make_shared<token_t>();

    shared_ptr<Node> n1 = make_shared<Node>(0, 1, token);
    shared_ptr<Node> n2 = make_shared<Node>(1, 1, nullptr);

    model->add(n1);
    model->add(n2);
    model->couple(n1, n1->out, n2, n2->in);
    model->couple(n2, n2->out, n1, n1->in);

    shared_ptr<Simulator> sim = make_shared<Simulator>(model);

    for (int i = 0; i < 10 && sim->nextEventTime() < DBL_MAX; i++) {
        cout << endl;
        sim->execNextEvent();
    }
    cout << endl;
    cout << "End of run!" << endl;

    return 0;
}
