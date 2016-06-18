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

using Gtk;

namespace QSubber {
    class Application : Gtk.Application {
        private File _current_file = null;

        /* Windows */
        private Window _main_window = null;
        private LogDialog _log_dialog = null;

        public OpenSubtitles os = null;

        public Window main_window {
            get { return _main_window; }
        }

        public LogDialog log_dialog {
            get { return _log_dialog; }
        }

        public File current_file {
            get { return _current_file; }
            set { _current_file = value; current_file_changed(_current_file); }
        }

        public signal void current_file_changed(File* new_file);

        public Application() {
            Object(application_id: "org.imvictor.qsubber", flags: GLib.ApplicationFlags.HANDLES_OPEN);

            os = new OpenSubtitles();
        }

        public static new Application get_default() {
            return (Application) GLib.Application.get_default();
        }

        public override void startup() {
            base.startup();

            create_windows();
            os.login();
        }

        public override void activate() {
            _main_window.show_all();
            _main_window.present();
        }

        public override void open(File[] files, string hint) {
            _main_window.show_all();
            _main_window.present();

            current_file = files[0];
        }

        public void create_windows() {
            if (_main_window == null) {
                _main_window = new Window(this);
            }

            if (_log_dialog == null) {
                _log_dialog = new LogDialog(_main_window);
            }
        }
    }
}
