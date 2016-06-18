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
    inline void append_dot_to_log(Soup.Message msg) {
        msg.got_chunk.connect((_) => {
            get_log_dlg().append_to_log(".");
        });
        msg.wrote_chunk.connect(() => {
            get_log_dlg().append_to_log(".");
        });
    }

    [GtkTemplate(ui = "/org/imvictor/qsubber/qsubber-log-dialog.ui")]
    class LogDialog : Gtk.Dialog {
        [GtkChild]
        Gtk.TextBuffer logBuffer;

        public LogDialog(Gtk.Window parent) {
            set_transient_for(parent);
            set_modal(true);
            set_destroy_with_parent(true);
        }

        public void popup(string? append) {
            show_all();
            present();

            logBuffer.text += append;
        }

        public void clear_log() {
            logBuffer.text = "";
        }

        public void append_to_log(string val) {
            logBuffer.text += val;
        }

        public void done() {
            clear_log();
            hide();
        }
    }
}
