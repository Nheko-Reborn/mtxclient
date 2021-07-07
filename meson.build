project(
  'mtxclient',
  'cpp',
version : '0.5.1',
meson_version : '>=0.57.0',
license : 'MIT',
default_options : 'cpp_std=c++17'
)

cmake = import('cmake')

coeurl_dep = dependency('coeurl', required: true)
thread_dep = dependency('threads', required: true)
openssl_dep = dependency('openssl', version: '>=1.1', required: true)

olm_dep = dependency('Olm', method: 'cmake', fallback: ['olm-wrap', 'olm_dep'], required: true)

json_dep = dependency('nlohmann_json', version: '>=3.2.0', required: true)

deps = [
  coeurl_dep,
  thread_dep,
  olm_dep,
  openssl_dep,
  json_dep
]

inc = include_directories('include')
src = [
	'lib/crypto/client.cpp',
	'lib/crypto/encoding.cpp',
	'lib/crypto/types.cpp',
	'lib/crypto/utils.cpp',
	'lib/http/client.cpp',
	'lib/log.cpp',
	'lib/structs/common.cpp',
	'lib/structs/errors.cpp',
	'lib/structs/events.cpp',
	'lib/structs/events/account_data/fully_read.cpp',
	'lib/structs/events/aliases.cpp',
	'lib/structs/events/avatar.cpp',
	'lib/structs/events/canonical_alias.cpp',
	'lib/structs/events/collections.cpp',
	'lib/structs/events/common.cpp',
	'lib/structs/events/create.cpp',
	'lib/structs/events/encrypted.cpp',
	'lib/structs/events/encryption.cpp',
	'lib/structs/events/ephemeral/receipt.cpp',
	'lib/structs/events/ephemeral/typing.cpp',
	'lib/structs/events/guest_access.cpp',
	'lib/structs/events/history_visibility.cpp',
	'lib/structs/events/join_rules.cpp',
	'lib/structs/events/member.cpp',
	'lib/structs/events/messages/audio.cpp',
	'lib/structs/events/messages/emote.cpp',
	'lib/structs/events/messages/file.cpp',
	'lib/structs/events/messages/image.cpp',
	'lib/structs/events/messages/notice.cpp',
	'lib/structs/events/messages/text.cpp',
	'lib/structs/events/messages/video.cpp',
	'lib/structs/events/mscs/image_packs.cpp',
	'lib/structs/events/name.cpp',
	'lib/structs/events/nheko_extensions/hidden_events.cpp',
	'lib/structs/events/pinned_events.cpp',
	'lib/structs/events/power_levels.cpp',
	'lib/structs/events/presence.cpp',
	'lib/structs/events/reaction.cpp',
	'lib/structs/events/redaction.cpp',
	'lib/structs/events/spaces.cpp',
	'lib/structs/events/tag.cpp',
	'lib/structs/events/tombstone.cpp',
	'lib/structs/events/topic.cpp',
	'lib/structs/events/unknown.cpp',
	'lib/structs/events/voip.cpp',
	'lib/structs/identifiers.cpp',
	'lib/structs/pushrules.cpp',
	'lib/structs/requests.cpp',
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
	'lib/structs/responses/public_rooms.cpp',
	'lib/structs/responses/register.cpp',
	'lib/structs/responses/sync.cpp',
	'lib/structs/responses/turn_server.cpp',
	'lib/structs/responses/version.cpp',
	'lib/structs/responses/well-known.cpp',
	'lib/structs/secret_storage.cpp',
	'lib/structs/user_interactive.cpp',
	'lib/utils.cpp',
]

matrix_client = library('matrix_client',
    src,
    dependencies: deps,
    include_directories : inc,
    install : true)

matrix_client_dep = declare_dependency(
  link_with: matrix_client,
  dependencies: deps,
  include_directories: inc)

meson.override_dependency('mtxclient', matrix_client_dep)

pkg = import('pkgconfig')
pkg.generate(matrix_client,
  libraries : [matrix_client],
  version : meson.project_version(),
  filebase : meson.project_name(),
  description : 'Client API library for Matrix.',
  url : 'https://github.com/Nheko-Reborn/mtxclient)')

conf = configuration_data()

cmake.write_basic_package_version_file(
  name: 'MatrixClient',
  compatibility: 'AnyNewerVersion',
  version: meson.project_version())
cmake.configure_package_config_file(
  name: 'MatrixClient',
  input: 'cmake/MatrixClientConfig.cmake.in',
  configuration: conf)

if get_option('examples')
  subdir('examples')
endif

if get_option('tests')
  subdir('tests')
endif
