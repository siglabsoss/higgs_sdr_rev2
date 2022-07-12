

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//     Reverse  Schedule
//
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Total Subcarriers: [944, 945, 946, 947, 948, 949, 950, 951, 952, 953, 954, 955, 956, 957, 958, 959, 960, 961, 962, 963, 964, 965, 966, 967, 968, 969, 970, 971, 972, 973, 974, 975, 976, 977, 978, 979, 980, 981, 982, 983, 984, 985, 986, 987, 988, 989, 990, 991, 992, 993, 994, 995, 996, 997, 998, 999, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007]
// input_stride = 4
// output_stride = 64
//
// global constants:
enabled_subcarriers = 64;
number_active_schedules = 4;

// Reverse for chunk(0): [944, 945, 946, 947, 948, 949, 950, 951, 952, 953, 954, 955, 956, 957, 958, 959]
vmem_schedules[0] = (VmemSchedule) {
{ SRC_ROW_REV+59, SRC_ROW_REV+59, SRC_ROW_REV+59, SRC_ROW_REV+59, SRC_ROW_REV+59, SRC_ROW_REV+59, SRC_ROW_REV+59, SRC_ROW_REV+59, SRC_ROW_REV+59, SRC_ROW_REV+59, SRC_ROW_REV+59, SRC_ROW_REV+59, SRC_ROW_REV+59, SRC_ROW_REV+59, SRC_ROW_REV+59, SRC_ROW_REV+59 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0), (0x10 << 12) | (DST_ROW_REV + 0) },
{ 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4 }
};
// Reverse for chunk(1): [960, 961, 962, 963, 964, 965, 966, 967, 968, 969, 970, 971, 972, 973, 974, 975]
vmem_schedules[1] = (VmemSchedule) {
{ SRC_ROW_REV+60, SRC_ROW_REV+60, SRC_ROW_REV+60, SRC_ROW_REV+60, SRC_ROW_REV+60, SRC_ROW_REV+60, SRC_ROW_REV+60, SRC_ROW_REV+60, SRC_ROW_REV+60, SRC_ROW_REV+60, SRC_ROW_REV+60, SRC_ROW_REV+60, SRC_ROW_REV+60, SRC_ROW_REV+60, SRC_ROW_REV+60, SRC_ROW_REV+60 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1), (0x10 << 12) | (DST_ROW_REV + 1) },
{ 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4 }
};
// Reverse for chunk(2): [976, 977, 978, 979, 980, 981, 982, 983, 984, 985, 986, 987, 988, 989, 990, 991]
vmem_schedules[2] = (VmemSchedule) {
{ SRC_ROW_REV+61, SRC_ROW_REV+61, SRC_ROW_REV+61, SRC_ROW_REV+61, SRC_ROW_REV+61, SRC_ROW_REV+61, SRC_ROW_REV+61, SRC_ROW_REV+61, SRC_ROW_REV+61, SRC_ROW_REV+61, SRC_ROW_REV+61, SRC_ROW_REV+61, SRC_ROW_REV+61, SRC_ROW_REV+61, SRC_ROW_REV+61, SRC_ROW_REV+61 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (0x10 << 12) | (DST_ROW_REV + 2), (0x10 << 12) | (DST_ROW_REV + 2), (0x10 << 12) | (DST_ROW_REV + 2), (0x10 << 12) | (DST_ROW_REV + 2), (0x10 << 12) | (DST_ROW_REV + 2), (0x10 << 12) | (DST_ROW_REV + 2), (0x10 << 12) | (DST_ROW_REV + 2), (0x10 << 12) | (DST_ROW_REV + 2), (0x10 << 12) | (DST_ROW_REV + 2), (0x10 << 12) | (DST_ROW_REV + 2), (0x10 << 12) | (DST_ROW_REV + 2), (0x10 << 12) | (DST_ROW_REV + 2), (0x10 << 12) | (DST_ROW_REV + 2), (0x10 << 12) | (DST_ROW_REV + 2), (0x10 << 12) | (DST_ROW_REV + 2), (0x10 << 12) | (DST_ROW_REV + 2) },
{ 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4 }
};
// Reverse for chunk(3): [992, 993, 994, 995, 996, 997, 998, 999, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007]
vmem_schedules[3] = (VmemSchedule) {
{ SRC_ROW_REV+62, SRC_ROW_REV+62, SRC_ROW_REV+62, SRC_ROW_REV+62, SRC_ROW_REV+62, SRC_ROW_REV+62, SRC_ROW_REV+62, SRC_ROW_REV+62, SRC_ROW_REV+62, SRC_ROW_REV+62, SRC_ROW_REV+62, SRC_ROW_REV+62, SRC_ROW_REV+62, SRC_ROW_REV+62, SRC_ROW_REV+62, SRC_ROW_REV+62 },
{ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 },
{ (0x10 << 12) | (DST_ROW_REV + 3), (0x10 << 12) | (DST_ROW_REV + 3), (0x10 << 12) | (DST_ROW_REV + 3), (0x10 << 12) | (DST_ROW_REV + 3), (0x10 << 12) | (DST_ROW_REV + 3), (0x10 << 12) | (DST_ROW_REV + 3), (0x10 << 12) | (DST_ROW_REV + 3), (0x10 << 12) | (DST_ROW_REV + 3), (0x10 << 12) | (DST_ROW_REV + 3), (0x10 << 12) | (DST_ROW_REV + 3), (0x10 << 12) | (DST_ROW_REV + 3), (0x10 << 12) | (DST_ROW_REV + 3), (0x10 << 12) | (DST_ROW_REV + 3), (0x10 << 12) | (DST_ROW_REV + 3), (0x10 << 12) | (DST_ROW_REV + 3), (0x10 << 12) | (DST_ROW_REV + 3) },
{ 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4 }
};