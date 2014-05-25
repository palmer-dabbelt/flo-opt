#include "tempdir.bash"
#include "emulator.bash"

set -ex
set -o pipefail

# Extract the actual name of the test from the provided argument list
testname="$(basename "$(dirname "$0")")"
pattern="$(echo "$testname" | cut -d- -f1)"
args="$(echo "$testname" | cut -d- -f2-)"

# Generate a Flo file to be optimized, along with the correct answer
flo-patterns --show $pattern $args
cat Torture.flo
cat Torture.vcd

# Actually optimize that Flo file
mv Torture.flo Torture-unopt.flo
$PTEST_BINARY Torture-unopt.flo Torture.flo

# Build that Flo output into a C++ emulator
flo-llvm --torture Torture.flo

# Build the test harness that goes along with this code
flo-llvm-torture Torture.flo --harness > harness.c++
g++ harness.c++ -c -o harness.o

# Link everything together
g++ harness.o Torture.o -o Torture

# Convert the VCD's input nodes into something the Chisel tester
# understands.
vcd2step Torture.vcd Torture.flo test.in
cat test.in

# Run the tests to see what happens
mv Torture.vcd Torture-gold.vcd
cat test.in | ./Torture

cat Torture.vcd
cat Torture-gold.vcd

# Check and see if the results are the same!
vcddiff Torture-gold.vcd Torture.vcd
vcddiff Torture.vcd Torture-gold.vcd
