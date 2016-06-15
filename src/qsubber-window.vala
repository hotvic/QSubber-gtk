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
    [GtkTemplate(ui = "/org/imvictor/qsubber/qsubber-window.ui")]
    class Window : Gtk.ApplicationWindow {
        [GtkChild]
        private Gtk.Entry mediaEntry;

        [GtkChild]
        private Gtk.Entry nameEntry;

        [GtkChild]
        private Gtk.Entry seasonEntry;

        [GtkChild]
        private Gtk.Entry episodeEntry;

        public Window(Gtk.Application application) {
            GLib.Object(application: application);

            Application.get_default().current_file_changed.connect(current_file_changed);
        }

        public void current_file_changed(File* file) {
            mediaEntry.text = file->get_path();

            Regex[] exps = {
                new Regex("([a-zA-Z0-9. ]+)[ -_.]+[Ss]([0-9]{0,2})[Ee]([0-9]{0,2})"),
                new Regex("([a-zA-Z0-9. ]+)[ -_.]+([0-9]+)[Xx]([0-9]+)"),
                new Regex("([a-zA-Z0-9. ]+)[ -_.]+([0-9]{1,2})([0-9]{2})")
            };

            foreach (Regex exp in exps) {
                string filename = file->get_basename();

                if (exp.match(filename)) {
                    string[] data = exp.split(filename);

                    nameEntry.text = data[1].replace(".", " ");
                    seasonEntry.text = data[2];
                    episodeEntry.text = data[3];

                    break;
                }
            }
        }
    }
}
