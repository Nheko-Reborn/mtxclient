FROM matrixdotorg/synapse:v1.63.1

COPY setup-synapse.sh /setup-synapse.sh
COPY entrypoint.sh /entrypoint.sh
COPY service /service

RUN /setup-synapse.sh

ENTRYPOINT ["/entrypoint.sh"]

EXPOSE 8008

