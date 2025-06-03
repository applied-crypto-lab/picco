RSS Test: Complete Workflow

Tested on: rigel-01, rigel-02, rigel-03 (Ubuntu 20.04, GCC, OpenSSL)

1. Preparation - Build the Project

./compile-rss.sh 32

2. Key Generation - On each machine (or centrally and distribute):

# On rigel-01:

openssl genrsa -out private_01.pem 2048
openssl rsa -in private_01.pem -outform PEM -pubout -out public_01.pem

# On rigel-02:

openssl genrsa -out private_02.pem 2048
openssl rsa -in private_02.pem -outform PEM -pubout -out public_02.pem

# On rigel-03:

openssl genrsa -out private\*03.pem 2048
openssl rsa -in private_03.pem -outform PEM -pubout -out public_03.pem

Distribute public\*\*.pem as needed so all machines have all public keys.

3. Create runtime-config File
   This file must be identical on all three machines.
   Place it in the working directory (~/Documents/picco/compute/).

Example (adjust ports as needed):

1,rigel-01,9647,public_01.pem
2,rigel-02,9646,public_02.pem
3,rigel-03,9645,public_03.pem

4. Check Connectivity
   Ensure you can ping each node from every other node.

ping rigel-02
ping rigel-03

5. Run the Program
   5.1. Launch rss_main on All Three Nodes
   On each machine in ~/Documents/picco/compute:

./build/rss*main <id> runtime-config private*<id>.pem 3 32 <batch_size>
<id>: 1, 2, or 3 for each node.

runtime-config: Path to your config file (identical on all).

private\_<id>.pem: Private key for this node.

3: Number of parties.

32: Ring size (as used in compile).

<batch_size>: Number of tests to run (set to length of your test array).

Example:

On rigel-01:
./build/rss_main 1 runtime-config private_01.pem 3 32 26

On rigel-02:
./build/rss_main 2 runtime-config private_02.pem 3 32 26

On rigel-03:
./build/rss_main 3 runtime-config private_03.pem 3 32 26

Tip: Start all three programs within a few seconds of each other to avoid connection timeouts.

5.2. Expected Output
All nodes should print successful connection messages and then print FLLT test results for each input pair.
