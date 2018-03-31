#ifndef QSUBBER_SETTINGS_H
#define QSUBBER_SETTINGS_H

#include <json-glib/json-glib.h>

G_BEGIN_DECLS
typedef struct {
  /* Auth object: $.auth */
  const char *auth_username;
  const char *auth_password;

  /* Files object: $.files */
  JsonArray *files_patterns;

  /* Previous selected language: $.prevSelectedLang */
  const char *prev_selected_lang;

  /* Search langs array: $.searchLangs */
  JsonArray *search_langs;
} QSubberOptions;

#define QSUBBER_TYPE_SETTINGS (qsubber_settings_get_type())
G_DECLARE_FINAL_TYPE(QSubberSettings, qsubber_settings, QSUBBER, SETTINGS, GObject)

QSubberSettings *qsubber_settings_new();
void qsubber_settings_load_from_data(QSubberSettings *sets, GBytes *data);
void qsubber_settings_load_from_resource(QSubberSettings *sets, const char *path);
void qsubber_settings_load_from_file(QSubberSettings *sets, const char *filename);

G_END_DECLS

#endif /* QSUBBER_S */
