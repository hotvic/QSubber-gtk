#include "opensubtitles.h"

#include "application.h"
#include "config.h"
#include "macros.h"

#include <string.h>
#include <libsoup/soup.h>

struct _QSubberOpensubtitles {
  GObject parent_instance;

  SoupSession *session;
  GString *token;
  GFile *media_file;
};

G_DEFINE_TYPE(QSubberOpensubtitles, qsubber_opensubtitles, G_TYPE_OBJECT)

static GFile* qsubber_opensubtitles_get_subtitle_file();
static void qsubber_opensubtitles_login_cb(SoupSession *session, SoupMessage *message, void *data);
static void qsubber_opensubtitles_search_cb(SoupSession *session, SoupMessage *message, void *data);
static void qsubber_opensubtitles_download_cb(SoupSession *session, SoupMessage *message, void *data);

void qsubber_opensubtitles_init(QSubberOpensubtitles *os) {
  GValue ua = G_VALUE_INIT;
  g_value_init(&ua, G_TYPE_STRING);
  g_value_set_static_string(&ua, USER_AGENT);

  os->session = soup_session_new();
  g_object_set_property(G_OBJECT(os->session), "user-agent", &ua);
  os->token = g_string_new(NULL);
}

void qsubber_opensubtitles_class_init(QSubberOpensubtitlesClass *class) {
  g_signal_new("error", QSUBBER_TYPE_OPENSUBTITLES,
    G_SIGNAL_RUN_FIRST, 0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_POINTER);
  g_signal_new("new-sublist", QSUBBER_TYPE_OPENSUBTITLES,
    G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_POINTER);
  g_signal_new("progress", QSUBBER_TYPE_OPENSUBTITLES,
    G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_INT);
}

QSubberOpensubtitles *qsubber_opensubtitles_new() {
  return g_object_new(QSUBBER_TYPE_OPENSUBTITLES, NULL);
}

int qsubber_opensubtitles_is_logged_in(QSubberOpensubtitles *os) {
  return os->token->len != 0;
}

void qsubber_opensubtitles_login(QSubberOpensubtitles *os, const char *user, const char *pass) {
  SoupMessage *message;
  GVariant *args;
  GError *err = NULL;

  args = g_variant_new("(ssss)", user, pass, "en", USER_AGENT);

  message = soup_xmlrpc_message_new(API_ENDPOINT, "LogIn", args, &err);

  if (err == NULL) {
    soup_session_queue_message(os->session, message, qsubber_opensubtitles_login_cb, os);

    g_signal_emit_by_name(os, "progress", "login", 10);
  } else {
    g_signal_emit_error(os, 0, "SoupXMLRPC failed to build message: (%d) %s", err->code, err->message);
  }
}

void qsubber_opensubtitles_search(QSubberOpensubtitles *os, const char *langs, const char *name, const char *season,
  const char *episode, const char *size, const char *hash) {
  GVariantBuilder builder;
  GVariantDict query_params;
  GVariant *queries;
  GVariant *args;
  SoupMessage *message;
  GError *err = NULL;

  g_return_if_fail(qsubber_opensubtitles_is_logged_in(os));

  g_variant_dict_init(&query_params, NULL);

  g_variant_dict_insert(&query_params, "sublanguageid", "s", langs);

  if (strlen(name) != 0) {
    g_variant_dict_insert(&query_params, "query", "s", name);
    g_variant_dict_insert(&query_params, "season", "s", season);
    g_variant_dict_insert(&query_params, "episode", "s", episode);
  } else {
    if (strlen(size) != 0) {
      g_variant_dict_insert(&query_params, "moviebytesize", "s", size);
    }

    if (strlen(hash) != 0) {
      g_variant_dict_insert(&query_params, "moviehash", "s", hash);
    }
  }

  g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);
  g_variant_builder_add(&builder, "v", g_variant_dict_end(&query_params));
  queries = g_variant_builder_end(&builder);
  args = g_variant_new("(sv)", os->token->str, queries);

  message = soup_xmlrpc_message_new(API_ENDPOINT, "SearchSubtitles", args, &err);

  if (err == NULL) {
    soup_session_queue_message(os->session, message, qsubber_opensubtitles_search_cb, os);

    g_signal_emit_by_name(os, "progress", "search", 10);
  } else {
    g_signal_emit_error(os, 0, "SoupXMLRPC failed to build message: (%d) %s", err->code, err->message);
  }
}

void qsubber_opensubtitles_download(QSubberOpensubtitles *os, const char *url, GFile *media_file) {
  g_return_if_fail(qsubber_opensubtitles_is_logged_in(os));

  os->media_file = media_file;

  SoupMessage *message = soup_message_new("GET", url);

  soup_session_queue_message(os->session, message, qsubber_opensubtitles_download_cb, os);

  g_signal_emit_by_name(os, "progress", "download", 10);
}

static GFile* qsubber_opensubtitles_get_subtitle_file(QSubberOpensubtitles *os, GFile *media_file) {
  GFile *retval = NULL;
  GError *err = NULL;
  GRegex *exp;
  char *srt_filename = NULL;
  char *filename;

  filename = g_file_get_basename(media_file);

  exp = g_regex_new("\\.([a-zA-Z0-9]{3,4})$", 0, 0, &err);

  if (err == NULL) {
    srt_filename = g_regex_replace_literal(exp, filename, -1, 0, ".srt", 0, &err);

    if (err != NULL) {
      g_signal_emit_error(os, 0, "Failed to match Regex: (%d) %s", err->code, err->message);
    }
  } else {
    g_signal_emit_error(os, 0, "Failed to build Regex: (%d) %s", err->code, err->message);
  }

  g_free(filename);

  if (err == NULL) {
    retval = g_file_get_parent(media_file);

    if (retval != NULL) {
      retval = g_file_get_child(retval, srt_filename);
    }
  }

  g_free(srt_filename);

  return retval;
}

static void qsubber_opensubtitles_login_cb(SoupSession *session, SoupMessage *message, void *data) {
  QSubberOpensubtitles *os = QSUBBER_OPENSUBTITLES(data);
  SoupBuffer *body;
  GVariant *response;
  GError *err = NULL;
  int status;

  g_object_get(G_OBJECT(message), "status-code", &status, NULL);

  if (status == 200) {
    body = soup_message_body_flatten(message->response_body);

    response = soup_xmlrpc_parse_response(body->data, -1, NULL, &err);

    if (err == NULL) {
      GVariant *status = g_variant_lookup_value(response, "status", G_VARIANT_TYPE_STRING);

      if (strcmp(g_variant_get_string(status, NULL), "200 OK") == 0) {
        GVariant *token = g_variant_lookup_value(response, "token", G_VARIANT_TYPE_STRING);

        g_string_printf(os->token, "%s", g_variant_get_string(token, NULL));

        g_signal_emit_by_name(os, "progress", "login", 100);
      } else {
        char *message;

        g_variant_lookup(response, "status", "&s", &message);

        g_signal_emit_error(os, 0, "Login failed: %s", message);
      }
    } else  {
      g_signal_emit_error(os, 0, "Failed to parse XML response from server... Error: %s", err->message);
    }
  } else {
    g_signal_emit_error(os, 0, "Server error; status code: %d", status);
  }
}

static void qsubber_opensubtitles_search_cb(SoupSession *session, SoupMessage *message, void *data) {
  QSubberOpensubtitles *os = QSUBBER_OPENSUBTITLES(data);
  SoupBuffer *body;
  GVariant *response;
  GError *err = NULL;

  body = soup_message_body_flatten(message->response_body);

  response = soup_xmlrpc_parse_response(body->data, -1, NULL, &err);

  if (err == NULL) {
    GVariant *status = g_variant_lookup_value(response, "status", G_VARIANT_TYPE_STRING);

    if (strcmp(g_variant_get_string(status, NULL), "200 OK") == 0) {
      GVariant *subs = g_variant_lookup_value(response, "data", G_VARIANT_TYPE_ARRAY);

      g_signal_emit_by_name(os, "new-sublist", subs);

      g_signal_emit_by_name(os, "progress", "search", 100);
    } else {
      char *message;

      g_variant_lookup(response, "status", "&s", &message);
      g_signal_emit_error(os, 0, "Search error: %s", message);
    }
  } else {
    g_signal_emit_error(os, 0, "Failed to parse XML response from server... Error: %s", err->message);
  }
}

static void qsubber_opensubtitles_download_cb(SoupSession *session, SoupMessage *message, void *data) {
  QSubberOpensubtitles *os = QSUBBER_OPENSUBTITLES(data);
  GFileIOStream *tmpstream = NULL;
  GFile *tmpfile;
  GError *err = NULL;

  g_signal_emit_by_name(os, "progress", "download", 100);

  tmpfile = g_file_new_tmp(NULL, &tmpstream, &err);

  if (err == NULL) {
    GOutputStream *ostream = g_io_stream_get_output_stream(G_IO_STREAM(tmpstream));

    g_output_stream_write(ostream, message->response_body->data, message->response_body->length, NULL, &err);

    if (err == NULL) {
      GFile *subtitle_file;
      GFileInputStream *is;
      GFileOutputStream *ostream;
      GOutputStream *converter;

      subtitle_file = qsubber_opensubtitles_get_subtitle_file(os, os->media_file);
      ostream = g_file_replace(subtitle_file, NULL, FALSE, 0, NULL, &err);

      converter = g_converter_output_stream_new(G_OUTPUT_STREAM(ostream),
        G_CONVERTER(g_zlib_decompressor_new(G_ZLIB_COMPRESSOR_FORMAT_GZIP)));

      is = g_file_read(tmpfile, NULL, &err);

      if (err == NULL) {
        g_output_stream_splice(converter, G_INPUT_STREAM(is),
          G_OUTPUT_STREAM_SPLICE_CLOSE_SOURCE | G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET, NULL, &err);

        if (err != NULL) {
          g_signal_emit_error(os, 0, "Failed to wirte file: (%d) %s", err->code, err->message);
        }
      } else {
        g_signal_emit_error(os, 0, "Failed to open temporary file: (%d) %s", err->code, err->message);
      }
    } else {
      g_signal_emit_error(os, 0, "Failed to write content do temporary file: (%d) %s", err->code, err->message);
    }

    g_output_stream_close(ostream, NULL, NULL);
  } else {
    g_signal_emit_error(os, 0, "Failed to open file, reason: (%d) %s", err->code, err->message);
  }
}
