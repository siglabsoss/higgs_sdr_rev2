#include "xbaseband.h"
#include "csr_control.h"
#include "dma.h"
#include "ringbus.h"
#include "circular_buffer_pow2.h"
#include "vmem.h"
#include "random.h"

#include <stdint.h>
#include <stdbool.h>

#include "pipeline_work.h"

#include "feedback_bus.h"

#include "ringbus2_pre.h"
#include "ringbus2_post.h"


// we are peer 3 (or something)


VMEM_SECTION feedback_frame_ringbus_filled_t ringbus_frame_janson; // peer 0
VMEM_SECTION feedback_frame_ringbus_filled_t ringbus_frame_zhen; // peer 1
VMEM_SECTION feedback_frame_ringbus_filled_t ringbus_frame_common;

VMEM_SECTION unsigned int junk[16] = {0x123,0xfa32,0x34,0x942,0x1a32};

VMEM_SECTION feedback_frame_stream_filled_t ringbus_frame_stream_fixed;
VMEM_SECTION feedback_frame_stream_filled_t ringbus_frame_stream_variable;

VMEM_SECTION feedback_frame_vector_filled_t ringbus_frame_vec_variable;


VMEM_SECTION unsigned int stream_data[1024] = {
4026531840, 4026531841, 4026531842, 4026531843, 4026531844, 4026531845, 4026531846, 4026531847, 4026531848, 4026531849, 4026531850, 4026531851, 4026531852, 4026531853, 4026531854, 4026531855, 4026531856, 4026531857, 4026531858, 4026531859, 4026531860, 4026531861, 4026531862, 4026531863, 4026531864, 4026531865, 4026531866, 4026531867, 4026531868, 4026531869, 4026531870, 4026531871, 4026531872, 4026531873, 4026531874, 4026531875, 4026531876, 4026531877, 4026531878, 4026531879, 4026531880, 4026531881, 4026531882, 4026531883, 4026531884, 4026531885, 4026531886, 4026531887, 4026531888, 4026531889, 4026531890, 4026531891, 4026531892, 4026531893, 4026531894, 4026531895, 4026531896, 4026531897, 4026531898, 4026531899, 4026531900, 4026531901, 4026531902, 4026531903, 4026531904, 4026531905, 4026531906, 4026531907, 4026531908, 4026531909, 4026531910, 4026531911, 4026531912, 4026531913, 4026531914, 4026531915, 4026531916, 4026531917, 4026531918, 4026531919, 4026531920, 4026531921, 4026531922, 4026531923, 4026531924, 4026531925, 4026531926, 4026531927, 4026531928, 4026531929, 4026531930, 4026531931, 4026531932, 4026531933, 4026531934, 4026531935, 4026531936, 4026531937, 4026531938, 4026531939, 4026531940, 4026531941, 4026531942, 4026531943, 4026531944, 4026531945, 4026531946, 4026531947, 4026531948, 4026531949, 4026531950, 4026531951, 4026531952, 4026531953, 4026531954, 4026531955, 4026531956, 4026531957, 4026531958, 4026531959, 4026531960, 4026531961, 4026531962, 4026531963, 4026531964, 4026531965, 4026531966, 4026531967, 4026531968, 4026531969, 4026531970, 4026531971, 4026531972, 4026531973, 4026531974, 4026531975, 4026531976, 4026531977, 4026531978, 4026531979, 4026531980, 4026531981, 4026531982, 4026531983, 4026531984, 4026531985, 4026531986, 4026531987, 4026531988, 4026531989, 4026531990, 4026531991, 4026531992, 4026531993, 4026531994, 4026531995, 4026531996, 4026531997, 4026531998, 4026531999, 4026532000, 4026532001, 4026532002, 4026532003, 4026532004, 4026532005, 4026532006, 4026532007, 4026532008, 4026532009, 4026532010, 4026532011, 4026532012, 4026532013, 4026532014, 4026532015, 4026532016, 4026532017, 4026532018, 4026532019, 4026532020, 4026532021, 4026532022, 4026532023, 4026532024, 4026532025, 4026532026, 4026532027, 4026532028, 4026532029, 4026532030, 4026532031, 4026532032, 4026532033, 4026532034, 4026532035, 4026532036, 4026532037, 4026532038, 4026532039, 4026532040, 4026532041, 4026532042, 4026532043, 4026532044, 4026532045, 4026532046, 4026532047, 4026532048, 4026532049, 4026532050, 4026532051, 4026532052, 4026532053, 4026532054, 4026532055, 4026532056, 4026532057, 4026532058, 4026532059, 4026532060, 4026532061, 4026532062, 4026532063, 4026532064, 4026532065, 4026532066, 4026532067, 4026532068, 4026532069, 4026532070, 4026532071, 4026532072, 4026532073, 4026532074, 4026532075, 4026532076, 4026532077, 4026532078, 4026532079, 4026532080, 4026532081, 4026532082, 4026532083, 4026532084, 4026532085, 4026532086, 4026532087, 4026532088, 4026532089, 4026532090, 4026532091, 4026532092, 4026532093, 4026532094, 4026532095, 4026532096, 4026532097, 4026532098, 4026532099, 4026532100, 4026532101, 4026532102, 4026532103, 4026532104, 4026532105, 4026532106, 4026532107, 4026532108, 4026532109, 4026532110, 4026532111, 4026532112, 4026532113, 4026532114, 4026532115, 4026532116, 4026532117, 4026532118, 4026532119, 4026532120, 4026532121, 4026532122, 4026532123, 4026532124, 4026532125, 4026532126, 4026532127, 4026532128, 4026532129, 4026532130, 4026532131, 4026532132, 4026532133, 4026532134, 4026532135, 4026532136, 4026532137, 4026532138, 4026532139, 4026532140, 4026532141, 4026532142, 4026532143, 4026532144, 4026532145, 4026532146, 4026532147, 4026532148, 4026532149, 4026532150, 4026532151, 4026532152, 4026532153, 4026532154, 4026532155, 4026532156, 4026532157, 4026532158, 4026532159, 4026532160, 4026532161, 4026532162, 4026532163, 4026532164, 4026532165, 4026532166, 4026532167, 4026532168, 4026532169, 4026532170, 4026532171, 4026532172, 4026532173, 4026532174, 4026532175, 4026532176, 4026532177, 4026532178, 4026532179, 4026532180, 4026532181, 4026532182, 4026532183, 4026532184, 4026532185, 4026532186, 4026532187, 4026532188, 4026532189, 4026532190, 4026532191, 4026532192, 4026532193, 4026532194, 4026532195, 4026532196, 4026532197, 4026532198, 4026532199, 4026532200, 4026532201, 4026532202, 4026532203, 4026532204, 4026532205, 4026532206, 4026532207, 4026532208, 4026532209, 4026532210, 4026532211, 4026532212, 4026532213, 4026532214, 4026532215, 4026532216, 4026532217, 4026532218, 4026532219, 4026532220, 4026532221, 4026532222, 4026532223, 4026532224, 4026532225, 4026532226, 4026532227, 4026532228, 4026532229, 4026532230, 4026532231, 4026532232, 4026532233, 4026532234, 4026532235, 4026532236, 4026532237, 4026532238, 4026532239, 4026532240, 4026532241, 4026532242, 4026532243, 4026532244, 4026532245, 4026532246, 4026532247, 4026532248, 4026532249, 4026532250, 4026532251, 4026532252, 4026532253, 4026532254, 4026532255, 4026532256, 4026532257, 4026532258, 4026532259, 4026532260, 4026532261, 4026532262, 4026532263, 4026532264, 4026532265, 4026532266, 4026532267, 4026532268, 4026532269, 4026532270, 4026532271, 4026532272, 4026532273, 4026532274, 4026532275, 4026532276, 4026532277, 4026532278, 4026532279, 4026532280, 4026532281, 4026532282, 4026532283, 4026532284, 4026532285, 4026532286, 4026532287, 4026532288, 4026532289, 4026532290, 4026532291, 4026532292, 4026532293, 4026532294, 4026532295, 4026532296, 4026532297, 4026532298, 4026532299, 4026532300, 4026532301, 4026532302, 4026532303, 4026532304, 4026532305, 4026532306, 4026532307, 4026532308, 4026532309, 4026532310, 4026532311, 4026532312, 4026532313, 4026532314, 4026532315, 4026532316, 4026532317, 4026532318, 4026532319, 4026532320, 4026532321, 4026532322, 4026532323, 4026532324, 4026532325, 4026532326, 4026532327, 4026532328, 4026532329, 4026532330, 4026532331, 4026532332, 4026532333, 4026532334, 4026532335, 4026532336, 4026532337, 4026532338, 4026532339, 4026532340, 4026532341, 4026532342, 4026532343, 4026532344, 4026532345, 4026532346, 4026532347, 4026532348, 4026532349, 4026532350, 4026532351, 4026532352, 4026532353, 4026532354, 4026532355, 4026532356, 4026532357, 4026532358, 4026532359, 4026532360, 4026532361, 4026532362, 4026532363, 4026532364, 4026532365, 4026532366, 4026532367, 4026532368, 4026532369, 4026532370, 4026532371, 4026532372, 4026532373, 4026532374, 4026532375, 4026532376, 4026532377, 4026532378, 4026532379, 4026532380, 4026532381, 4026532382, 4026532383, 4026532384, 4026532385, 4026532386, 4026532387, 4026532388, 4026532389, 4026532390, 4026532391, 4026532392, 4026532393, 4026532394, 4026532395, 4026532396, 4026532397, 4026532398, 4026532399, 4026532400, 4026532401, 4026532402, 4026532403, 4026532404, 4026532405, 4026532406, 4026532407, 4026532408, 4026532409, 4026532410, 4026532411, 4026532412, 4026532413, 4026532414, 4026532415, 4026532416, 4026532417, 4026532418, 4026532419, 4026532420, 4026532421, 4026532422, 4026532423, 4026532424, 4026532425, 4026532426, 4026532427, 4026532428, 4026532429, 4026532430, 4026532431, 4026532432, 4026532433, 4026532434, 4026532435, 4026532436, 4026532437, 4026532438, 4026532439, 4026532440, 4026532441, 4026532442, 4026532443, 4026532444, 4026532445, 4026532446, 4026532447, 4026532448, 4026532449, 4026532450, 4026532451, 4026532452, 4026532453, 4026532454, 4026532455, 4026532456, 4026532457, 4026532458, 4026532459, 4026532460, 4026532461, 4026532462, 4026532463, 4026532464, 4026532465, 4026532466, 4026532467, 4026532468, 4026532469, 4026532470, 4026532471, 4026532472, 4026532473, 4026532474, 4026532475, 4026532476, 4026532477, 4026532478, 4026532479, 4026532480, 4026532481, 4026532482, 4026532483, 4026532484, 4026532485, 4026532486, 4026532487, 4026532488, 4026532489, 4026532490, 4026532491, 4026532492, 4026532493, 4026532494, 4026532495, 4026532496, 4026532497, 4026532498, 4026532499, 4026532500, 4026532501, 4026532502, 4026532503, 4026532504, 4026532505, 4026532506, 4026532507, 4026532508, 4026532509, 4026532510, 4026532511, 4026532512, 4026532513, 4026532514, 4026532515, 4026532516, 4026532517, 4026532518, 4026532519, 4026532520, 4026532521, 4026532522, 4026532523, 4026532524, 4026532525, 4026532526, 4026532527, 4026532528, 4026532529, 4026532530, 4026532531, 4026532532, 4026532533, 4026532534, 4026532535, 4026532536, 4026532537, 4026532538, 4026532539, 4026532540, 4026532541, 4026532542, 4026532543, 4026532544, 4026532545, 4026532546, 4026532547, 4026532548, 4026532549, 4026532550, 4026532551, 4026532552, 4026532553, 4026532554, 4026532555, 4026532556, 4026532557, 4026532558, 4026532559, 4026532560, 4026532561, 4026532562, 4026532563, 4026532564, 4026532565, 4026532566, 4026532567, 4026532568, 4026532569, 4026532570, 4026532571, 4026532572, 4026532573, 4026532574, 4026532575, 4026532576, 4026532577, 4026532578, 4026532579, 4026532580, 4026532581, 4026532582, 4026532583, 4026532584, 4026532585, 4026532586, 4026532587, 4026532588, 4026532589, 4026532590, 4026532591, 4026532592, 4026532593, 4026532594, 4026532595, 4026532596, 4026532597, 4026532598, 4026532599, 4026532600, 4026532601, 4026532602, 4026532603, 4026532604, 4026532605, 4026532606, 4026532607, 4026532608, 4026532609, 4026532610, 4026532611, 4026532612, 4026532613, 4026532614, 4026532615, 4026532616, 4026532617, 4026532618, 4026532619, 4026532620, 4026532621, 4026532622, 4026532623, 4026532624, 4026532625, 4026532626, 4026532627, 4026532628, 4026532629, 4026532630, 4026532631, 4026532632, 4026532633, 4026532634, 4026532635, 4026532636, 4026532637, 4026532638, 4026532639, 4026532640, 4026532641, 4026532642, 4026532643, 4026532644, 4026532645, 4026532646, 4026532647, 4026532648, 4026532649, 4026532650, 4026532651, 4026532652, 4026532653, 4026532654, 4026532655, 4026532656, 4026532657, 4026532658, 4026532659, 4026532660, 4026532661, 4026532662, 4026532663, 4026532664, 4026532665, 4026532666, 4026532667, 4026532668, 4026532669, 4026532670, 4026532671, 4026532672, 4026532673, 4026532674, 4026532675, 4026532676, 4026532677, 4026532678, 4026532679, 4026532680, 4026532681, 4026532682, 4026532683, 4026532684, 4026532685, 4026532686, 4026532687, 4026532688, 4026532689, 4026532690, 4026532691, 4026532692, 4026532693, 4026532694, 4026532695, 4026532696, 4026532697, 4026532698, 4026532699, 4026532700, 4026532701, 4026532702, 4026532703, 4026532704, 4026532705, 4026532706, 4026532707, 4026532708, 4026532709, 4026532710, 4026532711, 4026532712, 4026532713, 4026532714, 4026532715, 4026532716, 4026532717, 4026532718, 4026532719, 4026532720, 4026532721, 4026532722, 4026532723, 4026532724, 4026532725, 4026532726, 4026532727, 4026532728, 4026532729, 4026532730, 4026532731, 4026532732, 4026532733, 4026532734, 4026532735, 4026532736, 4026532737, 4026532738, 4026532739, 4026532740, 4026532741, 4026532742, 4026532743, 4026532744, 4026532745, 4026532746, 4026532747, 4026532748, 4026532749, 4026532750, 4026532751, 4026532752, 4026532753, 4026532754, 4026532755, 4026532756, 4026532757, 4026532758, 4026532759, 4026532760, 4026532761, 4026532762, 4026532763, 4026532764, 4026532765, 4026532766, 4026532767, 4026532768, 4026532769, 4026532770, 4026532771, 4026532772, 4026532773, 4026532774, 4026532775, 4026532776, 4026532777, 4026532778, 4026532779, 4026532780, 4026532781, 4026532782, 4026532783, 4026532784, 4026532785, 4026532786, 4026532787, 4026532788, 4026532789, 4026532790, 4026532791, 4026532792, 4026532793, 4026532794, 4026532795, 4026532796, 4026532797, 4026532798, 4026532799, 4026532800, 4026532801, 4026532802, 4026532803, 4026532804, 4026532805, 4026532806, 4026532807, 4026532808, 4026532809, 4026532810, 4026532811, 4026532812, 4026532813, 4026532814, 4026532815, 4026532816, 4026532817, 4026532818, 4026532819, 4026532820, 4026532821, 4026532822, 4026532823, 4026532824, 4026532825, 4026532826, 4026532827, 4026532828, 4026532829, 4026532830, 4026532831, 4026532832, 4026532833, 4026532834, 4026532835, 4026532836, 4026532837, 4026532838, 4026532839, 4026532840, 4026532841, 4026532842, 4026532843, 4026532844, 4026532845, 4026532846, 4026532847, 4026532848, 4026532849, 4026532850, 4026532851, 4026532852, 4026532853, 4026532854, 4026532855, 4026532856, 4026532857, 4026532858, 4026532859, 4026532860, 4026532861, 4026532862, 4026532863
};

VMEM_SECTION unsigned int vector_data[16] = {
0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};


// test with this larger than 1024+16
#define MAX_ZERO_JAMMING (77)

// if we init the first item to 0, riscv will boot faster in tb
VMEM_SECTION uint32_t zeros[MAX_ZERO_JAMMING] = {0};

unsigned int seed_set = 0;

void seed_callback(unsigned int data) {
    seed_set = 1;
    simple_random_seed(data);
}

int main(void)
{
    // ringbus types
    init_feedback_ringbus(&ringbus_frame_janson, FEEDBACK_PEER_0);
    init_feedback_ringbus(&ringbus_frame_zhen, FEEDBACK_PEER_1);
    init_feedback_ringbus(&ringbus_frame_common, FEEDBACK_PEER_0 | FEEDBACK_PEER_1 );


    // stream types
    init_feedback_stream(&ringbus_frame_stream_fixed, FEEDBACK_PEER_SELF, false, true, 0);
    set_feedback_stream_length(&ringbus_frame_stream_fixed, 1024);

    init_feedback_stream(&ringbus_frame_stream_variable, FEEDBACK_PEER_SELF, false, true, 0);

    // vector types
    init_feedback_vector(&ringbus_frame_vec_variable, FEEDBACK_PEER_SELF, false, true, 0);



    unsigned int occupancy;

    ring_register_callback(seed_callback, EDGE_EDGE_IN);


    Ringbus rb;

    while(!seed_set) {
        check_ring(&rb);
    }

    // unsigned int fixed_pulls[64] = {2, 1, 1, 2, 0, 2, 4};

    unsigned int pull,pull1,pull2;
    Ringbus ringbus;
    while(1) {

        // minimum mod is largest state below
        // larger then that causes test to slow down due to stalls
        pull = simple_random() % 12;

        // 2 here is magic number, largest state below
        if( pull <= 5 ) {
            SET_REG(x3, 0xe0000 | pull);
        }

        switch(pull) {
            case 0:
                pull1 = simple_random();
                fill_feedback_ringbus_2(&ringbus_frame_common, 
                    RING_ENUM_CS11, 0xdeadbeef,   // first rb
                    RING_ENUM_CS01, pull1);       // second rb

                dma_block_send(VMEM_DMA_ADDRESS(&ringbus_frame_common), FEEDBACK_HEADER_WORDS);
                break;
            case 1:
                pull1 = simple_random();
                fill_feedback_ringbus_3(&ringbus_frame_common, 
                    RING_ENUM_CS11, 0xdeadfeed,   // first rb
                    RING_ENUM_CS01, pull1,        // second rb
                    RING_ENUM_CS11, pull1+1);       // third rb

                dma_block_send(VMEM_DMA_ADDRESS(&ringbus_frame_common), FEEDBACK_HEADER_WORDS);
                break;
            case 2:
                // using mod will return [0,the mod value) up to the value or 1 less, but we dont want to pull 0, so 
                // just add 1, and then we will be [1.the mod value]
                pull1 = (simple_random() % MAX_ZERO_JAMMING) + 1;
                SET_REG(x3,0xee00000 | pull1);
                dma_block_send(VMEM_DMA_ADDRESS(zeros), pull1);
                break;
            case 3:
                // we already called set_feedback_stream_length() so we can just fire the header right away
                dma_block_send(VMEM_DMA_ADDRESS(&ringbus_frame_stream_fixed), FEEDBACK_HEADER_WORDS);

                // now we can dma 1024 words from any buffer we want
                dma_block_send(VMEM_DMA_ADDRESS(stream_data), 1024);
                break;
            case 4:
                // decide how many to send
                pull1 = (simple_random() % 1024) + 1;
                SET_REG(x3,0xee00000 | pull1);
                // we need to set length every time because it's changing
                set_feedback_stream_length(&ringbus_frame_stream_variable, pull1);

                // set any extra params by hand if we want
                ringbus_frame_stream_variable.stype = pull;


                // send the header now that it's done
                dma_block_send(VMEM_DMA_ADDRESS(&ringbus_frame_stream_variable), FEEDBACK_HEADER_WORDS);

                // now we can dma from anywhere as long as it's pull1 in length
                // send pull1 items starting from the beginnign of this array
                dma_block_send(VMEM_DMA_ADDRESS(stream_data), pull1);
                break;
            case 5:
                pull1 = (simple_random() % 1024) + 1;
                SET_REG(x3,0xee00000 | pull1);
                // we need to set length every time because it's changing
                set_feedback_stream_length(&ringbus_frame_stream_variable, pull1);

                // set any extra params by hand if we want
                ringbus_frame_stream_variable.stype = pull;

                // send the header now that it's done
                dma_block_send(VMEM_DMA_ADDRESS(&ringbus_frame_stream_variable), FEEDBACK_HEADER_WORDS);

                // now we can dma from anywhere as long as it's pull1 in length
                // send pull1 ending at the END of this array
                dma_block_send(VMEM_DMA_ADDRESS(stream_data)+1024-pull1, pull1);

                break;

            case 6:
                // decide how many to send
                pull1 = (simple_random() % 16) + 1;
                SET_REG(x3,0xee00000 | pull1);
                // we need to set length every time because it's changing
                set_feedback_vector_length(&ringbus_frame_vec_variable, pull1);

                // set any extra params by hand if we want
                ringbus_frame_vec_variable.vtype = pull;

                // send the header now that it's done
                dma_block_send(VMEM_DMA_ADDRESS(&ringbus_frame_vec_variable), FEEDBACK_HEADER_WORDS);

                // now we can dma from anywhere as long as it's pull1 in length
                // send pull1 items starting from the beginnign of this arr
                dma_block_send(VMEM_DMA_ADDRESS(vector_data), pull1);
                break;
            default:
                STALL(2);
            break;
        } // switch

        // block so that output dma's are 1:1
        // this was giving me issues earler
        // so I had to put it in

        while(1) {
            CSR_READ(DMA_1_SCHEDULE_OCCUPANCY, occupancy);
            if( occupancy == 0) {
                break;
            }
        }

        SET_REG(x3, 0);

    } // while 1

// no_exit_stream();
}