# Changelog

## [0.6.0] -- Unreleased

* Use new login parameters instead of deprecated user
* Workaround servers returning null for .well-known in /login
* Various edit relation fixes
* Support space state events
* Use coeurl as the http backend
* Remove boost dependency
* Support retrieval of specific state events
* Use a generic UIAHandler for all UIA endpoints
* Support registration tokens in UIA (callum)
* Support email and telephone numbers in UIA
* Support sticker and emote packs (MSC2545)
* Timeout connections properly
* Properly clear bit 63 of the IV when doing AES
* Allow converting a private key to a public key
* Add knocks and restricted rooms support
* Implement online key backup session encryption
* Fix parsing query_keys responses with optional keys
* Fix compilation with Olm 3.2.5
* Support meson for compilation. This does not generate the cmake files needed by cmake projects to find mtxclient
* Add bootstrapping for SSSS, online key backup and cross-signing keys
* Implement the device query and update endpoints

## [0.5.1] -- 2021-04-20

* Allow exporting a session with a specific minimum index instead of all known indices.

## [0.5.0] -- 2021-04-18

* Support edits
* New relation format and API
* Fixes for voip versioning by trilene
* Proper TLS support
* Support for pushers by vurpo
* Fix crashes when using the epoll reactor on ARM
* Switch to gitlab CI
* Support for [MSC2545](https://github.com/matrix-org/matrix-doc/pull/2545), sticker and emote packs
