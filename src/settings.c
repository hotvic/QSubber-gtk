#include "settings.h"

#include "macros.h"
#include "qsubber-resources.h"

struct _QSubberSettings {
  GObject parent_instance;

  QSubberOptions *options;
  JsonParser *parser;
  JsonPath *path;
};

G_DEFINE_TYPE(QSubberSettings, qsubber_settings, G_TYPE_OBJECT)

static void qsubber_settings_init(QSubberSettings *sets) {
  sets->options = g_new(QSubberOptions, 1);
  sets->path = json_path_new();
  sets->parser = json_parser_new();
}

static void qsubber_settings_dispose(GObject *object) {
  QSubberSettings *sets = QSUBBER_SETTINGS(object);

  g_free(sets->options);
  g_object_unref(G_OBJECT(sets->parser));
  g_object_unref(G_OBJECT(sets->path));

  G_OBJECT_GET_CLASS(object)->dispose(object);
}

static void qsubber_settings_class_init(QSubberSettingsClass *class) {
  G_OBJECT_CLASS(class)->dispose = qsubber_settings_dispose;


  g_signal_new("error", QSUBBER_TYPE_SETTINGS,
    G_SIGNAL_RUN_FIRST, 0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_POINTER);
  g_signal_new("changed", QSUBBER_TYPE_SETTINGS,
    G_SIGNAL_RUN_FIRST, 0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_POINTER);
}

QSubberSettings *qsubber_settings_new() {
  return g_object_new(QSUBBER_TYPE_SETTINGS, NULL);
}

static void qsubber_settings_read(QSubberSettings *sets) {
  JsonArray *matches;

  json_path_compile(sets->path, "$.auth.username", NULL);
  settings_set_option(json_parser_get_root(sets->parser), matches, sets->path, string, sets->options->auth_username);
  json_path_compile(sets->path, "$.auth.password", NULL);
  settings_set_option(json_parser_get_root(sets->parser), matches, sets->path, string, sets->options->auth_password);
  json_path_compile(sets->path, "$.files.patterns", NULL);
  settings_set_option(json_parser_get_root(sets->parser), matches, sets->path, array, sets->options->files_patterns);
  json_path_compile(sets->path, "$.prevSelectedLang", NULL);
  settings_set_option(json_parser_get_root(sets->parser), matches, sets->path, string, sets->options->prev_selected_lang);
  json_path_compile(sets->path, "$.searchLangs", NULL);
  settings_set_option(json_parser_get_root(sets->parser), matches, sets->path, array, sets->options->search_langs);

  g_signal_emit_by_name(sets, "changed", sets->options);
}

void qsubber_settings_load_from_data(QSubberSettings *sets, GBytes *data) {
  GError *err = NULL;

  json_parser_load_from_data(sets->parser, g_bytes_get_data(data, NULL), -1, &err);

  if (err == NULL) {
    qsubber_settings_read(sets);
  } else {
    g_signal_emit_by_name(sets, "error", err);
  }
}

void qsubber_settings_load_from_resource(QSubberSettings *sets, const char *path) {
  GResource *res = qsubber_get_resource();
  GBytes *bytes;
  GError *err = NULL;

  bytes = g_resource_lookup_data(res, path, 0, &err);

  if (err == NULL) {
    qsubber_settings_load_from_data(sets, bytes);
  } else {
    g_signal_emit_by_name(sets, "error", err);
  }
}

void qsubber_settings_load_from_file(QSubberSettings *sets, const char *filename) {

}
