#!/bin/sh

set -e

export DEBIAN_FRONTEND=noninteractive

apt-get update && apt-get -y install --no-install-recommends runit postgresql openssl


mkdir /data2

mkdir /data2/db
chown postgres /data2/db

# Initialise & start the database
su -c '/usr/lib/postgresql/13/bin/initdb -D /data2/db -E "UTF-8" --lc-collate="C" --lc-ctype="C" --username=postgres' postgres
su -c '/usr/lib/postgresql/13/bin/pg_ctl -w -D /data2/db start' postgres
su -c '/usr/lib/postgresql/13/bin/createuser synapse_user' postgres
su -c '/usr/lib/postgresql/13/bin/createdb -O synapse_user synapse' postgres

sed -i 's,/data,/data2,g' /start.py
sed -i 's,/data,/data2,g' /conf/homeserver.yaml

SYNAPSE_SERVER_NAME=synapse SYNAPSE_REPORT_STATS=no /start.py generate

openssl req -x509 -newkey rsa:4096 -keyout data2/synapse.tls.key -out data2/synapse.tls.crt -days 365 -subj '/CN=synapse' -nodes
chmod 0777 data2/synapse.tls.crt
chmod 0777 data2/synapse.tls.key

sed -i 's/tls: false/tls: true/g;' data2/homeserver.yaml

# yes, the empty line is needed
cat <<EOF >> /data2/homeserver.yaml


enable_registration: true
enable_registration_without_verification: true

tls_certificate_path: "/data2/synapse.tls.crt"
tls_private_key_path: "/data2/synapse.tls.key"

database:
  name: psycopg2
  args:
    user: synapse_user
    database: synapse
    host: localhost
    cp_min: 5
    cp_max: 10

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
EOF

# start synapse and create users
/start.py &

echo Waiting for synapse to start...
until curl -s -f -k https://localhost:8008/_matrix/client/versions; do echo "Checking ..."; sleep 2; done
echo Register alice
register_new_matrix_user --admin -u alice -p secret -c /data2/homeserver.yaml https://localhost:8008
echo Register bob
register_new_matrix_user --admin -u bob -p secret -c /data2/homeserver.yaml https://localhost:8008
echo Register carl
register_new_matrix_user --admin -u carl -p secret -c /data2/homeserver.yaml https://localhost:8008

exit 0

