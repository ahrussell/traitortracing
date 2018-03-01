# GKRW Traitor Tracing

Implementation of the pairing-based broadcast encryption with traitor tracing scheme described in [GKRW17](https://eprint.iacr.org/2017/1117.pdf). This can be used as a library or command-line interface. Currently, this implementation only works with the [BN254](https://eprint.iacr.org/2005/133.pdf) pairing-friendly curve family.

## Build instructions

First, you will need the following libraries:

* [RELIC](https://github.com/relic-toolkit/relic) (must be installed with cmake option -DFP_PRIME=254 to have RELIC use a BN254 curve)
* [libsodium](https://download.libsodium.org/doc/)
* [JSON for Modern C++](https://github.com/nlohmann/json)

This project should be compiled with C++11 or greater. There is a Makefile that may need to be customized to your particular system.

## Notes / Disclaimers

This library is an alpha proof of concept ONLY and should not be used in production. Several potential vulnerabilities:

* [RELIC random point generation potentially biased](https://github.com/relic-toolkit/relic/issues/46)
* Parameter and secret key generation leaks information via a timing attack.