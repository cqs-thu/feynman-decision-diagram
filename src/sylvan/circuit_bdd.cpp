#include "../simulator.h"

#include "adapter.h"

int main(int argc, char** argv) {
    simulate_circuit<sylvan_mtbdd_adapter>(argc, argv);
}
