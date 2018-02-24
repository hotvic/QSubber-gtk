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

#define g_signal_emit_error(instance, code, message, ...) \
{ \
  GQuark domain = g_quark_from_static_string("opensubtitles-backend-error"); \
  GError* signal_error = g_error_new(domain, code, message, __VA_ARGS__); \
  g_signal_emit_by_name(instance, "error", signal_error); \
}
