#!/bin/bash

CMD=""

if [[ ! -z $TRAVIS_OS_NAME ]]; then
    CMD="sudo"
fi

(
cat <<HEREDOC

server_name: "localhost"
pid_file: /data/homeserver.pid
listeners:
  - port: 8008
    tls: true
    type: http
    x_forwarded: true
    resources:
      - names: [client, federation]
        compress: false
  - port: 8009
    tls: false
    type: http
    x_forwarded: true
    resources:
      - names: [client, federation]
        compress: false
database:
  name: sqlite3
  args:
    database: /data/homeserver.db
log_config: "/data/localhost.log.config"
media_store_path: /data/media_store
registration_shared_secret: "&=bz^dG2c34^NHZYMidt7crX~ZheXN0r1dV02uKapb9uxmktR:"
report_stats: false
macaroon_secret_key: "Ea#Ny0z,r=kv&2.H47au0QSsL&QDjpZqxYX0NcUe9EKsX~Eyrz"
form_secret: ";,kjZhkoUnRLbUq@H21PJcX#T+p&MuNi4O9qbH*gZk2+84ree+"
signing_key_path: "/data/localhost.signing.key"
trusted_key_servers: []


enable_registration: true
enable_registration_without_verification: true

room_list_publication_rules:
  - action: allow

tls_certificate_path: "/data/localhost.tls.crt"
tls_private_key_path: "/data/localhost.tls.key"

rc_message:
  per_second: 10000
  burst_count: 100000

rc_registration:
  per_second: 10000
  burst_count: 30000

rc_login:
  address:
    per_second: 10000
    burst_count: 30000
  account:
    per_second: 10000
    burst_count: 30000
  failed_attempts:
    per_second: 10000
    burst_count: 30000

rc_admin_redaction:
  per_second: 1000
  burst_count: 5000

rc_joins:
  local:
    per_second: 10000
    burst_count: 100000
  remote:
    per_second: 10000
    burst_count: 100000

rc_presence:
  per_user:
    per_second: 10000
    burst_count: 100000

experimental_features:
  msc3266_enabled: true


HEREDOC
) | $CMD tee data/homeserver.yaml

$CMD openssl req -x509 -newkey rsa:4096 -keyout data/localhost.tls.key -out data/localhost.tls.crt -days 365 -subj '/CN=localhost' -nodes

$CMD chmod 0777 data/localhost.tls.crt
$CMD chmod 0777 data/localhost.tls.key
