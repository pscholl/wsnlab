#ifndef animation_h
#define animation_h

#include <map>
#include <string>
#include <fstream>
#include <chrono>

#include <signal.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <vector>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <chrono>
#include "./display_edison.h"

#include "./platypus.h"
#include "./imu_edison.h"
#include "./batgauge_edison.h"

//---------------------------- defines -----------------------------------

#define x_offset_default 34     // = 96/2 - 15 48
#define y_offset_default 40

#define x_sep_line1_start_default 0
#define y_sep_line1_start_default 19
#define x_sep_line1_stop_default 95
#define y_sep_line1_stop_default 19

#define x_sep_line2_start_default 87
#define y_sep_line2_start_default 0
#define x_sep_line2_stop_default 87
#define y_sep_line2_stop_default 9

#define x_sep_line3_start_default 87
#define y_sep_line3_start_default 9
#define x_sep_line3_stop_default 95
#define y_sep_line3_stop_default 9

#define x_text_1_default 1
#define y_text_1_default 1
#define x_text_2_default 1
#define y_text_2_default 10

#define x_arrow_left_1_default 3
#define y_arrow_left_1_default 24
#define x_arrow_left_2_default 13
#define y_arrow_left_2_default 21
#define x_arrow_left_3_default 8
#define y_arrow_left_3_default 27

#define x_arrow_right_1_default 92
#define y_arrow_right_1_default 24
#define x_arrow_right_2_default 82
#define y_arrow_right_2_default 21
#define x_arrow_right_3_default 87
#define y_arrow_right_3_default 27

//Strichmann Grundfigur
#define Kopf_size 7
#define x_Genick_default 15
#define y_Genick_default 10
#define x_Brust_default 15
#define y_Brust_default 20
#define x_Schritt_default 15
#define y_Schritt_default 30

#define x_Knie_rechts_default 25
#define y_Knie_rechts_default 40
#define x_Knie_links_default 5
#define y_Knie_links_default 40

#define x_Ferse_links_default 5
#define y_Ferse_links_default 50
#define x_Ferse_rechts_default 25
#define y_Ferse_rechts_default 50

#define x_Schuh_links_default 0
#define y_Schuh_links_default 50
#define x_Schuh_rechts_default 30
#define y_Schuh_rechts_default 50

#define x_Elle_links_default 5
#define y_Elle_links_default 20
#define x_Elle_rechts_default 25
#define y_Elle_rechts_default 20

#define x_Arm_links_default 0
#define y_Arm_links_default 15
#define x_Arm_rechts_default 30
#define y_Arm_rechts_default 15

//---------------------------- / defines ---------------------------------

class animation {
 public:
    void update_stickman();
    void init_drawing();
    void drawOnScreen();
    void update_clientID(int client);
    void draw_X(int Dicke);
    void resetStickman();
    void draw_found();
    void draw_default();

    void update_text1(char *text);
    void update_text2(char *text);

    void wave_left(int times);
    void wave_right(int times);

    void shift_right(int shift, int step);
    void shift_left(int shift, int step);
    void shift_up(int shift, int step);
    void shift_down(int shift, int step);
    void draw_empty();

    void leave_display_right(int step);
    void leave_display_left(int step);

    void join_display_from_left(int step);
    void join_display_from_right(int step);

    void clearScreen();
    void closeScreen();

 private:

};

#endif // animation_h

