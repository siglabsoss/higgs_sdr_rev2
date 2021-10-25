#include "fill.h"
#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"

#include "ringbus2_pre.h"
#define OUR_RING_ENUM RING_ENUM_CS21
#include "ringbus2_post.h"

#include "unit_test_ring.h"

int main(void)
{
  ring_unit_test_as(OUR_RING_ENUM);
}
