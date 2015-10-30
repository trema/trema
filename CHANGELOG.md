# Changelog

## develop (unreleased)


## 0.8.0 (10/30/2015)
### New features
* [#409](https://github.com/trema/trema/pull/409): Support rubies installed using apt.
* [#414](https://github.com/trema/trema/pull/414): Add netns command.


## 0.7.1 (9/28/2015)
### Changes
* Pio 0.27.1 (fixes [trema/pio#238](https://github.com/trema/pio/issues/238))


## 0.7.0 (9/16/2015)
### New features
* [#408](https://github.com/trema/trema/pull/408): Add `Trema::Controller#description_stats_reply` handler.


## 0.6.0 (9/9/2015)
### New features
* [#398](https://github.com/trema/trema/pull/398): Add `Trema::Controller#barrier_reply` handler.
* [#400](https://github.com/trema/trema/pull/400): Add `Trema::Controller#hello_failed` handler.
* [#404](https://github.com/trema/trema/pull/404): Add `--logging_level` option to trema run command.


## 0.5.1 (8/4/2015)
### Bugs fixed
* [#396](https://github.com/trema/trema/issues/396): `send_flow_mod_add` doesn't allow `:idle_timeout` and `:hard_timeout` options.


## 0.5.0 (8/2/2015)
### Changes
* [#392](https://github.com/trema/trema/pull/392): Allow packet_in, etc during startup.


## 0.4.8 (6/29/2015)
### Changes
* [#384](https://github.com/trema/trema/pull/384): Add trema run --port (-p) option to override the default OpenFlow channel listen port.
* [#383](https://github.com/trema/trema/pull/383): Use the IANA-assigned port 6653 by default.
* [#389](https://github.com/trema/trema/pull/389): Update pio and other gems.
