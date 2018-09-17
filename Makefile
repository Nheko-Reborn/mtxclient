FILES=`find lib include tests examples -type f -type f \( -iname "*.cpp" -o -iname "*.hpp" \)`

SYNAPSE_IMAGE="avhost/docker-matrix:v0.33.4"

DEPS_BUILD_DIR=.deps
DEPS_SOURCE_DIR=deps

help: ## This help message
	@grep -E '^[0-9a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@# Change the digit following by an 's' to adjust the width of the help text

third-party: ## Build & install third party dependencies
	@cmake -GNinja -H${DEPS_SOURCE_DIR} -B${DEPS_BUILD_DIR} -DCMAKE_BUILD_TYPE=Release -DUSE_BUNDLED_BOOST=OFF
	@cmake --build ${DEPS_BUILD_DIR}

debug: ## Create a debug build
	@cmake -GNinja -H. -Bbuild \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
		-DOPENSSL_ROOT_DIR=/usr/local/opt/openssl \
		-DCMAKE_INSTALL_PREFIX=${DEPS_BUILD_DIR}/usr
	@cmake --build build

release: ## Create an optimized build
	@cmake -GNinja -H. -Bbuild \
		-DCMAKE_BUILD_TYPE=Release \
		-DOPENSSL_ROOT_DIR=/usr/local/opt/openssl \
		-DCMAKE_INSTALL_PREFIX=${DEPS_BUILD_DIR}/usr
	@cmake --build build

test: ## Run the tests
	@cd build/tests && GTEST_COLOR=1 ctest --verbose

asan: ## Create a debug build using address sanitizers
	@cmake -GNinja -H. -Bbuild \
		-DCMAKE_BUILD_TYPE=Debug \
		-DOPENSSL_ROOT_DIR=/usr/local/opt/openssl \
		-DCMAKE_INSTALL_PREFIX=${DEPS_BUILD_DIR}/usr \
		-DASAN=1
	@cmake --build build

image:
	docker build -t mtxclient-dev .

synapse: ## Start a synapse instance on docker
	@docker run -v `pwd`/data:/data --rm \
		-e SERVER_NAME=localhost -e REPORT_STATS=no ${SYNAPSE_IMAGE} generate
	@./.ci/adjust-config.sh
	@docker run -d \
		--name synapse \
		-p 443:8448 \
		-p 8448:8448 \
		-p 8008:8008 \
		-v `pwd`/data:/data ${SYNAPSE_IMAGE} start
	@echo Waiting for synapse to start...
	@until curl -s -f -k https://localhost:443/_matrix/client/versions; do echo "Checking ..."; sleep 2; done
	@echo Register alice
	@docker exec synapse /bin/bash -c 'register_new_matrix_user --admin -u alice -p secret -c /data/homeserver.yaml http://localhost:8008'
	@echo Register bob
	@docker exec synapse /bin/bash -c 'register_new_matrix_user --admin -u bob -p secret -c /data/homeserver.yaml http://localhost:8008'
	@echo Register carl
	@docker exec synapse /bin/bash -c 'register_new_matrix_user --admin -u carl -p secret -c /data/homeserver.yaml http://localhost:8008'

stop-synapse: ## Stop any running instance of synapse
	@rm -rf ./data/*
	@docker rm -f synapse 2>&1>/dev/null

restart: stop-synapse synapse

lint: ## Run clang-format on the source code
	@clang-format -i ${FILES} && git diff --exit-code

clean: ## Delete the build directory
	rm -rf build
