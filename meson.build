project(
  'mtxclient',
  'cpp',
version : '0.3.1',
meson_version : '>=0.55.0',
license : 'MIT',
default_options : 'cpp_std=c++17'
)

cmake = import('cmake')

boost_dep = dependency('boost', version: '>=1.70', modules : ['iostreams', 'system', 'thread'])
thread_dep = dependency('threads')
openssl_dep = dependency('OpenSSL', version: '>=1.1')

olm_dep = dependency('Olm', method: 'cmake', fallback: ['olm-wrap', 'olm_dep'])

json_dep = dependency('nlohmann_json', version: '>=3.2.0')

deps = [
  boost_dep,
  thread_dep,
  olm_dep,
  openssl_dep,
  json_dep
]

inc = include_directories('include')
src = [
  'lib/http/client.cpp',
  'lib/http/session.cpp',
  'lib/crypto/client.cpp',
  'lib/crypto/encoding.cpp',
  'lib/crypto/types.cpp',
  'lib/crypto/utils.cpp',
  'lib/utils.cpp',
  'lib/log.cpp',
  'lib/structs/common.cpp',
  'lib/structs/errors.cpp',
  'lib/structs/events.cpp',
  'lib/structs/identifiers.cpp',
  'lib/structs/pushrules.cpp',
  'lib/structs/requests.cpp',
  'lib/structs/secret_storage.cpp',
  'lib/structs/user_interactive.cpp',
  'lib/structs/events/aliases.cpp',
  'lib/structs/events/avatar.cpp',
  'lib/structs/events/canonical_alias.cpp',
  'lib/structs/events/common.cpp',
  'lib/structs/events/collections.cpp',
  'lib/structs/events/create.cpp',
  'lib/structs/events/encrypted.cpp',
  'lib/structs/events/encryption.cpp',
  'lib/structs/events/guest_access.cpp',
  'lib/structs/events/history_visibility.cpp',
  'lib/structs/events/join_rules.cpp',
  'lib/structs/events/member.cpp',
  'lib/structs/events/name.cpp',
  'lib/structs/events/pinned_events.cpp',
  'lib/structs/events/power_levels.cpp',
  'lib/structs/events/presence.cpp',
  'lib/structs/events/reaction.cpp',
  'lib/structs/events/redaction.cpp',
  'lib/structs/events/tag.cpp',
  'lib/structs/events/tombstone.cpp',
  'lib/structs/events/topic.cpp',
  'lib/structs/events/voip.cpp',
  'lib/structs/events/nheko_extensions/hidden_events.cpp',
  'lib/structs/events/messages/audio.cpp',
  'lib/structs/events/messages/emote.cpp',
  'lib/structs/events/messages/file.cpp',
  'lib/structs/events/messages/image.cpp',
  'lib/structs/events/messages/notice.cpp',
  'lib/structs/events/messages/text.cpp',
  'lib/structs/events/messages/video.cpp',
  'lib/structs/responses/common.cpp',
  'lib/structs/responses/create_room.cpp',
  'lib/structs/responses/crypto.cpp',
  'lib/structs/responses/empty.cpp',
  'lib/structs/responses/groups.cpp',
  'lib/structs/responses/login.cpp',
  'lib/structs/responses/media.cpp',
  'lib/structs/responses/messages.cpp',
  'lib/structs/responses/notifications.cpp',
  'lib/structs/responses/profile.cpp',
  'lib/structs/responses/register.cpp',
  'lib/structs/responses/sync.cpp',
  'lib/structs/responses/turn_server.cpp',
  'lib/structs/responses/version.cpp',
  'lib/structs/responses/well-known.cpp'
]

matrix_client = shared_library('matrix_client',
    src,
    dependencies: deps,
    include_directories : inc,
    install : true)

matrix_client_dep = declare_dependency(
  link_with: matrix_client,
  dependencies: deps,
  include_directories: inc)

conf = configuration_data()

cmake.write_basic_package_version_file(
  name: 'MatrixClient',
  compatibility: 'AnyNewerVersion',
  version: meson.project_version())
cmake.configure_package_config_file(
  name: 'MatrixClient',
  input: 'cmake/MatrixClientConfig.cmake.in',
  configuration: conf)

if get_option('examples').enabled()
  subdir('examples')
endif

if get_option('tests').enabled()
  subdir('tests')
endif
