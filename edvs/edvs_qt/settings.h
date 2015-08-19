#ifndef SETTINGS
#define SETTINGS


#define DEBUG               1
#define DISPLAY_ONLY        0

#define ONLINE              0
#define OFFLINE_FILE        "../edvs_frames02.dat"
#define EDVS_OUT_FILE       "edvs_frames.dat"


/*---Tracker---*/
#define T_WINDOW_NAME       "tracker window"
#define T_NUM_OBJ_DESC      5
#define T_MIN_MATCHES       15



/*---MainWindow---*/
#define DATA_LEN            128*128
#define UPDATE_INTERVAL     20
#define FRAME_WIDTH         400
#define FRAME_HEIGHT        400



#if DISPLAY_ONLY
#undef DEBUG
#define DEBUG               0
#endif


#endif // SETTINGS

