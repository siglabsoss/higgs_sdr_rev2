#define STREAM_CHUNK (256)
#define BLINK_WHILE_FORWARD_RATE (1000)

#include "do_forward_stream.h"
#include "self_sync.h"

int main2(void);
int main(void)
{
    self_sync_block_boot();
    main2();
    return 0;
}
int main2(void) {
    no_exit_stream();
    return 0;
}