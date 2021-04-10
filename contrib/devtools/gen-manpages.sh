#!/bin/bash

TOPDIR=${TOPDIR:-$(git rev-parse --show-toplevel)}
SRCDIR=${SRCDIR:-$TOPDIR/src}
MANDIR=${MANDIR:-$TOPDIR/doc/man}

NEOND=${NEOND:-$SRCDIR/neond}
NEONCLI=${NEONCLI:-$SRCDIR/neon-cli}
NEONTX=${NEONTX:-$SRCDIR/neon-tx}
NEONQT=${NEONQT:-$SRCDIR/qt/neon-qt}

[ ! -x $NEOND ] && echo "$NEOND not found or not executable." && exit 1

# The autodetected version git tag can screw up manpage output a little bit
NEONVER=($($NEONCLI --version | head -n1 | awk -F'[ -]' '{ print $6, $7 }'))

# Create a footer file with copyright content.
# This gets autodetected fine for bitcoind if --version-string is not set,
# but has different outcomes for bitcoin-qt and bitcoin-cli.
echo "[COPYRIGHT]" > footer.h2m
$NEOND --version | sed -n '1!p' >> footer.h2m

for cmd in $NEOND $NEONCLI $NEONTX $NEONQT; do
  cmdname="${cmd##*/}"
  help2man -N --version-string=${NEONVER[0]} --include=footer.h2m -o ${MANDIR}/${cmdname}.1 ${cmd}
  sed -i "s/\\\-${NEONVER[1]}//g" ${MANDIR}/${cmdname}.1
done

rm -f footer.h2m
