#ifndef SETTINGS
#define SETTINGS


#define DEBUG                       1
#define DISPLAY_ONLY                0
#define SLIDING_FRAMES              0
#define ONLINE                      0
#define USE_SPHERO                  0



/*---I/O---*/
#define OFFLINE_FILE                "../edvs_frames03.dat"
#define EDVS_OUT_FILE               "edvs_frames.dat"


/*---Tracker---*/
#define T_NUM_OBJ_DESC              3
#define T_MIN_MATCHES               5
#define T_MIN_GOOD_MATCHES          5

//Algorithms
#define USE_ORB                     0
#define USE_SIFT                    0
#define USE_SURF                    0
#define USE_BRIEF_ONLY              0
#define USE_WEIGHTS                 0
#define USE_AKAZE                   0
#define USE_KNN                     1

#define USE_DYNAMIC_BB              0

#define MEASURE_TIME                0
#define FILTER_IMAGE                1


#define ORB_THRESHOLD               13                  /*default: 31*/
#define BRIEF_DESCRIPTOR_LENGTH     16                  /*16/32/64*/

/*
 * Possible matcher types (see opencv-doc):
 *   BruteForce (it uses L2 )
 *   BruteForce-L1
 *   BruteForce-Hamming
 *   BruteForce-Hamming(2)
 *   FlannBased
*/
#define MATCHER_SIFT                "BruteForce"
#define MATCHER_SURF                "FlannBased"
#define MATCHER_ORB                 "BruteForce-Hamming"
#define MATCHER_BRIEF               "BruteForce-Hamming"
#define MATCHER_AKAZE               "BruteForce"

#if USE_ORB
#define MEASURE_TIME_OF             "orb_time.dat"
#elif USE_SIFT
#define MEASURE_TIME_OF             "sift_time.dat"
#elif USE_SURF
#define MEASURE_TIME_OF             "surf_time.dat"
#else
#define MEASURE_TIME_OF             "default_time.dat"
#endif

#if USE_BRIEF_ONLY
#undef FILTER_IMAGE
#define FILTER_IMAGE                1
#endif /*USE_BRIEF_ONLY*/


/*---MainWindow---*/
#define DISPLAY_FRAME_WIDTH         400
#define DISPLAY_FRAME_HEIGHT        400



/*---Frame Config---*/
#define UPDATE_INTERVAL             20
#define DATA_LEN                    128*128
#define FRAME_OVERLAPPING           10



/*---Sphero---*/
#define SPHERO_MAC                  "00:06:66:44:64:F7"
#define SPHERO_SETUP_SPEED          0x2f



#if DISPLAY_ONLY
#undef DEBUG
#define DEBUG                       0
#endif

#if FRAME_OVERLAPPING >= UPDATE_INTERVAL && SLIDING_FRAMES
#error FRAME_OVERLAPPING must be less than UPDATE_INTERVAL
#endif


#if (USE_ORB && USE_SIFT) || (USE_ORB && USE_SURF) || (USE_SIFT && USE_SURF) \
    || (USE_ORB && USE_BRIEF_ONLY) || (USE_SURF && USE_BRIEF_ONLY) || (USE_SIFT && USE_BRIEF_ONLY) \
    || (USE_ORB && USE_WEIGHTS) || (USE_SURF && USE_WEIGHTS) || (USE_SIFT && USE_WEIGHTS) \
    || (USE_WEIGHTS && USE_BRIEF_ONLY) \
    || (USE_AKAZE && USE_SIFT) || (USE_ORB && USE_AKAZE) || (USE_AKAZE && USE_SURF) \
    || (USE_AKAZE && USE_BRIEF_ONLY) || (USE_AKAZE && USE_WEIGHTS) \
    || (USE_KNN && USE_SIFT) || (USE_ORB && USE_KNN) || (USE_KNN && USE_SURF) \
    || (USE_KNN && USE_BRIEF_ONLY) || (USE_KNN && USE_WEIGHTS) || (USE_KNN && USE_AKAZE)
#error only one kp-algortihm may be active
#endif

#if !USE_ORB && !USE_SIFT && !USE_SURF && !USE_BRIEF_ONLY && !USE_WEIGHTS && !USE_AKAZE && !USE_KNN
#error please select a kp-algorithm
#endif


#if USE_DYNAMIC_BB
#undef T_NUM_OBJ_DESC
#define T_NUM_OBJ_DESC              2
#endif /*USE_DYNAMIC_ORB*/


#define EDVS_ON_EVT                 1
#define EDVS_OFF_EVT                -1

#endif // SETTINGS

