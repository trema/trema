#!/bin/sh

# This helper script fetches each RIR's current delegation statistics and
# produces an aggregated CIDR version of the data.

usage()
{
	echo "Usage: $0 [-f]"
	echo " -f  force data download"
	exit 1
}

[ $# -eq 0 -o $# -eq 1 ] || usage

case $# in
	0) FORCE=0 ;;
	1)
		case "$1" in
			-f) FORCE=1 ;;
			*)  usage ;;
		esac
		;;
	*) usage ;;
esac

if ! which aggregate >/dev/null 2>&1; then
	echo 'Error: "aggregate" is not available!'
	exit 2
fi
if which lftpget >/dev/null 2>&1; then
	FETCH=lftpget
elif which wget >/dev/null 2>&1; then
	FETCH=wget
elif which curl >/dev/null 2>&1; then
	FETCH=curl
else
	echo 'Error: neither "lftpget" nor "wget" nor "curl" is available!'
	exit 2
fi

for rir in afrinic apnic arin lacnic ripencc; do
	# Fetch NROESF, ARIN has discontinued RIRSEF.
	dfile=delegated-$rir-extended-latest
	afile=aggregated-delegated-$rir.txt
	if [ ! -s $dfile -o $FORCE -eq 1 ]; then
		[ -s $dfile ] && rm -f $dfile
		lftpget ftp://ftp.ripe.net/pub/stats/$rir/$dfile
	fi
	if [ ! -s $afile -o $dfile -nt $afile ]; then
		cat $dfile | ./stats-to-cidrs.rb | aggregate > $afile
		wc -l $afile
	fi
done
