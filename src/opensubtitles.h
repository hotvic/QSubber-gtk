#ifndef OPENSUBTITLES_H
#define OPENSUBTITLES_H

#include <gio/gio.h>

G_BEGIN_DECLS

#define QSUBBER_TYPE_OPENSUBTITLES (qsubber_opensubtitles_get_type())
G_DECLARE_FINAL_TYPE(QSubberOpensubtitles, qsubber_opensubtitles, QSUBBER, OPENSUBTITLES, GObject)

QSubberOpensubtitles *qsubber_opensubtitles_new();
int qsubber_opensubtitles_is_logged_in(QSubberOpensubtitles *os);
void qsubber_opensubtitles_login(QSubberOpensubtitles *os, const char *user, const char *pass);
void qsubber_opensubtitles_search(QSubberOpensubtitles *os, const char *langs, const char *name, const char *season,
  const char *episode, const char *size, const char *hash);
void qsubber_opensubtitles_download(QSubberOpensubtitles *os, const char *url, GFile *media_file);

G_END_DECLS

#endif /* OPENSUBTITLES_H */
