FROM matrixdotorg/synapse:v1.61.0
# see https://github.com/matrix-org/synapse/issues/13375

COPY setup-synapse.sh /setup-synapse.sh
COPY entrypoint.sh /entrypoint.sh
COPY service /service

RUN /setup-synapse.sh

ENTRYPOINT ["/entrypoint.sh"]

EXPOSE 8008

