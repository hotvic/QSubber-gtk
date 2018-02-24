#ifndef QSUBBER_PROGRESS_LOG_H
#define QSUBBER_PROGRESS_LOG_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define QSUBBER_TYPE_PROGRESS_LOG (qsubber_progress_log_get_type())
G_DECLARE_FINAL_TYPE(QSubberProgressLog, qsubber_progress_log, QSUBBER, PROGRESS_LOG, GtkMenuButton)

QSubberProgressLog *qsubber_progress_log_new();
void qsubber_progress_log_popup(QSubberProgressLog *pl, const char *operation, gboolean pausable, gboolean retryable);
void qsubber_progress_log_set_description(QSubberProgressLog *pl, const char* desc);

G_END_DECLS

#endif /* QSUBBER_PROGRESS_LOG_H */
