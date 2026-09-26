
// Total Subcarrier #: 16








//   Bins:
// 00 (00): []
// 01 (02): [33, 49]
// 02 (00): []
// 03 (02): [19, 35]
// 04 (00): []
// 05 (02): [21, 37]
// 06 (00): []
// 07 (02): [23, 39]
// 08 (00): []
// 09 (02): [25, 41]
// 10 (00): []
// 11 (02): [27, 43]
// 12 (00): []
// 13 (02): [29, 45]
// 14 (00): []
// 15 (02): [31, 47]
// Longest bin: 2

// Schedule usage
// 00: 50.00 %
// 01: 50.00 %











//////////////////////////////////////////////////////////////////////////////////////////////////
//
//     Forward  Schedule
//
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Total Subcarriers: [19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49]
// input_stride = 1
// output_stride = 64
//
// global constants:
enabled_subcarriers = 16;
number_active_schedules = 2;

// Forward for chunk(0): [19, 21, 23, 25, 27, 29, 31, 33]
//                       [33, 19, 21, 23, 25, 27, 29, 31]
vmem_schedules[0] = (VmemSchedule) {
{ MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0 },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
{ (0 << 12) | (GARBAGE_ROW), (0x6 << 12) | (DST_ROW + 2), (0 << 12) | (GARBAGE_ROW), (0xd << 12) | (DST_ROW + 1), (0 << 12) | (GARBAGE_ROW), (0xc << 12) | (DST_ROW + 1), (0 << 12) | (GARBAGE_ROW), (0xb << 12) | (DST_ROW + 1), (0 << 12) | (GARBAGE_ROW), (0xa << 12) | (DST_ROW + 1), (0 << 12) | (GARBAGE_ROW), (0x9 << 12) | (DST_ROW + 1), (0 << 12) | (GARBAGE_ROW), (0x8 << 12) | (DST_ROW + 1), (0 << 12) | (GARBAGE_ROW), (0x7 << 12) | (DST_ROW + 1) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};
// Forward for chunk(1): [35, 37, 39, 41, 43, 45, 47, 49]
//                       [49, 35, 37, 39, 41, 43, 45, 47]
vmem_schedules[1] = (VmemSchedule) {
{ MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0, MOVER_SRC_ROW+0 },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
{ (0 << 12) | (GARBAGE_ROW), (0xe << 12) | (DST_ROW + 3), (0 << 12) | (GARBAGE_ROW), (0x5 << 12) | (DST_ROW + 2), (0 << 12) | (GARBAGE_ROW), (0x4 << 12) | (DST_ROW + 2), (0 << 12) | (GARBAGE_ROW), (0x3 << 12) | (DST_ROW + 2), (0 << 12) | (GARBAGE_ROW), (0x2 << 12) | (DST_ROW + 2), (0 << 12) | (GARBAGE_ROW), (0x1 << 12) | (DST_ROW + 2), (0 << 12) | (GARBAGE_ROW), (0x0 << 12) | (DST_ROW + 2), (0 << 12) | (GARBAGE_ROW), (0xf << 12) | (DST_ROW + 2) },
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }
};
