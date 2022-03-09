# Changelog

## [0.7.0] -- 2022-03-09

- Allow querying a reusable server url for the current server
- Explicit SSO identity providers
- Reasons for redactions
- Treat all exceptions as errors
- Support hidden read receipts
- Support fetching the current room state
- Stabilize registration tokens
- Widget events
- Support the space hierarchy API
- Allow checking a usernames availability
- Allow querying registration flows ahead of time
- Properly validate matrix ids
- Fix ambiguous conversion of pushrule actions
- Reduce copies for pushrules
- Remove mentions of Boost.ASIO
- Fix documentation

## [0.6.2] -- 2022-02-22

- Fix exception on "new" version string format

## [0.6.1] -- 2021-12-20

- Set counter for encrypted files always to 0
- Add bigobj flag to meson build
- Add script to plot history from memberstats
- Add example that collects member event history from a room
- Add endpoint to list room members
- Fix pinned message events not being parsed
- Fix version tag in meson.build
- Actually read out rule_id in PushRule's from_json
- Support error code to string conversions
- Add support for m.direct

## [0.6.0] -- 2021-11-17

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
