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
    namespace Utils {
        public string? calculate_hash_for_file(File* file) {
            try {
                FileInfo info = file->query_info("*", FileQueryInfoFlags.NONE);
                DataInputStream dis = new DataInputStream(file->read());

                dis.set_byte_order(DataStreamByteOrder.LITTLE_ENDIAN);

                uint64 hash = info.get_size();

                for (int i = 0; i < 65536 / sizeof(uint64); i++) {
                    hash += dis.read_uint64();
                }

                dis.seek(0, SeekType.SET);

                dis.skip((size_t) (info.get_size() - 65536));

                for (int i = 0; i < 65536 / sizeof(uint64); i++) {
                    hash += dis.read_uint64();
                }

                return ("%016" + uint64.FORMAT_MODIFIER + "x").printf(hash);

            } catch (Error e) {
                stderr.printf("Utils: Failed to open or stat the file, reason: %s", e.message);
            }

            return null;
        }

        public string pretty_print_size(string byte_size) {
            double size = double.parse(byte_size);

            if (size >= 1024 && size <= Math.pow(1024, 2))
                return "%f kB".printf(size / 1024);
            else if (size >= Math.pow(1024, 2) && size <= Math.pow(1024, 3))
                return "%f MB".printf(size / Math.pow(1024, 2));

            return "%f B".printf(size);
        }
    }
}
