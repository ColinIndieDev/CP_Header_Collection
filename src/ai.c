#include "../cpai/cpai.h"

int main() {
    veci hs_neurons;
    veci_reserve(&hs_neurons, 2);
    veci_push_back(&hs_neurons, 256);
    veci_push_back(&hs_neurons, 128);

    veci h_activations;
    veci_init(&h_activations, 2, ReLU);

    neural_network *net =
        cpai_create_network(784, hs_neurons, 10, h_activations, Softmax, CEL,
                            0.9f, 0.99f, 0.1f, true);
    cpai_init_weights(net);

    cpai_load_network_bin(net, net->save_path);

    cpai_load_train_data_network(net, "data/train-images.idx3-ubyte",
                                 "data/train-labels.idx1-ubyte", 10);
    cpai_load_test_data_network(net, "data/t10k-images.idx3-ubyte",
                                "data/t10k-labels.idx1-ubyte");

    // cpai_train_network(net, 0.05f, 100, 64);
    cpai_test_network(net);
    cpai_destroy_network(net);
}
