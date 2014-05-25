set -ex

tempdir=`mktemp -d -t ptest-flo-opt.XXXXXXXXXX`
trap "rm -rf $tempdir" EXIT
cd $tempdir
