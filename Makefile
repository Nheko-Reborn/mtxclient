FILES=`find src tests examples -type f -type f \( -iname "*.cpp" -o -iname "*.hpp" \)`

SYNAPSE_IMAGE="avhost/docker-matrix:v0.27.2.1"

debug:
	@cmake -GNinja -H. -Bbuild \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
		-DOPENSSL_ROOT_DIR=/usr/local/opt/openssl
	@cmake --build build

release:
	@cmake -GNinja -H. -Bbuild -DCMAKE_BUILD_TYPE=Release
	@cmake --build build

test:
	@cd build && GTEST_COLOR=1 ctest --verbose

asan:
	@cmake -GNinja -H. -Bbuild \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
		-DOPENSSL_ROOT_DIR=/usr/local/opt/openssl \
		-DBUILD_OLM=1 \
		-DASAN=1
	@cmake --build build

synapse:
	@docker run -v `pwd`/data:/data --rm \
		-e SERVER_NAME=localhost -e REPORT_STATS=no ${SYNAPSE_IMAGE} generate
	@./.ci/adjust-config.sh
	@docker run -d \
		--name synapse \
		-p 443:8448 -p 8448:8448 \
		-v `pwd`/data:/data ${SYNAPSE_IMAGE} start
	@echo Waiting for synapse to start...
	@sleep 5
	@echo Register alice
	@docker exec synapse /bin/bash -c 'register_new_matrix_user --admin -u alice -p secret -c /data/homeserver.yaml http://localhost:8008'
	@echo Register bob
	@docker exec synapse /bin/bash -c 'register_new_matrix_user --admin -u bob -p secret -c /data/homeserver.yaml http://localhost:8008'
	@echo Register carl
	@docker exec synapse /bin/bash -c 'register_new_matrix_user --admin -u carl -p secret -c /data/homeserver.yaml http://localhost:8008'

stop-synapse:
	@rm -rf ./data/*
	@docker rm -f synapse 2>&1>/dev/null

lint:
	@clang-format -i ${FILES} && git diff --exit-code

clean:
	rm -rf build
