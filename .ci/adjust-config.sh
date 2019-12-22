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
rc_message:
  per_second: 1000
  burst_count: 10000

rc_registration:
  per_second: 1000
  burst_count: 3000

rc_login:
  address:
    per_second: 1000
    burst_count: 3000
  account:
    per_second: 1000
    burst_count: 3000
  failed_attempts:
    per_second: 1000
    burst_count: 3000

rc_admin_redaction:
  per_second: 1000
  burst_count: 5000
HEREDOC
) | $CMD tee -a data/homeserver.yaml

$CMD perl -pi -w -e \
    's/#enable_registration: false/enable_registration: true/g;' data/homeserver.yaml
$CMD perl -pi -w -e \
    's/tls: false/tls: true/g;' data/homeserver.yaml
$CMD perl -pi -w -e \
    's/#tls_certificate_path:/tls_certificate_path:/g;' data/homeserver.yaml
$CMD perl -pi -w -e \
    's/#tls_private_key_path:/tls_private_key_path:/g;' data/homeserver.yaml

$CMD openssl req -x509 -newkey rsa:4096 -keyout data/localhost.tls.key -out data/localhost.tls.crt -days 365 -subj '/CN=localhost' -nodes

$CMD chmod 0777 data/localhost.tls.crt
$CMD chmod 0777 data/localhost.tls.key
