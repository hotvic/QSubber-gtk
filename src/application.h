#ifndef QSUBBER_APPLICATION_H
#define QSUBBER_APPLICATION_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define QSUBBER_TYPE_APPLICATION (qsubber_application_get_type())
G_DECLARE_FINAL_TYPE(QSubberApplication, qsubber_application, QSUBBER, APPLICATION, GtkApplication)

QSubberApplication *qsubber_application_new();
QSubberApplication *qsubber_application_get_default();

GFile *qsubber_application_get_current_file(QSubberApplication *app);
void qsubber_application_set_current_file(QSubberApplication *app, GFile *file);
GFile *qsubber_application_find_ui_file(QSubberApplication *app, const char *name);

G_END_DECLS

#endif /* QSUBBER_APPLICATION_H */