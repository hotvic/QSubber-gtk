#ifndef QSUBBER_WINDOW_PRIVATE_H
#define QSUBBER_WINDOW_PRIVATE_H

#include "progress-log.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _QSubberWindowPrivate QSubberWindowPrivate;

struct _QSubberWindowPrivate {
  GtkTreeSelection *selected_subtitle;

  GtkListStore *subtitle_list;

  GtkButton *open_button;
  GtkButton *download_button;
  GtkButton *hash_button;
  GtkButton *size_button;
  GtkButton *name_button;
  GtkButton *hash_size_button;

  GtkEntry *media_entry;
  GtkEntry *name_entry;
  GtkEntry *season_entry;
  GtkEntry *episode_entry;

  QSubberProgressLog *progress_log;
};

G_END_DECLS

#endif /* QSUBBER_WINDOW_PRIVATE_H */
