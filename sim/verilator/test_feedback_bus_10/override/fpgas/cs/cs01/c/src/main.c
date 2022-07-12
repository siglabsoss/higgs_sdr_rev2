#include "dma.h"
#include "vmem.h"
#include "csr_control.h"
#include "vmalloc.h"
#include "circular_buffer_pow2.h"
#include "fill.h"
#include "ringbus.h"
#include "sig_utils.h"
#include "nco_data.h"

#include "flush_config_word_data.h"
#include "fft_1024_3914.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"

#include "config_word_cmul_eq_0f.h"
#include "config_word_conj_eq_0f.h"

#define MY_ASSERT(x) if(!(x)) { ring_block_send_eth(0xe0000000|__LINE__);}

register volatile unsigned int check_x4 asm("x4");
register volatile unsigned int check_x3 asm("x3");

void fft_accept_new(unsigned int dma_ptr);
void dma_out_set_safe(unsigned int dma_ptr, unsigned int size);

// declare as global
VMalloc mgr;

#define DMA_IN_CHUNK 1024

// #define FFT_CP_SAMPLES (384) // works (3/8)
// #define FFT_CP_SAMPLES (320) // works (5/16)
// #define FFT_CP_SAMPLES (288) // works (9/32)
#define FFT_CP_SAMPLES (256) // works (1/4)
// #define FFT_CP_SAMPLES (128) // does not work


// must be power of two, must change next as well
#define DMA_IN_COUNT (4)
#define DMA_IN_COUNT_MASK 0x3


#define DEBUG (0)

// Check status/control registers for underflow / overflow conditions
// #define CHECK_ERROR_COUNTERS


// #define DEBUG_PASSTHROUGH_VALUES

// disables normal output
// #define DEBUG_OUTPUT_COUNTER

#define DMA_OUT_CIRBUF_SIZE (4)

#define LIFETIME_COUNTER_MASK (0xffffff)

unsigned int test_location = 0;


unsigned int fsm_state;

int dma_state = 0;
int fft_state = 0;
int cfo_state = 0;
int buff_state = 0;

int dma_a_empty = 1;
int dma_b_empty = 1;
unsigned int dma_in_ptr[2];

int fft_a_empty = 1;
int fft_b_empty = 1;
unsigned int fft_ptr[2];

int cfo_a_empty = 1;
int cfo_b_empty = 1;
unsigned int cfo_ptr[2];

unsigned int rbus_theta_shift = 0;
unsigned int current_angle = 0;
unsigned int next_angle = 0;
unsigned int rbus_omega_sign = 0;
unsigned int rbus_omega = 0;
unsigned int delta_low  = 0;

unsigned int expected_0 = 0;
unsigned int expected_1 = 0;
int dma_out_ready = 0;

static unsigned int then;

unsigned int nco_length = 1024+256;

unsigned int cfg_cmulti_location;
unsigned int cfg_cmulco_location;

unsigned int dma_in_dma_ptr;

unsigned int dma_trig_next = 0;

circular_buf_pow2_t dma_out_started = CIRBUF_POW2_STATIC_CONSTRUCTOR(dma_out_started, DMA_OUT_CIRBUF_SIZE);
circular_buf_pow2_t dma_out_size = CIRBUF_POW2_STATIC_CONSTRUCTOR(dma_out_size, DMA_OUT_CIRBUF_SIZE);


VMEM_SECTION unsigned int counter_1280[] = {4026531840, 4026531841, 4026531842, 4026531843, 4026531844, 4026531845, 4026531846, 4026531847, 4026531848, 4026531849, 4026531850, 4026531851, 4026531852, 4026531853, 4026531854, 4026531855, 4026531856, 4026531857, 4026531858, 4026531859, 4026531860, 4026531861, 4026531862, 4026531863, 4026531864, 4026531865, 4026531866, 4026531867, 4026531868, 4026531869, 4026531870, 4026531871, 4026531872, 4026531873, 4026531874, 4026531875, 4026531876, 4026531877, 4026531878, 4026531879, 4026531880, 4026531881, 4026531882, 4026531883, 4026531884, 4026531885, 4026531886, 4026531887, 4026531888, 4026531889, 4026531890, 4026531891, 4026531892, 4026531893, 4026531894, 4026531895, 4026531896, 4026531897, 4026531898, 4026531899, 4026531900, 4026531901, 4026531902, 4026531903, 4026531904, 4026531905, 4026531906, 4026531907, 4026531908, 4026531909, 4026531910, 4026531911, 4026531912, 4026531913, 4026531914, 4026531915, 4026531916, 4026531917, 4026531918, 4026531919, 4026531920, 4026531921, 4026531922, 4026531923, 4026531924, 4026531925, 4026531926, 4026531927, 4026531928, 4026531929, 4026531930, 4026531931, 4026531932, 4026531933, 4026531934, 4026531935, 4026531936, 4026531937, 4026531938, 4026531939, 4026531940, 4026531941, 4026531942, 4026531943, 4026531944, 4026531945, 4026531946, 4026531947, 4026531948, 4026531949, 4026531950, 4026531951, 4026531952, 4026531953, 4026531954, 4026531955, 4026531956, 4026531957, 4026531958, 4026531959, 4026531960, 4026531961, 4026531962, 4026531963, 4026531964, 4026531965, 4026531966, 4026531967, 4026531968, 4026531969, 4026531970, 4026531971, 4026531972, 4026531973, 4026531974, 4026531975, 4026531976, 4026531977, 4026531978, 4026531979, 4026531980, 4026531981, 4026531982, 4026531983, 4026531984, 4026531985, 4026531986, 4026531987, 4026531988, 4026531989, 4026531990, 4026531991, 4026531992, 4026531993, 4026531994, 4026531995, 4026531996, 4026531997, 4026531998, 4026531999, 4026532000, 4026532001, 4026532002, 4026532003, 4026532004, 4026532005, 4026532006, 4026532007, 4026532008, 4026532009, 4026532010, 4026532011, 4026532012, 4026532013, 4026532014, 4026532015, 4026532016, 4026532017, 4026532018, 4026532019, 4026532020, 4026532021, 4026532022, 4026532023, 4026532024, 4026532025, 4026532026, 4026532027, 4026532028, 4026532029, 4026532030, 4026532031, 4026532032, 4026532033, 4026532034, 4026532035, 4026532036, 4026532037, 4026532038, 4026532039, 4026532040, 4026532041, 4026532042, 4026532043, 4026532044, 4026532045, 4026532046, 4026532047, 4026532048, 4026532049, 4026532050, 4026532051, 4026532052, 4026532053, 4026532054, 4026532055, 4026532056, 4026532057, 4026532058, 4026532059, 4026532060, 4026532061, 4026532062, 4026532063, 4026532064, 4026532065, 4026532066, 4026532067, 4026532068, 4026532069, 4026532070, 4026532071, 4026532072, 4026532073, 4026532074, 4026532075, 4026532076, 4026532077, 4026532078, 4026532079, 4026532080, 4026532081, 4026532082, 4026532083, 4026532084, 4026532085, 4026532086, 4026532087, 4026532088, 4026532089, 4026532090, 4026532091, 4026532092, 4026532093, 4026532094, 4026532095, 4026532096, 4026532097, 4026532098, 4026532099, 4026532100, 4026532101, 4026532102, 4026532103, 4026532104, 4026532105, 4026532106, 4026532107, 4026532108, 4026532109, 4026532110, 4026532111, 4026532112, 4026532113, 4026532114, 4026532115, 4026532116, 4026532117, 4026532118, 4026532119, 4026532120, 4026532121, 4026532122, 4026532123, 4026532124, 4026532125, 4026532126, 4026532127, 4026532128, 4026532129, 4026532130, 4026532131, 4026532132, 4026532133, 4026532134, 4026532135, 4026532136, 4026532137, 4026532138, 4026532139, 4026532140, 4026532141, 4026532142, 4026532143, 4026532144, 4026532145, 4026532146, 4026532147, 4026532148, 4026532149, 4026532150, 4026532151, 4026532152, 4026532153, 4026532154, 4026532155, 4026532156, 4026532157, 4026532158, 4026532159, 4026532160, 4026532161, 4026532162, 4026532163, 4026532164, 4026532165, 4026532166, 4026532167, 4026532168, 4026532169, 4026532170, 4026532171, 4026532172, 4026532173, 4026532174, 4026532175, 4026532176, 4026532177, 4026532178, 4026532179, 4026532180, 4026532181, 4026532182, 4026532183, 4026532184, 4026532185, 4026532186, 4026532187, 4026532188, 4026532189, 4026532190, 4026532191, 4026532192, 4026532193, 4026532194, 4026532195, 4026532196, 4026532197, 4026532198, 4026532199, 4026532200, 4026532201, 4026532202, 4026532203, 4026532204, 4026532205, 4026532206, 4026532207, 4026532208, 4026532209, 4026532210, 4026532211, 4026532212, 4026532213, 4026532214, 4026532215, 4026532216, 4026532217, 4026532218, 4026532219, 4026532220, 4026532221, 4026532222, 4026532223, 4026532224, 4026532225, 4026532226, 4026532227, 4026532228, 4026532229, 4026532230, 4026532231, 4026532232, 4026532233, 4026532234, 4026532235, 4026532236, 4026532237, 4026532238, 4026532239, 4026532240, 4026532241, 4026532242, 4026532243, 4026532244, 4026532245, 4026532246, 4026532247, 4026532248, 4026532249, 4026532250, 4026532251, 4026532252, 4026532253, 4026532254, 4026532255, 4026532256, 4026532257, 4026532258, 4026532259, 4026532260, 4026532261, 4026532262, 4026532263, 4026532264, 4026532265, 4026532266, 4026532267, 4026532268, 4026532269, 4026532270, 4026532271, 4026532272, 4026532273, 4026532274, 4026532275, 4026532276, 4026532277, 4026532278, 4026532279, 4026532280, 4026532281, 4026532282, 4026532283, 4026532284, 4026532285, 4026532286, 4026532287, 4026532288, 4026532289, 4026532290, 4026532291, 4026532292, 4026532293, 4026532294, 4026532295, 4026532296, 4026532297, 4026532298, 4026532299, 4026532300, 4026532301, 4026532302, 4026532303, 4026532304, 4026532305, 4026532306, 4026532307, 4026532308, 4026532309, 4026532310, 4026532311, 4026532312, 4026532313, 4026532314, 4026532315, 4026532316, 4026532317, 4026532318, 4026532319, 4026532320, 4026532321, 4026532322, 4026532323, 4026532324, 4026532325, 4026532326, 4026532327, 4026532328, 4026532329, 4026532330, 4026532331, 4026532332, 4026532333, 4026532334, 4026532335, 4026532336, 4026532337, 4026532338, 4026532339, 4026532340, 4026532341, 4026532342, 4026532343, 4026532344, 4026532345, 4026532346, 4026532347, 4026532348, 4026532349, 4026532350, 4026532351, 4026532352, 4026532353, 4026532354, 4026532355, 4026532356, 4026532357, 4026532358, 4026532359, 4026532360, 4026532361, 4026532362, 4026532363, 4026532364, 4026532365, 4026532366, 4026532367, 4026532368, 4026532369, 4026532370, 4026532371, 4026532372, 4026532373, 4026532374, 4026532375, 4026532376, 4026532377, 4026532378, 4026532379, 4026532380, 4026532381, 4026532382, 4026532383, 4026532384, 4026532385, 4026532386, 4026532387, 4026532388, 4026532389, 4026532390, 4026532391, 4026532392, 4026532393, 4026532394, 4026532395, 4026532396, 4026532397, 4026532398, 4026532399, 4026532400, 4026532401, 4026532402, 4026532403, 4026532404, 4026532405, 4026532406, 4026532407, 4026532408, 4026532409, 4026532410, 4026532411, 4026532412, 4026532413, 4026532414, 4026532415, 4026532416, 4026532417, 4026532418, 4026532419, 4026532420, 4026532421, 4026532422, 4026532423, 4026532424, 4026532425, 4026532426, 4026532427, 4026532428, 4026532429, 4026532430, 4026532431, 4026532432, 4026532433, 4026532434, 4026532435, 4026532436, 4026532437, 4026532438, 4026532439, 4026532440, 4026532441, 4026532442, 4026532443, 4026532444, 4026532445, 4026532446, 4026532447, 4026532448, 4026532449, 4026532450, 4026532451, 4026532452, 4026532453, 4026532454, 4026532455, 4026532456, 4026532457, 4026532458, 4026532459, 4026532460, 4026532461, 4026532462, 4026532463, 4026532464, 4026532465, 4026532466, 4026532467, 4026532468, 4026532469, 4026532470, 4026532471, 4026532472, 4026532473, 4026532474, 4026532475, 4026532476, 4026532477, 4026532478, 4026532479, 4026532480, 4026532481, 4026532482, 4026532483, 4026532484, 4026532485, 4026532486, 4026532487, 4026532488, 4026532489, 4026532490, 4026532491, 4026532492, 4026532493, 4026532494, 4026532495, 4026532496, 4026532497, 4026532498, 4026532499, 4026532500, 4026532501, 4026532502, 4026532503, 4026532504, 4026532505, 4026532506, 4026532507, 4026532508, 4026532509, 4026532510, 4026532511, 4026532512, 4026532513, 4026532514, 4026532515, 4026532516, 4026532517, 4026532518, 4026532519, 4026532520, 4026532521, 4026532522, 4026532523, 4026532524, 4026532525, 4026532526, 4026532527, 4026532528, 4026532529, 4026532530, 4026532531, 4026532532, 4026532533, 4026532534, 4026532535, 4026532536, 4026532537, 4026532538, 4026532539, 4026532540, 4026532541, 4026532542, 4026532543, 4026532544, 4026532545, 4026532546, 4026532547, 4026532548, 4026532549, 4026532550, 4026532551, 4026532552, 4026532553, 4026532554, 4026532555, 4026532556, 4026532557, 4026532558, 4026532559, 4026532560, 4026532561, 4026532562, 4026532563, 4026532564, 4026532565, 4026532566, 4026532567, 4026532568, 4026532569, 4026532570, 4026532571, 4026532572, 4026532573, 4026532574, 4026532575, 4026532576, 4026532577, 4026532578, 4026532579, 4026532580, 4026532581, 4026532582, 4026532583, 4026532584, 4026532585, 4026532586, 4026532587, 4026532588, 4026532589, 4026532590, 4026532591, 4026532592, 4026532593, 4026532594, 4026532595, 4026532596, 4026532597, 4026532598, 4026532599, 4026532600, 4026532601, 4026532602, 4026532603, 4026532604, 4026532605, 4026532606, 4026532607, 4026532608, 4026532609, 4026532610, 4026532611, 4026532612, 4026532613, 4026532614, 4026532615, 4026532616, 4026532617, 4026532618, 4026532619, 4026532620, 4026532621, 4026532622, 4026532623, 4026532624, 4026532625, 4026532626, 4026532627, 4026532628, 4026532629, 4026532630, 4026532631, 4026532632, 4026532633, 4026532634, 4026532635, 4026532636, 4026532637, 4026532638, 4026532639, 4026532640, 4026532641, 4026532642, 4026532643, 4026532644, 4026532645, 4026532646, 4026532647, 4026532648, 4026532649, 4026532650, 4026532651, 4026532652, 4026532653, 4026532654, 4026532655, 4026532656, 4026532657, 4026532658, 4026532659, 4026532660, 4026532661, 4026532662, 4026532663, 4026532664, 4026532665, 4026532666, 4026532667, 4026532668, 4026532669, 4026532670, 4026532671, 4026532672, 4026532673, 4026532674, 4026532675, 4026532676, 4026532677, 4026532678, 4026532679, 4026532680, 4026532681, 4026532682, 4026532683, 4026532684, 4026532685, 4026532686, 4026532687, 4026532688, 4026532689, 4026532690, 4026532691, 4026532692, 4026532693, 4026532694, 4026532695, 4026532696, 4026532697, 4026532698, 4026532699, 4026532700, 4026532701, 4026532702, 4026532703, 4026532704, 4026532705, 4026532706, 4026532707, 4026532708, 4026532709, 4026532710, 4026532711, 4026532712, 4026532713, 4026532714, 4026532715, 4026532716, 4026532717, 4026532718, 4026532719, 4026532720, 4026532721, 4026532722, 4026532723, 4026532724, 4026532725, 4026532726, 4026532727, 4026532728, 4026532729, 4026532730, 4026532731, 4026532732, 4026532733, 4026532734, 4026532735, 4026532736, 4026532737, 4026532738, 4026532739, 4026532740, 4026532741, 4026532742, 4026532743, 4026532744, 4026532745, 4026532746, 4026532747, 4026532748, 4026532749, 4026532750, 4026532751, 4026532752, 4026532753, 4026532754, 4026532755, 4026532756, 4026532757, 4026532758, 4026532759, 4026532760, 4026532761, 4026532762, 4026532763, 4026532764, 4026532765, 4026532766, 4026532767, 4026532768, 4026532769, 4026532770, 4026532771, 4026532772, 4026532773, 4026532774, 4026532775, 4026532776, 4026532777, 4026532778, 4026532779, 4026532780, 4026532781, 4026532782, 4026532783, 4026532784, 4026532785, 4026532786, 4026532787, 4026532788, 4026532789, 4026532790, 4026532791, 4026532792, 4026532793, 4026532794, 4026532795, 4026532796, 4026532797, 4026532798, 4026532799, 4026532800, 4026532801, 4026532802, 4026532803, 4026532804, 4026532805, 4026532806, 4026532807, 4026532808, 4026532809, 4026532810, 4026532811, 4026532812, 4026532813, 4026532814, 4026532815, 4026532816, 4026532817, 4026532818, 4026532819, 4026532820, 4026532821, 4026532822, 4026532823, 4026532824, 4026532825, 4026532826, 4026532827, 4026532828, 4026532829, 4026532830, 4026532831, 4026532832, 4026532833, 4026532834, 4026532835, 4026532836, 4026532837, 4026532838, 4026532839, 4026532840, 4026532841, 4026532842, 4026532843, 4026532844, 4026532845, 4026532846, 4026532847, 4026532848, 4026532849, 4026532850, 4026532851, 4026532852, 4026532853, 4026532854, 4026532855, 4026532856, 4026532857, 4026532858, 4026532859, 4026532860, 4026532861, 4026532862, 4026532863, 4026532864, 4026532865, 4026532866, 4026532867, 4026532868, 4026532869, 4026532870, 4026532871, 4026532872, 4026532873, 4026532874, 4026532875, 4026532876, 4026532877, 4026532878, 4026532879, 4026532880, 4026532881, 4026532882, 4026532883, 4026532884, 4026532885, 4026532886, 4026532887, 4026532888, 4026532889, 4026532890, 4026532891, 4026532892, 4026532893, 4026532894, 4026532895, 4026532896, 4026532897, 4026532898, 4026532899, 4026532900, 4026532901, 4026532902, 4026532903, 4026532904, 4026532905, 4026532906, 4026532907, 4026532908, 4026532909, 4026532910, 4026532911, 4026532912, 4026532913, 4026532914, 4026532915, 4026532916, 4026532917, 4026532918, 4026532919, 4026532920, 4026532921, 4026532922, 4026532923, 4026532924, 4026532925, 4026532926, 4026532927, 4026532928, 4026532929, 4026532930, 4026532931, 4026532932, 4026532933, 4026532934, 4026532935, 4026532936, 4026532937, 4026532938, 4026532939, 4026532940, 4026532941, 4026532942, 4026532943, 4026532944, 4026532945, 4026532946, 4026532947, 4026532948, 4026532949, 4026532950, 4026532951, 4026532952, 4026532953, 4026532954, 4026532955, 4026532956, 4026532957, 4026532958, 4026532959, 4026532960, 4026532961, 4026532962, 4026532963, 4026532964, 4026532965, 4026532966, 4026532967, 4026532968, 4026532969, 4026532970, 4026532971, 4026532972, 4026532973, 4026532974, 4026532975, 4026532976, 4026532977, 4026532978, 4026532979, 4026532980, 4026532981, 4026532982, 4026532983, 4026532984, 4026532985, 4026532986, 4026532987, 4026532988, 4026532989, 4026532990, 4026532991, 4026532992, 4026532993, 4026532994, 4026532995, 4026532996, 4026532997, 4026532998, 4026532999, 4026533000, 4026533001, 4026533002, 4026533003, 4026533004, 4026533005, 4026533006, 4026533007, 4026533008, 4026533009, 4026533010, 4026533011, 4026533012, 4026533013, 4026533014, 4026533015, 4026533016, 4026533017, 4026533018, 4026533019, 4026533020, 4026533021, 4026533022, 4026533023, 4026533024, 4026533025, 4026533026, 4026533027, 4026533028, 4026533029, 4026533030, 4026533031, 4026533032, 4026533033, 4026533034, 4026533035, 4026533036, 4026533037, 4026533038, 4026533039, 4026533040, 4026533041, 4026533042, 4026533043, 4026533044, 4026533045, 4026533046, 4026533047, 4026533048, 4026533049, 4026533050, 4026533051, 4026533052, 4026533053, 4026533054, 4026533055, 4026533056, 4026533057, 4026533058, 4026533059, 4026533060, 4026533061, 4026533062, 4026533063, 4026533064, 4026533065, 4026533066, 4026533067, 4026533068, 4026533069, 4026533070, 4026533071, 4026533072, 4026533073, 4026533074, 4026533075, 4026533076, 4026533077, 4026533078, 4026533079, 4026533080, 4026533081, 4026533082, 4026533083, 4026533084, 4026533085, 4026533086, 4026533087, 4026533088, 4026533089, 4026533090, 4026533091, 4026533092, 4026533093, 4026533094, 4026533095, 4026533096, 4026533097, 4026533098, 4026533099, 4026533100, 4026533101, 4026533102, 4026533103, 4026533104, 4026533105, 4026533106, 4026533107, 4026533108, 4026533109, 4026533110, 4026533111, 4026533112, 4026533113, 4026533114, 4026533115, 4026533116, 4026533117, 4026533118, 4026533119};


//////////////////
//
//  Copied from rx chain 00
//  sample adjustment
//


// how many ofdm symbols go by before we adjust
unsigned int packet_num_SFO_adjustment_period = 0; //71111;

unsigned int packet_counter_SFO_adjustment = 0; // how many ofdm symbols we have, seen since last adjustment

// determines the direction we adjust, and disables when zero
// 0 is no adjustment
// 1 delete
// 2 add
unsigned int packet_SFO_adjustment_direction = 0;

// how many samples to + / - each period
// ALWAYS positive, as sign is encoded into  packet_SFO_adjustment_direction
unsigned int packet_SFO_adjustment_step = 0; 

unsigned int packet_STO_adjustment_step = 0;

unsigned int packet_STO_adjustment_neg_step = 0;


// temporary callback variable, (similar to delta_low in lower_callback())
unsigned int ringbus_sfo_adjustment_temp = 0;

// this is measured in frames
#define COUNTER_FUTURE (15)

// always increasing frame counter
unsigned int lifetime_frame_counter = 0;
// true if we are waiting to apply
unsigned int pending_lifetime_update;
// if pending is true, when we apply
unsigned int future_lifetime_counter;

unsigned int pending_ringbus_sfo_adjustment_temp;
unsigned int pending_data;

// Call this first
void sfo_adjustment_callback(unsigned int data) {
    ringbus_sfo_adjustment_temp = data;
    // ring_block_send_eth(data);
}


circular_buf_pow2_t pending_ring_out = CIRBUF_POW2_STATIC_CONSTRUCTOR(pending_ring_out, 8);

// Call this second, settings are not applied till this is called
// 0 is disable sfo adjustment
// 1 is delete (1 sample)
// 2 is add (1 sample)
void sfo_sign_callback(unsigned int data) {
    
    // pick a frame value in the future
    // and send to cs20
    
    if( data == 1 || data == 2 || data == 0  ) {
        pending_lifetime_update = 1;
        pending_ringbus_sfo_adjustment_temp = ringbus_sfo_adjustment_temp;
        pending_data = data;
        future_lifetime_counter = (lifetime_frame_counter + COUNTER_FUTURE) & LIFETIME_COUNTER_MASK;
        // SET_REG(x4, lifetime_frame_counter);
        // SET_REG(x4, future_lifetime_counter);


        // CSR_WRITE(RING_ADDR_CS20, RING_ADDR_PC);
        // CSR_WRITE(RINGBUS_WRITE_DATA, SFO_COORDINATION_CMD|future_lifetime_counter);
        // CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

        circular_buf2_put(&pending_ring_out, RING_ADDR_CS11);
        circular_buf2_put(&pending_ring_out, SFO_COORDINATION_CMD|future_lifetime_counter);

        circular_buf2_put(&pending_ring_out, RING_ADDR_CS11);
        circular_buf2_put(&pending_ring_out, SFO_PERIODIC_ADJ_CMD|pending_ringbus_sfo_adjustment_temp);

        circular_buf2_put(&pending_ring_out, RING_ADDR_CS11);
        circular_buf2_put(&pending_ring_out, SFO_PERIODIC_SIGN_CMD|pending_data);
    }

    // CSR_WRITE(RINGBUS_WRITE_DATA, SFO_PERIODIC_ADJ_CMD|pending_ringbus_sfo_adjustment_temp);
    // CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

    // CSR_WRITE(RINGBUS_WRITE_DATA, SFO_PERIODIC_SIGN_CMD|pending_data);
    // CSR_WRITE_ZERO(RINGBUS_WRITE_EN);



    if( data == 0 ) 
    {
        // packet_num_SFO_adjustment_period = ringbus_sfo_adjustment_temp;
        // packet_SFO_adjustment_direction = data;
        // packet_SFO_adjustment_step = 0;
        // packet_counter_SFO_adjustment = 0;


        // circular_buf2_put(&pending_ring_out, RING_ADDR_CS20);
        // circular_buf2_put(&pending_ring_out, SFO_COORDINATION_CMD|0);

        // circular_buf2_put(&pending_ring_out, RING_ADDR_CS20);
        // circular_buf2_put(&pending_ring_out, SFO_PERIODIC_ADJ_CMD|0);

        // circular_buf2_put(&pending_ring_out, RING_ADDR_CS20);
        // circular_buf2_put(&pending_ring_out, SFO_PERIODIC_SIGN_CMD|0);



    } else if (data == 1 || data == 2) 
    {
        // handled later
    } 
    else if (data == 3)
    {
        packet_STO_adjustment_step = ringbus_sfo_adjustment_temp;
    }
    else if (data == 4)
    {
        packet_STO_adjustment_neg_step = ringbus_sfo_adjustment_temp;
    }



    // if( data == 0 ) 
    // {
    //     packet_num_SFO_adjustment_period = ringbus_sfo_adjustment_temp;
    //     packet_SFO_adjustment_direction = data;
    //     packet_SFO_adjustment_step = 0;
    //     packet_counter_SFO_adjustment = 0;
    // } else if (data == 1 || data == 2) 
    // {
    //     packet_num_SFO_adjustment_period = ringbus_sfo_adjustment_temp;
    //     packet_SFO_adjustment_direction = data;
    //     packet_SFO_adjustment_step = 1;
    //     packet_counter_SFO_adjustment = 0;
    // } 
    // else if (data == 3)
    // {
    //     packet_STO_adjustment_step = ringbus_sfo_adjustment_temp;
    // }
    // else if (data == 4)
    // {
    //     packet_STO_adjustment_neg_step = ringbus_sfo_adjustment_temp;
    // }

    
    // ring_block_send_eth(ringbus_sfo_adjustment_temp);
    // ring_block_send_eth(data);

}
//////////////////


void future_apply_sfo(void) {

    if(pending_lifetime_update && (future_lifetime_counter == lifetime_frame_counter)) {

        pending_lifetime_update = 0;


        // ring_block_send_eth(0x12345678);
        // ring_block_send_eth(pending_ringbus_sfo_adjustment_temp);
        // ring_block_send_eth(pending_data);


        if( pending_data == 0 ) 
        {
            packet_SFO_adjustment_direction = 0;
            packet_SFO_adjustment_step = 0;

            // we reset the COUNTER also
            packet_counter_SFO_adjustment = 0;

        } else if (pending_data == 1 || pending_data == 2) 
        {
            packet_num_SFO_adjustment_period = pending_ringbus_sfo_adjustment_temp;
            packet_SFO_adjustment_direction = pending_data;
            packet_SFO_adjustment_step = 1;
            // packet_counter_SFO_adjustment = 0;
        } 
        else if (pending_data == 3)
        {
        }
        else if (pending_data == 4)
        {
        }


        // ring_block_send_eth(0xdead0010);
        // ring_block_send_eth(future_lifetime_counter);
        // ring_block_send_eth(packet_num_SFO_adjustment_period);
        // ring_block_send_eth(packet_SFO_adjustment_direction);



    }

}




















vmem_t* vmalloc_double(VMalloc *mgr) {
  vmem_t* rtn = vmalloc_single(mgr);
  vmalloc_single(mgr);

  return rtn;
}

vmem_t* vmalloc_eight(VMalloc *mgr) {
  vmem_t* rtn = vmalloc_double(mgr);
  vmalloc_double(mgr);
  vmalloc_double(mgr);
  vmalloc_double(mgr);

  return rtn;
}

// returns a DMA pointer
unsigned int dma_idx_to_ptr(unsigned int idx) {
  return dma_in_dma_ptr + (idx * 2048);
}


void make_cfo(unsigned int nco_loc, unsigned int angle, unsigned int delta){

  CSR_WRITE(NCO_START_ANGLE, angle);
  CSR_WRITE(NCO_LENGTH, nco_length);
  CSR_WRITE(NCO_DELTA, delta); 
  CSR_WRITE(NCO_PUSH_SCHEDULE, 0); // any value

  CSR_WRITE(DMA_2_START_ADDR, nco_loc);
  CSR_WRITE(DMA_2_LENGTH, nco_length);
  CSR_WRITE(DMA_2_TIMER_VAL, 0xffffffff);  // start right away
  CSR_WRITE(DMA_2_PUSH_SCHEDULE, 0); // any value

}

void run_cfo(unsigned int is_conj, unsigned int input_data_fft, unsigned int input_data_nco, unsigned int output_data_location){
    if (is_conj == 0x1){
        MVXV_KNOP(V0, cfg_cmulco_location);
    } else {    
        MVXV_KNOP(V0, cfg_cmulti_location);
    }
    
    VNOP_LK14(V0); //k14 configuration word

    MVXV_KNOP(V1, input_data_fft+48);  
    MVXV_KNOP(V2, input_data_nco);
    MVXV_KNOP(V3, output_data_location);
    MVXV_KNOP(V4, 0x1);

    for (unsigned int i = 0; i < 4; i++){
        ADD_LK8(V1, V1, V4, 0x0); 
        ADD_LK9(V2, V2, V4, 0x0);   
    }

    for (unsigned int i = 0; i < 12; i++){
        // VNOP_LK8(V1); // load k8 from memory
        // VNOP_LK9(V2); // load k9 from memory
        // VNOP_SK1(V3);
        ADD_LK8(V1, V1, V4, 0x0);
        ADD_LK9(V2, V2, V4, 0x0);
        ADD_SK1(V3, V3, V4, 0x0);
    }

    MVXV_KNOP(V1, input_data_fft);
    for (unsigned int i = 0; i < 4; i++){
        ADD_SK1(V3, V3, V4, 0x0);
        ADD_LK8(V1, V1, V4, 0x0);
        ADD_LK9(V2, V2, V4, 0x0);
    }

    for (unsigned int i = 0; i < 60; i++){
        ADD_LK8(V1, V1, V4, 0x0);
        ADD_LK9(V2, V2, V4, 0x0);
        ADD_SK1(V3, V3, V4, 0x0);
    }
    for (unsigned int i = 0; i < 4; i++){
        ADD_SK1(V3, V3, V4, 0x0);   
    }

}

void move(unsigned int input_data, unsigned int output_data, unsigned int size){
    MVXV_KNOP(V2, input_data);
    MVXV_KNOP(V3, output_data);
    MVXV_KNOP(V4, 0x1);

       for (unsigned int i = 0; i < size; i++){
           VNOP_LK13(V2);
           VNOP_SK13(V3);
           ADD_KNOP(V2, V2, V4, 0x0);
           ADD_KNOP(V3, V3, V4, 0x0);
       }
  }

// 
void trig_dma_in(unsigned int idx, unsigned int timer_start) {
  // dma_in_set(VMEM_DMA_ADDRESS(dma_in_ptr[idx]), DMA_IN_CHUNK);

  // static unsigned int timer_start = 4096;

  CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(dma_in_ptr[idx]));
  CSR_WRITE(DMA_0_LENGTH, DMA_IN_CHUNK);
  CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
  CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);   // any value

  // timer_start += 4096;
}

void setup_dma_in(void) {
  dma_in_ptr[0] = (unsigned int)vmalloc_single(&mgr);
  dma_in_ptr[1] = (unsigned int)vmalloc_single(&mgr);

  trig_dma_in(0, 0xffffffff);
}

fft1024_t active_plan;

void setup_fft(void) {
  fft_ptr[0] = (unsigned int)vmalloc_single(&mgr);
  fft_ptr[1] = (unsigned int)vmalloc_single(&mgr);
  fft_a_empty = 1;
  fft_b_empty = 1;

  active_plan = get_fft1024_plan(0, 0);
}

void setup_cfo(void) {
  cfo_ptr[0] = (unsigned int)vmalloc_double(&mgr);
  cfo_ptr[1] = (unsigned int)vmalloc_double(&mgr);

  cfo_a_empty = 1;
  cfo_b_empty = 1;

  cfg_cmulti_location=VMEM_ADDRESS(config_word_cmul_eq_0f);
  cfg_cmulco_location=VMEM_ADDRESS(config_word_conj_eq_0f);

}

void setup_dma_out(void) {
  dma_in_dma_ptr = VMEM_DMA_ADDRESS(vmalloc_eight(&mgr));

  CIRBUF_POW2_RUNTIME_INITIALIZE(dma_out_started);

  CIRBUF_POW2_RUNTIME_INITIALIZE(dma_out_size);

}

void pet_dma_in(void) {
  int error;

  unsigned int dma_occupancy;

  CSR_READ(DMA_0_SCHEDULE_OCCUPANCY, dma_occupancy);  // switch to dma_occupancy

  if(dma_occupancy == 0) {

    // Check current process for available space
    if (dma_state == 0){
      if (dma_a_empty == 0){
        return;
      }
      dma_a_empty = 0; // hold a
    }

    if (dma_state == 1){
      if (dma_b_empty == 0){
        return;
      }
      dma_b_empty = 0; // hold b
    } 

    dma_state = (dma_state+1)&0x1;
    // Do something with the data   
    trig_dma_in(dma_state, 0xffffffff);
  }

}



void pet_fft(void) {
  // example only starts to work once we have 2 items in the queue

  int error;
  static unsigned updates = 1;

  unsigned int consume_idx;

  // deals with dma telling us that we have new data to consume

  // Check current process for available space
  if (fft_state == 0){
    if (dma_a_empty == 1){  
      return;  // nothing in dma a
    }
    if (fft_a_empty == 0){
      return; //  something in fft a
    }
    dma_a_empty = 1;  // release dma a
    fft_a_empty = 0;  // hold fft a
  }

  if (fft_state == 1){
    if (dma_b_empty == 1){
      return; // nothing in dma b
    }
    if (fft_b_empty == 0){
      return; // something in fft b
    }
    dma_b_empty = 1; //  release dma b
    fft_b_empty = 0; // hold fft b
  } 

  consume_idx = fft_state;

  fft_state = (fft_state+1)&0x1;

  // CSR_WRITE(GPIO_WRITE, (check_x4<<24) | (0xFF1<<8) | (consume_idx & 0xFF));
  // check_x3 = vector_memory[VMEM_DMA_ADDRESS(dma_in_ptr[consume_idx])];
  // check_x4 = VMEM_DMA_ADDRESS(dma_in_ptr[consume_idx]);

  // Do something with the data
  unsigned int current_angle = next_angle + rbus_theta_shift;

  rbus_theta_shift = 0;

  make_cfo(VMEM_DMA_ADDRESS(cfo_ptr[consume_idx]), current_angle, rbus_omega); 

  next_angle = current_angle + nco_length*rbus_omega; 


  unsigned int* cpu_ptr_from_dma = (unsigned int*)dma_in_ptr[consume_idx];
  unsigned int* cpu_ptr_fft = (unsigned int*)fft_ptr[consume_idx];


  active_plan.data_location   = VMEM_ROW_ADDRESS(cpu_ptr_from_dma);  //input
  active_plan.data_location_0 = VMEM_ROW_ADDRESS(cpu_ptr_fft);       //output 


  if (DEBUG){
    unsigned int input = VMEM_DMA_ADDRESS(dma_in_ptr[consume_idx]);
    unsigned int output = VMEM_DMA_ADDRESS(fft_ptr[consume_idx]);
    unsigned int tmp;
    for (unsigned int i = 0; i < 1024; i++){
      // vector_memory[output + i] = vs1024[i];
      tmp = vector_memory[input+i];
      if (tmp != expected_0 + i){
        check_x3 = tmp;
      } else {
        check_x3 = 0xdead;
      }
      vector_memory[output+i] = tmp;
    }
    expected_0 = expected_0 + 1024;
    // check_x3 = output;
  } else {
    fft_1024_run(&active_plan);

#ifdef DEBUG_PASSTHROUGH_VALUES
    unsigned int* out_ptr = fft_ptr[consume_idx];
    out_ptr[0] = cpu_ptr_from_dma[0];
    out_ptr[1] = cpu_ptr_from_dma[1];
#endif
  }

}



// similar to just calling dma_out_set
void dma_out_set_safe(unsigned int dma_ptr, unsigned int size) {

 
  unsigned int occupancy;
  // unsigned int occupancy_busy;
  // unsigned int occupancy_combined1, occupancy_combined2;
  while(1) {
    CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
    if(occupancy < DMA_1_SCHEDULE_DEPTH) {
      break; // should break on first go
    } else {
      // stuck, can report this with ringbus
      //ring_block_send_eth(0xd0000000);
    }
  }
  
  dma_out_set(dma_ptr, size);
}

#define IS_SECOND_DMA (0x2)
#define DMA_A_B_MASK (0x1)

unsigned int dma_out_extra = 0;



void pet_cfo(void) {
  // example only starts to work once we have 2 items in the queue

  int error;
  unsigned int consume_idx;
  unsigned int occupancy;

  // Check current process for available space
  occupancy = circular_buf2_occupancy(&dma_out_started); 

  if (occupancy >= DMA_OUT_CIRBUF_SIZE - 1){
    return; // abort
  }

  if (cfo_state == 0){
    if (fft_a_empty == 1){
      return;
    }
    // if (cfo_a_empty == 0){
    //   return;
    // }
    fft_a_empty = 1;  // release a
    // cfo_a_empty = 0;
  }

  if (cfo_state == 1){
    if (fft_b_empty == 1){
      return;
    }
    // if (cfo_b_empty == 0){
    //   return;
    // }
    fft_b_empty = 1; //  release b
    // cfo_b_empty = 0;
  } 

  consume_idx = cfo_state;

  cfo_state = (cfo_state+1)&0x1;

  // this is a DMA pointer
  unsigned int dma_out_ptr = dma_idx_to_ptr(dma_trig_next);
  
  // Do something with the data

  // CSR_WRITE(GPIO_WRITE, (check_x4<<24) | (0xFF1<<8) | (consume_idx & 0xFF));
  // check_x3 = vector_memory[VMEM_DMA_ADDRESS(fft_ptr[consume_idx])];
  // check_x4 = VMEM_DMA_ADDRESS(fft_ptr[consume_idx]);

  ////////
  //
  // Warning only insert 256,1024 sizes here, or it may break pet_dma_out()
  //
#ifndef DEBUG_OUTPUT_COUNTER

  if (rbus_omega != 0){



    run_cfo(rbus_omega_sign,VMEM_ROW_ADDRESS(fft_ptr[consume_idx]), VMEM_ROW_ADDRESS(cfo_ptr[consume_idx]), VMEM_DMA_ADDRESS_TO_ROW(dma_out_ptr));


    // ALWAYS use 2 output DMA for now because of zero insert bug
    error = circular_buf2_put(&dma_out_started, dma_out_ptr); MY_ASSERT(error == 0);
    error = circular_buf2_put(&dma_out_size, 256); MY_ASSERT(error == 0);

    error = circular_buf2_put(&dma_out_started, dma_out_ptr+256); MY_ASSERT(error == 0);
    error = circular_buf2_put(&dma_out_size, 1024); MY_ASSERT(error == 0);

  } else {
    move(VMEM_ROW_ADDRESS(fft_ptr[consume_idx]),VMEM_DMA_ADDRESS_TO_ROW(dma_out_ptr),64);
    // check_x3 = VMEM_ROW_ADDRESS(fft_ptr[consume_idx]);
    // check_x4 = VMEM_DMA_ADDRESS_TO_ROW(dma_out_ptr);
    if (DEBUG) {
      // error = circular_buf2_put(&dma_out_started, dma_out_ptr); MY_ASSERT(error == 0);
      // error = circular_buf2_put(&dma_out_size, 1024); MY_ASSERT(error == 0);
    } else {
      error = circular_buf2_put(&dma_out_started, dma_out_ptr + 768); MY_ASSERT(error == 0);
      error = circular_buf2_put(&dma_out_size, 256); MY_ASSERT(error == 0);

      error = circular_buf2_put(&dma_out_started, dma_out_ptr); MY_ASSERT(error == 0);
      error = circular_buf2_put(&dma_out_size, 1024); MY_ASSERT(error == 0);
    }      
  }

#ifdef DEBUG_PASSTHROUGH_VALUES
    unsigned int* in_buf = fft_ptr[consume_idx];
    unsigned int* out_buf = REVERSE_VMEM_DMA_ADDRESS(dma_out_ptr);
    // SET_REG(x3, 0xfeed0000);
    // SET_REG(x3, in_buf[0]);
    // SET_REG(x3, in_buf[1]);
    out_buf[0] = in_buf[0];
    out_buf[1] = in_buf[1];
#endif

#else
  // output a dummy counter starting at 0xf0000000
  unsigned int counter_dma_addr = VMEM_DMA_ADDRESS(counter_1280);
  // Note: I guess this section will be faster because we are not running CFO, does this matter?
  error = circular_buf2_put(&dma_out_started, counter_dma_addr); MY_ASSERT(error == 0);
  error = circular_buf2_put(&dma_out_size, 256); MY_ASSERT(error == 0);

  error = circular_buf2_put(&dma_out_started, counter_dma_addr+256); MY_ASSERT(error == 0);
  error = circular_buf2_put(&dma_out_size, 1024); MY_ASSERT(error == 0);

#endif

  // common to debug and non debug
  dma_trig_next = (dma_trig_next+1)&0x3;


  // CSR_WRITE(GPIO_WRITE, (check_x4<<24) | (0xED<<8) | 0);    

}

// This function takes the logic from the input DMA on cs00
// and copies it over here to 10, as a "black box"
// we simply adjust dma in/out values without messing with previous dma trigger structure
void adjust_dma_out_size(unsigned int* addr, unsigned int* length) {
    if( packet_SFO_adjustment_direction == 0) {
        *addr += packet_STO_adjustment_neg_step;
        *length += -packet_STO_adjustment_neg_step+packet_STO_adjustment_step;
        // CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(dma_in_ptr[idx])+packet_STO_adjustment_neg_step);
        // CSR_WRITE(DMA_0_LENGTH, DMA_IN_CHUNK+advance-packet_STO_adjustment_neg_step+packet_STO_adjustment_step);
        // CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
        // CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);   // any value
        packet_STO_adjustment_neg_step = 0;
        packet_STO_adjustment_step = 0;
        // SET_REG(x3, 0xf0000000);
    } else {

        if(packet_counter_SFO_adjustment != (packet_num_SFO_adjustment_period-1)) {
            *addr += packet_STO_adjustment_neg_step;
            *length += -packet_STO_adjustment_neg_step+packet_STO_adjustment_step;

            // CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(dma_in_ptr[idx])+packet_STO_adjustment_neg_step);
            // CSR_WRITE(DMA_0_LENGTH, DMA_IN_CHUNK+advance-packet_STO_adjustment_neg_step+packet_STO_adjustment_step);
            // CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
            // CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);   // any value
            packet_STO_adjustment_neg_step = 0;
            packet_STO_adjustment_step = 0;
            packet_counter_SFO_adjustment++;
        } else {
            // mode 1 delete 
            // SET_REG(x3, 0xd0000000);
            // SET_REG(x4, packet_SFO_adjustment_direction);
            if( packet_SFO_adjustment_direction == 1 ) {
                //SET_REG(x3, 0xc0000000);
                // *addr += // not modified
                *length += packet_SFO_adjustment_step;
                // CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(dma_in_ptr[idx]));
                // CSR_WRITE(DMA_0_LENGTH, DMA_IN_CHUNK+advance+packet_SFO_adjustment_step);
                // CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
                // CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);   // any value
            } else if (packet_SFO_adjustment_direction == 2) {
                //SET_REG(x3, 0xb0000000);
                // mode 2 add
                // *addr += // not modified
                *length += -packet_SFO_adjustment_step;
                // CSR_WRITE(DMA_0_START_ADDR, VMEM_DMA_ADDRESS(dma_in_ptr[idx]));
                // CSR_WRITE(DMA_0_LENGTH, DMA_IN_CHUNK+advance-packet_SFO_adjustment_step);
                // CSR_WRITE(DMA_0_TIMER_VAL, timer_start); // start right away
                // CSR_WRITE(DMA_0_PUSH_SCHEDULE, 0);

                // in add, we just copy
                if(packet_SFO_adjustment_step!=0)
                {
                    //SET_REG(x3, 0xa0000000);
                    // FIXME: only valid when packet_SFO_adjustment_step = 1.
                    // FIXME: REPLACE SAMPLE DOUBLING CODE HERE
                    // vector_memory[VMEM_DMA_ADDRESS(dma_in_ptr[idx])+1279] = vector_memory[VMEM_DMA_ADDRESS(dma_in_ptr[idx])+1278];
                }
                
            }
            // if we hit the counter
            packet_counter_SFO_adjustment = 0;
        
        }
    }
}

void pet_dma_out(void) {
  int helper;
  int consume_idx;
  unsigned int occupancy;
  int error;
  unsigned int data;
  unsigned int addr;
  unsigned int size;
  unsigned int output_blocked = 0;
  unsigned int dma_occupancy_combined;
  unsigned int dma_occupancy;
  unsigned int dma_occupancy_status;


  occupancy = circular_buf2_occupancy(&dma_out_started); 

  if (occupancy == 0){
    return; // abort
  }

  // I decided it was easier to apply these adjustments as we pull thing out of the 
  // buffers because it would be complicuated to do it where we put the values in the buffer

  error = circular_buf2_get(&dma_out_started, &addr);
  error = circular_buf2_get(&dma_out_size, &size);

  // we only mess with dma start and count at ONE of the output DMA.
  // with understanding, this could be changed to 1024
  if(size == 1024) {
      future_apply_sfo();
      adjust_dma_out_size(&addr, &size);
      //SET_REG(x4, lifetime_frame_counter);
      lifetime_frame_counter = (lifetime_frame_counter+1)&LIFETIME_COUNTER_MASK;
      
  }

  SET_REG(x4, packet_counter_SFO_adjustment);
  SET_REG(x3, 0x88888888);
  SET_REG(x3, lifetime_frame_counter);


  dma_out_set_safe(addr,size);

  // if (DEBUG){
  //   unsigned int tmp;
  //   for (unsigned int i = 0; i < 1024; i++){
  //     // vector_memory[output + i] = vs1024[i];
  //     tmp = vector_memory[addr+i];
  //     if (tmp != expected_1 + i){
  //       check_x4 = tmp;
  //     } else {
  //       check_x4 = 0xdead;
  //     }
  //   }
  //   expected_1 = expected_1 + 1024;
  // }

}

// run first
void lower_callback(unsigned int data) {
    // check_x4 = data;
    delta_low = (data & 0xffff);
}

// run to trigger
// void upper_callback(unsigned int data) {
//     // check_x4 = data;
//     rbus_omega = (((data & 0xffff) << 16) | delta_low);
// }

// set upper last, copies and clears lower
void upper_callback(unsigned int data) {
    unsigned int cmd = ((data & 0x00FF0000) >> 16);
    data = ((data & 0xffff) << 16);
    if (cmd == 0x00){
        rbus_omega = (data | delta_low);
        rbus_omega_sign = 0;
    } else if (cmd == 0x01){
        rbus_omega = (data | delta_low);
        rbus_omega_sign = 1;
    } else if (cmd == 0x02){
        rbus_theta_shift = (data | delta_low);
    }
    delta_low = 0;
    // check_x3 = 0xbabe;

    // ring_block_send_eth(rbus_theta_shift);
}

void reset_dac_underflow(void);

void check_dac_underflow(void) {
    // CSR_WRITE(CS_CONTROL, err_state);
    unsigned int riscv_status;
    CSR_READ(CS_STATUS, riscv_status);

    if( riscv_status & 0x1 ) {
        // set LED
        CSR_SET_BITS(GPIO_WRITE, LED_GPIO_BIT);
    }

    SET_REG(x4, 0xdead0000 | riscv_status);

    reset_dac_underflow();
}

void reset_dac_underflow(void) {
    static unsigned int last_reset = 0;
    unsigned int now;
    CSR_READ(TIMER_VALUE, now);

    // 125000000
    if(now - last_reset > 125000000) {
        CSR_WRITE(CS_CONTROL, 0x1);
        CSR_WRITE(CS_CONTROL, 0x0);
        last_reset = now;
        CSR_CLEAR_BITS(GPIO_WRITE, LED_GPIO_BIT);
    }
}

// this is in clock cylces
#define MINIMUM_RING_TIMING (6000)

void pet_pending_ring(void) {
    static unsigned int last_send = 0;
    unsigned int now;
    unsigned int ttl;
    unsigned int data;
    unsigned int occupancy;

    if( pending_ring_out.occupancy > 1) {

        CSR_READ(RINGBUS_SCHEDULE_OCCUPANCY, occupancy);

        CSR_READ(TIMER_VALUE, now);

        if( (occupancy < RINGBUS_SCHEDULE_DEPTH) && ((now - last_send) > MINIMUM_RING_TIMING) ) {
            circular_buf2_get(&pending_ring_out, &ttl);
            circular_buf2_get(&pending_ring_out, &data);

            CSR_WRITE(RINGBUS_WRITE_ADDR, ttl);
            CSR_WRITE(RINGBUS_WRITE_DATA, data);
            CSR_READ(TIMER_VALUE, last_send); // read right before send
            CSR_WRITE_ZERO(RINGBUS_WRITE_EN);

            
        }
    }
}


void check_error_counter(void) {
    // CSR_WRITE(CS_CONTROL, err_state);
    unsigned int riscv_status;
    CSR_READ(CS_STATUS, riscv_status);
    unsigned int now;
    CSR_READ(TIMER_VALUE, now);

    // if( riscv_status > 0 ) {
    //     // set LED
    //     // CSR_SET_BITS(GPIO_WRITE, LED_GPIO_BIT);
    //     CSR_WRITE(CS_CONTROL, 0x1); // Send counter reset
    //     CSR_WRITE(CS_CONTROL, 0x0);
    //     // 
    //     ring_block_send_eth_debug(riscv_status);
    // } else if(now > then) {
    //     ring_block_send_eth_debug(riscv_status);
    //     then = now + 125000000;
    // }  

    if(now < then){ // now wrapped
        then = 0;
    }

    if(now-then > (125000000)) {
        // ring_block_send_eth_debug(riscv_status);
        // ring_block_send_eth(riscv_status);
        ring_block_send_eth(TX_UNDERFLOW|(riscv_status & 0xffffff));
        CSR_WRITE(CS_CONTROL, 0x1);
        CSR_WRITE(CS_CONTROL, 0x0);
        
        then = now;
    }  

}


VMEM_SECTION unsigned int junk[16];


int main(void) {

    CSR_WRITE_ZERO(DMA_1_FLUSH_SCHEDULE);
    CSR_WRITE_ZERO(DMA_2_FLUSH_SCHEDULE);
    flush_input_dma(VMEM_DMA_ADDRESS(junk), 16, 8192);

  init_VMalloc(&mgr);
  // unsigned int burn = 16;
  // for(unsigned int i = 0)

  CSR_WRITE(GPIO_WRITE_EN, 0xffffffff);

  ring_register_callback(&sfo_adjustment_callback, SFO_PERIODIC_ADJ_CMD);
  ring_register_callback(&sfo_sign_callback, SFO_PERIODIC_SIGN_CMD);

  ring_register_callback(&lower_callback, TX_CFO_LOWER_CMD);
  ring_register_callback(&upper_callback, TX_CFO_UPPER_CMD);

  CIRBUF_POW2_RUNTIME_INITIALIZE(pending_ring_out);

  setup_dma_in();
  setup_fft();
  setup_dma_out();
  setup_cfo();

  unsigned int counter = 0;
  // CSR_WRITE(GPIO_WRITE, 0xbabe << 8);

  // ring_block_send_eth(dma_in_ptr[0]);
  // ring_block_send_eth(dma_in_ptr[1]);
  // ring_block_send_eth(fft_ptr[0]);
  // ring_block_send_eth(fft_ptr[1]);

  CSR_WRITE(GPIO_WRITE_EN, LED_GPIO_BIT);
  CSR_CLEAR_BITS(GPIO_WRITE, LED_GPIO_BIT);

  Ringbus ringbus;

  while(1) {
    check_x3 = 0x1;
    pet_dma_in();
    check_x3 = 0x2;
    pet_fft();
    check_x3 = 0x3;
    pet_cfo();
    check_x3 = 0x4;
    pet_dma_out();
    pet_dma_out();
    check_x3 = 0x5;
    pet_pending_ring();

#ifdef CHECK_ERROR_COUNTERS    
    check_error_counter();
#endif

    check_ring(&ringbus);
    // pet_dma_out();
    // check_x4 = 0x5;
    // pet_dma_out();
    // check_x4 = 0x6;
    // pet_dma_out();

    // check_dac_underflow();

    // if(counter == 2000) {
    //   // check_ring(&ringbus);
    //   // unsigned int mem_free = vmalloc_available(&mgr);
    //   // ring_block_send_eth(0xc0000000 | mem_free);
    //   counter = 0;
    // }
    // counter++;
  }
  


  // unsigned int dma_in_index;

  // unsigned int* in_a = vmalloc_single(&mgr);
  // unsigned int* in_b = vmalloc_single(&mgr);




} 
