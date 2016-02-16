// Author: Stephan Stegmeir
#include "./animation.h"

double refreshtime = 100000;

// Node #
// Todo: ClientID itoa node
char node[10] = "?";
int x_node = 89;
int y_node = 1;

// Textfelder
char *text_1 = "";
char *text_2 = "";

// Pfeile
bool arrow_left = false;
bool arrow_right = false;

bool init_state = false;
bool x_draw_state = false;
bool drawStickmanOnScreen = false;
bool sep_lines = true;

int thickness = 0;

bool stickmanOnScreen = false;

// Initialisierung der Offsetwerte
//x bzw. y - Achsenverschiebung
int x_offset = x_offset_default;
int y_offset = y_offset_default;
int x_offset_new = 0;
int y_offset_new = 0;

//Oberkörper
int x_Genick = x_Genick_default + x_offset;
int y_Genick = y_Genick_default + y_offset;
int x_Brust = x_Brust_default + x_offset;
int y_Brust = y_Brust_default + y_offset;
int x_Schritt = x_Schritt_default + x_offset;
int y_Schritt = y_Schritt_default + y_offset;

//Unterkörper
int x_Knie_rechts = x_Knie_rechts_default + x_offset;
int y_Knie_rechts = y_Knie_rechts_default + y_offset;
int x_Knie_links = x_Knie_links_default + x_offset;
int y_Knie_links = y_Knie_links_default + y_offset;

int x_Ferse_links = x_Ferse_links_default + x_offset;
int y_Ferse_links = y_Ferse_links_default + y_offset;
int x_Ferse_rechts = x_Ferse_rechts_default + x_offset;
int y_Ferse_rechts = y_Ferse_rechts_default + y_offset;

int x_Schuh_links = x_Schuh_links_default + x_offset;
int y_Schuh_links = y_Schuh_links_default + y_offset;
int x_Schuh_rechts = x_Schuh_rechts_default + x_offset;
int y_Schuh_rechts = y_Schuh_rechts_default + y_offset;

//Arme
int x_Elle_links = x_Elle_links_default + x_offset;
int y_Elle_links = y_Elle_links_default + y_offset;
int x_Elle_rechts = x_Elle_rechts_default + x_offset;
int y_Elle_rechts = y_Elle_rechts_default + y_offset;

int x_Arm_links = x_Arm_links_default + x_offset;
int y_Arm_links = y_Arm_links_default + y_offset;
int x_Arm_rechts = x_Arm_rechts_default + x_offset;
int y_Arm_rechts = y_Arm_rechts_default + y_offset;

int x_Arm_links_new = 0;
int y_Arm_links_new = 0;
int x_Arm_rechts_new = 0;
int y_Arm_rechts_new = 0;

// Textfelder
int x_text_1 = x_text_1_default;
int y_text_1 = y_text_1_default;
int x_text_2 = x_text_2_default;
int y_text_2 = y_text_2_default;

// arrow left
int x_arrow_left_1 = x_arrow_left_1_default;
int y_arrow_left_1 = y_arrow_left_1_default;
int x_arrow_left_2 = x_arrow_left_2_default;
int y_arrow_left_2 = y_arrow_left_2_default;
int x_arrow_left_3 = x_arrow_left_3_default;
int y_arrow_left_3 = y_arrow_left_3_default;

// arrow right
int x_arrow_right_1 = x_arrow_right_1_default;
int y_arrow_right_1 = y_arrow_right_1_default;
int x_arrow_right_2 = x_arrow_right_2_default;
int y_arrow_right_2 = y_arrow_right_2_default;
int x_arrow_right_3 = x_arrow_right_3_default;
int y_arrow_right_3 = y_arrow_right_3_default;

// Separationlines
int x_sep_line1_start = x_sep_line1_start_default;
int y_sep_line1_start = y_sep_line1_start_default;
int x_sep_line1_stop = x_sep_line1_stop_default;
int y_sep_line1_stop = y_sep_line1_stop_default;

int x_sep_line2_start = x_sep_line2_start_default;
int y_sep_line2_start = y_sep_line2_start_default;
int x_sep_line2_stop = x_sep_line2_stop_default;
int y_sep_line2_stop = y_sep_line2_stop_default;

int x_sep_line3_start = x_sep_line3_start_default;
int y_sep_line3_start = y_sep_line3_start_default;
int x_sep_line3_stop = x_sep_line3_stop_default;
int y_sep_line3_stop = y_sep_line3_stop_default;


//---------------------------- Funktionen -------------------------------------

void animation::closeScreen() {
  text_1 = "Abgemeldet";
  text_2 = "";
  init_state = false;
  x_draw_state = false;
  drawStickmanOnScreen = false;
  sep_lines = false;
  drawOnScreen();
}

void animation::update_stickman() {
  //Updaten des Strichmännchens:
  init_drawing();
  // Oberkörper
  x_Genick = x_Genick + x_offset_new;
  y_Genick = y_Genick + y_offset_new;
  x_Brust = x_Brust + x_offset_new;
  y_Brust = y_Brust + y_offset_new;
  x_Schritt = x_Schritt + x_offset_new;
  y_Schritt = y_Schritt + y_offset_new;

  // Unterkörper
  x_Knie_rechts = x_Knie_rechts + x_offset_new;
  y_Knie_rechts = y_Knie_rechts + y_offset_new;
  x_Knie_links = x_Knie_links + x_offset_new;
  y_Knie_links = y_Knie_links + y_offset_new;

  x_Ferse_links = x_Ferse_links + x_offset_new;
  y_Ferse_links = y_Ferse_links + y_offset_new;
  x_Ferse_rechts = x_Ferse_rechts + x_offset_new;
  y_Ferse_rechts = y_Ferse_rechts + y_offset_new;

  x_Schuh_links = x_Schuh_links + x_offset_new;
  y_Schuh_links = y_Schuh_links + y_offset_new;
  x_Schuh_rechts = x_Schuh_rechts + x_offset_new;
  y_Schuh_rechts = y_Schuh_rechts + y_offset_new;

  // Arme
  x_Elle_links = x_Elle_links + x_offset_new;
  y_Elle_links = y_Elle_links + y_offset_new;
  x_Elle_rechts = x_Elle_rechts + x_offset_new;
  y_Elle_rechts = y_Elle_rechts + y_offset_new;

  x_Arm_links = x_Arm_links + x_Arm_links_new + x_offset_new;
  y_Arm_links = y_Arm_links + y_Arm_links_new + y_offset_new;
  x_Arm_rechts = x_Arm_rechts + x_Arm_rechts_new + x_offset_new;
  y_Arm_rechts = y_Arm_rechts + y_Arm_rechts_new + y_offset_new;
}

void animation::init_drawing() {
//  int i = 0;
  if(init_state == false){
//    i++;
    HAL_LCD_initDisplay();
//    printf("initialisierung %d",i);
    init_state = true;
  }
}

void animation::update_clientID(int client) {
  init_drawing();
  sprintf(node, "%d", client);
  drawOnScreen();
  printf("[Display] setze ClientID auf %d\n",client);
}

void animation::update_text1(char *text) {
  init_drawing();
  text_1 = text;
  drawOnScreen();
  printf("[Display] schreibe in Textfeld 1 %s\n",text);
}

void animation::update_text2(char *text) {
  init_drawing();
  text_2 = text;
  drawOnScreen();
  printf("[Display] schreibe in Textfeld 2 %s\n",text);
}

void animation::clearScreen() {
  init_drawing();
  sep_lines = true;
  if (stickmanOnScreen == true) {
    printf("[Display] Stickman ist gerade auf dem Bildschirm\n");
  } else {
    printf("[Display] Stickman ist NICHT auf dem Bildschirm\n");
  }
  draw_empty();
}

void animation::draw_empty() {
  init_drawing();
  text_1 = "";
  text_2 = "";
  drawStickmanOnScreen = false;
  x_draw_state = false;
  sep_lines = true;
  drawOnScreen();
}

void animation::draw_default() {
  init_drawing();
  resetStickman();
  drawOnScreen();
}

void animation::draw_found() {
    init_drawing();
    resetStickman();
    drawOnScreen();
    printf("[Display] zeichne Bildschirm: Stickman found\n");
}

void animation::draw_X(int Dicke) {
  thickness = Dicke;
  init_drawing();
  draw_empty();
  x_draw_state = true;
  drawOnScreen();
  printf("[Display] zeige ein Kreuz der Dicke %d\n",Dicke);
}

void animation::wave_left(int times){

    x_offset_new = 0;
    y_offset_new = 0;
    for(int i=0; i<times;i++){
        x_Arm_links_new = 7;
        update_stickman();
        drawOnScreen();
        x_Arm_links_new = -7;
        update_stickman();
        drawOnScreen();
    }
    x_Arm_links_new = 0;
    update_stickman();
    drawOnScreen();
    printf("[Display] Strichmann winkt links\n");
}

void animation::wave_right(int times){

    x_offset_new = 0;
    y_offset_new = 0;
    for(int i=0; i<times;i++){
        x_Arm_rechts_new = -7;
        update_stickman();
        drawOnScreen();
        x_Arm_rechts_new = 7;
        update_stickman();
        drawOnScreen();
    }
    x_Arm_rechts_new = 0;
    update_stickman();
    drawOnScreen();
    printf("[Display] Strichmann winkt rechts\n");
}

void animation::shift_right(int shift, int step){

  while(shift>=0){

    if(step<shift){
      x_offset_new = step;
    }else{
      x_offset_new = shift;
    }
    shift = shift - step;
    update_stickman();
    drawOnScreen();
  }
  printf("[Display] Strichmann springt %i Pixel nach rechts in %i er Schritten\n", shift, step);
}

void animation::shift_left(int shift, int step) {

  while(shift>=0){

    if(step<shift){
      x_offset_new = -step;
    }else{
      x_offset_new = shift;
    }
    shift = shift - step;
    update_stickman();
    drawOnScreen();
  }
  printf("[Display] Strichmann springt %i Pixel nach links in %i er Schritten\n", shift, step);
}

void animation::shift_up(int shift, int step) {

  while(shift>=0){

    if(step<shift){
      y_offset_new = -step;
    }else{
      y_offset_new = shift;
    }
    shift = shift - step;
    update_stickman();
    drawOnScreen();
  }
  printf("[Display] Strichmann springt %i Pixel nach oben in %i er Schritten\n", shift, step);
}

void animation::shift_down(int shift, int step) {

  while(shift>=0){

    if(step<shift){
      y_offset_new = step;
    }else{
      y_offset_new = shift;
    }
    shift = shift - step;
    update_stickman();
    drawOnScreen();
  }
  printf("[Display] Strichmann springt %i Pixel nach unten in %i er Schritten\n", shift, step);
}

void animation::leave_display_right(int step) {

    text_1 = "Leaving Node";
    text_2 = "rechts";
    while(x_Schuh_links<96){
        if (arrow_right == false) arrow_right = true;
        if (arrow_right == true) arrow_right = false;
        x_offset_new = step;
        update_stickman();
        drawOnScreen();
    }

    text_1 = "Left Node";
    text_2 = "I am in Node 3"; // insert Node Number
    drawOnScreen();
    printf("[Display] Strichmann verlässt Bildschirm in Richtung rechts\n");
}

void animation::leave_display_left(int step) {

  text_1 = "Leaving Node";
  text_2 = "links";
  while(x_Schuh_rechts>-1){
    if (arrow_left == false) arrow_left = true;
    if (arrow_left == true) arrow_left = false;
    x_offset_new = -step;
    update_stickman();
    drawOnScreen();
  }

  text_1 = "Left Node";
  text_2 = "I am in Node 3"; // insert Node Number
  drawOnScreen();
  printf("[Display] Strichmann verlässt Bildschirm in Richtung links\n");
}

void animation::join_display_from_left(int step) {

  resetStickman();
  // setzt Stickmann an den linken Bildschirmrand
  x_offset_new = -78;
  text_1 = "Coming from";
  text_2 = "Node 2 left";  // insert Node Number
  update_stickman();
  drawOnScreen();
  while(x_Brust<48){
    if (arrow_left == false) arrow_left = true;
    if (arrow_left == true) arrow_left = false;
    x_offset_new = step;
    update_stickman();
    drawOnScreen();
  }

  text_1 = "Arrived";
  text_2 = ""; // insert Node Number
  drawOnScreen();
  printf("[Display] Strichmann kommt in den Bildschirm von links\n");
}

void animation::join_display_from_right(int step) {

    resetStickman();
    x_offset_new = 48;  // setzt Stickmann an den rechten Bildschirmrand
    text_1 = "Coming from";
    text_2 = "Node 2 right";  // insert Node Number
    update_stickman();
    drawOnScreen();
    while(x_Brust>48){
        if (arrow_right == false) arrow_right = true;
        if (arrow_right == true) arrow_right = false;
        x_offset_new = -step;
        update_stickman();
        drawOnScreen();
    }

    text_1 = "Arrived";
    text_2 = ""; // insert Node Number
    drawOnScreen();
    printf("[Display] Strichmann kommt in den Bildschirm von rechts\n");
}

void animation::resetStickman() {

  drawStickmanOnScreen = true;
  x_draw_state = false;
  sep_lines = true;

  x_offset = x_offset_default;
  y_offset = y_offset_default;

  x_Genick = x_Genick_default + x_offset;
  y_Genick = y_Genick_default + y_offset;
  x_Brust = x_Brust_default + x_offset;
  y_Brust = y_Brust_default + y_offset;
  x_Schritt = x_Schritt_default + x_offset;
  y_Schritt = y_Schritt_default + y_offset;

  //Unterkörper
  x_Knie_rechts = x_Knie_rechts_default + x_offset;
  y_Knie_rechts = y_Knie_rechts_default + y_offset;
  x_Knie_links = x_Knie_links_default + x_offset;
  y_Knie_links = y_Knie_links_default + y_offset;

  x_Ferse_links = x_Ferse_links_default + x_offset;
  y_Ferse_links = y_Ferse_links_default + y_offset;
  x_Ferse_rechts = x_Ferse_rechts_default + x_offset;
  y_Ferse_rechts = y_Ferse_rechts_default + y_offset;

  x_Schuh_links = x_Schuh_links_default + x_offset;
  y_Schuh_links = y_Schuh_links_default + y_offset;
  x_Schuh_rechts = x_Schuh_rechts_default + x_offset;
  y_Schuh_rechts = y_Schuh_rechts_default + y_offset;

  //Arme
  x_Elle_links = x_Elle_links_default + x_offset;
  y_Elle_links = y_Elle_links_default + y_offset;
  x_Elle_rechts = x_Elle_rechts_default + x_offset;
  y_Elle_rechts = y_Elle_rechts_default + y_offset;

  x_Arm_links = x_Arm_links_default + x_offset;
  y_Arm_links = y_Arm_links_default + y_offset;
  x_Arm_rechts = x_Arm_rechts_default + x_offset;
  y_Arm_rechts = y_Arm_rechts_default + y_offset;

  x_Arm_links_new = 0;
  y_Arm_links_new = 0;
  x_Arm_rechts_new = 0;
  y_Arm_rechts_new = 0;

  // textfelder
  x_text_1 = x_text_1_default;
  y_text_1 = y_text_1_default;
  x_text_2 = x_text_2_default;
  y_text_2 = y_text_2_default;

  // arrow left
  x_arrow_left_1 = x_arrow_left_1_default;
  y_arrow_left_1 = y_arrow_left_1_default;
  x_arrow_left_2 = x_arrow_left_2_default;
  y_arrow_left_2 = y_arrow_left_2_default;
  x_arrow_left_3 = x_arrow_left_3_default;
  y_arrow_left_3 = y_arrow_left_3_default;

  // arrow right
  x_arrow_right_1 = x_arrow_right_1_default;
  y_arrow_right_1 = y_arrow_right_1_default;
  x_arrow_right_2 = x_arrow_right_2_default;
  y_arrow_right_2 = y_arrow_right_2_default;
  x_arrow_right_3 = x_arrow_right_3_default;
  y_arrow_right_3 = y_arrow_right_3_default;

  // Separationlines
  x_sep_line1_start = x_sep_line1_start_default;
  y_sep_line1_start = y_sep_line1_start_default;
  x_sep_line1_stop = x_sep_line1_stop_default;
  y_sep_line1_stop = y_sep_line1_stop_default;

  x_sep_line2_start = x_sep_line2_start_default;
  y_sep_line2_start = y_sep_line2_start_default;
  x_sep_line2_stop = x_sep_line2_stop_default;
  y_sep_line2_stop = y_sep_line2_stop_default;

  x_sep_line3_start = x_sep_line3_start_default;
  y_sep_line3_start = y_sep_line3_start_default;
  x_sep_line3_stop = x_sep_line3_stop_default;
  y_sep_line3_stop = y_sep_line3_stop_default;
}

void animation::drawOnScreen() {

  usleep(refreshtime);
  init_drawing();
  tContext g_sContext;
  Graphics_initContext(&g_sContext, &g_sharp96x96LCD);
  Graphics_setForegroundColor(&g_sContext, ClrBlack);
  Graphics_setBackgroundColor(&g_sContext, ClrWhite);
  Graphics_setFont(&g_sContext, &g_sFontFixed6x8);
  Graphics_clearDisplay(&g_sContext);

  // Pfeile
  if(arrow_left == true) {
    Graphics_drawLine(&g_sContext,x_arrow_left_1,y_arrow_left_1,x_arrow_left_2,y_arrow_left_1);
    Graphics_drawLine(&g_sContext,x_arrow_left_1,y_arrow_left_1,x_arrow_left_3,y_arrow_left_3);
    Graphics_drawLine(&g_sContext,x_arrow_left_1,y_arrow_left_1,x_arrow_left_3,y_arrow_left_2);
  }
  if(arrow_right == true) {
    Graphics_drawLine(&g_sContext,x_arrow_right_1,y_arrow_right_1,x_arrow_right_2,y_arrow_right_1);
    Graphics_drawLine(&g_sContext,x_arrow_right_1,y_arrow_right_1,x_arrow_right_3,y_arrow_right_3);
    Graphics_drawLine(&g_sContext,x_arrow_right_1,y_arrow_right_1,x_arrow_right_3,y_arrow_right_2);
  }

  if(x_draw_state == true && drawStickmanOnScreen == false) {
    for(int i=0;i<thickness;i++){
    Graphics_drawLine(&g_sContext,47-20+i,57-20,47+20,57+20-i);
    Graphics_drawLine(&g_sContext,47+20-i,57-20,47-20,57+20-i);
    }
  }

  // Textfelder
  Graphics_drawString(&g_sContext, text_1, 14, x_text_1, y_text_1, OPAQUE_TEXT); //Textfeld1
  Graphics_drawString(&g_sContext, text_2, 14, x_text_2, y_text_2, OPAQUE_TEXT); //Textfeld2
  Graphics_drawString(&g_sContext, node, 1, x_node, y_node, OPAQUE_TEXT); //Knotennummer

  // Separationslinien
  Graphics_drawLine(&g_sContext,x_sep_line2_start,y_sep_line2_start,x_sep_line2_stop,y_sep_line2_stop);
  Graphics_drawLine(&g_sContext,x_sep_line3_start,y_sep_line3_start,x_sep_line3_stop,y_sep_line3_stop);

  if(sep_lines == true) {
  Graphics_drawLine(&g_sContext,x_sep_line1_start,y_sep_line1_start,x_sep_line1_stop,y_sep_line1_stop);
  }

  if(drawStickmanOnScreen == true) {
    //Male das neue Strichmaenchen
    //Kopf
    Graphics_drawCircle(&g_sContext, x_Genick, y_Genick - Kopf_size, Kopf_size); //Koerper
    Graphics_drawLine(&g_sContext,x_Brust,y_Brust,x_Schritt,y_Schritt); //Oberkoerper
    Graphics_drawLine(&g_sContext,x_Genick,y_Genick,x_Brust,y_Brust); //Hals
    //Beine links
    Graphics_drawLine(&g_sContext,x_Schritt,y_Schritt,x_Knie_links,y_Knie_links); //linker Oberschenkel
    Graphics_drawLine(&g_sContext,x_Knie_links,y_Knie_links,x_Ferse_links,y_Ferse_links); //linker Unterschenkel
    Graphics_drawLine(&g_sContext,x_Ferse_links,y_Ferse_links,x_Schuh_links,y_Schuh_links); //linker Fuﬂ
    //Beine rechts
    Graphics_drawLine(&g_sContext,x_Schritt,y_Schritt,x_Knie_rechts,y_Knie_rechts); //rechter Oberschenkel
    Graphics_drawLine(&g_sContext,x_Knie_rechts,y_Knie_rechts,x_Ferse_rechts,y_Ferse_rechts); //rechter Unterschenkel
    Graphics_drawLine(&g_sContext,x_Ferse_rechts,y_Ferse_rechts,x_Schuh_rechts,y_Schuh_rechts); //rechter Fuﬂ
    //Arme
    Graphics_drawLine(&g_sContext,x_Brust,y_Brust,x_Elle_links,y_Elle_links); //linker Oberarm
    Graphics_drawLine(&g_sContext,x_Elle_links,y_Elle_links,x_Arm_links,y_Arm_links); //linker Unterarm
    Graphics_drawLine(&g_sContext,x_Brust,y_Brust,x_Elle_rechts,y_Elle_rechts); //rechter Oberarm
    Graphics_drawLine(&g_sContext,x_Elle_rechts,y_Elle_rechts,x_Arm_rechts,y_Arm_rechts); //rechter Unterarm
    stickmanOnScreen = true;
  }
  Graphics_flushBuffer(&g_sContext);

  //    printf("[Display] Zeichne...\n");

  //GRUNDBFEHLE ZUM MALEN
  //Graphics_drawLine(const Graphics_Context *context, int32_t  x1, int32_t  y1, int32_t  x2, int32_t  y2);
  //Graphics_drawLine(&g_sContext,1,1,126,126);

  //horizontal: DrawLineH: (const Graphics_Context *context, int32_t  x1, int32_t  x2, int32_t  y);
  //Graphics_drawLineH(&g_sContext,10,90,30);

  //vertikal: drawLineV(const Graphics_Context *context, int32_t  x, int32_t  y1, int32_t  y2);
  //Graphics_drawLineV(&g_sContext,63,108,70);

  //Kreis: Graphics_drawCircle(const Graphics_Context *context, int32_t  x, int32_t  y, int32_t  lRadius);
  //Graphics_drawCircle(&g_sContext, 20, 10, 10);

  //Graphics_drawString(&g_sContext, "Wie geht es dir?", 16, 10, 35, OPAQUE_TEXT);
  //void Graphics_drawString(const Graphics_Context *context, char *string, int32_t  length, int32_t  x, int32_t  y, bool  opaque)
}







//// Init für Drehen Display
//
//    int x_y_swap = 1;
//    int x_y_dont_swap = 0;
//    int x_conv = 1;
//    int y_conv = 0;
//

//void drehenDisplay(){
//
//// TODO !!!!!
//    // Textfeld
//    int x_text_1_new = x_y_swap*abs(y_text_1 - 95*y_conv) + x_y_dont_swap*abs(x_text_1 - 95*x_conv);
//    int y_text_1_new = x_y_swap*abs(x_text_1 - 95*x_conv) + x_y_dont_swap*abs(y_text_1 - 95*y_conv);
//    int x_text_2_new = x_y_swap*abs(y_text_2 - 95*y_conv) + x_y_dont_swap*abs(x_text_2 - 95*x_conv);
//    int y_text_2_new = x_y_swap*abs(x_text_2 - 95*x_conv) + x_y_dont_swap*abs(y_text_2 - 95*y_conv);
//
//    // arrow left
//    int x_arrow_left_1_new = x_y_swap*abs(y_arrow_left_1 - 95*y_conv) + x_y_dont_swap*abs(x_arrow_left_1 - 95*x_conv);
//    int y_arrow_left_1_new = x_y_swap*abs(x_arrow_left_1 - 95*x_conv) + x_y_dont_swap*abs(y_arrow_left_1 - 95*y_conv);
//
//    int x_arrow_left_2_new = x_y_swap*abs(y_arrow_left_2 - 95*y_conv) + x_y_dont_swap*abs(x_arrow_left_2 - 95*x_conv);
//    int y_arrow_left_2_new = x_y_swap*abs(x_arrow_left_2 - 95*x_conv) + x_y_dont_swap*abs(y_arrow_left_2 - 95*y_conv);
//
//    int x_arrow_left_3_new = x_y_swap*abs(y_arrow_left_3 - 95*y_conv) + x_y_dont_swap*abs(x_arrow_left_3 - 95*x_conv);
//    int y_arrow_left_3_new = x_y_swap*abs(x_arrow_left_3 - 95*x_conv) + x_y_dont_swap*abs(y_arrow_left_3 - 95*y_conv);
//
//    // arrow right
//    int x_arrow_right_1_new = x_y_swap*abs(y_arrow_right_1 - 95*y_conv) + x_y_dont_swap*abs(x_arrow_right_1 - 95*x_conv);
//    int y_arrow_right_1_new = x_y_swap*abs(x_arrow_right_1 - 95*x_conv) + x_y_dont_swap*abs(y_arrow_right_1 - 95*y_conv);
//
//    int x_arrow_right_2_new = x_y_swap*abs(y_arrow_right_2 - 95*y_conv) + x_y_dont_swap*abs(x_arrow_right_2 - 95*x_conv);
//    int y_arrow_right_2_new = x_y_swap*abs(x_arrow_right_2 - 95*x_conv) + x_y_dont_swap*abs(y_arrow_right_2 - 95*y_conv);
//
//    int x_arrow_right_3_new = x_y_swap*abs(y_arrow_right_3 - 95*y_conv) + x_y_dont_swap*abs(x_arrow_right_3 - 95*x_conv);
//    int y_arrow_right_3_new = x_y_swap*abs(x_arrow_right_3 - 95*x_conv) + x_y_dont_swap*abs(y_arrow_right_3 - 95*y_conv);
//
//
//    // Separationlines
//    int x_sep_line1_start_new = x_y_swap*abs(y_sep_line1_start - 95*y_conv) + x_y_dont_swap*abs(x_sep_line1_start - 95*x_conv);
//    int y_sep_line1_start_new = x_y_swap*abs(x_sep_line1_start - 95*x_conv) + x_y_dont_swap*abs(y_sep_line1_start - 95*y_conv);
//
//    int x_sep_line1_stop_new = x_y_swap*abs(y_sep_line1_stop - 95*y_conv) + x_y_dont_swap*abs(x_sep_line1_stop - 95*x_conv);
//    int y_sep_line1_stop_new = x_y_swap*abs(x_sep_line1_stop - 95*x_conv) + x_y_dont_swap*abs(y_sep_line1_stop - 95*y_conv);
//
//    int x_sep_line2_start_new = x_y_swap*abs(y_sep_line2_start - 95*y_conv) + x_y_dont_swap*abs(x_sep_line2_start - 95*x_conv);
//    int y_sep_line2_start_new = x_y_swap*abs(x_sep_line2_start - 95*x_conv) + x_y_dont_swap*abs(y_sep_line2_start - 95*y_conv);
//
//    int x_sep_line2_stop_new = x_y_swap*abs(y_sep_line2_stop - 95*y_conv) + x_y_dont_swap*abs(x_sep_line2_stop - 95*x_conv);
//    int y_sep_line2_stop_new = x_y_swap*abs(x_sep_line2_stop - 95*x_conv) + x_y_dont_swap*abs(y_sep_line2_stop - 95*y_conv);
//
//    int x_sep_line3_start_new = x_y_swap*abs(y_sep_line3_start - 95*y_conv) + x_y_dont_swap*abs(x_sep_line3_start - 95*x_conv);
//    int y_sep_line3_start_new = x_y_swap*abs(x_sep_line3_start - 95*x_conv) + x_y_dont_swap*abs(y_sep_line3_start - 95*y_conv);
//
//    int x_sep_line3_stop_new = x_y_swap*abs(y_sep_line3_stop - 95*y_conv) + x_y_dont_swap*abs(x_sep_line3_stop - 95*x_conv);
//    int y_sep_line3_stop_new = x_y_swap*abs(x_sep_line3_stop - 95*x_conv) + x_y_dont_swap*abs(y_sep_line3_stop - 95*y_conv);
//
//}
//
//int angle = 0;
//void parse_angle(){
//
//    if (angle == 0) {
//        x_y_swap = 0;
//        x_y_dont_swap = 1;
//        x_conv = 0;
//        y_conv = 0;
//        printf("Display Winkel: 0°\n");
//    }
//    if (angle == 90) {
//        x_y_swap = 1;
//        x_y_dont_swap = 0;
//        x_conv = 0;
//        y_conv = 1;
//        printf("Display Winkel: 90°\n");
//    }
//    if (angle == 180) {
//        x_y_swap = 0;
//        x_y_dont_swap = 1;
//        x_conv = 1;
//        y_conv = 1;
//        printf("Display Winkel: 180°\n");
//    }
//    if (angle == 270) {
//        x_y_swap = 1;
//        x_y_dont_swap = 0;
//        x_conv = 1;
//        y_conv = 0;
//        printf("Display Winkel: 270°\n");
//    }
//}
//
//
//// textfelder
//int x_text_1 = x_y_swap*abs(y_text_1_default - 95*y_conv) + x_y_dont_swap*abs(x_text_1_default - 95*x_conv);
//int y_text_1 = x_y_swap*abs(x_text_1_default - 95*x_conv) + x_y_dont_swap*abs(y_text_1_default - 95*y_conv);
//int x_text_2 = x_y_swap*abs(y_text_2_default - 95*y_conv) + x_y_dont_swap*abs(x_text_2_default - 95*x_conv);
//int y_text_2 = x_y_swap*abs(x_text_2_default - 95*x_conv) + x_y_dont_swap*abs(y_text_2_default - 95*y_conv);
//
//
//// arrow left
//int x_arrow_left_1 = x_y_swap*abs(y_arrow_left_1_default - 95*y_conv) + x_y_dont_swap*abs(x_arrow_left_1_default - 95*x_conv);
//int y_arrow_left_1 = x_y_swap*abs(x_arrow_left_1_default - 95*x_conv) + x_y_dont_swap*abs(y_arrow_left_1_default - 95*y_conv);
//
//int x_arrow_left_2 = x_y_swap*abs(y_arrow_left_2_default - 95*y_conv) + x_y_dont_swap*abs(x_arrow_left_2_default - 95*x_conv);
//int y_arrow_left_2 = x_y_swap*abs(x_arrow_left_2_default - 95*x_conv) + x_y_dont_swap*abs(y_arrow_left_2_default - 95*y_conv);
//
//int x_arrow_left_3 = x_y_swap*abs(y_arrow_left_3_default - 95*y_conv) + x_y_dont_swap*abs(x_arrow_left_3_default - 95*x_conv);
//int y_arrow_left_3 = x_y_swap*abs(x_arrow_left_3_default - 95*x_conv) + x_y_dont_swap*abs(y_arrow_left_3_default - 95*y_conv);
//
//
//// arrow right
//int x_arrow_right_1 = x_y_swap*abs(y_arrow_right_1_default - 95*y_conv) + x_y_dont_swap*abs(x_arrow_right_1_default - 95*x_conv);
//int y_arrow_right_1 = x_y_swap*abs(x_arrow_right_1_default - 95*x_conv) + x_y_dont_swap*abs(y_arrow_right_1_default - 95*y_conv);
//
//int x_arrow_right_2 = x_y_swap*abs(y_arrow_right_2_default - 95*y_conv) + x_y_dont_swap*abs(x_arrow_right_2_default - 95*x_conv);
//int y_arrow_right_2 = x_y_swap*abs(x_arrow_right_2_default - 95*x_conv) + x_y_dont_swap*abs(y_arrow_right_2_default - 95*y_conv);
//
//int x_arrow_right_3 = x_y_swap*abs(y_arrow_right_3_default - 95*y_conv) + x_y_dont_swap*abs(x_arrow_right_3_default - 95*x_conv);
//int y_arrow_right_3 = x_y_swap*abs(x_arrow_right_3_default - 95*x_conv) + x_y_dont_swap*abs(y_arrow_right_3_default - 95*y_conv);
//
//
//// Separationlines
//int x_sep_line1_start = x_y_swap*abs(y_sep_line1_start_default - 95*y_conv) + x_y_dont_swap*abs(x_sep_line1_start_default - 95*x_conv);
//int y_sep_line1_start = x_y_swap*abs(x_sep_line1_start_default - 95*x_conv) + x_y_dont_swap*abs(y_sep_line1_start_default - 95*y_conv);
//
//int x_sep_line1_stop = x_y_swap*abs(y_sep_line1_stop_default - 95*y_conv) + x_y_dont_swap*abs(x_sep_line1_stop_default - 95*x_conv);
//int y_sep_line1_stop = x_y_swap*abs(x_sep_line1_stop_default - 95*x_conv) + x_y_dont_swap*abs(y_sep_line1_stop_default - 95*y_conv);
//
//int x_sep_line2_start = x_y_swap*abs(y_sep_line2_start_default - 95*y_conv) + x_y_dont_swap*abs(x_sep_line2_start_default - 95*x_conv);
//int y_sep_line2_start = x_y_swap*abs(x_sep_line2_start_default - 95*x_conv) + x_y_dont_swap*abs(y_sep_line2_start_default - 95*y_conv);
//
//int x_sep_line2_stop = x_y_swap*abs(y_sep_line2_stop_default - 95*y_conv) + x_y_dont_swap*abs(x_sep_line2_stop_default - 95*x_conv);
//int y_sep_line2_stop = x_y_swap*abs(x_sep_line2_stop_default - 95*x_conv) + x_y_dont_swap*abs(y_sep_line2_stop_default - 95*y_conv);
//
//int x_sep_line3_start = x_y_swap*abs(y_sep_line3_start_default - 95*y_conv) + x_y_dont_swap*abs(x_sep_line3_start_default - 95*x_conv);
//int y_sep_line3_start = x_y_swap*abs(x_sep_line3_start_default - 95*x_conv) + x_y_dont_swap*abs(y_sep_line3_start_default - 95*y_conv);
//
//int x_sep_line3_stop = x_y_swap*abs(y_sep_line3_stop_default - 95*y_conv) + x_y_dont_swap*abs(x_sep_line3_stop_default - 95*x_conv);
//int y_sep_line3_stop = x_y_swap*abs(x_sep_line3_stop_default - 95*x_conv) + x_y_dont_swap*abs(y_sep_line3_stop_default - 95*y_conv);

//// Test Drehen
//    // 0 Grad
//    angle = 0;
//    parse_angle();
//    update_stickman();
//    drawOnScreen();
//    printf("[TEST] 0Grad \n");
//    std::cin.ignore();
//
//    // 90 Grad
//    angle = 90;
//    parse_angle();
//    update_stickman();
//
//    drawOnScreen();
//    printf("[TEST] 90Grad \n");
//    std::cin.ignore();
//
//    // 180 Grad
//    angle = 180;
//    parse_angle();
//    update_stickman();
//    drawOnScreen();
//    update_stickman();
//    drawOnScreen();
//    printf("[TEST] 180Grad \n");
//    std::cin.ignore();
//
//    // 270 Grad
//    angle = 270;
//    parse_angle();
//    update_stickman();
//    drawOnScreen();
//    update_stickman();
//    drawOnScreen();
//    printf("[TEST] 270Grad \n");
//    std::cin.ignore();



// Winken
//
//    printf("[TEST] Strichmann winkt %i mal rechts\n", w_r_times);
//    wave_right(w_r_times);
//    std::cin.ignore();
//    printf("[TEST] Strichmann winkt %i mal links\n", w_l_times);
//    wave_left(w_l_times);
//    std::cin.ignore();


