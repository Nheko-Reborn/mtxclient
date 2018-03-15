#!/bin/bash

CMD=""

if [[ ! -z $TRAVIS_OS_NAME ]]; then
    CMD="sudo"
fi

$CMD perl -pi -w -e \
    's/rc_messages_per_second.*/rc_messages_per_second: 100/g;' data/homeserver.yaml
$CMD perl -pi -w -e \
    's/rc_message_burst_count.*/rc_message_burst_count: 1000/g;' data/homeserver.yaml
$CMD perl -pi -w -e \
    's/enable_registration.*/enable_registration: True/g;' data/homeserver.yaml
