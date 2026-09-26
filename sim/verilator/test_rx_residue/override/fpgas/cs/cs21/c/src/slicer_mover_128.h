//////////////////////////////////////////////////////////////////////////////////////////////////
//
//     Reverse  Schedule
//
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Total Subcarriers: [1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63, 65, 67, 69, 71, 73, 75, 77, 79, 81, 83, 85, 87, 89, 91, 93, 95, 97, 99, 101, 103, 105, 107, 109, 111, 113, 115, 117, 119, 121, 123, 125, 127, 897, 899, 901, 903, 905, 907, 909, 911, 913, 915, 917, 919, 921, 923, 925, 927, 929, 931, 933, 935, 937, 939, 941, 943, 945, 947, 949, 951, 953, 955, 957, 959, 961, 963, 965, 967, 969, 971, 973, 975, 977, 979, 981, 983, 985, 987, 989, 991, 993, 995, 997, 999, 1001, 1003, 1005, 1007, 1009, 1011, 1013, 1015, 1017, 1019, 1021, 1023]
// input_stride = 8
// output_stride = 64
//
// global constants:
enabled_subcarriers_data = 128;
number_active_schedules_data = 16;

// Reverse for chunk(0): [1, 3, 5, 7, 9, 11, 13, 15]
//                       [1, 3, 5, 7, 9, 11, 13, 15]
vmem_schedules[0+4] = (VmemSchedule) {
{ SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0, SRC_ROW_REV+0 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (0x1 << 12) | (DST_ROW_REV2 + 0), (0x2 << 12) | (DST_ROW_REV2 + 0), (0x3 << 12) | (DST_ROW_REV2 + 0), (0x4 << 12) | (DST_ROW_REV2 + 0), (0x5 << 12) | (DST_ROW_REV2 + 0), (0x6 << 12) | (DST_ROW_REV2 + 0), (0x7 << 12) | (DST_ROW_REV2 + 0), (0x8 << 12) | (DST_ROW_REV2 + 0), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW) },
{ 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8 }
};
// Reverse for chunk(1): [17, 19, 21, 23, 25, 27, 29, 31]
//                       [17, 19, 21, 23, 25, 27, 29, 31]
vmem_schedules[1+4] = (VmemSchedule) {
{ SRC_ROW_REV+0, SRC_ROW_REV+1, SRC_ROW_REV+0, SRC_ROW_REV+1, SRC_ROW_REV+0, SRC_ROW_REV+1, SRC_ROW_REV+0, SRC_ROW_REV+1, SRC_ROW_REV+0, SRC_ROW_REV+1, SRC_ROW_REV+0, SRC_ROW_REV+1, SRC_ROW_REV+0, SRC_ROW_REV+1, SRC_ROW_REV+0, SRC_ROW_REV+1 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (0x9 << 12) | (DST_ROW_REV2 + 0), (0xa << 12) | (DST_ROW_REV2 + 0), (0xb << 12) | (DST_ROW_REV2 + 0), (0xc << 12) | (DST_ROW_REV2 + 0), (0xd << 12) | (DST_ROW_REV2 + 0), (0xe << 12) | (DST_ROW_REV2 + 0), (0xf << 12) | (DST_ROW_REV2 + 0), (0x10 << 12) | (DST_ROW_REV2 + 0) },
{ 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8 }
};
// Reverse for chunk(2): [33, 35, 37, 39, 41, 43, 45, 47]
//                       [33, 35, 37, 39, 41, 43, 45, 47]
vmem_schedules[2+4] = (VmemSchedule) {
{ SRC_ROW_REV+0, SRC_ROW_REV+2, SRC_ROW_REV+0, SRC_ROW_REV+2, SRC_ROW_REV+0, SRC_ROW_REV+2, SRC_ROW_REV+0, SRC_ROW_REV+2, SRC_ROW_REV+0, SRC_ROW_REV+2, SRC_ROW_REV+0, SRC_ROW_REV+2, SRC_ROW_REV+0, SRC_ROW_REV+2, SRC_ROW_REV+0, SRC_ROW_REV+2 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (0x1 << 12) | (DST_ROW_REV2 + 1), (0x2 << 12) | (DST_ROW_REV2 + 1), (0x3 << 12) | (DST_ROW_REV2 + 1), (0x4 << 12) | (DST_ROW_REV2 + 1), (0x5 << 12) | (DST_ROW_REV2 + 1), (0x6 << 12) | (DST_ROW_REV2 + 1), (0x7 << 12) | (DST_ROW_REV2 + 1), (0x8 << 12) | (DST_ROW_REV2 + 1), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW) },
{ 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8 }
};
// Reverse for chunk(3): [49, 51, 53, 55, 57, 59, 61, 63]
//                       [49, 51, 53, 55, 57, 59, 61, 63]
vmem_schedules[3+4] = (VmemSchedule) {
{ SRC_ROW_REV+0, SRC_ROW_REV+3, SRC_ROW_REV+0, SRC_ROW_REV+3, SRC_ROW_REV+0, SRC_ROW_REV+3, SRC_ROW_REV+0, SRC_ROW_REV+3, SRC_ROW_REV+0, SRC_ROW_REV+3, SRC_ROW_REV+0, SRC_ROW_REV+3, SRC_ROW_REV+0, SRC_ROW_REV+3, SRC_ROW_REV+0, SRC_ROW_REV+3 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (0x9 << 12) | (DST_ROW_REV2 + 1), (0xa << 12) | (DST_ROW_REV2 + 1), (0xb << 12) | (DST_ROW_REV2 + 1), (0xc << 12) | (DST_ROW_REV2 + 1), (0xd << 12) | (DST_ROW_REV2 + 1), (0xe << 12) | (DST_ROW_REV2 + 1), (0xf << 12) | (DST_ROW_REV2 + 1), (0x10 << 12) | (DST_ROW_REV2 + 1) },
{ 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8 }
};
// Reverse for chunk(4): [65, 67, 69, 71, 73, 75, 77, 79]
//                       [65, 67, 69, 71, 73, 75, 77, 79]
vmem_schedules[4+4] = (VmemSchedule) {
{ SRC_ROW_REV+0, SRC_ROW_REV+4, SRC_ROW_REV+0, SRC_ROW_REV+4, SRC_ROW_REV+0, SRC_ROW_REV+4, SRC_ROW_REV+0, SRC_ROW_REV+4, SRC_ROW_REV+0, SRC_ROW_REV+4, SRC_ROW_REV+0, SRC_ROW_REV+4, SRC_ROW_REV+0, SRC_ROW_REV+4, SRC_ROW_REV+0, SRC_ROW_REV+4 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (0x1 << 12) | (DST_ROW_REV2 + 2), (0x2 << 12) | (DST_ROW_REV2 + 2), (0x3 << 12) | (DST_ROW_REV2 + 2), (0x4 << 12) | (DST_ROW_REV2 + 2), (0x5 << 12) | (DST_ROW_REV2 + 2), (0x6 << 12) | (DST_ROW_REV2 + 2), (0x7 << 12) | (DST_ROW_REV2 + 2), (0x8 << 12) | (DST_ROW_REV2 + 2), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW) },
{ 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8 }
};
// Reverse for chunk(5): [81, 83, 85, 87, 89, 91, 93, 95]
//                       [81, 83, 85, 87, 89, 91, 93, 95]
vmem_schedules[5+4] = (VmemSchedule) {
{ SRC_ROW_REV+0, SRC_ROW_REV+5, SRC_ROW_REV+0, SRC_ROW_REV+5, SRC_ROW_REV+0, SRC_ROW_REV+5, SRC_ROW_REV+0, SRC_ROW_REV+5, SRC_ROW_REV+0, SRC_ROW_REV+5, SRC_ROW_REV+0, SRC_ROW_REV+5, SRC_ROW_REV+0, SRC_ROW_REV+5, SRC_ROW_REV+0, SRC_ROW_REV+5 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (0x9 << 12) | (DST_ROW_REV2 + 2), (0xa << 12) | (DST_ROW_REV2 + 2), (0xb << 12) | (DST_ROW_REV2 + 2), (0xc << 12) | (DST_ROW_REV2 + 2), (0xd << 12) | (DST_ROW_REV2 + 2), (0xe << 12) | (DST_ROW_REV2 + 2), (0xf << 12) | (DST_ROW_REV2 + 2), (0x10 << 12) | (DST_ROW_REV2 + 2) },
{ 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8 }
};
// Reverse for chunk(6): [97, 99, 101, 103, 105, 107, 109, 111]
//                       [97, 99, 101, 103, 105, 107, 109, 111]
vmem_schedules[6+4] = (VmemSchedule) {
{ SRC_ROW_REV+0, SRC_ROW_REV+6, SRC_ROW_REV+0, SRC_ROW_REV+6, SRC_ROW_REV+0, SRC_ROW_REV+6, SRC_ROW_REV+0, SRC_ROW_REV+6, SRC_ROW_REV+0, SRC_ROW_REV+6, SRC_ROW_REV+0, SRC_ROW_REV+6, SRC_ROW_REV+0, SRC_ROW_REV+6, SRC_ROW_REV+0, SRC_ROW_REV+6 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (0x1 << 12) | (DST_ROW_REV2 + 3), (0x2 << 12) | (DST_ROW_REV2 + 3), (0x3 << 12) | (DST_ROW_REV2 + 3), (0x4 << 12) | (DST_ROW_REV2 + 3), (0x5 << 12) | (DST_ROW_REV2 + 3), (0x6 << 12) | (DST_ROW_REV2 + 3), (0x7 << 12) | (DST_ROW_REV2 + 3), (0x8 << 12) | (DST_ROW_REV2 + 3), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW) },
{ 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8 }
};
// Reverse for chunk(7): [113, 115, 117, 119, 121, 123, 125, 127]
//                       [113, 115, 117, 119, 121, 123, 125, 127]
vmem_schedules[7+4] = (VmemSchedule) {
{ SRC_ROW_REV+0, SRC_ROW_REV+7, SRC_ROW_REV+0, SRC_ROW_REV+7, SRC_ROW_REV+0, SRC_ROW_REV+7, SRC_ROW_REV+0, SRC_ROW_REV+7, SRC_ROW_REV+0, SRC_ROW_REV+7, SRC_ROW_REV+0, SRC_ROW_REV+7, SRC_ROW_REV+0, SRC_ROW_REV+7, SRC_ROW_REV+0, SRC_ROW_REV+7 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (0x9 << 12) | (DST_ROW_REV2 + 3), (0xa << 12) | (DST_ROW_REV2 + 3), (0xb << 12) | (DST_ROW_REV2 + 3), (0xc << 12) | (DST_ROW_REV2 + 3), (0xd << 12) | (DST_ROW_REV2 + 3), (0xe << 12) | (DST_ROW_REV2 + 3), (0xf << 12) | (DST_ROW_REV2 + 3), (0x10 << 12) | (DST_ROW_REV2 + 3) },
{ 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8 }
};
// Reverse for chunk(8): [897, 899, 901, 903, 905, 907, 909, 911]
//                       [897, 899, 901, 903, 905, 907, 909, 911]
vmem_schedules[8+4] = (VmemSchedule) {
{ SRC_ROW_REV+0, SRC_ROW_REV+56, SRC_ROW_REV+0, SRC_ROW_REV+56, SRC_ROW_REV+0, SRC_ROW_REV+56, SRC_ROW_REV+0, SRC_ROW_REV+56, SRC_ROW_REV+0, SRC_ROW_REV+56, SRC_ROW_REV+0, SRC_ROW_REV+56, SRC_ROW_REV+0, SRC_ROW_REV+56, SRC_ROW_REV+0, SRC_ROW_REV+56 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (0x1 << 12) | (DST_ROW_REV2 + 4), (0x2 << 12) | (DST_ROW_REV2 + 4), (0x3 << 12) | (DST_ROW_REV2 + 4), (0x4 << 12) | (DST_ROW_REV2 + 4), (0x5 << 12) | (DST_ROW_REV2 + 4), (0x6 << 12) | (DST_ROW_REV2 + 4), (0x7 << 12) | (DST_ROW_REV2 + 4), (0x8 << 12) | (DST_ROW_REV2 + 4), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW) },
{ 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8 }
};
// Reverse for chunk(9): [913, 915, 917, 919, 921, 923, 925, 927]
//                       [913, 915, 917, 919, 921, 923, 925, 927]
vmem_schedules[9+4] = (VmemSchedule) {
{ SRC_ROW_REV+0, SRC_ROW_REV+57, SRC_ROW_REV+0, SRC_ROW_REV+57, SRC_ROW_REV+0, SRC_ROW_REV+57, SRC_ROW_REV+0, SRC_ROW_REV+57, SRC_ROW_REV+0, SRC_ROW_REV+57, SRC_ROW_REV+0, SRC_ROW_REV+57, SRC_ROW_REV+0, SRC_ROW_REV+57, SRC_ROW_REV+0, SRC_ROW_REV+57 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (0x9 << 12) | (DST_ROW_REV2 + 4), (0xa << 12) | (DST_ROW_REV2 + 4), (0xb << 12) | (DST_ROW_REV2 + 4), (0xc << 12) | (DST_ROW_REV2 + 4), (0xd << 12) | (DST_ROW_REV2 + 4), (0xe << 12) | (DST_ROW_REV2 + 4), (0xf << 12) | (DST_ROW_REV2 + 4), (0x10 << 12) | (DST_ROW_REV2 + 4) },
{ 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8 }
};
// Reverse for chunk(10): [929, 931, 933, 935, 937, 939, 941, 943]
//                        [929, 931, 933, 935, 937, 939, 941, 943]
vmem_schedules[10+4] = (VmemSchedule) {
{ SRC_ROW_REV+0, SRC_ROW_REV+58, SRC_ROW_REV+0, SRC_ROW_REV+58, SRC_ROW_REV+0, SRC_ROW_REV+58, SRC_ROW_REV+0, SRC_ROW_REV+58, SRC_ROW_REV+0, SRC_ROW_REV+58, SRC_ROW_REV+0, SRC_ROW_REV+58, SRC_ROW_REV+0, SRC_ROW_REV+58, SRC_ROW_REV+0, SRC_ROW_REV+58 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (0x1 << 12) | (DST_ROW_REV2 + 5), (0x2 << 12) | (DST_ROW_REV2 + 5), (0x3 << 12) | (DST_ROW_REV2 + 5), (0x4 << 12) | (DST_ROW_REV2 + 5), (0x5 << 12) | (DST_ROW_REV2 + 5), (0x6 << 12) | (DST_ROW_REV2 + 5), (0x7 << 12) | (DST_ROW_REV2 + 5), (0x8 << 12) | (DST_ROW_REV2 + 5), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW) },
{ 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8 }
};
// Reverse for chunk(11): [945, 947, 949, 951, 953, 955, 957, 959]
//                        [945, 947, 949, 951, 953, 955, 957, 959]
vmem_schedules[11+4] = (VmemSchedule) {
{ SRC_ROW_REV+0, SRC_ROW_REV+59, SRC_ROW_REV+0, SRC_ROW_REV+59, SRC_ROW_REV+0, SRC_ROW_REV+59, SRC_ROW_REV+0, SRC_ROW_REV+59, SRC_ROW_REV+0, SRC_ROW_REV+59, SRC_ROW_REV+0, SRC_ROW_REV+59, SRC_ROW_REV+0, SRC_ROW_REV+59, SRC_ROW_REV+0, SRC_ROW_REV+59 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (0x9 << 12) | (DST_ROW_REV2 + 5), (0xa << 12) | (DST_ROW_REV2 + 5), (0xb << 12) | (DST_ROW_REV2 + 5), (0xc << 12) | (DST_ROW_REV2 + 5), (0xd << 12) | (DST_ROW_REV2 + 5), (0xe << 12) | (DST_ROW_REV2 + 5), (0xf << 12) | (DST_ROW_REV2 + 5), (0x10 << 12) | (DST_ROW_REV2 + 5) },
{ 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8 }
};
// Reverse for chunk(12): [961, 963, 965, 967, 969, 971, 973, 975]
//                        [961, 963, 965, 967, 969, 971, 973, 975]
vmem_schedules[12+4] = (VmemSchedule) {
{ SRC_ROW_REV+0, SRC_ROW_REV+60, SRC_ROW_REV+0, SRC_ROW_REV+60, SRC_ROW_REV+0, SRC_ROW_REV+60, SRC_ROW_REV+0, SRC_ROW_REV+60, SRC_ROW_REV+0, SRC_ROW_REV+60, SRC_ROW_REV+0, SRC_ROW_REV+60, SRC_ROW_REV+0, SRC_ROW_REV+60, SRC_ROW_REV+0, SRC_ROW_REV+60 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (0x1 << 12) | (DST_ROW_REV2 + 6), (0x2 << 12) | (DST_ROW_REV2 + 6), (0x3 << 12) | (DST_ROW_REV2 + 6), (0x4 << 12) | (DST_ROW_REV2 + 6), (0x5 << 12) | (DST_ROW_REV2 + 6), (0x6 << 12) | (DST_ROW_REV2 + 6), (0x7 << 12) | (DST_ROW_REV2 + 6), (0x8 << 12) | (DST_ROW_REV2 + 6), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW) },
{ 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8 }
};
// Reverse for chunk(13): [977, 979, 981, 983, 985, 987, 989, 991]
//                        [977, 979, 981, 983, 985, 987, 989, 991]
vmem_schedules[13+4] = (VmemSchedule) {
{ SRC_ROW_REV+0, SRC_ROW_REV+61, SRC_ROW_REV+0, SRC_ROW_REV+61, SRC_ROW_REV+0, SRC_ROW_REV+61, SRC_ROW_REV+0, SRC_ROW_REV+61, SRC_ROW_REV+0, SRC_ROW_REV+61, SRC_ROW_REV+0, SRC_ROW_REV+61, SRC_ROW_REV+0, SRC_ROW_REV+61, SRC_ROW_REV+0, SRC_ROW_REV+61 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (0x9 << 12) | (DST_ROW_REV2 + 6), (0xa << 12) | (DST_ROW_REV2 + 6), (0xb << 12) | (DST_ROW_REV2 + 6), (0xc << 12) | (DST_ROW_REV2 + 6), (0xd << 12) | (DST_ROW_REV2 + 6), (0xe << 12) | (DST_ROW_REV2 + 6), (0xf << 12) | (DST_ROW_REV2 + 6), (0x10 << 12) | (DST_ROW_REV2 + 6) },
{ 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8 }
};
// Reverse for chunk(14): [993, 995, 997, 999, 1001, 1003, 1005, 1007]
//                        [993, 995, 997, 999, 1001, 1003, 1005, 1007]
vmem_schedules[14+4] = (VmemSchedule) {
{ SRC_ROW_REV+0, SRC_ROW_REV+62, SRC_ROW_REV+0, SRC_ROW_REV+62, SRC_ROW_REV+0, SRC_ROW_REV+62, SRC_ROW_REV+0, SRC_ROW_REV+62, SRC_ROW_REV+0, SRC_ROW_REV+62, SRC_ROW_REV+0, SRC_ROW_REV+62, SRC_ROW_REV+0, SRC_ROW_REV+62, SRC_ROW_REV+0, SRC_ROW_REV+62 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (0x1 << 12) | (DST_ROW_REV2 + 7), (0x2 << 12) | (DST_ROW_REV2 + 7), (0x3 << 12) | (DST_ROW_REV2 + 7), (0x4 << 12) | (DST_ROW_REV2 + 7), (0x5 << 12) | (DST_ROW_REV2 + 7), (0x6 << 12) | (DST_ROW_REV2 + 7), (0x7 << 12) | (DST_ROW_REV2 + 7), (0x8 << 12) | (DST_ROW_REV2 + 7), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW) },
{ 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8 }
};
// Reverse for chunk(15): [1009, 1011, 1013, 1015, 1017, 1019, 1021, 1023]
//                        [1009, 1011, 1013, 1015, 1017, 1019, 1021, 1023]
vmem_schedules[15+4] = (VmemSchedule) {
{ SRC_ROW_REV+0, SRC_ROW_REV+63, SRC_ROW_REV+0, SRC_ROW_REV+63, SRC_ROW_REV+0, SRC_ROW_REV+63, SRC_ROW_REV+0, SRC_ROW_REV+63, SRC_ROW_REV+0, SRC_ROW_REV+63, SRC_ROW_REV+0, SRC_ROW_REV+63, SRC_ROW_REV+0, SRC_ROW_REV+63, SRC_ROW_REV+0, SRC_ROW_REV+63 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (GARBAGE_ROW), (0x9 << 12) | (DST_ROW_REV2 + 7), (0xa << 12) | (DST_ROW_REV2 + 7), (0xb << 12) | (DST_ROW_REV2 + 7), (0xc << 12) | (DST_ROW_REV2 + 7), (0xd << 12) | (DST_ROW_REV2 + 7), (0xe << 12) | (DST_ROW_REV2 + 7), (0xf << 12) | (DST_ROW_REV2 + 7), (0x10 << 12) | (DST_ROW_REV2 + 7) },
{ 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8 }
};

