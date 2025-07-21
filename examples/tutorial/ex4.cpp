
#include "adevs/adevs.h"
#include <map>
#include <list>
#include <iostream>
#include <random>

/**
 * This is an example of a computer processor with 
 * pipelined fetch, decode, execute stages and multiple
 * cores for executing instructions in parallel. We use
 * the model to calculate instructions per second. Each
 * triple in the array below describes a possible instruction
 * for our hypothetical machine. The first number is the
 * time needed to fetch the instruction, then the time
 * to decode it, and then the time to execute it. Our
 * simple machine has just 5 instructions.
 */
const int instructions[5][3] = {
    // { fetch, decode, execute }
    { 1, 1, 1 },
    { 1, 2, 2 },
    { 1, 2, 5 },
    { 1, 3, 6 },
    { 1, 1, 2 }
};

/**
 * Time in this simulation is measured by the integer
 * number of clock cycles. The value type used for
 * input and output is an integer also; this value
 * will be the identify of the instruction to execute
 * or the identity of the core that is ready to receive
 * an instruction. We define Atomic, Coupled, and
 * Simulator types accordingly.
 */

using Simulator = adevs::Simulator<int,int>;
using Atomic = adevs::Atomic<int,int>;
using MealyAtomic = adevs::MealyAtomic<int,int>;
using Coupled = adevs::Coupled<int,int>;
using PinValue = adevs::PinValue<int>;

/**
  * The InstructionSource generates one instruction each
  * time it receives a `ready' signal. Each `ready' line
  * is associated with a processor core that will receive
  * the instruction.
  */
class InstructionSource : public MealyAtomic {
public:
    /// The instructor creates a random number generator that
    /// is used to select instructions from the list of possible
    /// instructions.
    InstructionSource() : 
        MealyAtomic(),
        instruction(0,4), // generate random numbers in [0,4]
        generator(std::random_device{}()),
        instructions_complete(0), // count number of completed instructions
        instructions_issued(0) {} // number ofinstructions issued
    /// Map a ready pin to an output pin. The core will produce an instruction
    /// on the output_pin when it received an input on the ready_pin.
    void map_ready_to_core(adevs::pin_t ready_pin, adevs::pin_t output_pin) {
        ready_to_output[ready_pin] = output_pin;
    }
    /// We issue instruction immediately if none have been issued. Otherwise
    /// the model is driven entirely by input.
    int ta() { return (instructions_issued == 0) ? 0 : adevs_inf<int>(); }
    /// When the time advance expires, we produce an instruction
    /// for each ready core.
    void output_func(std::list<PinValue> &yb) {
        /// This is our initial issuance of instructions. Send one to
        /// each processor
        assert(instructions_issued == 0);
        for (auto processor: ready_to_output) {
            yb.push_back(PinValue(processor.second, instruction(generator)));
        }
    }
    /// This output function is called immediately prior to an external
    /// event. The elapsed time and list of input are exactly those that
    /// will be passed to the external transition function. In this 
    /// output function we produce an instruction as output for each
    /// ready event that we receive as input
    void external_output_func(int, std::list<PinValue> const &xb, std::list<PinValue> &yb) {
        for (auto x: xb) {
            yb.push_back(PinValue(ready_to_output[x.pin],instruction(generator)));
        }
    }
    /// This output function is called immediately prior to a confluent event.
    /// It is never called in this simulation (can you explain why?)
    void confluent_output_func(std::list<PinValue> const &, std::list<PinValue>&) {}
    /// The only internal event is at the start of the simulation, when we
    /// issue an instruction to each processor core.
    void delta_int() { instructions_issued += ready_to_output.size(); }
    /// We have completed some some number of instructions and issued
    /// new instructions.
    void delta_ext(int, std::list<PinValue> const& xb) {
        instructions_complete += xb.size();
        instructions_issued += xb.size();
    }
    /// The simulator should never call our confluent method (can you 
    /// explain why?)
    void delta_conf(std::list<PinValue> const&) { assert(false); }

    int get_instructions_complete() const { return instructions_complete; }
    int get_instructions_issued() const { return instructions_issued; }

private:
    /// Map of ready pins to output pins. This is used to map
    /// a ready signal from a core to the output pin that 
    /// must be used to issue a new instruction to the ready
    /// core
    std::map<adevs::pin_t,adevs::pin_t> ready_to_output;
    std::uniform_int_distribution<> instruction;
    std::default_random_engine generator;
    /// Total number of instructions completed
    int instructions_complete;
    /// Total number of instructions issued
    int instructions_issued;
};

/**
 * This is a single stage in a pipelined processor. When the stage is
 * ready to receive an instruction, it emits an event on its
 * ready_to_receive pin. The stage will release its instruction
 * when it receives an input on its ready_to_transmit pin.
 * The receive_instruction pin is used to receive an instruction. If
 * this is stage 2 then it is the end of the pipeline and does not
 * wait for a ready_to_transmit input.
 */
class Stage : public Atomic {
public:
    /// Constructor assigns a stage number and sets the model
    /// to be passive.
    Stage(int stage) :
        Atomic(),
        stage(stage),
        time_to_finish(0),
        instruction(-1),
        downstream_ready(true) {}

    int ta() {
        // Wait until we are finished with our instruction
        if (time_to_finish > 0) {
            return time_to_finish;
        // If we are finished with our instruction, then emit
        // it if the next stage is ready to receive it
        } else if (downstream_ready && instruction != -1) {
            return 0;
        // Otherwise nothing to do but wait
        } else {
            return adevs_inf<int>();
        }
    }
    /// The output function produces the completed instruction
    /// and indicates are readiness for a new instruction
    void output_func(std::list<PinValue>& yb) {
        assert(instruction != -1);
        /// Don't issue our instruction of the downstream core is not
        /// ready to receive it
        if (!downstream_ready) return;
        yb.push_back(PinValue(transmit_instruction, instruction));
        yb.push_back(PinValue(ready_to_receive, -1));
    }
    /// Clear the instruction and wait for a new instruction
    void delta_int() {
        /// We are finished processing our instruction
        time_to_finish = 0;
        /// If we emitted the instruction, then we are
        /// ready for a new instruction and our downstream
        /// stage is busy with the instruction we emitted
        if (downstream_ready) {
            instruction = -1;
            /// Stage 2 has no downstream stage and
            /// so that non-existent stage is always
            /// ready
            downstream_ready = stage == 2;
        }
    }
    void delta_ext(int e, std::list<PinValue> const &xb) {
        /// Account for time working on an active instruction
        if (time_to_finish > 0) {
            time_to_finish -= e;
        }
        /// Process our input
        for (const auto &input : xb) {
            if (input.pin == ready_to_transmit) {
                downstream_ready = true;
            } else if (input.pin == receive_instruction) {
                /// We had better not be processing an instruction
                /// when we receive a new one
                assert(instruction == -1);
                instruction = input.value;
                time_to_finish = instructions[instruction][stage];
            }
        }
    }
    void delta_conf(std::list<PinValue> const& xb) {
        delta_int();
        delta_ext(0,xb);
    }

    /// Finished instructions are put on this pin
    const adevs::pin_t transmit_instruction;
    /// New instructions are received on this pin
    const adevs::pin_t receive_instruction;
    /// Permission to transmit an instruction is received
    /// on this pin
    const adevs::pin_t ready_to_transmit;
    /// Indicate we are ready for a new instruction by
    /// sending an event on this pin
    const adevs::pin_t ready_to_receive;

private:
    /// Which stage in the pipeline is assigned to us
    const int stage;
    /// The time remaining to finish the current instruction.
    int time_to_finish;
    /// The instruction being processed. -1 means no instruction.
    int instruction;
    /// Is the downstream stage ready to receive an instruction
    bool downstream_ready;
};

/**
 * A processor is made up of three stages connected
 * end to end.
 */
class Processor: public Coupled {
public:
    Processor() : Coupled() {
        /// Create the stages and add them to the model
        for (int i = 0; i < 3; i++) {
            stage[i] = std::make_shared<Stage>(i);
            add_atomic(stage[i]);
        }
        /// The decoding stage informs the instruction
        /// source that we are ready for a new instruction
        create_coupling(stage[0]->ready_to_receive,ready_to_receive);
        /// New instructions go to the decoding stage
        create_coupling(receive_instruction,stage[0]->receive_instruction);
        create_coupling(stage[0]->receive_instruction,stage[0]);
        /// Couple the stages to each other
        for (int i = 1; i < 3; i++) {
            create_coupling(stage[i]->ready_to_receive,stage[i-1]->ready_to_transmit);
            create_coupling(stage[i-1]->ready_to_transmit,stage[i-1]);
            create_coupling(stage[i-1]->transmit_instruction,stage[i]->receive_instruction);
            create_coupling(stage[i]->receive_instruction,stage[i]);
        }
    }

    /// New instructions are received on this pin
    const adevs::pin_t receive_instruction;
    /// Indicate we are ready for a new instruction by
    /// sending an event on this pin
    const adevs::pin_t ready_to_receive;

private:
    std::shared_ptr<Stage> stage[3];
};

/**
 * Create a computer with the indicated number
 * of processors.
 */
class Computer : public Coupled {
public:
    Computer(int processors) {
        source = make_shared<InstructionSource>();
        add_atomic(source);
        for (int i = 0; i < processors; i++) {
            adevs::pin_t execute_instruction;
            std::shared_ptr<Processor> processor = make_shared<Processor>();
            add_coupled_model(processor);
            source->map_ready_to_core(processor->ready_to_receive,execute_instruction);
            create_coupling(execute_instruction,processor->receive_instruction);
            create_coupling(processor->ready_to_receive,source);
        }
    }

    int get_instructions_complete() const { return source->get_instructions_complete(); }

private:
    std::shared_ptr<InstructionSource> source;
};

/**
 * The main function creates a our model and runs
 * the simulation for 10 units of time.
 */
int main() {
    int time = 0;
    const int num_processors = 2;
    auto computer = std::make_shared<Computer>(num_processors);
    Simulator simulator(computer);
    /// Run the simulator for 2 units of time
    while (computer->get_instructions_complete() < 1000) {
        time = simulator.execNextEvent();
    }
    std::cout << "Processors:   " << num_processors << std::endl;
    std::cout << "Instructions: " << computer->get_instructions_complete() << std::endl;
    std::cout << "Time:         " << time << std::endl;
    std::cout << "Rate:         " << ((double)computer->get_instructions_complete())/(double)time << std::endl;
    /// Done!
    return 0;
}
