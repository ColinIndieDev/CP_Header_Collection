#pragma once

#include <stdio.h>

#include <immintrin.h>
#include <omp.h>

#include "../cpstd/cpbase.h"
#include "../cpstd/cpmath.h"
#include "../cpstd/cprng.h"
#include "../cpstd/cpvec.h"

// No fcking idea what this is doing but it
// should increase performance though
static f32 cpai_dot_avx2(const f32 *a, const f32 *b, u32 n) {
    __m256 sum = _mm256_setzero_ps();
    u32 i = 0;

    for (; i + 8 <= n; i += 8) {
        __m256 va = _mm256_loadu_ps(a + i);
        __m256 vb = _mm256_loadu_ps(b + i);
        sum = _mm256_fmadd_ps(va, vb, sum);
    }

    __m128 lo = _mm256_castps256_ps128(sum);
    __m128 hi = _mm256_extractf128_ps(sum, 1);
    __m128 s = _mm_add_ps(lo, hi);
    s = _mm_add_ps(s, _mm_movehl_ps(s, s));
    s = _mm_add_ss(s, _mm_shuffle_ps(s, s, 1));
    f32 result = _mm_cvtss_f32(s);

    for (; i < n; i++) {
        result += a[i] * b[i];
    }

    return result;
}

// {{{ Types

typedef enum {
    Sigmoid,
    ReLU,
    Leaky_ReLU,
    Tanh,
    Softmax,
    Linear
} activation_type;
typedef enum { MSE, CEL } loss_type;

typedef struct {
    i32 neurons;
} input_layer;

typedef struct {
    mat2D weight;
    mat2D d_weight;
    mat2D v_weight;
    vecf bias;
    vecf d_bias;
    vecf v_bias;

    i32 neurons;
    activation_type a_type;
} hidden_layer;

typedef struct {
    mat2D weight;
    mat2D d_weight;
    mat2D v_weight;
    vecf bias;
    vecf d_bias;
    vecf v_bias;

    i32 neurons;
    activation_type a_type;
    loss_type l_type;
} output_layer;

VEC_DEF(i32, veci)
VEC_DEF(hidden_layer, vec_h_layer)
VEC_DEF(vecf *, vec_vecf)

typedef struct {
    char *save_path;
    mat2D train_data;
    veci train_labels;
    mat2D test_data;
    veci test_labels;
    mat2D train_sol;

    input_layer i_layer;
    vec_h_layer h_layers;
    output_layer o_layer;

    f32 momentum;
    f32 learn_rate_decay;
    f32 label_smooth_rate;
    b8 use_avx2;

    i32 cur_epoch;

    vec_vecf work_hidden;
    vec_vecf work_hidden_z;
} neural_network;

// }}}

// {{{ Forward Declarations

neural_network *cpai_create_network(i32 i_neurons, veci hs_neurons,
                                    i32 o_neurons, veci hidden_activations,
                                    activation_type output_actiation,
                                    loss_type output_loss, f32 momentum,
                                    f32 learn_rate_decay, f32 label_smooth_rate,
                                    b8 use_avx2);
f32 cpai_randf_normal();
void cpai_init_weights(neural_network *net);
void cpai_destroy_network(neural_network *net);

i32 cpai_reverse_i32(i32 i);
mat2D cpai_load_images(char *path);
veci cpai_load_labels(char *path);
void cpai_load_network_bin(neural_network *net, char *path);
void cpai_save_network_bin(neural_network *net, char *path);
void cpai_load_train_data_network(neural_network *net, char *data_path,
                                  char *label_path, i32 sol_row_len);
void cpai_load_test_data_network(neural_network *net, char *data_path,
                                 char *label_path);

static f32 cpai_sigmoid(f32 x);
static f32 cpai_sigmoid_deriv(f32 y);
static f32 cpai_relu(f32 x);
static f32 cpai_leaky_relu(f32 x);
static f32 cpai_leaky_relu_deriv(f32 y);
static f32 cpai_relu_deriv(f32 y);
static f32 cpai_tanh(f32 x);
static f32 cpai_tanh_deriv(f32 y);
static void cpai_softmax(vecf *v);
static f32 cpai_network_func(f32 x, activation_type a_type);
static f32 cpai_network_func_deriv(f32 x, activation_type a_type);

vecf cpai_feed_forward_avx2(neural_network *net, vecf *input);
vecf cpai_feed_forward(neural_network *net, vecf *input);

void cpai_reset_gradients(neural_network *net);
void cpai_calc_output_avx2(neural_network *net, vec_vecf *hidden,
                           vec_vecf *hidden_z, vecf *output, vecf *input);
void cpai_calc_output(neural_network *net, vec_vecf *hidden, vec_vecf *hidden_z,
                      vecf *output, vecf *input);
void cpai_calc_delta(neural_network *net, vec_vecf *hidden, vec_vecf *hidden_z,
                     vecf *output, vecf *input, vecf *target);
void cpai_accumulate_gradient(neural_network *net, vecf *X, vecf *Y);
void cpai_gradient_clipping(neural_network *net);
b8 cpai_check_nan(vecf *v);
void cpai_apply_gradient(neural_network *net, i32 batch_size, f32 learn_rate);

f32 capi_calc_loss(neural_network *net, vecf *output, vecf *target);
void cpai_shuffle_indices(u32 *indices, u32 count);
void cpai_train_network(neural_network *net, f32 learn_rate, i32 epochs,
                        i32 batch_size);
f32 cpai_test_network(neural_network *net);

// }}}

#ifdef CPAI_IMPLEMENTATION

// {{{ Init & destroy

neural_network *cpai_create_network(i32 i_neurons, veci hs_neurons,
                                    i32 o_neurons, veci hidden_activations,
                                    activation_type output_actiation,
                                    loss_type output_loss, f32 momentum,
                                    f32 learn_rate_decay, f32 label_smooth_rate,
                                    b8 use_avx2) {
    neural_network *net = malloc(sizeof(neural_network));

    net->i_layer.neurons = i_neurons;

    vec_h_layer_init(&net->h_layers, hs_neurons.size, (hidden_layer){});

    for (u32 i = 0; i < net->h_layers.size; i++) {
        hidden_layer *h_layer = &net->h_layers.data[i];
        h_layer->neurons = hs_neurons.data[i];
        h_layer->a_type = hidden_activations.data[i];

        vecf_init(&h_layer->bias, h_layer->neurons, 0.0f);
        vecf_init(&h_layer->d_bias, h_layer->neurons, 0.0f);
        vecf_init(&h_layer->v_bias, h_layer->neurons, 0.0f);

        if (i == 0) {
            mat2D_init(&h_layer->weight, h_layer->neurons, net->i_layer.neurons,
                       0.0f);
            mat2D_init(&h_layer->d_weight, h_layer->neurons,
                       net->i_layer.neurons, 0.0f);
            mat2D_init(&h_layer->v_weight, h_layer->neurons,
                       net->i_layer.neurons, 0.0f);
        } else {
            mat2D_init(&h_layer->weight, h_layer->neurons,
                       net->h_layers.data[i - 1].neurons, 0.0f);
            mat2D_init(&h_layer->d_weight, h_layer->neurons,
                       net->h_layers.data[i - 1].neurons, 0.0f);
            mat2D_init(&h_layer->v_weight, h_layer->neurons,
                       net->h_layers.data[i - 1].neurons, 0.0f);
        }
    }

    net->o_layer.neurons = o_neurons;
    net->o_layer.a_type = output_actiation;
    net->o_layer.l_type = output_loss;

    vecf_init(&net->o_layer.bias, o_neurons, 0.0f);
    vecf_init(&net->o_layer.d_bias, o_neurons, 0.0f);
    vecf_init(&net->o_layer.v_bias, o_neurons, 0.0f);

    i32 last_hidden_neurons =
        net->h_layers.data[net->h_layers.size - 1].neurons;
    mat2D_init(&net->o_layer.weight, o_neurons, last_hidden_neurons, 0.0f);
    mat2D_init(&net->o_layer.d_weight, o_neurons, last_hidden_neurons, 0.0f);
    mat2D_init(&net->o_layer.v_weight, o_neurons, last_hidden_neurons, 0.0f);

    net->momentum = momentum;
    net->learn_rate_decay = learn_rate_decay;
    net->label_smooth_rate = label_smooth_rate;
    net->use_avx2 = use_avx2;
    net->cur_epoch = 0;

    net->save_path = "save.bin";

    i32 hid_layer_cnt = (i32)net->h_layers.size;
    vec_vecf_reserve(&net->work_hidden, hid_layer_cnt);
    vec_vecf_reserve(&net->work_hidden_z, hid_layer_cnt);
    for (u32 i = 0; i < hid_layer_cnt; i++) {
        i32 neuronCount = vec_h_layer_at(&net->h_layers, i)->neurons;
        vecf *inner1 = malloc(sizeof(vecf));
        vecf *inner2 = malloc(sizeof(vecf));
        vecf_init(inner1, neuronCount, 0.0f);
        vecf_init(inner2, neuronCount, 0.0f);
        vec_vecf_push_back(&net->work_hidden, inner1);
        vec_vecf_push_back(&net->work_hidden_z, inner2);
    }

    return net;
}

f32 cpai_randf_normal() {
    f32 u1 = cprng_randf_range(1e-6f, 1.0f);
    f32 u2 = cprng_randf_range(0.0f, 1.0f);
    return cpm_sqrt(-2.0f * cpm_logf(u1)) * cpm_cosf(2.0f * CPM_PI * u2);
}

void cpai_init_weights(neural_network *net) {
    for (u32 l = 0; l < net->h_layers.size; l++) {
        hidden_layer *hidden = vec_h_layer_at(&net->h_layers, l);
        i32 inputSize = l == 0 ? net->i_layer.neurons
                               : vec_h_layer_at(&net->h_layers, l - 1)->neurons;

        if (hidden->a_type == Sigmoid || hidden->a_type == Tanh) {
            // Xavier / Glorot
            f32 limit = cpm_sqrt(6.0f / (f32)(inputSize + hidden->neurons));
            for (u32 i = 0; i < hidden->weight.data.size; i++) {
                hidden->weight.data.data[i] = cprng_randf_range(-limit, limit);
            }
        } else {
            // He (ReLU / Leaky_ReLU)
            f32 stddev = cpm_sqrt(2.0f / (f32)inputSize);
            for (u32 i = 0; i < hidden->weight.data.size; i++) {
                hidden->weight.data.data[i] = cpai_randf_normal() * stddev;
            }
        }
    }

    i32 lastNeurons =
        vec_h_layer_at(&net->h_layers, net->h_layers.size - 1)->neurons;
    if (net->o_layer.a_type == Softmax) {
        // Xavier (Softmax)
        f32 limit = cpm_sqrt(2.0f / (f32)(lastNeurons + net->o_layer.neurons));
        for (u32 i = 0; i < net->o_layer.weight.data.size; i++) {
            net->o_layer.weight.data.data[i] = cprng_randf_range(-limit, limit);
        }
    } else if (net->o_layer.a_type == Sigmoid || net->o_layer.a_type == Tanh) {
        // Xavier / Glorot
        f32 limit = cpm_sqrt(6.0f / (f32)(lastNeurons + net->o_layer.neurons));
        for (u32 i = 0; i < net->o_layer.weight.data.size; i++) {
            net->o_layer.weight.data.data[i] = cprng_randf_range(-limit, limit);
        }
    } else {
        // He (ReLU / Leaky_ReLU)
        f32 stddev = cpm_sqrt(2.0f / (f32)lastNeurons);
        for (u32 i = 0; i < net->o_layer.weight.data.size; i++) {
            net->o_layer.weight.data.data[i] = cpai_randf_normal() * stddev;
        }
    }
}

void cpai_destroy_network(neural_network *net) {
    for (u32 l = 0; l < net->h_layers.size; l++) {
        hidden_layer *hidden = vec_h_layer_at(&net->h_layers, l);
        mat2D_destroy(&hidden->weight);
        mat2D_destroy(&hidden->d_weight);
        mat2D_destroy(&hidden->v_weight);
        vecf_destroy(&hidden->bias);
        vecf_destroy(&hidden->d_bias);
        vecf_destroy(&hidden->v_bias);
    }
    output_layer *out = &net->o_layer;
    mat2D_destroy(&out->weight);
    mat2D_destroy(&out->d_weight);
    mat2D_destroy(&out->v_weight);
    vecf_destroy(&out->bias);
    vecf_destroy(&out->d_bias);
    vecf_destroy(&out->v_bias);

    mat2D_destroy(&net->train_data);
    veci_destroy(&net->train_labels);
    mat2D_destroy(&net->train_sol);
    mat2D_destroy(&net->test_data);
    veci_destroy(&net->test_labels);

    vec_h_layer_destroy(&net->h_layers);

    for (u32 i = 0; i < net->work_hidden.size; i++) {
        vecf_destroy(net->work_hidden.data[i]);
        free(net->work_hidden.data[i]);
    }
    vec_vecf_destroy(&net->work_hidden);

    for (u32 i = 0; i < net->work_hidden_z.size; i++) {
        vecf_destroy(net->work_hidden_z.data[i]);
        free(net->work_hidden_z.data[i]);
    }
    vec_vecf_destroy(&net->work_hidden_z);
    free(net);
}

// }}}

// {{{ Load & Save

// {{{ MNIST Loader

i32 cpai_reverse_i32(i32 i) {
    u8 c1 = i & 255;
    u8 c2 = i >> 8 & 255;
    u8 c3 = i >> 16 & 255;
    u8 c4 = i >> 24 & 255;
    return ((i32)c1 << 24) + ((i32)c2 << 16) + ((i32)c3 << 8) + c4;
}

mat2D cpai_load_images(char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "[CPAI] [ERROR]: Cannot open file: %s\n", path);
        exit(EXIT_FAILURE);
    }

    i32 magicNum = 0;
    i32 imageCount = 0;
    i32 rows = 0;
    i32 cols = 0;
    fread(&magicNum, sizeof(i32), 1, file);
    fread(&imageCount, sizeof(i32), 1, file);
    fread(&rows, sizeof(i32), 1, file);
    fread(&cols, sizeof(i32), 1, file);

    magicNum = cpai_reverse_i32(magicNum);
    imageCount = cpai_reverse_i32(imageCount);
    rows = cpai_reverse_i32(rows);
    cols = cpai_reverse_i32(cols);

    mat2D images;
    mat2D_init(&images, imageCount, rows * cols, 0.0f);
    for (u32 i = 0; i < imageCount; i++) {
        for (u32 j = 0; j < rows * cols; j++) {
            u8 pixel = 0;
            fread(&pixel, sizeof(u8), 1, file);
            *mat2D_at(&images, i, j) = (f32)pixel / 255.0f;
        }
    }
    fclose(file);

    return images;
}

veci cpai_load_labels(char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "[CPAI] [ERROR]: Cannot open file: %s\n", path);
        exit(EXIT_FAILURE);
    }

    i32 magicNum = 0;
    i32 itemCount = 0;
    fread(&magicNum, sizeof(i32), 1, file);
    fread(&itemCount, sizeof(i32), 1, file);

    magicNum = cpai_reverse_i32(magicNum);
    itemCount = cpai_reverse_i32(itemCount);

    veci labels;
    veci_init(&labels, itemCount, 0);
    for (u32 i = 0; i < itemCount; i++) {
        u8 label = 0;
        fread(&label, sizeof(u8), 1, file);
        *veci_at(&labels, i) = label;
    }
    fclose(file);

    return labels;
}

// }}}

void cpai_load_network_bin(neural_network *net, char *path) {
    FILE *in = fopen(path, "rb");
    if (!(b8)in) {
        fprintf(stderr, "[CPAI] [SAVE]: Cannot find file: %s\n", path);
        return;
    }

    fread(&net->cur_epoch, sizeof(i32), 1, in);

    i32 is = 0;
    i32 hls = 0;
    i32 os = 0;
    fread(&is, sizeof(i32), 1, in);
    fread(&hls, sizeof(i32), 1, in);

    for (u32 l = 0; l < hls; l++) {
        hidden_layer *hidden = vec_h_layer_at(&net->h_layers, l);
        i32 *hs = &hidden->neurons;
        fread(hs, sizeof(i32), 1, in);
    }
    fread(&os, sizeof(i32), 1, in);

    for (u32 l = 0; l < hls; l++) {
        i32 inputSize =
            l == 0 ? is : vec_h_layer_at(&net->h_layers, l - 1)->neurons;
        hidden_layer *hidden = vec_h_layer_at(&net->h_layers, l);
        for (u32 h = 0; h < hidden->neurons; h++) {
            fread(mat2D_row_ptr(&hidden->weight, h), inputSize * sizeof(f32), 1,
                  in);
        }
    }
    for (u32 o = 0; o < net->o_layer.neurons; o++) {
        hidden_layer *last_h =
            vec_h_layer_at(&net->h_layers, net->h_layers.size - 1);
        fread(mat2D_row_ptr(&net->o_layer.weight, o),
              last_h->neurons * sizeof(f32), 1, in);
    }

    for (u32 l = 0; l < hls; l++) {
        hidden_layer *hidden = vec_h_layer_at(&net->h_layers, l);
        fread(hidden->bias.data, hidden->neurons * sizeof(f32), 1, in);
    }
    fread(net->o_layer.bias.data, net->o_layer.neurons * sizeof(f32), 1, in);

    fclose(in);
    printf("[CPAI] [LOAD]: Neural network loaded from: %s\n", path);
    printf("[CPAI] [LOAD]: Loaded neural network currently has %d epochs!\n",
           net->cur_epoch);
}

void cpai_save_network_bin(neural_network *net, char *path) {
    FILE *out = fopen(path, "wb");
    if (!(b8)out) {
        fprintf(stderr, "[CPAI] [SAVE]: Cannot open path: %s\n", path);
        return;
    }

    i32 ce = net->cur_epoch;
    fwrite(&ce, sizeof(i32), 1, out);

    i32 is = net->i_layer.neurons;
    i32 hls = (i32)net->h_layers.size;
    i32 os = net->o_layer.neurons;

    fwrite(&is, sizeof(i32), 1, out);
    fwrite(&hls, sizeof(i32), 1, out);
    for (u32 i = 0; i < hls; i++) {
        i32 hs = vec_h_layer_at(&net->h_layers, i)->neurons;
        fwrite(&hs, sizeof(i32), 1, out);
    }
    fwrite(&os, sizeof(i32), 1, out);

    for (u32 l = 0; l < hls; l++) {
        hidden_layer *hidden = vec_h_layer_at(&net->h_layers, l);
        i32 inputSize =
            l == 0 ? is : vec_h_layer_at(&net->h_layers, l - 1)->neurons;
        for (u32 h = 0; h < hidden->neurons; h++) {
            fwrite(mat2D_row_ptr(&hidden->weight, h), inputSize * sizeof(f32),
                   1, out);
        }
    }
    for (u32 o = 0; o < os; o++) {
        hidden_layer *last_h =
            vec_h_layer_at(&net->h_layers, net->h_layers.size - 1);
        fwrite(mat2D_row_ptr(&net->o_layer.weight, o),
               last_h->neurons * sizeof(f32), 1, out);
    }

    for (u32 l = 0; l < hls; l++) {
        hidden_layer *hidden = vec_h_layer_at(&net->h_layers, l);
        fwrite(hidden->bias.data, hidden->neurons * sizeof(f32), 1, out);
    }
    fwrite(net->o_layer.bias.data, net->o_layer.neurons * sizeof(f32), 1, out);

    fclose(out);
    printf("[CPAI] [SAVE]: Neural network saved in: %s\n", path);
}

void cpai_load_train_data_network(neural_network *net, char *data_path,
                                  char *label_path, i32 sol_row_len) {
    net->train_data = cpai_load_images(data_path);

    net->train_labels = cpai_load_labels(label_path);

    mat2D_init(&net->train_sol, net->train_labels.size, 10, 0.0f);
    for (u32 sample = 0; sample < net->train_labels.size; sample++) {
        i32 label = veci_get(&net->train_labels, sample);

        for (u32 col = 0; col < sol_row_len; col++) {
            f32 one_hot = 1.0f - net->label_smooth_rate;
            f32 not_hot = net->label_smooth_rate / ((f32)sol_row_len - 1.0f);
            *mat2D_at(&net->train_sol, sample, col) =
                col == label ? one_hot : not_hot;
        }
    }
}

void cpai_load_test_data_network(neural_network *net, char *data_path,
                                 char *label_path) {
    net->test_data = cpai_load_images(data_path);

    net->test_labels = cpai_load_labels(label_path);
}

// }}}

// {{{ Functions

static f32 cpai_sigmoid(f32 x) { return 1.0f / (1.0f + cpm_expf(-x)); }
static f32 cpai_sigmoid_deriv(f32 y) { return y * (1.0f - y); }
static f32 cpai_relu(f32 x) { return CPM_MAX(0.0f, x); }
static f32 cpai_leaky_relu(f32 x) {
    f32 alpha = 0.01f;
    return x > 0.0f ? x : alpha * x;
}
static f32 cpai_leaky_relu_deriv(f32 y) {
    f32 alpha = 0.01f;
    return y > 0.0f ? 1.0f : alpha;
}
static f32 cpai_relu_deriv(f32 y) { return y > 0.0f ? 1.0f : 0.0f; }
static f32 cpai_tanh(f32 x) { return cpm_tanhf(x); }
static f32 cpai_tanh_deriv(f32 y) { return 1.0f - (y * y); }
static void cpai_softmax(vecf *v) {
    f32 max_val = -CPM_F32_MAX;
    for (u32 i = 0; i < v->size; i++) {
        if (v->data[i] > max_val) {
            max_val = v->data[i];
        }
    }

    f32 sum = 0.0f;
    for (u32 i = 0; i < v->size; i++) {
        f32 *x = &v->data[i];
        *x = cpm_expf(*x - max_val);
        sum += *x;
    }

    sum = CPM_MAX(sum, 1e-8f);
    for (u32 i = 0; i < v->size; i++) {
        f32 *x = &v->data[i];
        *x /= sum;
    }
}

static f32 cpai_network_func(f32 x, activation_type a_type) {
    switch (a_type) {
    case Sigmoid:
        return cpai_sigmoid(x);
        break;
    case ReLU:
        return cpai_relu(x);
        break;
    case Leaky_ReLU:
        return cpai_leaky_relu(x);
        break;
    case Tanh:
        return cpai_tanh(x);
        break;
    case Softmax: {
        printf("[CPAI] [ERROR]: Softmax activation output layer only! ReLu "
               "will be used instead\n");
        return cpai_relu(x);
        break;
    }
    case Linear: {
        printf("[CPAI] [ERROR]: Linear activation output layer only! ReLu "
               "will be used instead\n");
        return cpai_relu(x);
        break;
    }
    }
    return cpai_sigmoid(x);
}
static f32 cpai_network_func_deriv(f32 x, activation_type a_type) {
    switch (a_type) {
    case Sigmoid:
        return cpai_sigmoid_deriv(x);
        break;
    case ReLU:
        return cpai_relu_deriv(x);
        break;
    case Leaky_ReLU:
        return cpai_leaky_relu_deriv(x);
        break;
    case Tanh:
        return cpai_tanh_deriv(x);
        break;
    case Softmax: {
        printf("[CPAI] [ERROR]: Softmax activation is invalid here! ReLu "
               "will be used instead\n");
        return cpai_relu_deriv(x);
        break;
    }
    case Linear: {
        printf("[CPAI] [ERROR]: Linear activation is invalid here! ReLu "
               "will be used instead\n");
        return cpai_relu_deriv(x);
        break;
    }
    }
    return cpai_sigmoid_deriv(x);
}

// }}}

// {{{ Forwardpropagation

vecf cpai_feed_forward_avx2(neural_network *net, vecf *input) {
    vecf output;
    vecf_init(&output, net->o_layer.neurons, 0.0f);
    u32 hid_layer_cnt = net->h_layers.size;
    vec_vecf hidden;
    vec_vecf_reserve(&hidden, hid_layer_cnt);
    for (u32 i = 0; i < hid_layer_cnt; i++) {
        vecf *inner = malloc(sizeof(vecf));
        vecf_init(inner, vec_h_layer_at(&net->h_layers, i)->neurons, 0.0f);
        vec_vecf_push_back(&hidden, inner);
    }
    for (u32 l = 0; l < hid_layer_cnt; l++) {
        hidden_layer *layer = vec_h_layer_at(&net->h_layers, l);
        vecf *prev = l == 0 ? input : *vec_vecf_at(&hidden, l - 1);
        for (u32 h = 0; h < layer->neurons; h++) {
            f32 sum = vecf_get(&layer->bias, h) +
                      cpai_dot_avx2(mat2D_row_ptr(&layer->weight, h),
                                    prev->data, prev->size);
            *vecf_at((*vec_vecf_at(&hidden, l)), h) =
                cpai_network_func(sum, layer->a_type);
        }
    }
    vecf *last_h = *vec_vecf_at(&hidden, hidden.size - 1);
    for (u32 o = 0; o < net->o_layer.neurons; o++) {
        f32 sum = vecf_get(&net->o_layer.bias, o) +
                  cpai_dot_avx2(mat2D_row_ptr(&net->o_layer.weight, o),
                                last_h->data, last_h->size);
        *vecf_at(&output, o) = sum;
    }
    if (net->o_layer.a_type == Softmax) {
        cpai_softmax(&output);
    } else {
        for (u32 i = 0; i < output.size; i++) {
            f32 *o = vecf_at(&output, i);
            *o = cpai_network_func(*o, net->o_layer.a_type);
        }
    }
    for (u32 i = 0; i < hidden.size; i++) {
        vecf_destroy(hidden.data[i]);
        free(hidden.data[i]);
    }
    vec_vecf_destroy(&hidden);
    return output;
}

vecf cpai_feed_forward(neural_network *net, vecf *input) {
    vecf output;
    vecf_init(&output, net->o_layer.neurons, 0.0f);

    u32 hid_layer_cnt = net->h_layers.size;

    vec_vecf hidden;
    vec_vecf_reserve(&hidden, hid_layer_cnt);
    for (u32 i = 0; i < hid_layer_cnt; i++) {
        vecf *inner = malloc(sizeof(vecf));
        vecf_init(inner, vec_h_layer_at(&net->h_layers, i)->neurons, 0.0f);
        vec_vecf_push_back(&hidden, inner);
    }

    for (u32 l = 0; l < hid_layer_cnt; l++) {
        hidden_layer *layer = vec_h_layer_at(&net->h_layers, l);
        for (u32 h = 0; h < layer->neurons; h++) {
            f32 sum = vecf_get(&layer->bias, h);
            if (l == 0) {
                for (u32 i = 0; i < input->size; i++) {
                    sum += mat2D_get(&layer->weight, h, i) * vecf_get(input, i);
                }
            } else {
                for (u32 prevH = 0;
                     prevH < (*vec_vecf_at(&hidden, l - 1))->size; prevH++) {
                    sum += mat2D_get(&layer->weight, h, prevH) *
                           vecf_get((*vec_vecf_at(&hidden, l - 1)), prevH);
                }
            }

            *vecf_at((*vec_vecf_at(&hidden, l)), h) =
                cpai_network_func(sum, layer->a_type);
        }
    }

    for (u32 o = 0; o < net->o_layer.neurons; o++) {
        f32 sum = vecf_get(&net->o_layer.bias, o);
        for (u32 h = 0; h < (*vec_vecf_at(&hidden, hidden.size - 1))->size;
             h++) {
            sum += mat2D_get(&net->o_layer.weight, o, h) *
                   vecf_get((*vec_vecf_at(&hidden, hidden.size - 1)), h);
        }
        *vecf_at(&output, o) = sum;
    }

    if (net->o_layer.a_type == Softmax) {
        cpai_softmax(&output);
    } else {
        for (u32 i = 0; i < output.size; i++) {
            f32 *o = vecf_at(&output, i);
            *o = cpai_network_func(*o, net->o_layer.a_type);
        }
    }

    for (u32 i = 0; i < hidden.size; i++) {
        vecf_destroy(hidden.data[i]);
        free(hidden.data[i]);
    }
    vec_vecf_destroy(&hidden);

    return output;
}

// }}}

// {{{ Backwardpropagation

void cpai_reset_gradients(neural_network *net) {
    for (u32 i = 0; i < net->h_layers.size; i++) {
        hidden_layer *h = vec_h_layer_at(&net->h_layers, i);

        for (u32 j = 0; j < mat2D_size(&h->d_weight); j++) {
            *vecf_at(&h->d_weight.data, j) = 0.0f;
        }
        for (u32 j = 0; j < h->d_bias.size; j++) {
            *vecf_at(&h->d_bias, j) = 0.0f;
        }
    }
    for (u32 i = 0; i < mat2D_size(&net->o_layer.d_weight); i++) {
        *vecf_at(&net->o_layer.d_weight.data, i) = 0.0f;
    }
    for (u32 i = 0; i < net->o_layer.d_bias.size; i++) {
        *vecf_at(&net->o_layer.d_bias, i) = 0.0f;
    }
}

void cpai_calc_output_avx2(neural_network *net, vec_vecf *hidden,
                           vec_vecf *hidden_z, vecf *output, vecf *input) {
    i32 outputNeuronCount = net->o_layer.neurons;
    i32 hiddenLayerCount = (i32)net->h_layers.size;
    for (u32 l = 0; l < hiddenLayerCount; l++) {
        hidden_layer *layer = vec_h_layer_at(&net->h_layers, l);
        vecf *prev = l == 0 ? input : *vec_vecf_at(hidden, l - 1);
        for (u32 h = 0; h < layer->neurons; h++) {
            f32 sum = vecf_get(&layer->bias, h) +
                      cpai_dot_avx2(mat2D_row_ptr(&layer->weight, h),
                                    prev->data, prev->size);
            *vecf_at((*vec_vecf_at(hidden_z, l)), h) = sum;
            *vecf_at((*vec_vecf_at(hidden, l)), h) =
                cpai_network_func(sum, layer->a_type);
        }
    }
    vecf *last_h = *vec_vecf_at(hidden, hidden->size - 1);
    for (u32 o = 0; o < outputNeuronCount; o++) {
        f32 sum = vecf_get(&net->o_layer.bias, o) +
                  cpai_dot_avx2(mat2D_row_ptr(&net->o_layer.weight, o),
                                last_h->data, last_h->size);
        *vecf_at(output, o) = sum;
    }
}

void cpai_calc_output(neural_network *net, vec_vecf *hidden, vec_vecf *hidden_z,
                      vecf *output, vecf *input) {
    i32 outputNeuronCount = net->o_layer.neurons;
    i32 hiddenLayerCount = (i32)net->h_layers.size;

    for (u32 l = 0; l < hiddenLayerCount; l++) {
        hidden_layer *layer = vec_h_layer_at(&net->h_layers, l);
        for (u32 h = 0; h < layer->neurons; h++) {
            f32 sum = vecf_get(&layer->bias, h);
            if (l == 0) {
                for (u32 i = 0; i < input->size; i++) {
                    sum += mat2D_get(&layer->weight, h, i) * vecf_get(input, i);
                }
            } else {
                for (u32 prevH = 0; prevH < (*vec_vecf_at(hidden, l - 1))->size;
                     prevH++) {
                    sum += mat2D_get(&layer->weight, h, prevH) *
                           vecf_get((*vec_vecf_at(hidden, l - 1)), prevH);
                }
            }

            *vecf_at((*vec_vecf_at(hidden_z, l)), h) = sum;
            *vecf_at((*vec_vecf_at(hidden, l)), h) =
                cpai_network_func(sum, layer->a_type);
        }
    }

    for (u32 o = 0; o < outputNeuronCount; o++) {
        f32 sum = vecf_get(&net->o_layer.bias, o);
        vecf *last_h = *vec_vecf_at(hidden, hidden->size - 1);
        for (u32 h = 0; h < last_h->size; h++) {
            sum += mat2D_get(&net->o_layer.weight, o, h) * vecf_get(last_h, h);
        }
        *vecf_at(output, o) = sum;
    }
}

void cpai_calc_delta(neural_network *net, vec_vecf *hidden, vec_vecf *hidden_z,
                     vecf *output, vecf *input, vecf *target) {
    i32 inputNeuronCount = net->i_layer.neurons;
    i32 hiddenLayerCount = (i32)net->h_layers.size;
    i32 outputNeuronCount = net->o_layer.neurons;

    vecf deltaOut;
    vecf_init(&deltaOut, outputNeuronCount, 0.0f);
    activation_type *activation = &net->o_layer.a_type;
    loss_type *loss = &net->o_layer.l_type;
    if (*activation == Softmax && *loss == CEL) {
        for (u32 o = 0; o < outputNeuronCount; o++) {
            *vecf_at(&deltaOut, o) = vecf_get(output, o) - vecf_get(target, o);
        }
    } else if (*loss == MSE) {
        for (u32 o = 0; o < outputNeuronCount; o++) {
            f32 diff = vecf_get(output, o) - vecf_get(target, o);
            if (*activation == Sigmoid) {
                *vecf_at(&deltaOut, o) =
                    diff * cpai_network_func_deriv(vecf_get(output, o),
                                                   net->o_layer.a_type);
            } else {
                *vecf_at(&deltaOut, 0) = diff;
            }
        }
    }

    vec_vecf deltaHid;
    vec_vecf_reserve(&deltaHid, hiddenLayerCount);
    for (u32 i = 0; i < hiddenLayerCount; i++) {
        vecf *inner = malloc(sizeof(vecf));
        vecf_init(inner, net->h_layers.data[i].neurons, 0.0f);
        vec_vecf_push_back(&deltaHid, inner);
    }
    for (i32 l = hiddenLayerCount - 1; l >= 0; l--) {
        hidden_layer *layer = vec_h_layer_at(&net->h_layers, l);
        for (u32 h = 0; h < layer->neurons; h++) {
            f32 sum = 0;
            if (l == net->h_layers.size - 1) {
                for (int dO = 0; dO < outputNeuronCount; dO++) {
                    sum += vecf_get(&deltaOut, dO) *
                           mat2D_get(&net->o_layer.weight, dO, h);
                }
            } else {
                hidden_layer *prevLayer = vec_h_layer_at(&net->h_layers, l + 1);
                for (int dh = 0; dh < prevLayer->neurons; dh++) {
                    sum += vecf_get((*vec_vecf_at(&deltaHid, l + 1)), dh) *
                           mat2D_get(&prevLayer->weight, dh, h);
                }
            }

            if (layer->a_type == ReLU || layer->a_type == Leaky_ReLU) {
                *vecf_at((*vec_vecf_at(&deltaHid, l)), h) =
                    sum * cpai_network_func_deriv(
                              vecf_get((*vec_vecf_at(hidden_z, l)), h),
                              layer->a_type);
            } else {
                *vecf_at((*vec_vecf_at(&deltaHid, l)), h) =
                    sum *
                    cpai_network_func_deriv(
                        vecf_get((*vec_vecf_at(hidden, l)), h), layer->a_type);
            }
        }
    }

    vecf *last_h = *vec_vecf_at(hidden, hidden->size - 1);
    for (u32 o = 0; o < outputNeuronCount; o++) {
        for (u32 h = 0; h < last_h->size; h++) {
            *mat2D_at(&net->o_layer.d_weight, o, h) +=
                vecf_get(&deltaOut, o) * vecf_get(last_h, h);
        }
        *vecf_at(&net->o_layer.d_bias, o) += vecf_get(&deltaOut, o);
    }

    for (u32 l = 0; l < hiddenLayerCount; l++) {
        hidden_layer *layer = vec_h_layer_at(&net->h_layers, l);
        for (u32 h = 0; h < layer->neurons; h++) {
            if (l == 0) {
                for (int i = 0; i < inputNeuronCount; i++) {
                    *mat2D_at(&layer->d_weight, h, i) +=
                        vecf_get((*vec_vecf_at(&deltaHid, l)), h) *
                        vecf_get(input, i);
                }
            } else {
                vecf *prevLayer = *vec_vecf_at(hidden, l - 1);
                for (int dh = 0; dh < prevLayer->size; dh++) {
                    *mat2D_at(&layer->d_weight, h, dh) +=
                        vecf_get((*vec_vecf_at(&deltaHid, l)), h) *
                        vecf_get(prevLayer, dh);
                }
            }
            *vecf_at(&layer->d_bias, h) +=
                vecf_get((*vec_vecf_at(&deltaHid, l)), h);
        }
    }

    for (u32 i = 0; i < deltaHid.size; i++) {
        vecf_destroy(deltaHid.data[i]);
        free(deltaHid.data[i]);
    }
    vec_vecf_destroy(&deltaHid);
    vecf_destroy(&deltaOut);
}

void cpai_accumulate_gradient(neural_network *net, vecf *X, vecf *Y) {
    vecf *input = X;
    vecf *target = Y;

    i32 hiddenLayerCount = (i32)net->h_layers.size;
    i32 outputNeuronCount = net->o_layer.neurons;

    vecf output;
    vecf_init(&output, outputNeuronCount, 0.0f);

    if (net->use_avx2) {
        cpai_calc_output_avx2(net, &net->work_hidden, &net->work_hidden_z,
                              &output, input);
    } else {
        cpai_calc_output(net, &net->work_hidden, &net->work_hidden_z, &output,
                         input);
    }

    if (net->o_layer.a_type == Softmax) {
        cpai_softmax(&output);
    } else {
        for (u32 o = 0; o < outputNeuronCount; o++) {
            *vecf_at(&output, o) =
                cpai_network_func(vecf_get(&output, o), net->o_layer.a_type);
        }
    }

    cpai_calc_delta(net, &net->work_hidden, &net->work_hidden_z, &output, input,
                    target);

    vecf_destroy(&output);
}

void cpai_gradient_clipping(neural_network *net) {
    f32 norm = 0.0f;
    for (u32 h = 0; h < net->h_layers.size; h++) {
        hidden_layer *hidden = vec_h_layer_at(&net->h_layers, h);

        for (u32 i = 0; i < mat2D_size(&hidden->d_weight); i++) {
            f32 g = vecf_get(&hidden->d_weight.data, i);
            norm += g * g;
        }
        for (u32 i = 0; i < hidden->d_bias.size; i++) {
            f32 g = vecf_get(&hidden->d_bias, i);
            norm += g * g;
        }
    }

    for (u32 i = 0; i < mat2D_size(&net->o_layer.d_weight); i++) {
        f32 g = vecf_get(&net->o_layer.d_weight.data, i);
        norm += g * g;
    }
    for (u32 i = 0; i < net->o_layer.d_bias.size; i++) {
        f32 g = vecf_get(&net->o_layer.d_bias, i);
        norm += g * g;
    }

    norm = cpm_sqrt(norm);

    if (norm > 5.0f) {
        f32 scale = 5.0f / norm;

        for (u32 h = 0; h < net->h_layers.size; h++) {
            hidden_layer *hidden = vec_h_layer_at(&net->h_layers, h);

            for (u32 i = 0; i < mat2D_size(&hidden->d_weight); i++) {
                f32 *g = vecf_at(&hidden->d_weight.data, i);
                *g *= scale;
            }
            for (u32 i = 0; i < hidden->d_bias.size; i++) {
                f32 *g = vecf_at(&hidden->d_bias, i);
                *g *= scale;
            }
        }

        for (u32 i = 0; i < mat2D_size(&net->o_layer.d_weight); i++) {
            f32 *g = vecf_at(&net->o_layer.d_weight.data, i);
            *g *= scale;
        }
        for (u32 i = 0; i < net->o_layer.d_bias.size; i++) {
            f32 *g = vecf_at(&net->o_layer.d_bias, i);
            *g *= scale;
        }
    }
}

b8 cpai_check_nan(vecf *v) {
    for (u32 i = 0; i < v->size; i++) {
        if (cpm_isnan(vecf_get(v, i))) {
            return true;
        }
    }
    return false;
}

void cpai_apply_gradient(neural_network *net, i32 batch_size, f32 learn_rate) {
    f32 scale = learn_rate / (f32)batch_size;

    if (cpai_check_nan(&net->o_layer.d_weight.data) ||
        cpai_check_nan(&net->o_layer.d_bias)) {
        printf("[CPAI] [WARNING]: NaN detected, skipping update\n");
        return;
    }
    for (u32 i = 0; i < net->h_layers.size; i++) {
        hidden_layer *hidden = vec_h_layer_at(&net->h_layers, i);
        if (cpai_check_nan(&hidden->d_weight.data) ||
            cpai_check_nan(&hidden->d_bias)) {
            printf("[CPAI] [WARNING]: NaN detected, skipping update\n");
            return;
        }
    }

    i32 inputNeuronCount = net->i_layer.neurons;
    i32 hiddenLayerCount = (i32)net->h_layers.size;
    i32 outputNeuronCount = net->o_layer.neurons;

    cpai_gradient_clipping(net);

    for (u32 l = 0; l < hiddenLayerCount; l++) {
        hidden_layer *hidden = vec_h_layer_at(&net->h_layers, l);

        i32 prevLayerSize =
            l == 0 ? inputNeuronCount
                   : vec_h_layer_at(&net->h_layers, l - 1)->neurons;

        for (i32 h = 0; h < hidden->neurons; h++) {
            for (i32 i = 0; i < prevLayerSize; i++) {
                *mat2D_at(&hidden->v_weight, h, i) =
                    (net->momentum * mat2D_get(&hidden->v_weight, h, i)) +
                    mat2D_get(&hidden->d_weight, h, i);
                *mat2D_at(&hidden->weight, h, i) -=
                    scale * mat2D_get(&hidden->v_weight, h, i);
            }
            *vecf_at(&hidden->v_bias, h) =
                (net->momentum * vecf_get(&hidden->v_bias, h)) +
                vecf_get(&hidden->d_bias, h);
            *vecf_at(&hidden->bias, h) -= scale * vecf_get(&hidden->v_bias, h);
        }
    }
    for (u32 o = 0; o < outputNeuronCount; o++) {
        for (u32 h = 0;
             h <
             vec_h_layer_at(&net->h_layers, net->h_layers.size - 1)->neurons;
             h++) {
            *mat2D_at(&net->o_layer.v_weight, o, h) =
                (net->momentum * mat2D_get(&net->o_layer.v_weight, o, h)) +
                mat2D_get(&net->o_layer.d_weight, o, h);
            *mat2D_at(&net->o_layer.weight, o, h) -=
                scale * mat2D_get(&net->o_layer.v_weight, o, h);
        }
        *vecf_at(&net->o_layer.v_bias, o) =
            (net->momentum * vecf_get(&net->o_layer.v_bias, o)) +
            vecf_get(&net->o_layer.d_bias, o);
        *vecf_at(&net->o_layer.bias, o) -=
            scale * vecf_get(&net->o_layer.d_bias, o);
    }
}

// }}}

// {{{ Train & Testing

f32 capi_calc_loss(neural_network *net, vecf *output, vecf *target) {
    f32 loss = 0.0f;
    if (net->o_layer.l_type == CEL) {
        for (u32 i = 0; i < target->size; i++) {
            loss -= vecf_get(target, i) * cpm_logf(vecf_get(output, i) + 1e-8f);
        }
    } else {
        for (u32 i = 0; i < target->size; i++) {
            f32 d = vecf_get(output, i) - vecf_get(target, i);
            loss += 0.5f * d * d;
        }
    }
    return loss;
}

void cpai_shuffle_indices(u32 *indices, u32 count) {
    for (u32 i = count - 1; i > 0; i--) {
        u32 j = cprng_rand() % (i + 1);
        u32 tmp = indices[i];
        indices[i] = indices[j];
        indices[j] = tmp;
    }
}

void cpai_train_network(neural_network *net, f32 learn_rate, i32 epochs,
                        i32 batch_size) {
    u32 sample_count = net->train_data.rows;
    u32 data_cols = net->train_data.cols;
    u32 sol_cols = net->train_sol.cols;

    u32 *indices = malloc(sample_count * sizeof(u32));
    for (u32 i = 0; i < sample_count; i++) {
        indices[i] = i;
    }

    vecf input;
    vecf target;
    vecf_init(&input, data_cols, 0.0f);
    vecf_init(&target, sol_cols, 0.0f);

    for (i32 epoch = 0; epoch < epochs; epoch++) {
        clock_t start = clock();

        cpai_shuffle_indices(indices, sample_count);

        f32 cur_learn_rate = learn_rate;
        for (i32 e = 0; e < net->cur_epoch; e++) {
            cur_learn_rate *= net->learn_rate_decay;
        }

        for (u32 n = 0; n < sample_count; n += (u32)batch_size) {
            cpai_reset_gradients(net);

            i32 real_batch = batch_size;
            if (n + (u32)batch_size > sample_count) {
                real_batch = (i32)(sample_count - n);
            }

            for (i32 b = 0; b < real_batch; b++) {
                u32 idx = indices[n + b];

                memcpy(input.data, mat2D_row_ptr(&net->train_data, idx),
                       data_cols * sizeof(f32));
                memcpy(target.data, mat2D_row_ptr(&net->train_sol, idx),
                       sol_cols * sizeof(f32));

                cpai_accumulate_gradient(net, &input, &target);
            }

            cpai_apply_gradient(net, real_batch, cur_learn_rate);
        }

        f32 cpu_time_used = ((f32)(clock() - start)) / CLOCKS_PER_SEC;
        printf("[CPAI] [INFO]: Training epoch took %f s\n", cpu_time_used);

        f32 avg_weight = 0.0f;
        for (u32 l = 0; l < net->h_layers.size; l++) {
            hidden_layer *h = vec_h_layer_at(&net->h_layers, l);
            f32 layer_avg = 0.0f;
            for (u32 i = 0; i < h->weight.data.size; i++) {
                layer_avg += h->weight.data.data[i];
            }
            avg_weight += layer_avg / (f32)h->weight.data.size;
        }
        avg_weight /= (f32)net->h_layers.size;
        printf("[CPAI] [TRAIN]: "
               "\n---------------------------------------------\nAverage "
               "Weight = %f\n",
               avg_weight);

        f32 avg_bias = 0.0f;
        for (u32 l = 0; l < net->h_layers.size; l++) {
            hidden_layer *h = vec_h_layer_at(&net->h_layers, l);
            f32 layer_avg = 0.0f;
            for (u32 i = 0; i < h->bias.size; i++) {
                layer_avg += h->bias.data[i];
            }
            avg_bias += layer_avg / (f32)h->bias.size;
        }
        avg_bias /= (f32)net->h_layers.size;
        printf("Average Bias = %f\n", avg_bias);

        f32 total_loss = 0.0f;
        u32 loss_samples = sample_count < 1000 ? sample_count : 1000;
#pragma omp parallel for reduction(+ : total_loss)
        for (u32 i = 0; i < loss_samples; i++) {
            vecf inp;
            vecf tgt;
            vecf_init(&inp, data_cols, 0.0f);
            vecf_init(&tgt, sol_cols, 0.0f);
            memcpy(inp.data, mat2D_row_ptr(&net->train_data, indices[i]),
                   data_cols * sizeof(f32));
            memcpy(tgt.data, mat2D_row_ptr(&net->train_sol, indices[i]),
                   sol_cols * sizeof(f32));
            vecf output = cpai_feed_forward(net, &inp);
            total_loss += capi_calc_loss(net, &output, &tgt);
            vecf_destroy(&output);
            vecf_destroy(&inp);
            vecf_destroy(&tgt);
        }
        printf("Average Loss = %f\n", total_loss / (f32)loss_samples);

        net->cur_epoch++;
        printf("Epoch(s) trained: %d / %d (Total epochs: "
               "%d)\n---------------------------------------------\n",
               epoch + 1, epochs, net->cur_epoch);

        cpai_save_network_bin(net, net->save_path);
    }

    vecf_destroy(&input);
    vecf_destroy(&target);
    free(indices);
}

f32 cpai_test_network(neural_network *net) {
    clock_t start = clock();

    i32 correct = 0;
    i32 total = (i32)net->test_data.rows;
    u32 data_cols = net->test_data.cols;

#pragma omp parallel for reduction(+ : correct)
    for (i32 i = 0; i < total; i++) {
        vecf inp;
        vecf_init(&inp, data_cols, 0.0f);
        memcpy(inp.data, mat2D_row_ptr(&net->test_data, (u32)i),
               data_cols * sizeof(f32));

        vecf output;
        if (net->use_avx2) {
            output = cpai_feed_forward_avx2(net, &inp);
        } else {
            output = cpai_feed_forward(net, &inp);
        }

        i32 predicted = 0;
        for (u32 j = 1; j < output.size; j++) {
            if (output.data[j] > output.data[predicted]) {
                predicted = (i32)j;
            }
        }
        vecf_destroy(&output);
        vecf_destroy(&inp);

        if (predicted == veci_get(&net->test_labels, (u32)i)) {
            correct++;
        }
    }

    f32 cpu_time_used = ((f32)(clock() - start)) / CLOCKS_PER_SEC;
    printf("[CPAI] [INFO]: Testing took %f s\n", cpu_time_used);

    f32 accuracy = 100.0f * (f32)correct / (f32)total;
    printf("[CPAI] [TEST]: Test accuracy: %.2f%% (%d / %d)\n", accuracy,
           correct, total);
    return accuracy;
}

// }}}

#endif
