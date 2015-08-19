#ifndef SETTINGS
#define SETTINGS


#define DEBUG                       1
#define DISPLAY_ONLY                0
#define SLIDING_FRAMES              0
#define ONLINE                      1
#define USE_SPHERO                  1



/*---I/O---*/
#define OFFLINE_FILE                "../edvs_frames02.dat"
#define EDVS_OUT_FILE               "edvs_frames.dat"


/*---Tracker---*/
#define T_WINDOW_NAME               "tracker window"
#define T_NUM_OBJ_DESC              5
#define T_MIN_MATCHES               10



/*---MainWindow---*/
#define DISPLAY_FRAME_WIDTH         400
#define DISPLAY_FRAME_HEIGHT        400



/*---Frame Config---*/
#define UPDATE_INTERVAL             20
#define DATA_LEN                    128*128
#define FRAME_OVERLAPPING           10



/*---Sphero---*/
#define SPHERO_MAC                  "00:06:66:44:64:F7"



#if DISPLAY_ONLY
#undef DEBUG
#define DEBUG                       0
#endif

#if FRAME_OVERLAPPING >= UPDATE_INTERVAL
#error FRAME_OVERLAPPING must be less than UPDATE_INTERVAL
#endif

#endif // SETTINGS

