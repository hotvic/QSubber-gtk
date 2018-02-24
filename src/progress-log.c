#include "progress-log.h"

typedef struct _QSubberProgressLogPrivate QSubberProgressLogPrivate;

struct _QSubberProgressLog {
  GtkMenuButton parent_instance;
  QSubberProgressLogPrivate *priv;
};

struct _QSubberProgressLogPrivate {
  GtkPopover *popover;

  GtkButton *pause_button;
  GtkButton *retry_button;

  GtkLabel *operation_label;
  GtkLabel *description_label;
};

G_DEFINE_TYPE_WITH_PRIVATE(QSubberProgressLog, qsubber_progress_log, GTK_TYPE_MENU_BUTTON)

static void qsubber_progress_log_init(QSubberProgressLog *pl) {
  pl->priv = qsubber_progress_log_get_instance_private(pl);

  gtk_widget_init_template(GTK_WIDGET(pl));
}

static void qsubber_progress_log_class_init(QSubberProgressLogClass *class) {
  gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class), "/org/vaurelios/qsubber/progress-log.ui");

  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberProgressLog, popover);
  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberProgressLog, pause_button);
  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberProgressLog, retry_button);
  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberProgressLog, operation_label);
  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberProgressLog, description_label);
}

QSubberProgressLog *qsubber_progress_log_new() {
  return g_object_new(QSUBBER_TYPE_PROGRESS_LOG, NULL);
}

void qsubber_progress_log_popup(QSubberProgressLog *pl, const char *operation, gboolean pausable, gboolean retryable) {
  gtk_widget_set_visible(GTK_WIDGET(pl), TRUE);

  gtk_label_set_text(pl->priv->operation_label, operation);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pl), TRUE);

  gtk_widget_set_sensitive(GTK_WIDGET(pl->priv->pause_button), pausable);
  gtk_widget_set_sensitive(GTK_WIDGET(pl->priv->retry_button), retryable);
}

void qsubber_progress_log_set_description(QSubberProgressLog *pl, const char* desc) {
  gtk_widget_show(GTK_WIDGET(pl->priv->description_label));

  gtk_label_set_text(pl->priv->description_label, desc);
}
