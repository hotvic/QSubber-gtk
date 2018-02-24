#ifndef QSUBBER_WINDOW_H
#define QSUBBER_WINDOW_H

#include "application.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define QSUBBER_TYPE_WINDOW (qsubber_window_get_type())
G_DECLARE_FINAL_TYPE(QSubberWindow, qsubber_window, QSUBBER, WINDOW, GtkApplicationWindow)

QSubberWindow *qsubber_window_new(QSubberApplication *application);

G_END_DECLS

#endif /* QSUBBER_WINDOW_H */