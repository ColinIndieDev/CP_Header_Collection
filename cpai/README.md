# CPAIntelligence

# About
My AI library I am currently making from scratch without any third-party libraries (only f.e. OpenMP for parallelization) 
which should be something like PyTorch but it is written and used in C.

## Functionality
Currently you are able to create a neural network with fixed input and output layers but scalable hidden ones + you are free to choose the amount of neurons for each layer. Also you can choose your activation function like Sigmoid, ReLu, Leaky ReLu, Tanh and Softmax or Linear (both used for output only).
With the given resources you are able to make a typical digit recognition AI using the MNIST database. For now you are only restricted by classification not regression (I think) and supervised machine learning.

- load data from files (here MNIST labels & images)
- multiple activation functions for each layer (except input layer):
  - Sigmoid
  - Tanh
  - ReLU
  - Leaky ReLU
  - Softmax
  - Linear
- and loss types:
  - MSE Loss (Mean Squared Error)
  - CEL (Cross Entropy Loss)
- shuffling
- momentum
- label smoothing
- batching
- parallelization (with OpenMP)
- AVX2 (optimization type if using modern CPUs)

With all of this you can able can get high accuracies with only few epochs taking few seconds depending on the network's size.
More optimizations will be introduced eventually and more

## Example (digit recognition neural network)

Here is an example code for the neural network with 784 input neurons, 10 output neurons
and 2 hidden layers one with 256 the other 128 neurons. ReLU only is used with Softmax and Cross Entropy:

```c
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

    cpai_train_network(net, 0.05f, 10, 64);
    cpai_test_network(net);
    cpai_destroy_network(net);
}
```

In this example, we train 10 epochs (each epoch will automatically create a save of the network as a .bin extension.
We train with batches each 64 and 0.05 learn rate. Before we also set momentum to 90%, learn rate decay to 99% and label smoothing to 0.1

## Features

```c
neural_network *cpai_create_network(i32 i, veci h, i32 o, veci ha, activation_type oa, loss_type ol, f32 m, f32 lrd, f32 lsr, b8 ua);
/*
i = input neurons
h = array of hidden neurons (for multiple layers)
o = output neurons
ha = hidden activation function(s)
oa = output activation function
ol = output loss type
m = momentum rate
lrd = learn rate decay
lsr = label smooth rate
ua = toggle use of AVX2 (on modern CPUs for a little faster training if supported)
*/

void cpai_init_weights(neural_network *net);
// initialize weights and biases (with He, Xavier etc. depending on activations)

void cpai_destroy_network(neural_network *net);

void cpai_load_network_bin(neural_network *net, char *path);

void cpai_save_network_bin(neural_network *net, char *path);

void cpai_load_train_data_network(neural_network *net, char *data_path, char *label_path, i32 sol_row_len);

void cpai_load_test_data_network(neural_network *net, char *data_path, char *label_path);

vecf cpai_feed_forward(neural_network *net, vecf *input);
// returns results from output

void cpai_train_network(neural_network *net, f32 learn_rate, i32 epochs, i32 batch_size);

f32 cpai_test_network(neural_network *net);
// returns accuracy in %
```
