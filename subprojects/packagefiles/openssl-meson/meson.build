project('Openssl meson wrapper', 'c',
  meson_version : '>=0.55.0',
  version: '1.1.1k',
)

mod = import('unstable_external_project')

openssl = mod.add_project('Configure',
  configure_options : [
    '--prefix=@PREFIX@',
    '--libdir=@LIBDIR@',
    '-Dincludedir_ignore=@PREFIX@/@INCLUDEDIR@',
    target_machine.system() + '-' + target_machine.cpu_family(),
                      ],
  verbose: true,
)

ssl_dep = openssl.dependency('ssl')
crypto_dep = openssl.dependency('crypto')
openssl_dep = declare_dependency(dependencies: [ssl_dep, crypto_dep])
meson.override_dependency('openssl', ssl_dep)
