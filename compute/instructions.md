This is a supplemental document for compiling and running RSS protocols for development. It assumes you've read the main PICCO README and are familiar with terms like "runtime config" and  "public-private key pairs."

The specific file that is used for testing correctness of RSS operations is `smc-compute/RSS_testing.cpp`. The code is heavily documented, outlining the process for verifying a protocol is correctly implemented. It should ideally be read before proceeding with the remainder of the instructions to compile/run the code.

The compilation script `compile-rss.sh` utilizes an existing `CMakeLists.txt` file present in the `compute` directory (NOT the template `.CMAKE_TEMPLATE`). To compile the repo, run the following command:
```
./compiled-rss.sh <ring_size>
```
where `ring_size` is the bitlength of the ring. **NOTE**, if you want to test another ring size (e.g., 32-bit), you must re-compile the entire repository.

In `numParties` separate terminals, run the following command in each:
```
./build/rss_main <id> runtime-config private_<id>.pem <numParties> <ring_size> <batch_size>
```
where `id` is the party's id number, `private_<id>` is `id`'s private key, `numParties` is the total number of parties, and `batch_size` is the amount of data to be generated. Note, the parameter `ring_size` **must** match what is used earlier when compiling the repository.


For example, if you are performing 3-party computation in a 64-bit ring and want to test a batch size of `10^3`, then in one terminal you'd run for party 1:
```
./build/rss_main 1 runtime-config private01.pem 3 64 1000
```
and similar commands for parties 2 and 3.
