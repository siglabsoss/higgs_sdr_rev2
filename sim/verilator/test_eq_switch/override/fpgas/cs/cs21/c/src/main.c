#include "xbaseband.h"
#include "apb_bus.h"
#include "csr_control.h"
#include "pass_fail.h"
#include "dma.h"
#include "fill.h"
#include "mover.h"
#include "mapper.h"
#include "ringbus.h"
#include "circular_buffer.h"
#include "feedback_bus.h"



#define NORMAL_OPERATION

#ifdef NORMAL_OPERATION

// #define MODE_QPSK_128
// #define MODE_QAM_16_128
#define MODE_QAM_16_320


#define DMA_IN_EXTRA (16)

#define DMA_IN_EXTRA_USED (4)

// overwrite the 63th sc with index 62
// (63 is what s-modem uses for soft demod)
// comment this and main loop will not compile code to overwrite sc
#define OVERWRITE_CUSTOM_SC_INTO (62)


// #define DONT_SLICE_DATA

// #define FLUSH_AT_START


#include "ringbus2_pre.h"
#include "ringbus2_post.h"
#include "check_bootload.h"

#include "vmalloc.h"
// declare as global
// VMalloc mgr;

// #define OUTPUT_FRAME_COUNT_WORST_CASE (40)
#define FFT_SIZE (1024)


#define TRUE               (0x1)
#define FALSE              (0x0)

#define INIT_STATE         (0x0)
#define RX_STATE           (0x1)
#define MOVE_STATE         (0x3)
#define TX_STATE           (0x4)
#define WAITING_STATE      (0x5)
#define FINISH_STATE       (0x6)


unsigned int custom_subcarrier_index = 1;


unsigned int count_in = 0;
unsigned int count_out = 0;


unsigned int frame_num_all_output_period = 1024; //packet_num_SFO_adjustment_period
unsigned int frame_num_all_output_counter = 0;

// normal operation
// accept words, map to bpsk, move to subcarriers, output

// modes
// accepts words, map to "debug bpsk" (values of 0,1,2,3), move to subcarriers, output
// #define USE_FAKE_BPSK

// ignore input, map counter values to subcarriers, output
// when this is set, USE_FAKE_BPSK, has no effect
// #define USE_FAKE_MOVER_INPUT

// controlls which style of schedule is consumed
// simply enabling this is not enough, setup_mover() should also be edited
// old means dmem schedule and mover_schedule()
// new means vmem schedule and mover_load_vmem()
// #define USE_OLD_MOVER_SCHEDULE_FORMAT


// might only be valid when USE_OLD_MOVER_SCHEDULE_FORMAT is not enabled
// disabling this means we will wait for every dma output to complete before scheduling
// the next
// #define USE_DOUBLE_BUFFER


#define MY_ASSERT(x) if(!(x)) { ring_block_send_eth(0xe0000000|__LINE__);}


#define TEST_DATA_LENGTH 64

VMEM_SECTION feedback_frame_vector_filled_t vec_fine_sync;
VMEM_SECTION feedback_frame_vector_filled_t vec_demod_data;
VMEM_SECTION feedback_frame_vector_filled_t vec_eq_fb;
VMEM_SECTION feedback_frame_stream_filled_t stream_default;
VMEM_SECTION feedback_frame_stream_filled_t stream_all_sc;

#define ALL_ZERO_LENGTH (367*2)
VMEM_SECTION unsigned int counter_eq[512] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,256,257,258,259,260,261,262,263,264,265,266,267,268,269,270,271,272,273,274,275,276,277,278,279,280,281,282,283,284,285,286,287,288,289,290,291,292,293,294,295,296,297,298,299,300,301,302,303,304,305,306,307,308,309,310,311,312,313,314,315,316,317,318,319,320,321,322,323,324,325,326,327,328,329,330,331,332,333,334,335,336,337,338,339,340,341,342,343,344,345,346,347,348,349,350,351,352,353,354,355,356,357,358,359,360,361,362,363,364,365,366,367,368,369,370,371,372,373,374,375,376,377,378,379,380,381,382,383,384,385,386,387,388,389,390,391,392,393,394,395,396,397,398,399,400,401,402,403,404,405,406,407,408,409,410,411,412,413,414,415,416,417,418,419,420,421,422,423,424,425,426,427,428,429,430,431,432,433,434,435,436,437,438,439,440,441,442,443,444,445,446,447,448,449,450,451,452,453,454,455,456,457,458,459,460,461,462,463,464,465,466,467,468,469,470,471,472,473,474,475,476,477,478,479,480,481,482,483,484,485,486,487,488,489,490,491,492,493,494,495,496,497,498,499,500,501,502,503,504,505,506,507,508,509,510,511};
VMEM_SECTION unsigned int all_zeros[ALL_ZERO_LENGTH];
VMEM_SECTION unsigned int counter_data[1024] = {0x7fff8000, 0x7fff7fff, 0x80008000, 4026531843, 4026531844, 4026531845, 4026531846, 4026531847, 4026531848, 4026531849, 4026531850, 4026531851, 4026531852, 4026531853, 4026531854, 4026531855, 4026531856, 4026531857, 4026531858, 4026531859, 4026531860, 4026531861, 4026531862, 4026531863, 4026531864, 4026531865, 4026531866, 4026531867, 4026531868, 4026531869, 4026531870, 4026531871, 4026531872, 4026531873, 4026531874, 4026531875, 4026531876, 4026531877, 4026531878, 4026531879, 4026531880, 4026531881, 4026531882, 4026531883, 4026531884, 4026531885, 4026531886, 4026531887, 4026531888, 4026531889, 4026531890, 4026531891, 4026531892, 4026531893, 4026531894, 4026531895, 4026531896, 4026531897, 4026531898, 4026531899, 4026531900, 4026531901, 4026531902, 4026531903, 4026531904, 4026531905, 4026531906, 4026531907, 4026531908, 4026531909, 4026531910, 4026531911, 4026531912, 4026531913, 4026531914, 4026531915, 4026531916, 4026531917, 4026531918, 4026531919, 4026531920, 4026531921, 4026531922, 4026531923, 4026531924, 4026531925, 4026531926, 4026531927, 4026531928, 4026531929, 4026531930, 4026531931, 4026531932, 4026531933, 4026531934, 4026531935, 4026531936, 4026531937, 4026531938, 4026531939, 4026531940, 4026531941, 4026531942, 4026531943, 4026531944, 4026531945, 4026531946, 4026531947, 4026531948, 4026531949, 4026531950, 4026531951, 4026531952, 4026531953, 4026531954, 4026531955, 4026531956, 4026531957, 4026531958, 4026531959, 4026531960, 4026531961, 4026531962, 4026531963, 4026531964, 4026531965, 4026531966, 4026531967, 4026531968, 4026531969, 4026531970, 4026531971, 4026531972, 4026531973, 4026531974, 4026531975, 4026531976, 4026531977, 4026531978, 4026531979, 4026531980, 4026531981, 4026531982, 4026531983, 4026531984, 4026531985, 4026531986, 4026531987, 4026531988, 4026531989, 4026531990, 4026531991, 4026531992, 4026531993, 4026531994, 4026531995, 4026531996, 4026531997, 4026531998, 4026531999, 4026532000, 4026532001, 4026532002, 4026532003, 4026532004, 4026532005, 4026532006, 4026532007, 4026532008, 4026532009, 4026532010, 4026532011, 4026532012, 4026532013, 4026532014, 4026532015, 4026532016, 4026532017, 4026532018, 4026532019, 4026532020, 4026532021, 4026532022, 4026532023, 4026532024, 4026532025, 4026532026, 4026532027, 4026532028, 4026532029, 4026532030, 4026532031, 4026532032, 4026532033, 4026532034, 4026532035, 4026532036, 4026532037, 4026532038, 4026532039, 4026532040, 4026532041, 4026532042, 4026532043, 4026532044, 4026532045, 4026532046, 4026532047, 4026532048, 4026532049, 4026532050, 4026532051, 4026532052, 4026532053, 4026532054, 4026532055, 4026532056, 4026532057, 4026532058, 4026532059, 4026532060, 4026532061, 4026532062, 4026532063, 4026532064, 4026532065, 4026532066, 4026532067, 4026532068, 4026532069, 4026532070, 4026532071, 4026532072, 4026532073, 4026532074, 4026532075, 4026532076, 4026532077, 4026532078, 4026532079, 4026532080, 4026532081, 4026532082, 4026532083, 4026532084, 4026532085, 4026532086, 4026532087, 4026532088, 4026532089, 4026532090, 4026532091, 4026532092, 4026532093, 4026532094, 4026532095, 4026532096, 4026532097, 4026532098, 4026532099, 4026532100, 4026532101, 4026532102, 4026532103, 4026532104, 4026532105, 4026532106, 4026532107, 4026532108, 4026532109, 4026532110, 4026532111, 4026532112, 4026532113, 4026532114, 4026532115, 4026532116, 4026532117, 4026532118, 4026532119, 4026532120, 4026532121, 4026532122, 4026532123, 4026532124, 4026532125, 4026532126, 4026532127, 4026532128, 4026532129, 4026532130, 4026532131, 4026532132, 4026532133, 4026532134, 4026532135, 4026532136, 4026532137, 4026532138, 4026532139, 4026532140, 4026532141, 4026532142, 4026532143, 4026532144, 4026532145, 4026532146, 4026532147, 4026532148, 4026532149, 4026532150, 4026532151, 4026532152, 4026532153, 4026532154, 4026532155, 4026532156, 4026532157, 4026532158, 4026532159, 4026532160, 4026532161, 4026532162, 4026532163, 4026532164, 4026532165, 4026532166, 4026532167, 4026532168, 4026532169, 4026532170, 4026532171, 4026532172, 4026532173, 4026532174, 4026532175, 4026532176, 4026532177, 4026532178, 4026532179, 4026532180, 4026532181, 4026532182, 4026532183, 4026532184, 4026532185, 4026532186, 4026532187, 4026532188, 4026532189, 4026532190, 4026532191, 4026532192, 4026532193, 4026532194, 4026532195, 4026532196, 4026532197, 4026532198, 4026532199, 4026532200, 4026532201, 4026532202, 4026532203, 4026532204, 4026532205, 4026532206, 4026532207, 4026532208, 4026532209, 4026532210, 4026532211, 4026532212, 4026532213, 4026532214, 4026532215, 4026532216, 4026532217, 4026532218, 4026532219, 4026532220, 4026532221, 4026532222, 4026532223, 4026532224, 4026532225, 4026532226, 4026532227, 4026532228, 4026532229, 4026532230, 4026532231, 4026532232, 4026532233, 4026532234, 4026532235, 4026532236, 4026532237, 4026532238, 4026532239, 4026532240, 4026532241, 4026532242, 4026532243, 4026532244, 4026532245, 4026532246, 4026532247, 4026532248, 4026532249, 4026532250, 4026532251, 4026532252, 4026532253, 4026532254, 4026532255, 4026532256, 4026532257, 4026532258, 4026532259, 4026532260, 4026532261, 4026532262, 4026532263, 4026532264, 4026532265, 4026532266, 4026532267, 4026532268, 4026532269, 4026532270, 4026532271, 4026532272, 4026532273, 4026532274, 4026532275, 4026532276, 4026532277, 4026532278, 4026532279, 4026532280, 4026532281, 4026532282, 4026532283, 4026532284, 4026532285, 4026532286, 4026532287, 4026532288, 4026532289, 4026532290, 4026532291, 4026532292, 4026532293, 4026532294, 4026532295, 4026532296, 4026532297, 4026532298, 4026532299, 4026532300, 4026532301, 4026532302, 4026532303, 4026532304, 4026532305, 4026532306, 4026532307, 4026532308, 4026532309, 4026532310, 4026532311, 4026532312, 4026532313, 4026532314, 4026532315, 4026532316, 4026532317, 4026532318, 4026532319, 4026532320, 4026532321, 4026532322, 4026532323, 4026532324, 4026532325, 4026532326, 4026532327, 4026532328, 4026532329, 4026532330, 4026532331, 4026532332, 4026532333, 4026532334, 4026532335, 4026532336, 4026532337, 4026532338, 4026532339, 4026532340, 4026532341, 4026532342, 4026532343, 4026532344, 4026532345, 4026532346, 4026532347, 4026532348, 4026532349, 4026532350, 4026532351, 4026532352, 4026532353, 4026532354, 4026532355, 4026532356, 4026532357, 4026532358, 4026532359, 4026532360, 4026532361, 4026532362, 4026532363, 4026532364, 4026532365, 4026532366, 4026532367, 4026532368, 4026532369, 4026532370, 4026532371, 4026532372, 4026532373, 4026532374, 4026532375, 4026532376, 4026532377, 4026532378, 4026532379, 4026532380, 4026532381, 4026532382, 4026532383, 4026532384, 4026532385, 4026532386, 4026532387, 4026532388, 4026532389, 4026532390, 4026532391, 4026532392, 4026532393, 4026532394, 4026532395, 4026532396, 4026532397, 4026532398, 4026532399, 4026532400, 4026532401, 4026532402, 4026532403, 4026532404, 4026532405, 4026532406, 4026532407, 4026532408, 4026532409, 4026532410, 4026532411, 4026532412, 4026532413, 4026532414, 4026532415, 4026532416, 4026532417, 4026532418, 4026532419, 4026532420, 4026532421, 4026532422, 4026532423, 4026532424, 4026532425, 4026532426, 4026532427, 4026532428, 4026532429, 4026532430, 4026532431, 4026532432, 4026532433, 4026532434, 4026532435, 4026532436, 4026532437, 4026532438, 4026532439, 4026532440, 4026532441, 4026532442, 4026532443, 4026532444, 4026532445, 4026532446, 4026532447, 4026532448, 4026532449, 4026532450, 4026532451, 4026532452, 4026532453, 4026532454, 4026532455, 4026532456, 4026532457, 4026532458, 4026532459, 4026532460, 4026532461, 4026532462, 4026532463, 4026532464, 4026532465, 4026532466, 4026532467, 4026532468, 4026532469, 4026532470, 4026532471, 4026532472, 4026532473, 4026532474, 4026532475, 4026532476, 4026532477, 4026532478, 4026532479, 4026532480, 4026532481, 4026532482, 4026532483, 4026532484, 4026532485, 4026532486, 4026532487, 4026532488, 4026532489, 4026532490, 4026532491, 4026532492, 4026532493, 4026532494, 4026532495, 4026532496, 4026532497, 4026532498, 4026532499, 4026532500, 4026532501, 4026532502, 4026532503, 4026532504, 4026532505, 4026532506, 4026532507, 4026532508, 4026532509, 4026532510, 4026532511, 4026532512, 4026532513, 4026532514, 4026532515, 4026532516, 4026532517, 4026532518, 4026532519, 4026532520, 4026532521, 4026532522, 4026532523, 4026532524, 4026532525, 4026532526, 4026532527, 4026532528, 4026532529, 4026532530, 4026532531, 4026532532, 4026532533, 4026532534, 4026532535, 4026532536, 4026532537, 4026532538, 4026532539, 4026532540, 4026532541, 4026532542, 4026532543, 4026532544, 4026532545, 4026532546, 4026532547, 4026532548, 4026532549, 4026532550, 4026532551, 4026532552, 4026532553, 4026532554, 4026532555, 4026532556, 4026532557, 4026532558, 4026532559, 4026532560, 4026532561, 4026532562, 4026532563, 4026532564, 4026532565, 4026532566, 4026532567, 4026532568, 4026532569, 4026532570, 4026532571, 4026532572, 4026532573, 4026532574, 4026532575, 4026532576, 4026532577, 4026532578, 4026532579, 4026532580, 4026532581, 4026532582, 4026532583, 4026532584, 4026532585, 4026532586, 4026532587, 4026532588, 4026532589, 4026532590, 4026532591, 4026532592, 4026532593, 4026532594, 4026532595, 4026532596, 4026532597, 4026532598, 4026532599, 4026532600, 4026532601, 4026532602, 4026532603, 4026532604, 4026532605, 4026532606, 4026532607, 4026532608, 4026532609, 4026532610, 4026532611, 4026532612, 4026532613, 4026532614, 4026532615, 4026532616, 4026532617, 4026532618, 4026532619, 4026532620, 4026532621, 4026532622, 4026532623, 4026532624, 4026532625, 4026532626, 4026532627, 4026532628, 4026532629, 4026532630, 4026532631, 4026532632, 4026532633, 4026532634, 4026532635, 4026532636, 4026532637, 4026532638, 4026532639, 4026532640, 4026532641, 4026532642, 4026532643, 4026532644, 4026532645, 4026532646, 4026532647, 4026532648, 4026532649, 4026532650, 4026532651, 4026532652, 4026532653, 4026532654, 4026532655, 4026532656, 4026532657, 4026532658, 4026532659, 4026532660, 4026532661, 4026532662, 4026532663, 4026532664, 4026532665, 4026532666, 4026532667, 4026532668, 4026532669, 4026532670, 4026532671, 4026532672, 4026532673, 4026532674, 4026532675, 4026532676, 4026532677, 4026532678, 4026532679, 4026532680, 4026532681, 4026532682, 4026532683, 4026532684, 4026532685, 4026532686, 4026532687, 4026532688, 4026532689, 4026532690, 4026532691, 4026532692, 4026532693, 4026532694, 4026532695, 4026532696, 4026532697, 4026532698, 4026532699, 4026532700, 4026532701, 4026532702, 4026532703, 4026532704, 4026532705, 4026532706, 4026532707, 4026532708, 4026532709, 4026532710, 4026532711, 4026532712, 4026532713, 4026532714, 4026532715, 4026532716, 4026532717, 4026532718, 4026532719, 4026532720, 4026532721, 4026532722, 4026532723, 4026532724, 4026532725, 4026532726, 4026532727, 4026532728, 4026532729, 4026532730, 4026532731, 4026532732, 4026532733, 4026532734, 4026532735, 4026532736, 4026532737, 4026532738, 4026532739, 4026532740, 4026532741, 4026532742, 4026532743, 4026532744, 4026532745, 4026532746, 4026532747, 4026532748, 4026532749, 4026532750, 4026532751, 4026532752, 4026532753, 4026532754, 4026532755, 4026532756, 4026532757, 4026532758, 4026532759, 4026532760, 4026532761, 4026532762, 4026532763, 4026532764, 4026532765, 4026532766, 4026532767, 4026532768, 4026532769, 4026532770, 4026532771, 4026532772, 4026532773, 4026532774, 4026532775, 4026532776, 4026532777, 4026532778, 4026532779, 4026532780, 4026532781, 4026532782, 4026532783, 4026532784, 4026532785, 4026532786, 4026532787, 4026532788, 4026532789, 4026532790, 4026532791, 4026532792, 4026532793, 4026532794, 4026532795, 4026532796, 4026532797, 4026532798, 4026532799, 4026532800, 4026532801, 4026532802, 4026532803, 4026532804, 4026532805, 4026532806, 4026532807, 4026532808, 4026532809, 4026532810, 4026532811, 4026532812, 4026532813, 4026532814, 4026532815, 4026532816, 4026532817, 4026532818, 4026532819, 4026532820, 4026532821, 4026532822, 4026532823, 4026532824, 4026532825, 4026532826, 4026532827, 4026532828, 4026532829, 4026532830, 4026532831, 4026532832, 4026532833, 4026532834, 4026532835, 4026532836, 4026532837, 4026532838, 4026532839, 4026532840, 4026532841, 4026532842, 4026532843, 4026532844, 4026532845, 4026532846, 4026532847, 4026532848, 4026532849, 4026532850, 4026532851, 4026532852, 4026532853, 4026532854, 4026532855, 4026532856, 4026532857, 4026532858, 4026532859, 4026532860, 4026532861, 4026532862, 4026532863};

void setup_feedback_bus() {
    init_feedback_stream(&stream_default, FEEDBACK_PEER_SELF, false, true, FEEDBACK_STREAM_DEFAULT);
    init_feedback_stream(&stream_all_sc, FEEDBACK_PEER_SELF, false, true, FEEDBACK_STREAM_ALL_SC);

    init_feedback_vector(&vec_fine_sync, FEEDBACK_PEER_SELF, false, true, FEEDBACK_VEC_FINE_SYNC);
    init_feedback_vector(&vec_demod_data, FEEDBACK_PEER_SELF, false, true, FEEDBACK_VEC_DEMOD_DATA);
    init_feedback_vector(&vec_eq_fb, FEEDBACK_PEER_SELF, false, true, FEEDBACK_VEC_EQ_ANALOG);

    // NEVER directly set the length, the length field is not simply the length of data
    set_feedback_vector_length(&vec_fine_sync, DMA_IN_EXTRA_USED);
    set_feedback_vector_length(&vec_eq_fb, 512);

#ifdef DONT_SLICE_DATA
    set_feedback_vector_length(&vec_demod_data, 128);
#else
    set_feedback_vector_length(&vec_demod_data, 128/16); // 64
#endif

}

void set_stream_length(unsigned int len) {
    SET_REG(x3, 0xface);
    SET_REG(x3, len);
    set_feedback_stream_length(&stream_default, len);
    set_feedback_stream_length(&stream_all_sc, 1024); // length is all subcarriers
}


// register volatile unsigned int x3 asm("x3");
// register volatile unsigned int x4 asm("x4");


Table qpsk_table;
Table bpsk_table;

// #define MOVER_SRC_ROW          (mapper_output_row)
// #define MAPPER_DEST_ROW        (mapper_output_row)

#define DST_ROW_REV (VMEM_ROW_ADDRESS(dst_mem))
#define DST_ROW_REV2 (VMEM_ROW_ADDRESS(dst_mem_data))

unsigned int mapper_output_row;

unsigned int demod_mode = 0;

// unsigned int qpsk_table_dma;
// unsigned int bpsk_table_dma;


// void setup_mapper(void) {
//   // qpsk_table_dma = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));
// #ifdef USE_FAKE_BPSK
//   // qpsk_table = mapper_debug_qpsk_table(qpsk_table_dma);
// #else
//   // qpsk_table = mapper_qpsk_table(qpsk_table_dma);
// #endif
  
//   // bpsk_table_dma = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));
//   // bpsk_table = mapper_bpsk_table(bpsk_table_dma);

//   // output of mapper, input to mover
//   mapper_output_row = VMEM_ROW_ADDRESS(vmalloc_single(&mgr));
// }


// int mover_working_on;

// FIXME lame way of doing this
#define MAX_SCHEDULE_COUNT 44

#define GARBAGE_ROW       (garbage_row)

// in the reverse mover, DST_ROW is actually the source

#define SRC_ROW_REV           (VMEM_ROW_ADDRESS(input_dma))

unsigned int garbage_row;
unsigned int dma_in_full = 0;
unsigned int dma_out_done = 0;


#ifdef USE_OLD_MOVER_SCHEDULE_FORMAT
Schedule schedules[MAX_SCHEDULE_COUNT][NSLICES];
#else
// Directly create this in vmem (we could also load this at compile time with a compile time change to the value of DST_ROW)
VMEM_SECTION VmemSchedule vmem_schedules[MAX_SCHEDULE_COUNT];
#endif


// how many fft's to move at a time
// must be the same as DMA_IN_CHUNKS
#define FRAME_MOVE_CHUNK (1)

// the 2 is for ping/pong
VMEM_SECTION unsigned int input_dma[(FFT_SIZE+DMA_IN_EXTRA)*FRAME_MOVE_CHUNK*2] = {};

// worst case 1024 enabled subcarriers
VMEM_SECTION unsigned int dst_mem[FFT_SIZE*FRAME_MOVE_CHUNK*2] = {};
VMEM_SECTION unsigned int dst_mem_data[FFT_SIZE*FRAME_MOVE_CHUNK*2] = {};
VMEM_SECTION unsigned int dst_mem_copy[FFT_SIZE*FRAME_MOVE_CHUNK*2] = {};
VMEM_SECTION unsigned int garbage_mem[16] = {};

// the 2 is for ping/pong
VMEM_SECTION unsigned int empty_row[16] = {};
VMEM_SECTION unsigned int empty[1] = {0xcafebabe};
VMEM_SECTION unsigned int example_data[64] = {0x0, 0xa57ea57e, 0x0, 0xa57ea57e, 0x0, 0xa57e5a81, 0x0, 0xa57ea57e, 0x0, 0xa57e5a81, 0x0, 0xa57ea57e, 0x0, 0xa57ea57e, 0x0, 0xa57e5a81, 
                                            0x0, 0x5a81a57e, 0x0, 0xa57ea57e, 0x0, 0xa57e5a81, 0x0, 0xa57e5a81, 0x0, 0xa57e5a81, 0x0, 0xa57ea57e, 0x0, 0x5a81a57e, 0x0, 0xa57ea57e, 
                                            0x0, 0xa57e5a81, 0x0, 0xa57ea57e, 0x0, 0xa57ea57e, 0x0, 0xa57e5a81, 0x0, 0xa57e5a81, 0x0, 0xa57e5a81, 0x0, 0xa57ea57e, 0x0, 0xa57e5a81, 
                                            0x0, 0xa57e5a81, 0x0, 0xa57ea57e, 0x0, 0xa57ea57e, 0x0, 0xa57ea57e, 0x0, 0xa57e5a81, 0x0, 0xa57e5a81, 0x0, 0x5a815a81, 0x0, 0xa57ea57e};
unsigned int enabled_subcarriers; // delcared here but SET BY OUTPUT FROM schedule_maker.py
unsigned int number_active_schedules; // same as previous
unsigned int enabled_subcarriers_data; // delcared here but SET BY OUTPUT FROM schedule_maker.py
unsigned int number_active_schedules_data; // same as previous

unsigned int dma_in_dma_ptr;

void setup_mover(void) {
  garbage_row = VMEM_ROW_ADDRESS(garbage_mem);

  dma_in_dma_ptr = VMEM_DMA_ADDRESS(input_dma);










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

#ifdef MODE_QAM_16_128
#include "slicer_mover_128.h"
#endif

#ifdef MODE_QAM_16_320
#include "slicer_mover_320.h"
#endif

}

unsigned int input_frame_count;

#ifdef USE_DOUBLE_BUFFER
unsigned int mover_output_increment_words;
unsigned int mover_output_increment_row;
#endif
// call after setup_mover
void setup_mover_post() {
  input_frame_count = FRAME_MOVE_CHUNK;

#ifdef USE_DOUBLE_BUFFER
  // bumps for our "a" / "b" buffers
  mover_output_increment_words = input_frame_count << 10; // times 1024
  mover_output_increment_row = mover_output_increment_words >> 4; // over 16
#endif
}




// must be the same as FRAME_MOVE_CHUNK
#define DMA_IN_CHUNKS (1)

// in words
#define DMA_IN_SIZE ( (FFT_SIZE+DMA_IN_EXTRA)*DMA_IN_CHUNKS)

#define DMA_IN_CIRBUF_SIZE (DMA_IN_CHUNKS+1)
circular_buf_t dma_in_buffer;
unsigned int dma_in_buffer_storage[DMA_IN_CIRBUF_SIZE];

// setting this to 5 means buffer can hold 4
#define DMA_SCHEDULE_IN_SIZE (4+1)
circular_buf_t dma_schedule_in;
unsigned int dma_schedule_in_storage[DMA_SCHEDULE_IN_SIZE];

// #define DMA_SCHEDULE_OUT_SIZE (4+1)
// circular_buf_t dma_schedule_out;
// unsigned int dma_schedule_out_storage[DMA_SCHEDULE_OUT_SIZE];

unsigned int dma_trig_next = 0;

// converts a dma index (used in the cirbufs) to a dma_ptr
// the dma index counts each block of memory
unsigned int dma_idx_to_ptr(unsigned int idx) {
  return dma_in_dma_ptr + (idx * DMA_IN_SIZE);
}

// convert a dma_ptr to an index
unsigned int dma_ptr_to_idx(unsigned int ptr) {
  return (ptr - dma_in_dma_ptr) / DMA_IN_SIZE;
}

void trig_dma_in(unsigned int dma_ptr) {
  // ring_block_send_eth(dma_ptr);
  CSR_WRITE(DMA_0_START_ADDR, dma_ptr);
  CSR_WRITE(DMA_0_LENGTH, DMA_IN_SIZE);
  CSR_WRITE(DMA_0_TIMER_VAL, 0xffffffff); // start right away
  CSR_WRITE_ZERO(DMA_0_PUSH_SCHEDULE);
  count_in++;
}

void dma_block_send_sliced(
    unsigned int dma_ptr,
    unsigned int word_count,
    unsigned int slice_type,
    unsigned int qam_constellation) {
  unsigned int occupancy;
  while(1) {
    CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
    if( occupancy < DMA_1_SCHEDULE_DEPTH) {
      break;
    }
  }
  CSR_WRITE(DMA_1_START_ADDR, dma_ptr);
  CSR_WRITE(DMA_1_LENGTH, word_count);
  CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff);
  CSR_WRITE(SLICER, slice_type);
  CSR_WRITE(DEMAPPER_CONSTELLATION, qam_constellation);
  CSR_WRITE(DMA_1_LAST_RTL, 1);
  CSR_WRITE_ZERO(DMA_1_PUSH_SCHEDULE);
}
// void trig_dma_out(unsigned int dma_ptr) {
//   CSR_WRITE(DMA_1_START_ADDR, dma_ptr);
//   CSR_WRITE(DMA_1_LENGTH, DMA_IN_SIZE);
//   CSR_WRITE(DMA_1_TIMER_VAL, 0xffffffff); // start right away
//   CSR_WRITE_ZERO(DMA_1_PUSH_SCHEDULE);
// }


void trig_dma_in_next() {
    SET_REG(x3, 0xc0000000);

    trig_dma_in(dma_idx_to_ptr(dma_trig_next));

    circular_buf_put(&dma_schedule_in, dma_trig_next);

    dma_trig_next = (dma_trig_next+1) % DMA_IN_CHUNKS;
}
//////////////////////////////////////////
//
// We run 2 circular buffers2
// the first buffer keeps track of outstanding input dma so they are always overlapping
// as these they dump into the 2nd circular buffer which is the "pending data" and also our fill level

void setup_dma_in(void) {
  // dma_in_dma_ptr = VMEM_DMA_ADDRESS(vmalloc_single(&mgr));

  circular_buf_initialize(&dma_schedule_in, dma_schedule_in_storage, DMA_SCHEDULE_IN_SIZE);
  // circular_buf_initialize(&dma_in_buffer, dma_in_buffer_storage, DMA_IN_CIRBUF_SIZE);

  dma_in_full = 0;
  trig_dma_in_next();
}

// void setup_dma_out(void) {
//   circular_buf_initialize(&dma_schedule_out, dma_schedule_out_storage, DMA_SCHEDULE_OUT_SIZE);
// }

// unsigned int fake_work_todo = 0;

void recover_last() {
    ring_block_send_eth(DMA_LAST_ERROR_PCCMD | OUR_RING_ENUM);
    dma_run_till_last();
}

void pet_dma_inqueue() {
    int error;
    unsigned int just_finished_idx;
    unsigned int dma_occupancy;
    unsigned int status;
    // unsigned int dma_out_occupancy;

    CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, dma_occupancy);
    // CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, dma_out_occupancy);
    unsigned int filled = circular_buf_occupancy(&dma_schedule_in);

    unsigned int outgoing_buf_occupancy;

    SET_REG(x3, 0xe0000000 | count_in );
    SET_REG(x3, 0xf0000000 | count_out );

    if( dma_in_full == 0 ) {
        if(dma_occupancy != filled) {
            CSR_READ(DMA_0_STATUS, status);
            if( status ) {
                recover_last();
            }

            dma_in_full = 1;
            SET_REG(x3, 0xbb000000);

            return;
        }
    }

  if( dma_occupancy != filled && dma_out_done == 1) {
    // just_finished_idx is the index of the dma that just finished
    error = circular_buf_get(&dma_schedule_in, &just_finished_idx); MY_ASSERT(error == 0);

    // now that dma is done with this chunk, we add it to the next
    // circular buffer which signals the program that there is fresh data to be processed
    // circular_buf_put(&dma_in_buffer, just_finished_idx);
    // ring_block_send_eth(dma_occupancy);
    // ring_block_send_eth(filled);

    // outgoing_buf_occupancy = circular_buf_occupancy(&dma_in_buffer);

    SET_REG(x3, 0xb0000000 | 1);

    // fake_work_todo += 10;


    // ring_block_send_eth(data);
    dma_in_full = 0;
    dma_out_done = 0;
    trig_dma_in_next();
  }

}


void pet_dma_in() {
  pet_dma_inqueue();
}


// unsigned int debug_readout(unsigned int count) {
//   unsigned int dma_idx_just_finished;
//   unsigned int* dma_cpu_pointer;
//   int error;
//   for(unsigned i = 0; i < count; i++) {
//     error = circular_buf_get(&dma_in_buffer, &dma_idx_just_finished); MY_ASSERT(error == 0);

//     dma_cpu_pointer = REVERSE_VMEM_DMA_ADDRESS(dma_idx_to_ptr(dma_idx_just_finished));

//     for(unsigned int j = 0; j < 16; j++) {
//       ring_block_send_eth(dma_cpu_pointer[j]);
//     }


//   }
// }

unsigned int frame_track_counter;

void setup_frame_tracking() {
    frame_track_counter = 0;
}

// mover takes care of scheduling outputs
// because it runs so much faster we don't seem to have an output dma task
void pet_mover() {
  // SET_REG(x3, 0x2);

  unsigned int dma_idx_just_finished;
  unsigned int incomming_occupancy, outgoing_dma_occupancy;
  int error;
  int occupancy;

  // incomming_occupancy = circular_buf_occupancy(&dma_in_buffer);
  CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, outgoing_dma_occupancy);

  if(dma_in_full && outgoing_dma_occupancy == 0) {
    // error = circular_buf_get(&dma_in_buffer, &dma_idx_just_finished); MY_ASSERT(error == 0);

    dma_idx_just_finished = 0;

    SET_REG(x3, 0xa0000000 | dma_idx_just_finished);
    // SET_REG(x4, 0xcafe);
    // SET_REG(x4, dma_idx_just_finished);

    // index of first word
    unsigned int doff = (dma_idx_just_finished * (FFT_SIZE+DMA_IN_EXTRA) * FRAME_MOVE_CHUNK);

    // SET_REG(x4, doff);

    // SET_REG(x3, input_dma[0+doff]);
    // SET_REG(x3, input_dma[1+doff]);
    // SET_REG(x3, input_dma[2+doff]);
    // SET_REG(x3, input_dma[3+doff]);
    // SET_REG(x3, input_dma[1024+doff]);
    // SET_REG(x3, input_dma[1025+doff]);
    // SET_REG(x3, input_dma[1026+doff]);
    // SET_REG(x3, input_dma[1027+doff]);
    // SET_REG(x3, input_dma[1028+doff]);


    // PET full ofdm frame output
    // WE run this before the mover because
    //   1) mover does not modify input
    //   2) the output DMA will run while we are moving in parallel
    if( frame_num_all_output_counter == frame_num_all_output_period ) {
        // send header
        stream_all_sc.seq = frame_track_counter;
        dma_block_send(VMEM_DMA_ADDRESS(&stream_all_sc), FEEDBACK_HEADER_WORDS);

        bool use_real_data = true;
        if( use_real_data ) {
            dma_block_send(  VMEM_DMA_ADDRESS(input_dma)+doff, 1024); 
        } else {
            dma_block_send(  VMEM_DMA_ADDRESS(counter_data), 1024); 

        }

        // dma_block_send(  VMEM_DMA_ADDRESS(&(input_dma[0+doff]))  , 1024); 
        // is index in doff words from the beginning of dma_in_dma_ptr
        frame_num_all_output_counter = 0;
    } else {
        // do nothing
    }
    frame_num_all_output_counter++;


    if(frame_track_counter % 2 == 0) {
        vec_eq_fb.seq = frame_track_counter;
        dma_block_send(VMEM_DMA_ADDRESS(&vec_eq_fb), FEEDBACK_HEADER_WORDS);

        dma_block_send( VMEM_DMA_ADDRESS(counter_eq), 512);
    }
    ///////
    //
    //  Run Mover
    //

    // input is 1 fft's at once
    unsigned int mover_input_increment_row = (dma_idx_just_finished * (FFT_SIZE+DMA_IN_EXTRA) * FRAME_MOVE_CHUNK) / NSLICES;

    SET_REG(x4, mover_input_increment_row);

    for(unsigned int i = 0; i < number_active_schedules; i++) {
      // mover_load_offset_input( &(vmem_schedules[i]), mover_input_increment_row);
      // mover_roll(input_frame_count);
        mover_load_vmem_offset_input_single( &(vmem_schedules[i]), mover_input_increment_row);
      // SET_REG(x3, VMEM_ROW_ADDRESS(&(vmem_schedules[i])));
    }

    for(unsigned int i = 0; i < number_active_schedules_data; i++) {
      // mover_load_offset_input( &(vmem_schedules[i+4]), mover_input_increment_row);
      // mover_roll(input_frame_count);
        mover_load_vmem_offset_input_single( &(vmem_schedules[i+4]), mover_input_increment_row);
      // SET_REG(x3, VMEM_ROW_ADDRESS(&(vmem_schedules[i+4])));
      // SET_REG(x3, DST_ROW_REV);
    }

    // mover_copy(VMEM_ROW_ADDRESS(dst_mem_data),VMEM_ROW_ADDRESS(dst_mem_copy),8);
    // SET_REG(x3, VMEM_ROW_ADDRESS(dst_mem_copy));

    //
    ///////////

    ///////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    // unsigned int count1 = frame_track_counter % SCHEDULE_FRAMES;
    // unsigned int progress = count1 % SCHEDULE_LENGTH;

    // if(progress == 0) {
        
    // }
    ///////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

#ifdef OVERWRITE_CUSTOM_SC_INTO
    // unsigned int stream_dma_addr = DST_ROW_REV*NSLICES;
    unsigned int* stream_cpu_addr = (unsigned int*) REVERSE_VMEM_ROW_ADDRESS( DST_ROW_REV );
    unsigned int* original_cpu_addr = (unsigned int*) REVERSE_VMEM_ROW_ADDRESS( SRC_ROW_REV );

    stream_cpu_addr[OVERWRITE_CUSTOM_SC_INTO] = original_cpu_addr[custom_subcarrier_index];
#endif



    // output the header
    // since the length does not change, we can just send the same header every time
    // send the header first and then the data
    stream_default.seq = frame_track_counter;
    dma_block_send_finalized(VMEM_DMA_ADDRESS(&stream_default), FEEDBACK_HEADER_WORDS, 1);
    dma_block_send_finalized(DST_ROW_REV*NSLICES, enabled_subcarriers*FRAME_MOVE_CHUNK, 1);

    // now we need to sent the header for fine sync
    // this comes from the pre-moved buffer (could this also come before mover for efficieny?)
    vec_fine_sync.seq = frame_track_counter;
    dma_block_send_finalized(VMEM_DMA_ADDRESS(&vec_fine_sync), FEEDBACK_HEADER_WORDS, 1);
    // dma_out_set(  VMEM_DMA_ADDRESS(&(input_dma[doff]))  , DMA_IN_EXTRA_USED);
    dma_block_send_finalized(  VMEM_DMA_ADDRESS(input_dma) + doff+FFT_SIZE, DMA_IN_EXTRA_USED, 1);

    vec_demod_data.seq = frame_track_counter;
    if( demod_mode == 0 ) {
        set_feedback_vector_length(&vec_demod_data, 128/16);
    } else {
        // FIXME: only supports QAM16 currently
        unsigned int demod_body_length;
        #ifdef MODE_QAM_16_128
        demod_body_length = 16;
        #endif

        #ifdef MODE_QAM_16_320
        demod_body_length = 40;
        #endif


        set_feedback_vector_length(&vec_demod_data, demod_body_length);
    }
    dma_block_send_finalized(VMEM_DMA_ADDRESS(&vec_demod_data), FEEDBACK_HEADER_WORDS, 1);
    
#ifdef DONT_SLICE_DATA

    dma_block_send(DST_ROW_REV2*NSLICES, 128);

#else

    // wait until all dma out transations are finished
    while(1) {
      CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
      if( occupancy == 0) {
        break;
      }
    }

    if( demod_mode != 0 ) {
        SET_REG(x3, 0x0ffff);
    }

    unsigned int slice_input_length;
    #ifdef MODE_QAM_16_128
    slice_input_length = 128;
    #endif

    #ifdef MODE_QAM_16_320
    slice_input_length = 320;
    #endif


    // call this function
    dma_block_send_sliced(DST_ROW_REV2*NSLICES, slice_input_length, 1, demod_mode);

    if( demod_mode != 0 ) {
        SET_REG(x3, 0x1ffff);
    }

    // wait till it is also finished
    while(1) {
      CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
      if( occupancy == 0) {
        break;
      }
    }

    if( demod_mode != 0 ) {
        SET_REG(x3, 0x2ffff);
    }

    // back to normal for operations after this
    CSR_WRITE(DEMAPPER_CONSTELLATION, FEEDBACK_MAPMOV_QPSK);

    if( demod_mode != 0 ) {
        SET_REG(x3, 0x3ffff);
    }


#endif
    // for (int i = 0; i < TEST_DATA_LENGTH/2; ++i)
    // {
    //     dma_block_send_sliced(1+DST_ROW_REV*NSLICES+2*i, 1, 1);
    // }
    
    // Testing
    // for (int i = 0; i < TEST_DATA_LENGTH/2; ++i)
    // {
    //     dma_block_send_sliced(1+VMEM_DMA_ADDRESS(example_data)+2*i, 1, 1);
    // }

    // dma_block_send_sliced(DST_ROW_REV*NSLICES, TEST_DATA_LENGTH, 1);
    //Testing
    // dma_block_send_sliced(VMEM_DMA_ADDRESS(example_data), TEST_DATA_LENGTH, 1);

    unsigned int occupancy2;
    while(1) {
        CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy2);
        if( occupancy2 == 0) {
            break;
        }
    }

    count_out++;

    // dma_in_full = 1;
    dma_out_done = 1;



    // BUMP frame track counter
    frame_track_counter++;
  }


  CSR_WRITE(GPIO_WRITE, (0x200000) | 2);
    
}

void feedback_bus_callback(unsigned int data) {
    if( data == 0 ) {
        dma_block_send(VMEM_DMA_ADDRESS(&all_zeros), ALL_ZERO_LENGTH);
    }
}

// callback from ringbus
void advance_lifetime_counter_callback(unsigned int data) {
    unsigned int dmode = (data >> 16) & 0xff;
    unsigned int lower = (data) & 0xffff;
    switch(dmode) {
        case 0:
            break;
        case 1:
            frame_track_counter += lower;
            break;
        case 2:
            frame_track_counter -= lower;
            break;
    }

    ring_block_send_eth(RX_CS21_DID_ADJUST | data);

    // turnstile_advance(data);
}

void update_demod_mode_callback(unsigned int data) {
    demod_mode = data;
}

void update_custom_subcarrier_index_callback(unsigned int data) {
    if( data >= 1024 ) {
        ring_block_send_eth(APP_ASSERT_PCCMD | (OUR_RING_ENUM)<<24 | 1 );
        return;
    }

    custom_subcarrier_index = data;
}

int main(void)
{
  unsigned int rtn;
  Ringbus ringbus;

#ifdef FLUSH_AT_START

  CSR_WRITE(DMA_0_FLUSH_SCHEDULE, 0);
  CSR_WRITE(DMA_1_FLUSH_SCHEDULE, 0);
  CSR_WRITE(DMA_2_FLUSH_SCHEDULE, 0);

#endif

  // setup vmalloc
  // init_VMalloc(&mgr);

  // setup callbacks
  ring_register_callback(&feedback_bus_callback, FEEDBACK_BUS_CMD);
  ring_register_callback(&advance_lifetime_counter_callback, CS21_ADVANCE_LIFETIME);
  ring_register_callback(&update_demod_mode_callback, CS21_DEMOD_MODE);
  ring_register_callback(&check_bootload_status, CHECK_BOOTLOAD_CMD);
  ring_register_callback(&update_custom_subcarrier_index_callback, CS21_CHOOSE_CUSTOM_SC_CMD);

  // unsigned int* input_cpu_ptr =  vmalloc_single(&mgr);
  // unsigned int input_dma_ptr = VMEM_DMA_ADDRESS(input_cpu_ptr);

  // mover_working_on = 0;

  unsigned int a, b, c, d;

#ifdef FLUSH_AT_START
  CSR_WRITE(DMA_0_FLUSH_SCHEDULE,  0);
  CSR_WRITE(DMA_1_FLUSH_SCHEDULE,  0);
#endif


  demod_mode = FEEDBACK_MAPMOV_QAM16;


  // for(unsigned int i = 0; i < 1024; i++) {
  //   vector_memory[i+input_dma_ptr] = 0xf000d000 + i;
  // }

  // setup mapper
  // setup_mapper();

  setup_mover();
  setup_mover_post(); // must be called afer previous

  setup_dma_in();

  setup_feedback_bus();

  // default output stream length is set in setup_mover()
  set_stream_length(enabled_subcarriers*FRAME_MOVE_CHUNK);
  // setup_dma_out();
  // setup_fill_level();

  setup_frame_tracking();

  SET_REG(x3, 0xdeadbeef);
  SET_REG(x4, 0xdeadbeef);

  // ring_block_send_eth(0xdead); // boot

  // debug ringbus out row addresses of all memory for easy lookup into cs20.out (row+1 = linenumber)
  // ring_block_send_eth(input_dma_ptr);
  // ring_block_send_eth(scratch_dma_ptr);
  // ring_block_send_eth(debug_dma_ptr);
  // ring_block_send_eth(VMEM_ROW_ADDRESS(input_dma));
  // ring_block_send_eth(VMEM_ROW_ADDRESS(dst_mem));
  // ring_block_send_eth(MAPPER_DEST_ROW);
  // ring_block_send_eth(VMEM_ROW_ADDRESS(mover_output));
  // ring_block_send_eth(VMEM_ROW_ADDRESS(REVERSE_VMEM_DMA_ADDRESS(SCRATCH_DMA)));
  // ring_block_send_eth(DST_ROW);
  // ring_block_send_eth(DST_ROW+mover_output_increment_row);
  // ring_block_send_eth(VMEM_ROW_ADDRESS(&vs0));

  // ring_block_send_eth(VMEM_ROW_ADDRESS(input_cpu_ptr));
  // ring_block_send_eth(VMEM_ROW_ADDRESS(scrach_cpu_ptr));
  // ring_block_send_eth(GARBAGE_ROW);
  // ring_block_send_eth(mapper_output_row);
  // ring_block_send_eth(qpsk_table_dma/16);


  CSR_WRITE(GPIO_WRITE_EN, 0xffffffff);
  unsigned int fsm_state = INIT_STATE;
  unsigned int return_time;



  unsigned int incomming_occupancy;
  unsigned int outgoing_dma_occupancy;
  unsigned int dma_idx_just_finished;
  unsigned int needs_mapping_dma_ptr;
  int error;

  while(1) {
    pet_dma_in();

    pet_mover();

    check_ring(&ringbus);

    SET_REG(x3, 0x0);
  }

}

#else 

#define STREAM_CHUNK (64)

#include "do_forward_stream.h"

int main(void)
{
  no_exit_stream();
}




#endif
