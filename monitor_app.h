#ifndef monitor_app_h
#define monitor_app_h
//
#define BUFF_SIZE (1024 * 10)
#define CMD_FILE_PRE "-f"
#define CMD_BAUD_PRE "-b"
#define CMD_MONITOR_BUFFER "-a"
#define CMD_MONITOR_HELP "-h"
// additional function cmd
#define CMD_STA_STR "-m start"
#define CMD_CLS_STR "-m stop"
#define CMD_DIS_R_STR "-r dis"
#define CMD_DIS_W_STR "-w dis"
#define CMD_ENA_R_STR "-r ena"
#define CMD_ENA_W_STR "-w ena"
#define CMD_R_LINE_STR "-r line"
#define CMD_EXIT_R_L_STR_1 "q"
#define CMD_EXIT_R_L_STR_2 "e"

//
#define CMD_STA_MO ((int)(1 << 1))
#define CMD_CLS_MO ((int)(1 << 2))
#define CMD_R_LINE ((int)(1 << 3))
#define CMD_DIS_R ((int)(1 << 4))
#define CMD_ENA_R ((int)(1 << 5))
#define CMD_DIS_W ((int)(1 << 6))
#define CMD_ENA_W ((int)(1 << 7))
#define CMD_HELPER ((int)(1 << 8))
//
#endif