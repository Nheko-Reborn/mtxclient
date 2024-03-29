#!/bin/bash

CMD=""

if [[ ! -z $TRAVIS_OS_NAME ]]; then
    CMD="sudo"
fi

$CMD perl -pi -w -e \
    's/rc_messages_per_second.*/rc_messages_per_second: 1000/g;' data/homeserver.yaml
$CMD perl -pi -w -e \
    's/rc_message_burst_count.*/rc_message_burst_count: 10000/g;' data/homeserver.yaml

(
cat <<HEREDOC


enable_registration: true
enable_registration_without_verification: true

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

experimental_features:
  msc3266_enabled: true

HEREDOC
) | $CMD tee -a data/homeserver.yaml

$CMD perl -pi -w -e \
    's/tls: false/tls: true/g;' data/homeserver.yaml

$CMD openssl req -x509 -newkey rsa:4096 -keyout data/localhost.tls.key -out data/localhost.tls.crt -days 365 -subj '/CN=localhost' -nodes

$CMD chmod 0777 data/localhost.tls.crt
$CMD chmod 0777 data/localhost.tls.key
