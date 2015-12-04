
#include "./batgauge_edison.h"

int main(int argc, char** argv) {
  batgauge_edison bat;

  printf("Battery Voltage: %f V\n", bat.getVCell());

  printf("State of Charge: %d %%\n", bat.getSoC());

  printf("MAX17043 Version: %d \n", bat.getVersion());

  printf("Current Threshold: %d %%\n", bat.getAlertThreshold());

  bat.setAlertThreshold(10);

  printf("New Set Threshold: %d %%\n", bat.getAlertThreshold());

  if (bat.getAlertStatus())
    printf("Alert is triggered\n");
  else
    printf("Alert not triggered\n");

}

