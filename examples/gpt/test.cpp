#include "gpt.h"

int main() {
    Arguments args;

    args.generator_period = 1;
    args.processor_time = 2;
    args.observation_time = 10;
    return gpt(args);
}
