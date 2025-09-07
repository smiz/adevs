#include "gpt.h"

int main() {

    // In the future, use argparse or some other nicer method for handling input.

    // Get experiment parameters
    Arguments args;

    std::cout << "Generator period: ";
    cin >> args.generator_period;

    std::cout << "Processor time: ";
    cin >> args.processor_time;

    std::cout << "Observation time: ";
    cin >> args.observation_time;

    return gpt(args);
}
