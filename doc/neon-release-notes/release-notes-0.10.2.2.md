Neon Core version 0.10.2.2 is now available from:

  <https://download.litecoinca.sh/neon-0.10.2.2/>

This is a new major version release, bringing bug fixes and translation 
updates. It is recommended to upgrade to this version.

Please report bugs using the issue tracker at github:

  <https://github.com/neon-project/neon/issues>

Upgrading and downgrading
=========================

How to Upgrade
--------------

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions), then run the
installer (on Windows) or just copy over /Applications/Neon-Qt (on Mac) or
neond/neon-qt (on Linux).

Downgrade warning
------------------

Because release 0.10+ and later makes use of headers-first synchronization and
parallel block download (see further), the block files and databases are not
backwards-compatible with pre-0.10 versions of Neon Core or other software:

* Blocks will be stored on disk out of order (in the order they are
received, really), which makes it incompatible with some tools or
other programs. Reindexing using earlier versions will also not work
anymore as a result of this.

* The block index database will now hold headers for which no block is
stored on disk, which earlier versions won't support.

If you want to be able to downgrade smoothly, make a backup of your entire data
directory. Without this your node will need start syncing (or importing from
bootstrap.dat) anew afterwards. It is possible that the data from a completely
synchronised 0.10 node may be usable in older versions as-is, but this is not
supported and may break as soon as the older version attempts to reindex.

This does not affect wallet forward or backward compatibility.


Neon 0.10.2.2 Change log
============================
This release is based upon Bitcoin Core v0.10.2.  Their upstream changelog applies to us and
is included in as separate release-notes.  This section describes the Neon-specific differences.

Protocol:
- Scrypt Proof-of-Work instead of sha256d, however block hashes are sha256d for performance reasons.
- Neon TCP port 9333 (instead of 62458)
- RPC TCP port 62457 (instead of 8332)
- Testnet TCP port 19333 (instead of 62456)
- Testnet RPC TCP port 62455 (instead of 18332)
- 84 million coin limit  (instead of 21 million)
- Magic 0xfbc0b6db       (instead of 0xf9beb4d9)
- Target Block Time 2.5 minutes (instead of 10 minutes)
- Target Timespan 3.5 days      (instead of two weeks)
- bnProofOfWorkLimit = >> 20    (instead of >> 32)
- See 9a980612005adffdeb2a17ca7a09fe126dd45e0e for Genesis Parameters
- zeitgeist2 protection: b1b31d15cc720a1c186431b21ecc9d1a9062bcb6 Slightly different way to calculate difficulty changes.
- Neon Core v0.10.2.2 is protocol version 70003 (instead of 70002)

Relay:
- Neon Core rounds transaction size up to the nearest 1000 bytes before calculating fees.  This size rounding behavior is to mimic fee calculation of Neon v0.6 and v0.8.
- Bitcoin's IsDust() is disabled in favor of Neon's fee-based dust penalty.
- Fee-based Dust Penalty: For each transaction output smaller than DUST_THRESHOLD (currently 0.001 NEON) the default relay/mining policy will expect an additional 1000 bytes of fee.  Otherwise the transaction will be rejected from relay/mining.  Such transactions are also disqualified from the free/high-priority transaction rule.
- Miners and relays can adjust the expected fee per-KB with the -minrelaytxfee parameter.

Wallet:
- Coins smaller than 0.00001 NEON are by default ignored by the wallet.  Use the -mininput parameter if you want to see smaller coins.

Notable changes since Neon v0.8
===================================

- The Block data and indexes of v0.10 are incompatible with v0.8 clients.  You can upgrade from v0.8 but you downgrading is not possible.  For this reason you may want to make a backup copy of your Data Directory.
- neond no longer sends RPC commands.  You must use the separate neon-cli command line utility.
- Watch-Only addresses are now possible.

Credits
=======

Thanks to everyone who directly contributed to this release:

- Charles Lee
- pooler
- Gitju
- Adrian Gallagher
- Anton Yemelyanov
- Martin Smith
- Warren Togami
