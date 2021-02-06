#include <iostream>
#include <stdio.h>
#include <string.h>
#include <thread> 
#include <fstream>
#include <linux/input.h>

#include "XRequestManager.h"

#define MOUSEFILE "/dev/input/mice"
#define EVENTFILE "/dev/input/event7"

#define OUTFILE "/home/archie/Dev/C/swipe_gestures/swipe_input"

//The higher this value is, the faster/longer you have to swipe
#define SENSITIVITY 10

bool tripleTap = false;

int fds[2];
char buf[3];

// void switch_desktop(int input, Display *disp) {
//   unsigned long *cur_desktop = NULL;
//   Window root = DefaultRootWindow(disp);
//   cur_desktop = (unsigned long *)get_property(disp, root,
//             XA_CARDINAL, "_NET_CURRENT_DESKTOP", NULL);
// }

void printer(char data[]) {
  printf("%d, %d, %d\n", data[0], data[1], data[2]);
}

int write_output(int info) {
  FILE *fp;
  fp = fopen(OUTFILE, "w");
  int data_to_write = 0;

  if(info > SENSITIVITY){
    std::cout << info << std::endl;
    data_to_write = 1;
  }
  else if(info < -SENSITIVITY) {
    std::cout << info << std::endl;
    data_to_write = -1;
  }


  fprintf(fp, "%d", data_to_write);
  fclose(fp);
  return 0;
}

int nextDesktop(std::unique_ptr<XRequestManager> &xrm) {
  int current_desktop = xrm->get_property("_NET_CURRENT_DESKTOP");
  int num_desktops = xrm->get_property("_NET_NUMBER_OF_DESKTOPS");

  if(current_desktop == num_desktops - 1)
    xrm->switch_desktop(0);
  else
    xrm->switch_desktop(current_desktop + 1);

  return 0;
}

int prevDesktop(std::unique_ptr<XRequestManager> &xrm) {
  int current_desktop = xrm->get_property("_NET_CURRENT_DESKTOP");
  int num_desktops = xrm->get_property("_NET_NUMBER_OF_DESKTOPS");

  if(current_desktop == 0)
    xrm->switch_desktop(num_desktops - 1);
  else
    xrm->switch_desktop(current_desktop - 1);

  return 0;
}

void readMouseFile(XRequestManager *XRM) {
  std::ifstream mouseFile (MOUSEFILE, std::ios::in | std::ios::binary);

  char mouseData[3];
  size_t mouse_data_size = sizeof(mouseData);
  int moveDistance_x = 0;

  while(mouseFile.is_open()) {
    if(tripleTap) {
      mouseFile.read(mouseData, mouse_data_size);
      moveDistance_x = mouseData[1];
      // write_output(moveDistance_x);
      // std::cout << moveDistance_x << std::endl;
      if(moveDistance_x > SENSITIVITY)
        XRM->getAttrs();
      else if(moveDistance_x < -SENSITIVITY)
        XRM->getAttrs();
    }
  }
  mouseFile.close();
}

void readEventFile() {
  std::ifstream eventFile(EVENTFILE);

  struct input_event event;
  size_t eventSize = sizeof(event);
  char eventData[eventSize];

  while(eventFile.is_open()) {
    eventFile.read(eventData, eventSize);
    memcpy(&event, eventData, eventSize);
    if(event.code == BTN_TOOL_TRIPLETAP)
      tripleTap = !tripleTap;
  }
  eventFile.close();
}


// int main(int argc, char** argv) {
//   XRequestManager *XRM = new XRequestManager();
//   int numWS = XRM->getNumWorkspaces();
//   std::cout << numWS << std::endl;
//   // int curWS = XRM->getCurrentWorkspace();
//   std::thread eventReader (readEventFile);
//   std::thread mouseReader (readMouseFile, XRM);

//   eventReader.join();
//   mouseReader.join();
//   return 0;
// }

int main(int argc, char** argv) {
  std::unique_ptr<XRequestManager> xrm(XRequestManager::Create());
  if (!xrm) {
    std::cout << "Failed to initialize window manager.\n";
    return EXIT_FAILURE;
  }

  // std::cout << xrm->get_property("_NET_CURRENT_DESKTOP") << std::endl;
  // xrm->get_current_desktop();
  prevDesktop(xrm);
  return 0;
}