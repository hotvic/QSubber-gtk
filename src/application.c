#include "application.h"

#include "config.h"
#include "opensubtitles.h"
#include "window.h"

static QSubberApplication *instance = NULL;

struct _QSubberApplication {
  GtkApplication parent_instance;

  GFile *current_file;

  /* options */
  GString *ui_dir;
};

G_DEFINE_TYPE(QSubberApplication, qsubber_application, GTK_TYPE_APPLICATION)

static int qsubber_application_handle_local_options(GApplication *app, GVariantDict *options, gpointer user_data);

static void qsubber_application_activate(GApplication *app) {
  QSubberWindow *win;
  
  win = qsubber_window_new(QSUBBER_APPLICATION(app));
  gtk_window_present(GTK_WINDOW(win));
}

static void qsubber_application_open(GApplication *app, GFile **files, gint n_files, const gchar *hint) {
  GList *windows;
  QSubberWindow *win;

  windows = gtk_application_get_windows(GTK_APPLICATION(app));
  if (windows)
    win = QSUBBER_WINDOW(windows->data);
  else
    win = qsubber_window_new(QSUBBER_APPLICATION(app));

  qsubber_application_set_current_file(QSUBBER_APPLICATION(app), files[0]);

  gtk_window_present(GTK_WINDOW (win));
}

static GOptionEntry option_entries[] = {
  { "version", 'v', 0, G_OPTION_ARG_NONE, NULL, "Show version information", NULL },
  { "ui-dir", 'u', 0, G_OPTION_ARG_STRING, NULL, "Set .ui files directory", "DIRECTORY" }
};

static void qsubber_application_init(QSubberApplication *app) {
  app->ui_dir = g_string_new(NULL);

  g_application_add_main_option_entries(G_APPLICATION(app), option_entries);

  g_signal_connect(app, "handle-local-options", G_CALLBACK(qsubber_application_handle_local_options), NULL);
}

static void qsubber_application_class_init(QSubberApplicationClass *class) {
  G_APPLICATION_CLASS(class)->activate = qsubber_application_activate;
  G_APPLICATION_CLASS(class)->open = qsubber_application_open;

  g_signal_new("current-file-changed", QSUBBER_TYPE_APPLICATION,
    G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_POINTER);
}

QSubberApplication *qsubber_application_new() {
  return g_object_new(QSUBBER_TYPE_APPLICATION,
    "application-id", "org.vaurelios.qsubber",
    "flags", G_APPLICATION_HANDLES_OPEN, NULL);
}

QSubberApplication *qsubber_application_get_default() {
  if (instance == NULL) {
    instance = qsubber_application_new();
  }

  return instance;
}

GFile* qsubber_application_get_current_file(QSubberApplication *app) {
  return app->current_file;
}

void qsubber_application_set_current_file(QSubberApplication *app, GFile *file) {
  if (app->current_file)
    g_object_unref(app->current_file);

  app->current_file = file;

  g_signal_emit_by_name(app, "current-file-changed", file);
}

GFile* qsubber_application_find_ui_file(QSubberApplication *app, const char *name) {
  GFile *retval = NULL;
  char *filename = NULL;

  if (app->ui_dir->len != 0) {
    filename = g_build_filename(app->ui_dir->str, name, NULL);

    if (g_file_test(filename, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
      retval = g_file_new_for_path(filename);
    }
  } else {
    filename = g_build_filename(UI_DIR, name, NULL);

    if (g_file_test(filename, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
      retval = g_file_new_for_path(filename);
    }
  }

  if (retval == NULL) {
    g_warning("Failed to find UI file: %s", name);
  }

  g_free(filename);

  return retval;
}

static int qsubber_application_handle_local_options(GApplication *application, GVariantDict *options, gpointer user_data)
{
  QSubberApplication *app = QSUBBER_APPLICATION(application);
  const gchar *str;

  if (g_variant_dict_lookup(options, "version", "b")) {
    g_print("QSubber-gtk version %s\n", PACKAGE_VERSION);

    return 0;
  }

  if (g_variant_dict_lookup(options, "ui-dir", "&s", &str)) {
    g_string_erase(app->ui_dir, 0, -1);
    g_string_append(app->ui_dir, str);
  }

  return -1;
}