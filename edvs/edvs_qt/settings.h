#ifndef SETTINGS
#define SETTINGS


#define DEBUG                       1
#define DISPLAY_ONLY                0
#define SLIDING_FRAMES              0
#define ONLINE                      0
#define USE_SPHERO                  0



/*---I/O---*/
#define OFFLINE_FILE                "../edvs_frames02.dat"
#define EDVS_OUT_FILE               "edvs_frames.dat"


/*---Tracker---*/
#define T_WINDOW_NAME               "tracker window"
#define T_NUM_OBJ_DESC              5
#define T_MIN_MATCHES               5
#define USE_ORB                     0
#define USE_SIFT                    1
#define USE_SURF                    0



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

#if FRAME_OVERLAPPING >= UPDATE_INTERVAL
#error FRAME_OVERLAPPING must be less than UPDATE_INTERVAL
#endif


#if (USE_ORB && USE_SIFT) || (USE_ORB && USE_SURF) || (USE_SIFT && USE_SURF)
#error only one kp-algortihm may be active
#endif

#if !USE_ORB && !USE_SIFT && !USE_SURF
#error please select a kp-algorithm
#endif

#endif // SETTINGS

