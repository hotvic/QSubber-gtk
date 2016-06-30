/*
 * This file is part of QSubber.
 *
 * QSubber is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QSubber is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QSubber.  If not, see <http://www.gnu.org/licenses/>.
 */

namespace QSubber {
    class OpenSubtitles : GLib.Object {
        private Soup.Session session = null;
        private string token = null;

        public signal void message(string context, string msg);
        public signal void new_sublist(Variant sublist);

        public OpenSubtitles() {
            session = new Soup.Session();
            session.user_agent = USER_AGENT;
        }

        protected File? get_subtitle_file() {
            File media_file = Application.get_default().current_file;

            if (media_file == null)
                return null;

            string filename = media_file.get_basename();
            string srt_filename;

            try {
                Regex exp = new Regex("\\.([a-zA-Z0-9]{3,4})$");

                srt_filename = exp.replace(filename, filename.length, 0, ".srt");
            } catch (RegexError e) {
                stderr.printf("Failed to build Regex\n");

                return null;
            }

            return media_file.get_parent().get_child(srt_filename);
        }

        public bool is_logged_in() {
            return token != null;
        }

        public void login(string? user=null, string? pass=null) {
            VariantBuilder args = new VariantBuilder(VariantType.TUPLE);

            args.add("s", user == null ? "" : user);
            args.add("s", pass == null ? "" : pass);
            args.add("s", "en");
            args.add("s", USER_AGENT);

            try {
                Soup.Message msg = Soup.XMLRPC.message_new(API_ENDPOINT, "LogIn", args.end());

                append_dot_to_log(msg);

                session.queue_message(msg, login_cb);
            } catch (Error e) {
                stderr.printf("OpenSubtitles backend: Failed to build message for request... Error: %s\n", e.message);
            }

            get_log_dlg().popup("Logging in...");
        }

        public void search(VariantBuilder terms) {
            if (!is_logged_in())
                return;

            terms.add("{sv}", "sublanguageid", new Variant("s", "pob"));

            VariantBuilder args = new VariantBuilder(VariantType.TUPLE);

            args.add("s", token);
            args.add("(@a{sv})", terms.end());

            try {
                Soup.Message msg = Soup.XMLRPC.message_new(API_ENDPOINT, "SearchSubtitles", args.end());

                append_dot_to_log(msg);

                session.queue_message(msg, search_cb);

                get_log_dlg().popup("Searching...");
            } catch (Error e) {
                stderr.printf("OpenSubtitles backend: Failed to build message for request... Error: %s\n", e.message);
            }
        }

        public void download(string url) {
            Soup.Message msg = new Soup.Message("GET", url);

            session.queue_message(msg, download_cb);

            get_log_dlg().popup("Downloading...");
        }

        public void login_cb(Soup.Session _, Soup.Message msg) {
            try {
                Variant resp = Soup.XMLRPC.parse_response((string) msg.response_body.flatten().data, -1, null);

                if (resp.lookup_value("status", VariantType.STRING).get_string() == "200 OK") {
                    token = resp.lookup_value("token", VariantType.STRING).get_string();

                    get_log_dlg().done();

                    stdout.printf("Logged in! token: %s\n", token);
                } else {
                    get_log_dlg().append_to_log("Failed: %s".printf(resp.lookup_value("status", VariantType.STRING).get_string()));
                }
            } catch (Error e) {
                stderr.printf("OpenSubtitles backend: Failed to parse XML response from server... Error: %s\n", e.message);
            }
        }

        public void search_cb(Soup.Session _, Soup.Message msg) {
            try {
                Variant resp = Soup.XMLRPC.parse_response((string) msg.response_body.flatten().data, -1, null);

                if (resp.lookup_value("status", VariantType.STRING).get_string() == "200 OK") {
                    Variant subs = resp.lookup_value("data", VariantType.ARRAY);

                    new_sublist(subs);

                    get_log_dlg().done();
                }
            } catch (Error e) {
                stderr.printf("OpenSubtitles backend: Failed to parse XML response from server... Error: %s\n", e.message);
            }
        }

        public void download_cb(Soup.Session _, Soup.Message msg) {
            get_log_dlg().done();

            try {
                FileIOStream tmpstream;

                File tmpfile = File.new_tmp(null, out tmpstream);

                OutputStream os = tmpstream.output_stream;

                os.write(msg.response_body.data);
                os.close();

                FileOutputStream srtstream = get_subtitle_file().replace(null, false, 0);

                ConverterOutputStream convstream = new ConverterOutputStream(srtstream, new ZlibDecompressor(ZlibCompressorFormat.GZIP));

                convstream.splice(tmpfile.read(), OutputStreamSpliceFlags.CLOSE_SOURCE | OutputStreamSpliceFlags.CLOSE_TARGET);
            } catch (Error e) {
                stderr.printf("OpenSubtitles backend: Failed to open file, reason: %s\n", e.message);
            }
        }
    }
}
