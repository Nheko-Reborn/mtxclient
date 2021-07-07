spdlog_dep = dependency('spdlog', fallback: ['spdlog', 'spdlog_dep'])

room_feed = executable('room_feed',
    'room_feed.cpp',
    dependencies: [matrix_client_dep],
    include_directories : '../tests'
    )

if meson.get_compiler('cpp').has_header('filesystem')
media_downloader = executable('media_downloader',
    'media_downloader.cpp',
    dependencies: [matrix_client_dep],
    include_directories : '../tests'
    )
endif

simple_bot = executable('simple_bot',
    'simple_bot.cpp',
    dependencies: [matrix_client_dep],
    include_directories : '../tests'
    )
crypto_bot = executable('crypto_bot',
    'crypto_bot.cpp',
    dependencies: [matrix_client_dep, spdlog_dep],
    include_directories : '../tests'
    )
online_backup_exporter = executable('online_backup_exporter',
    'online_backup_exporter.cpp',
    dependencies: [matrix_client_dep],
    include_directories : '../tests'
    )
