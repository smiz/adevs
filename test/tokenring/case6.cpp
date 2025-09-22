#include <iostream>
#include <memory>
#include "node.h"


using Simulator = adevs::Simulator<std::shared_ptr<token_t>>;
using Graph = adevs::Graph<std::shared_ptr<token_t>>;

int main() {

    std::shared_ptr<Graph> model = std::make_shared<Graph>();

    std::shared_ptr<token_t> token0 = std::make_shared<token_t>(0);
    std::shared_ptr<token_t> token1 = std::make_shared<token_t>(1);

    std::shared_ptr<Node> n1 = std::make_shared<Node>(0, 1, token0);
    std::shared_ptr<Node> n2 = std::make_shared<Node>(1, 1, token1);
    std::shared_ptr<Node> n3 = std::make_shared<Node>(2, 3, nullptr);

    model->add_atomic(n1);
    model->add_atomic(n2);
    model->add_atomic(n3);
    model->connect(n1->in, n1);
    model->connect(n2->in, n2);
    model->connect(n3->in, n3);
    model->connect(n1->out, n2->in);
    model->connect(n2->out, n3->in);
    model->connect(n3->out, n1->in);

    std::shared_ptr<Simulator> sim = std::make_shared<Simulator>(model);

    for (int i = 0; i < 10 && sim->nextEventTime() < adevs_inf<double>(); i++) {
        std::cout << std::endl;
        sim->execNextEvent();
    }

    std::cout << std::endl;
    std::cout << "End of run!" << std::endl;

    return 0;
}
