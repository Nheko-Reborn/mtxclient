# Changelog

## [0.9.2] -- 2023-02-20

- Fix compilation with gcc13. (vitaly)
- Handle incorrect pushrules sent by construct better.
- Don't highlight everything, when an empty display name is set.
- Document security of the default parameters to PBKDF2.
- Fix double invokation of callback if callback throws.
- Fix compiling tests with LTO.
- Clean up some duplicate symbols in the library.

## [0.9.1] -- 2023-01-13

- Fix building with user specified toolchain files.

We still specified C++17 as the C++ standard, which was overwritten by our
toolchain file, but some distros use their own, so this fixes the build for
them. (This probably only affects building the tests and examples.)

## [0.9.0] -- 2023-01-12

- Support error formatting using {fmt}/spdlog.
- Don't require the events key in `state` and other keys in sync.
- Fix parsing members with `null` reason.
- Implement searching user directory. (Malte E)
- Add support for confetti events. (LorenDB)
- Support MSC3664, pushrules for relations.
- Support altsvc for http/3 switching.
- Allow evaluating pushrules locally.
- Use stable endpoints for cross-signing.
- Improve documentation in several places.
- Support voip v1. (r0hit05)
- Support threading.
- Switch to C++20.
- Fix /sync with invites without state.

## [0.8.2] -- 2022-09-09

- fix crash when fetching room summaries

## [0.8.1] -- 2022-09-07

- Allow creating rooms with custom create event but default version
- Update dependencies
- Support stable hidden read receipts
- Validate fields servers are required to validate again
- Fix voip v1 event parsing (contributed by r0hit)
- Use hidden friends to reduce overload sets
- Add support for the unstable polics room type
- Support querying server capabilities

## [0.8.0] -- 2022-07-22

- Update hidden read receipts to current MSC
- Add support for policy rules
- Support for v1 call events and use voip namespace (contributed by r0hit)
- Simplify unknown and redacted event handling
- Support listing, resolving and setting aliases
- Require Matrix v1.1
- Support the knock_restricted rule
- Get rid of redundant namespaces and using namespaces
- Remove support for groups
- Optionally disable implicit JSON conversions
- Support fallback keys
- Make sender_key optional
- Allow more base64 encodings for cross-signing events
- Use spdlog for logging
- Support reasons for knocking, joining leaving and inviting.
- Support initial state in createRoom

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
