#include "../simulator.h"

#include "adapter.h"

int main(int argc, char **argv) {
    simulate_circuit<cudd_add_adapter>(argc, argv);
}
