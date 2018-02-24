#include "application.h"

int main(int argc, char *argv[]) {
  QSubberApplication *app;

  app = qsubber_application_get_default();

  return g_application_run(G_APPLICATION(app), argc, argv);
}