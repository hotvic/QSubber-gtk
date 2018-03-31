#include "window.h"
#include "window-private.h"

#include "application.h"
#include "opensubtitles.h"
#include "progress-log.h"
#include "settings.h"
#include "utils.h"

#include <inttypes.h>
#include <string.h>

struct _QSubberWindow {
  GtkApplicationWindow parent_instance;

  QSubberWindowPrivate *priv;
  QSubberApplication *app;
  QSubberOptions *options;
  QSubberSettings *settings;

  QSubberOpensubtitles *rpc;
};

G_DEFINE_TYPE_WITH_PRIVATE(QSubberWindow, qsubber_window, GTK_TYPE_APPLICATION_WINDOW)

static void qsubber_window_current_file_changed(QSubberApplication *app, GFile *file, QSubberWindow *win);
static void qsubber_window_options_changed(QSubberSettings *settings, QSubberOptions *options, QSubberWindow *win);
static void qsubber_window_settings_error(QSubberSettings *settings, GError *err, QSubberWindow *win);
static void qsubber_window_sublist_changed(QSubberOpensubtitles *os, GVariant *sublist, QSubberWindow *win);
static void qsubber_window_rpc_progress(QSubberOpensubtitles *os, const char *operation, int val, QSubberWindow *win);
static void qsubber_window_rpc_error(QSubberOpensubtitles *os, GError *err, QSubberWindow *win);
static void qsubber_window_selected_subtitle_changed(GtkTreeSelection *selection, QSubberWindow *win);
static void qsubber_window_open_button_clicked(GtkButton *button, QSubberWindow *window);
static void qsubber_window_download_button_clicked(GtkButton *button, QSubberWindow *window);
static void qsubber_window_hash_button_clicked(GtkButton *button, QSubberWindow *window);
static void qsubber_window_size_button_clicked(GtkButton *button, QSubberWindow *window);
static void qsubber_window_name_button_clicked(GtkButton *button, QSubberWindow *window);
static void qsubber_window_hash_size_button_clicked(GtkButton *button, QSubberWindow *window);

static void qsubber_window_init(QSubberWindow *win) {
  /* ensure all private types */
  g_type_ensure(QSUBBER_TYPE_PROGRESS_LOG);

  win->priv = qsubber_window_get_instance_private(win);
  win->app = qsubber_application_get_default();
  win->settings = qsubber_settings_new();

  win->rpc = qsubber_opensubtitles_new();

  gtk_widget_init_template(GTK_WIDGET(win));

  g_signal_connect(win->app, "current-file-changed", G_CALLBACK(qsubber_window_current_file_changed), win);
  g_signal_connect(win->settings, "changed", G_CALLBACK(qsubber_window_options_changed), win);
  g_signal_connect(win->settings, "error", G_CALLBACK(qsubber_window_settings_error), win);
  g_signal_connect(win->rpc, "new-sublist", G_CALLBACK(qsubber_window_sublist_changed), win);
  g_signal_connect(win->rpc, "progress", G_CALLBACK(qsubber_window_rpc_progress), win);
  g_signal_connect(win->rpc, "error", G_CALLBACK(qsubber_window_rpc_error), win);
  g_signal_connect(win->priv->selected_subtitle, "changed", G_CALLBACK(qsubber_window_selected_subtitle_changed), win);
  g_signal_connect(win->priv->open_button, "clicked", G_CALLBACK(qsubber_window_open_button_clicked), win);
  g_signal_connect(win->priv->download_button, "clicked", G_CALLBACK(qsubber_window_download_button_clicked), win);
  g_signal_connect(win->priv->hash_button, "clicked", G_CALLBACK(qsubber_window_hash_button_clicked), win);
  g_signal_connect(win->priv->size_button, "clicked", G_CALLBACK(qsubber_window_size_button_clicked), win);
  g_signal_connect(win->priv->name_button, "clicked", G_CALLBACK(qsubber_window_name_button_clicked), win);
  g_signal_connect(win->priv->hash_size_button, "clicked", G_CALLBACK(qsubber_window_hash_size_button_clicked), win);

  qsubber_settings_load_from_resource(win->settings, "/org/vaurelios/qsubber/settings.json");

  for (int i = 0; i < json_array_get_length(win->options->search_langs); i++) {
    GtkTreeIter it;
    JsonObject *lang = json_array_get_object_element(win->options->search_langs, i);

    gtk_list_store_append(win->priv->lang_list, &it);
    gtk_list_store_set(win->priv->lang_list, &it,
                       0, json_object_get_string_member(lang, "display"),
                       1, json_object_get_string_member(lang, "code"),
                       -1);

    gtk_combo_box_set_active_id(win->priv->lang_combo, win->options->prev_selected_lang);
  }
}

static void qsubber_window_class_init(QSubberWindowClass *class) {
  gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class), "/org/vaurelios/qsubber/window.ui");

  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberWindow, selected_subtitle);
  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberWindow, lang_list);
  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberWindow, subtitle_list);
  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberWindow, open_button);
  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberWindow, download_button);
  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberWindow, hash_button);
  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberWindow, size_button);
  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberWindow, name_button);
  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberWindow, hash_size_button);
  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberWindow, media_entry);
  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberWindow, name_entry);
  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberWindow, season_entry);
  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberWindow, episode_entry);
  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberWindow, progress_log);
  gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), QSubberWindow, lang_combo);
}

QSubberWindow *qsubber_window_new(QSubberApplication *application) {
  return g_object_new(QSUBBER_TYPE_WINDOW, "application", application, NULL);
}

static void qsubber_window_current_file_changed(QSubberApplication *app, GFile *file, QSubberWindow *win) {
  gtk_entry_set_text(win->priv->media_entry, g_file_get_path(file));

  for (int i = 0; i < json_array_get_length(win->options->files_patterns); i++) {
    GRegex *exp;
    char* filename = g_file_get_basename(file);

    exp = g_regex_new(
      json_node_get_string(json_array_get_element(win->options->files_patterns, i)) , 0, 0, NULL);

    if (g_regex_match(exp, filename, 0, 0)) {
      char **data = g_regex_split(exp, filename, 0);

      gtk_entry_set_text(win->priv->name_entry, data[1]);
      gtk_entry_set_text(win->priv->season_entry, data[2]);
      gtk_entry_set_text(win->priv->episode_entry, data[3]);

      g_strfreev(data);
      break;
    }
  }

  gtk_widget_set_sensitive(GTK_WIDGET(win->priv->hash_button), TRUE);
  gtk_widget_set_sensitive(GTK_WIDGET(win->priv->size_button), TRUE);
  gtk_widget_set_sensitive(GTK_WIDGET(win->priv->name_button), TRUE);
  gtk_widget_set_sensitive(GTK_WIDGET(win->priv->hash_size_button), TRUE);
}

static void qsubber_window_options_changed(QSubberSettings *settings, QSubberOptions *options, QSubberWindow *win) {
  win->options = options;

  qsubber_opensubtitles_login(win->rpc, options->auth_username, options->auth_password);
}

static void qsubber_window_settings_error(QSubberSettings *settings, GError *err, QSubberWindow *win) {
  qsubber_progress_log_popup(win->priv->progress_log, g_strdup_printf("Error! (%d)", err->code), FALSE, FALSE);
  qsubber_progress_log_set_description(win->priv->progress_log,
                                       g_strdup_printf("Settings: %s", err->message));
}

static void qsubber_window_selected_subtitle_changed(GtkTreeSelection *selection, QSubberWindow *win) {
  if (gtk_tree_selection_get_selected(selection, NULL, NULL)) {
    gtk_widget_set_sensitive(GTK_WIDGET(win->priv->download_button), TRUE);
  } else {
    gtk_widget_set_sensitive(GTK_WIDGET(win->priv->download_button), FALSE);
  }
}

static void qsubber_window_open_button_clicked(GtkButton *button, QSubberWindow *win) {
  GtkWidget *dialog;

  dialog = gtk_file_chooser_dialog_new("Select Media File", GTK_WINDOW(win), GTK_FILE_CHOOSER_ACTION_OPEN,
    "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_OK, NULL);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    qsubber_application_set_current_file(win->app, gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog)));
  }

  gtk_window_close(GTK_WINDOW(dialog));
}

static void qsubber_window_download_button_clicked(GtkButton *button, QSubberWindow *win) {
  GtkTreeIter iter;

  if (gtk_tree_selection_get_selected(win->priv->selected_subtitle, NULL, &iter)) {
    char *url;

    gtk_tree_model_get(GTK_TREE_MODEL(win->priv->subtitle_list), &iter, 2, &url, -1);

    qsubber_opensubtitles_download(win->rpc, url, qsubber_application_get_current_file(win->app));
  }
}

static void qsubber_window_hash_button_clicked(GtkButton *button, QSubberWindow *win) {
  GFile *current_file = qsubber_application_get_current_file(win->app);
  GString *hash;

  g_return_if_fail(current_file != NULL);

  hash = qsubber_utils_calculate_hash_for_file(g_file_get_path(current_file));

  qsubber_opensubtitles_search(win->rpc, "pob", "", "", "", "", hash->str);

  g_string_free(hash, TRUE);
}

static void qsubber_window_size_button_clicked(GtkButton *button, QSubberWindow *win) {
  GFile *current_file = qsubber_application_get_current_file(win->app);
  GFileInfo *info;
  char *file_size;

  g_return_if_fail(current_file != NULL);

  info = g_file_query_info(current_file, "*", G_FILE_QUERY_INFO_NONE, NULL, NULL);

  if (info == NULL) return;

  file_size = g_strdup_printf("%" PRId64, g_file_info_get_size(info));

  qsubber_opensubtitles_search(win->rpc, "pob", "", "", "", file_size, "");

  g_free(file_size);
  g_object_unref(info);
}

static void qsubber_window_name_button_clicked(GtkButton *button, QSubberWindow *win) {
  qsubber_opensubtitles_search(win->rpc, "pob", gtk_entry_get_text(win->priv->name_entry),
    gtk_entry_get_text(win->priv->season_entry), gtk_entry_get_text(win->priv->episode_entry), "", "");
}

static void qsubber_window_hash_size_button_clicked(GtkButton *button, QSubberWindow *win) {
  GFile *current_file = qsubber_application_get_current_file(win->app);
  GFileInfo *info;
  GString *hash;
  char *file_size;

  g_return_if_fail(current_file != NULL);

  hash = qsubber_utils_calculate_hash_for_file(g_file_get_path(current_file));

  info = g_file_query_info(current_file, "*", G_FILE_QUERY_INFO_NONE, NULL, NULL);
  if (info == NULL) return;

  file_size = g_strdup_printf("%" PRId64, g_file_info_get_size(info));

  qsubber_opensubtitles_search(win->rpc, "pob", "", "", "", file_size, hash->str);

  g_free(file_size);
  g_string_free(hash, TRUE);
  g_object_unref(info);
}

static void qsubber_window_sublist_changed(QSubberOpensubtitles *os, GVariant *sublist, QSubberWindow *win) {
  GVariantIter iter;
  GVariant *sub;

  gtk_list_store_clear(win->priv->subtitle_list);

  g_variant_iter_init(&iter, sublist);

  while (g_variant_iter_next(&iter, "v", &sub)) {
    GtkTreeIter it;
    char *sub_filename;
    char *sub_size;
    char *sub_download_link;

    g_variant_lookup(sub, "SubFileName", "&s", &sub_filename);
    g_variant_lookup(sub, "SubSize", "&s", &sub_size);
    g_variant_lookup(sub, "SubDownloadLink", "&s", &sub_download_link);

    gtk_list_store_append(win->priv->subtitle_list, &it);

    gtk_list_store_set(win->priv->subtitle_list, &it, 0, sub_filename, 1, sub_size, 2, sub_download_link, -1);

    g_variant_unref(sub);
  }
}

static void qsubber_window_rpc_progress(QSubberOpensubtitles *os, const char *operation, int val, QSubberWindow *win) {
  const char *ope_status = "Unknown operation running...";

  if (strcmp(operation, "login") == 0) {
    if (val == 100) {
      ope_status = "Logging in... done!";
    } else {
      ope_status = "Logging in...";
    }
  }

  if (strcmp(operation, "search") == 0) {
    if (val == 100) {
      ope_status = "Searching... done!";
    } else {
      ope_status = "Searching...";
    }
  }

  if (strcmp(operation, "download") == 0) {
    if (val == 100) {
      ope_status = "Downloading... done!";
    } else {
      ope_status = "Downloading...";
    }
  }

  qsubber_progress_log_popup(win->priv->progress_log, ope_status, FALSE, FALSE);
}

static void qsubber_window_rpc_error(QSubberOpensubtitles *os, GError *err, QSubberWindow *win) {
  qsubber_progress_log_popup(win->priv->progress_log, g_strdup_printf("Error! (%d)", err->code), FALSE, FALSE);
  qsubber_progress_log_set_description(win->priv->progress_log,
                                       g_strdup_printf("OpenSubtitles backend: %s", err->message));
}
