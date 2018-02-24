project('qsubber-gtk','c', version: '1.0.0', meson_version: '>=0.36.0')
 
top_inc = include_directories('.')

deps = [ dependency('gtk+-3.0', version : '>=3.22'), dependency('glib-2.0'), dependency('libsoup-2.4') ]

datadir = join_paths([ get_option('prefix'), get_option('datadir') ])
pkgdatadir = join_paths([ get_option('prefix'), get_option('datadir'), 'qsubber-gtk' ])
bindir = join_paths([ get_option('prefix'), 'bin' ])

conf = configuration_data()
conf.set_quoted('PACKAGE_NAME', 'QSubber-gtk')
conf.set_quoted('PACKAGE_VERSION', meson.project_version())
conf.set_quoted('DATADIR', datadir)
conf.set_quoted('UI_DIR', join_paths([pkgdatadir,'uidir']))
conf.set_quoted('PKGDATADIR', pkgdatadir)
conf.set_quoted('USER_AGENT', 'QSubber v1')
conf.set_quoted('API_ENDPOINT', 'https://api.opensubtitles.org:443/xml-rpc')

configure_file(output : 'config.h', configuration : conf)

subdir('data')
subdir('src')
