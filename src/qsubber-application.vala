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

        public File current_file {
            get { return _current_file; }
            set { _current_file = value; current_file_changed(_current_file); }
        }

        public signal void current_file_changed(File* new_file);

        public Application() {
            Object(application_id: "org.imvictor.qsubber", flags: GLib.ApplicationFlags.HANDLES_OPEN);
        }

        public static new Application get_default() {
            return (Application) GLib.Application.get_default();
        }

        public override void activate() {
            create_main_window();
        }

        public override void open(File[] files, string hint)
        {
            create_main_window();

            current_file = files[0];
        }

        public void create_main_window() {
            Window window = null;

            window = (Window) get_active_window();

            if (window == null) {
                window = new Window(this);
            }

            window.show_all();
        }
    }
}
