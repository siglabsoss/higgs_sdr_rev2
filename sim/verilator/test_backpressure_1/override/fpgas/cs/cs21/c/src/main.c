#include "dma.h"
#include "vmem.h"
#include "csr_control.h"
#include "config_word_cmul_eq_0f.h"
#include "config_word_conj_eq_0f.h"
#include <stdint.h>
#include "vector_multiply.h"
#include "circular_buffer_pow2.h"


// #define ENABLE_TB_DEBUG

#ifndef ENABLE_TB_DEBUG
#define DISABLE_TB_DEBUG
#endif
#include "tb_debug.h"


#define VECTOR_INITIAL_VALUE 0x2
#include "vmem_vector_1k.h"
#define BUFFER_SIZE (1024)
#define CIRCULAR_BUFFER_SIZE (4)
#define INPUT_BUFFER_COUNT (2)
#define TRANSFORM_BUFFER_COUNT (2)
#define MAX_INPUT_OCCUPANCY (2)
#define MAX_OUTPUT_OCCUPANCY (2)

VMEM_SECTION unsigned int input_dma_buf_1[BUFFER_SIZE] = {};
VMEM_SECTION unsigned int input_dma_buf_2[BUFFER_SIZE] = {};
VMEM_SECTION unsigned int transform_dma_buf_1[BUFFER_SIZE] = {};
VMEM_SECTION unsigned int transform_dma_buf_2[BUFFER_SIZE] = {};

unsigned int input_dma_buf_addr[INPUT_BUFFER_COUNT] = {};
unsigned int transform_buf_addr[TRANSFORM_BUFFER_COUNT] = {};

//
circular_buf_pow2_t __input_dma_schedule =
CIRBUF_POW2_STATIC_CONSTRUCTOR(__input_dma_schedule, CIRCULAR_BUFFER_SIZE);
circular_buf_pow2_t* input_dma_schedule = &__input_dma_schedule;
//
circular_buf_pow2_t __transform_schedule =
CIRBUF_POW2_STATIC_CONSTRUCTOR(__transform_schedule, CIRCULAR_BUFFER_SIZE);
circular_buf_pow2_t* transform_schedule = &__transform_schedule;
//
circular_buf_pow2_t __available_input_dma =
CIRBUF_POW2_STATIC_CONSTRUCTOR(__available_input_dma, CIRCULAR_BUFFER_SIZE);
circular_buf_pow2_t* available_input_dma = &__available_input_dma;
//
circular_buf_pow2_t __output_dma_schedule =
CIRBUF_POW2_STATIC_CONSTRUCTOR(__output_dma_schedule, CIRCULAR_BUFFER_SIZE);
circular_buf_pow2_t* output_dma_schedule = &__output_dma_schedule;
//
circular_buf_pow2_t __available_transform_dma =
CIRBUF_POW2_STATIC_CONSTRUCTOR(__available_transform_dma, CIRCULAR_BUFFER_SIZE);
circular_buf_pow2_t* available_transform_dma = &__available_transform_dma;
//
circular_buf_pow2_t __output_schedule =
CIRBUF_POW2_STATIC_CONSTRUCTOR(__output_schedule, CIRCULAR_BUFFER_SIZE);
circular_buf_pow2_t* output_schedule = &__output_schedule;


/**
 * Enable two input DMA simultaneously and store DMA addresses in array
 */
void init_input_dma();

/**
 *
 */
void init_transform_buf();

/**
 *
 */
void init_circular_buf();

/**
 *
 */
void _update_transform_schedule(const unsigned int occupancy);

/**
 *
 */
void _schedule_input_dma(unsigned int occupancy);

/**
 *
 */
void input_dma_process();

/**
 *
 */
void _perform_transform(unsigned int available_dma);

/**
 *
 */
void _update_output_schedule(unsigned int available_dma);

/**
 *
 */
void _update_available_input_dma(unsigned int available_dma);

/**
 *
 */
void transform_process();

/**
 *
 */
void _schedule_output_dma(unsigned int out_occupancy);

/**
 *
 */
void _update_available_transform(unsigned int out_occupancy);

/**
 *
 */
void output_dma_process();

/**
 *
 */
int main(void) {

    setup_debug();
    STALL(50);
    _printf("Boot\n");
    init_circular_buf();
    init_input_dma();
    init_transform_buf();

    while (1) {
        input_dma_process();
        transform_process();
        output_dma_process();
    }

    return 0;
}

void init_input_dma() {
    int error_1;
    int error_2;
    unsigned int buffer_addr_1 = VMEM_DMA_ADDRESS(input_dma_buf_1);
    unsigned int buffer_addr_2 = VMEM_DMA_ADDRESS(input_dma_buf_2);
    input_dma_buf_addr[0] = buffer_addr_1;
    input_dma_buf_addr[1] = buffer_addr_2;
    dma_in_set(buffer_addr_1, BUFFER_SIZE);
    dma_in_set(buffer_addr_2, BUFFER_SIZE);
    error_1 = circular_buf2_put(input_dma_schedule, 0);
    error_2 = circular_buf2_put(input_dma_schedule, 1);
}

void init_transform_buf() {
    int error_1;
    int error_2;
    unsigned int buffer_addr_1 = VMEM_DMA_ADDRESS(transform_dma_buf_1);
    unsigned int buffer_addr_2 = VMEM_DMA_ADDRESS(transform_dma_buf_2);

    transform_buf_addr[0] = buffer_addr_1;
    transform_buf_addr[1] = buffer_addr_2;

    error_1 = circular_buf2_put(available_transform_dma, 0);
    error_2 = circular_buf2_put(available_transform_dma, 1);
}

void init_circular_buf() {
    CIRBUF_POW2_RUNTIME_INITIALIZE(__input_dma_schedule);
    CIRBUF_POW2_RUNTIME_INITIALIZE(__transform_schedule);
    CIRBUF_POW2_RUNTIME_INITIALIZE(__available_input_dma);
    CIRBUF_POW2_RUNTIME_INITIALIZE(__output_dma_schedule);
    CIRBUF_POW2_RUNTIME_INITIALIZE(__available_transform_dma);
    CIRBUF_POW2_RUNTIME_INITIALIZE(__output_schedule);
}

void _update_transform_schedule(const unsigned int occupancy) {
    unsigned int expected_input_occupancy;
    unsigned int occupancy_diff;

    expected_input_occupancy = circular_buf2_occupancy(input_dma_schedule);
    occupancy_diff = expected_input_occupancy - occupancy;
    if (occupancy_diff) {
        int error;
        unsigned int completed_input_dma;
        for (unsigned int i = 0; i < occupancy_diff; i++) {
            error = circular_buf2_get(input_dma_schedule, &completed_input_dma);
            if (error != -1) {
                circular_buf2_put(transform_schedule, completed_input_dma);
            }
        }
    }
}

void _schedule_input_dma(unsigned int occupancy) {
    if (occupancy < MAX_INPUT_OCCUPANCY) {
        int error;
        unsigned int available_dma;

        error = circular_buf2_get(available_input_dma, &available_dma);
        if (error != -1) {
            dma_in_set(input_dma_buf_addr[available_dma], BUFFER_SIZE);
            error = circular_buf2_put(input_dma_schedule, available_dma);
            _printf("%s%d\n", "    DMA     ", available_dma);
        }
    }
}

void input_dma_process() {
    unsigned int occupancy;
    CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, occupancy);
    _update_transform_schedule(occupancy);
    _schedule_input_dma(occupancy);
}

void _perform_transform(unsigned int available_dma) {
    unsigned int output_row_addr;
    unsigned int input_row_addr;

    input_row_addr = VMEM_ROW_ADDRESS(
                     REVERSE_VMEM_DMA_ADDRESS(
                     input_dma_buf_addr[available_dma]));
    output_row_addr = VMEM_ROW_ADDRESS(
                      REVERSE_VMEM_DMA_ADDRESS(
                      transform_buf_addr[available_dma]));
    vector_add_1024(
                    input_row_addr,
                    VMEM_ROW_ADDRESS(vmem_values),
                    output_row_addr);
    STALL(50);
}

void _update_output_schedule(unsigned int available_dma) {
    int error;

    error = circular_buf2_put(output_schedule, available_dma);
}

void _update_available_input_dma(unsigned int available_dma) {
    int error;

    error = circular_buf2_put(available_input_dma, available_dma);
}

void transform_process() {
    int dma_error;
    int transform_error;
    unsigned int available_dma;
    unsigned int available_transform;
    dma_error = circular_buf2_peek(transform_schedule, &available_dma);
    transform_error = circular_buf2_peek(available_transform_dma,
                                         &available_transform);
    if (dma_error != -1 && transform_error != -1) {
        dma_error = circular_buf2_get(transform_schedule, &available_dma);
        transform_error = circular_buf2_get(available_transform_dma,
                                            &available_transform);
        if (available_dma == available_transform) {
            _perform_transform(available_dma);
            _update_output_schedule(available_dma);
            _update_available_input_dma(available_dma);
            _printf("%s%d\n", "    TFM     ", available_dma);
        }
    }
}

void _schedule_output_dma(unsigned int out_occupancy) {
    if (out_occupancy < MAX_OUTPUT_OCCUPANCY) {
        int error;
        unsigned int available_dma;
        error = circular_buf2_get(output_schedule, &available_dma);

        if (error != -1) {
            dma_block_send(transform_buf_addr[available_dma], BUFFER_SIZE);
            error = circular_buf2_put(output_dma_schedule, available_dma);
            _printf("%s%d\n", "    ODMA    ", available_dma);
        }
    }
}

void _update_available_transform(unsigned int out_occupancy) {
    unsigned int expected_output_occupancy;
    unsigned int occupancy_diff;

    expected_output_occupancy = circular_buf2_occupancy(output_dma_schedule);
    occupancy_diff = expected_output_occupancy - out_occupancy;

    _printf("%s%d %s%d %s%d \n", "    ODMA -- ", out_occupancy,
            " ", expected_output_occupancy, " ", occupancy_diff);

    if (occupancy_diff) {
        int error;
        unsigned int completed_output_dma;
        for (unsigned int i = 0; i < occupancy_diff; i++) {
            error = circular_buf2_get(output_dma_schedule,
                                      &completed_output_dma);
            if (error != -1) {
                circular_buf2_put(available_transform_dma,
                                  completed_output_dma); 
            }
        }
    }
}

void output_dma_process() {
    unsigned int out_occupancy;
    CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, out_occupancy);
    _schedule_output_dma(out_occupancy);
    CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, out_occupancy);
    _update_available_transform(out_occupancy);
}
