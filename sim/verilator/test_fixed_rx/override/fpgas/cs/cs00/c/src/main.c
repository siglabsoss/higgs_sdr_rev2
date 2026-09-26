#include "fill.h"
#include "dma.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "turnstile.h"
#include "ringbus.h"
#include "circular_buffer.h"

#include "vmalloc.h"
// declare as global
VMalloc mgr;

/*
Essentially we're creating a software fifo read and write pointers however each pointer is to a block of memory with start and end property. 
Another aspect that differentiates this implementation is 
*/

#define DATA_LEN (1024)
#define NSLICE   (6)

#define OFFSET_CMD (0x11000000)

#define DMA_IN_COUNT (4)
#define DMA_IN_LEN (2)
#define DMA_IN_OFFSET (0)
#define DMA_IN_START(x) (DMA_IN_OFFSET+(DMA_IN_LEN*(x)))
#define DMA_IN_POINTER(x) (DMA_IN_OFFSET+(DMA_IN_LEN*(x)))
#define MEMORY_OFFSET (0)
// global signals


unsigned int global;
unsigned int offset = 0;
unsigned int dma_in_last = DMA_IN_COUNT;
unsigned int dma_in_last_consumed = DMA_IN_COUNT;
unsigned int dma_in_expected_occupancy;

char idma_en;                  // in DMA enable
char odma_en;	               // out DMA enable
char  idma_fifo_count;          // Keeps a count of the iDMAs in the fifo to prevent exceeding end of 4.
char  odma_fifo_count;
// 4 is probably too big for output but we do not care
#define DMA_OUT_COUNT (4)
#define DMA_OUT_LEN (1)
// start dma at 8, but we give ourselves 1 bank wrap around of buffer (wasted space)
// the DMA banks should never be operating on the same memory (at time of writing)
#define DMA_OUT_OFFSET (8 + 32)
#define DMA_OUT_START(x) (DMA_OUT_OFFSET+(DMA_OUT_LEN*(x)))
#define DMA_OUT_NEXT_INDEX() ((dma_out_last+1) % DMA_OUT_COUNT)

register volatile unsigned int x3 asm("x3");
register volatile unsigned int x4 asm("x4");

#define GARBAGE_ROW       (garbage_row)
#define SCRATCH_DMA       (scratch_dma_ptr)
#define DST_ROW           (VMEM_ROW_ADDRESS(mover_output))

unsigned int garbage_row;

// output of the mapper needs 16 * 1024 words of memory in a row
// currently vmalloc is not great for large blocks of memory, static allocaiton
// will work for now

// malloc this to the start of the input buffer
unsigned int dma_in_dma_ptr;
// malloc this to the start of the ouput buffer
unsigned int dma_out_dma_ptr;

// in words
#define DMA_IN_SIZE (16)

// 64 is maximum here unless more memory is vmalloc'd
#define DMA_IN_CHUNKS (64)

// setting this to 5 means buffer can hold 4
#define DMA_SCHEDULE_IN_SIZE (4+1)
circular_buf_t dma_schedule_in;
unsigned int dma_schedule_in_storage[DMA_SCHEDULE_IN_SIZE];

// define DMA_IN_CHUNKS to be the size of the input buffer 
#define DMA_IN_CIRBUF_SIZE (DMA_IN_CHUNKS+1)
circular_buf_t dma_in_buffer;
unsigned int dma_in_buffer_storage[DMA_IN_CIRBUF_SIZE];

// setting this to 5 means buffer can hold 4
#define DMA_SCHEDULE_OUT_SIZE (4+1)
circular_buf_t dma_schedule_out;
unsigned int dma_schedule_out_storage[DMA_SCHEDULE_OUT_SIZE];

unsigned int dma_trig_next = 0;

unsigned int dma_out_next = 0;

// void mask_callback(unsigned int data) {
// 	global=data;
// 	fsm_state=DMA_OUT_STATE; // change the direction of the FSM
// }
// converts a dma index (used in the cirbufs) to a dma_ptr
// the dma index counts each block of memory
unsigned int dma_idx_to_ptr(unsigned int idx) {
  return dma_in_dma_ptr + (idx * DATA_LEN);
}

unsigned int dma_idx_plus_start_to_ptr(unsigned int idx, unsigned start) {
  return start + (idx * DATA_LEN);
}

void trig_dma_in_next() {
  dma_in_set(dma_idx_to_ptr(dma_trig_next),DATA_LEN);

  circular_buf_put(&dma_schedule_in, dma_trig_next);

  dma_trig_next = (dma_trig_next+1) % DMA_IN_CHUNKS;

}

void trig_dma_out_next() {
  dma_out_set(dma_idx_plus_start_to_ptr(dma_out_next,dma_out_dma_ptr),DATA_LEN);

  circular_buf_put(&dma_schedule_in, dma_out_next);

  dma_out_next = (dma_out_next+1) % DMA_IN_CHUNKS;

}

void setup_dma_in(void) {
  dma_in_dma_ptr = VMEM_DMA_ADDRESS(vmalloc_single(&mgr)); //0x3fff0000

  circular_buf_initialize(&dma_schedule_in, dma_schedule_in_storage, DMA_SCHEDULE_IN_SIZE);
  circular_buf_initialize(&dma_in_buffer, dma_in_buffer_storage, DMA_IN_CIRBUF_SIZE);


  trig_dma_in_next();
  trig_dma_in_next();
  trig_dma_in_next();
  trig_dma_in_next();

  // how many chunks we get from a single vmalloc
  // unsigned int chunks = (VMALLOC_CHUNK_SIZE/4) / dma_in_size;

  // ring_block_send_eth(chunks);

  // trig_dma_in(0, 0xffffffff);
  // trig_dma_in(1, 0xffffffff);
}


void pet_dma_inqueue() {
  int error;
  unsigned int just_finished_idx;
  unsigned int dma_occupancy;

  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, dma_occupancy);
  unsigned int filled = circular_buf_occupancy(&dma_schedule_in);

  if( dma_occupancy != filled ) {
    // just_finished_idx is the index of the dma that just finished
    error = circular_buf_get(&dma_schedule_in, &just_finished_idx);

    // now that dma is done with this chunk, we add it to the next
    // circular buffer which signals the program that there is fresh data to be processed
    circular_buf_put(&dma_in_buffer, just_finished_idx);
    // ring_block_send_eth(dma_occupancy);
    // ring_block_send_eth(filled);

    // fake_work_todo += 10;

    // ring_block_send_eth(data);
    trig_dma_in_next();

  }

}

void trig_dma_out(unsigned int dma_ptr) {
	dma_out_set(dma_ptr, DATA_LEN);
}

void pet_dma_out() {
  int error;
  unsigned int data = 0;
  unsigned int dma_occupancy;

  CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, dma_occupancy);
  unsigned int incomming_occupancy = circular_buf_occupancy(&dma_in_buffer);

  // if input gave us something to work with
  x3 = incomming_occupancy;
  while (( incomming_occupancy > 1)) {

		error = circular_buf_get(&dma_in_buffer, &data); //MY_ASSERT(error == 0);

		// data is the index of memory which has completed fresh data ready to be processed



		// put this index onto the output dma tracker, that lets us know later
		// when dma finishes, which one finished
		circular_buf_put(&dma_schedule_out, data);
		trig_dma_out(dma_idx_to_ptr(data)+offset);

		CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, dma_occupancy);
		incomming_occupancy = circular_buf_occupancy(&dma_in_buffer);
  }
}

void setup_dma_out(void) {
  dma_out_dma_ptr = VMEM_DMA_ADDRESS(vmalloc_single(&mgr)); 

  circular_buf_initialize(&dma_schedule_out, dma_schedule_out_storage, DMA_SCHEDULE_OUT_SIZE);
}


void mask_callback(unsigned int data) {
	int error;
}

int main(void)
{
	Ringbus ringbus;

	init_VMalloc(&mgr);

	ring_register_callback(&mask_callback, OFFSET_CMD);

	setup_dma_in();  // initialize in schedule buffer, input buffer, input set up 4 input DMAs
	setup_dma_out(); // initialize schedule buffer
	x4 = vmalloc_single(&mgr);
	x4 = 0xdead;
	x4 = vmalloc_single(&mgr);
	// x4 = dma_in_dma_ptr;

	// x4 = dma_out_dma_ptr;
	while(1) {
		pet_dma_inqueue(); // get input dma occupancy, find out how many schedules are in schedule buffer, if occupancy is not 
		                   // equal to fill, add a new scheduel to the schedule buffer, send schedule to dma in fifo
		pet_dma_out(); // get output dma occupancy, find out how many DMAs arrived, if input buffer greater than 1 or 0 and out 
					   // dma occupancy less than dma fifo depth get location of the input buffer pointer, send out dma schedule 
					   // to dma fifo, add incremement output pointer, do again until schedule fifo is full if available input buffer
		check_ring(&ringbus);
		global = 0;
	}

}