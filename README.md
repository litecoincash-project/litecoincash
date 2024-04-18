Litecoin Cash Core integration/staging tree
===========================================
## What is Litecoin Cash?
<img src="https://static.coinpaprika.com/coin/lcc-litecoin-cash/logo.png?rev=10557741" width="150">
Litecoin Cash (LCC) is a cryptocurrency forked from Litecoin Coin. Users are able to generate LCC through the process of mining: both Pow SHA256  MinotaurX and Hive Mining, an agent-based mining system, which requires no specialized hardware: anything from a Raspberry Pi to your normal desktop PC is fine. Existing SHA256 miners and pools are not affected. Proof-of-work blocks are accepted by the network at any time, so SHA256 mining hardware never needs to stand idle. 51% network attacks are rendered ineffective. Worker bees won't mint blocks on an attacker's private chain, so they cannot accumulate enough chain work to perform a 51% hashpower attack. The mining process will be democratized, as any LCC holder can become a beekeeper.
Litecoin Cash is a fork of Litecoin supporting SHA256 MinotaurX PoW and Hive Mining. For full details, as well as prebuilt binaries for Windows, Mac and Linux, please visit our website at https://litecoinca.sh.

Litecoin Cash Core is the full node software that makes up the backbone of the LCC network.

### Join our <a href="https://discordapp.com/invite/F2nZXnW"><img src="https://img.shields.io/discord/406306237618061313.svg?style=flat&label=Discord%20Community&color=7289DA" alt="Discord"/></a>

License
-------

Litecoin Cash Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable. [Tags](https://github.com/litecoincash-project/litecoincash/tags) are created
regularly to indicate new official, stable release versions of Litecoin Cash Core.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md).

The developer [mailing list](https://groups.google.com/forum/#!forum/litecoincash-dev)
should be used to discuss complicated or controversial changes before working
on a patch set.

Developer IRC can be found on Freenode at #litecoincash-dev.

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](src/test/README.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`. Further details on running
and extending unit tests can be found in [/src/test/README.md](/src/test/README.md).

There are also [regression and integration tests](/test), written
in Python, that are run automatically on the build server.
These tests can be run (if the [test dependencies](/test) are installed) with: `test/functional/test_runner.py`

The Travis CI system makes sure that every pull request is built for Windows, Linux, and OS X, and that unit/sanity tests are run automatically.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

Translations
------------

Any translation corrections or expansions are welcomed as GitHub pull requests.
