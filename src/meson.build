src = []

src += ['qsubber.c', 'application.c', 'opensubtitles.c', 'progress-log.c', 'window.c', 'utils.c']

executable(meson.project_name(), qsubberresources, src, install : true, include_directories: top_inc, dependencies: deps,
  c_args: ['-UG_LOG_DOMAIN', '-DG_LOG_DOMAIN="QSubber-gtk"'])