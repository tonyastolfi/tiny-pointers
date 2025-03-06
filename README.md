# Tiny Pointers

This repo contains a partial implementation of the ideas from: [Tiny Pointers](https://dl.acm.org/doi/10.1145/3700594).

## Test Output

```
[==========] Running 5 tests from 4 test suites.
[----------] Global test environment set-up.
[----------] 1 test from UtilTest
[ RUN      ] UtilTest.ScaleU64
[       OK ] UtilTest.ScaleU64 (0 ms)
[----------] 1 test from UtilTest (0 ms total)

[----------] 2 tests from TinyPointersTest
[ RUN      ] TinyPointersTest.SimpleDereferenceTable
 sdt.load_factor() == 0.958333
 sdt.n_slots() == 10285056
 sdt.capacity() == 9856512
 usize{1} << sdt.tiny_pointer_size() == 524288
 sdt.slots_per_bucket() == 331776
 sdt.bucket_count() == 31
 std::log2(sdt.n_slots()) == 23.294
 sdt.log_n() == 24
 sdt.tiny_pointer_size() == 19
 4 * std::log2(std::log2(sdt.n_slots())) == 18.1676
[       OK ] TinyPointersTest.SimpleDereferenceTable (171 ms)
[ RUN      ] TinyPointersTest.SimpleDereferenceTable_LoadFactor
Connection to localhost closed.
 avg_load_factor == 0.996421 load_factor == 0.958333
[       OK ] TinyPointersTest.SimpleDereferenceTable_LoadFactor (22064 ms)
[----------] 2 tests from TinyPointersTest (22235 ms total)

[----------] 1 test from DataTest
[ RUN      ] DataTest.Load
 words.size() == 542778 word_set.size() == 249607
[       OK ] DataTest.Load (77 ms)
[----------] 1 test from DataTest (77 ms total)

[----------] 1 test from BitVecTest
[ RUN      ] BitVecTest.Test
[       OK ] BitVecTest.Test (0 ms)
[----------] 1 test from BitVecTest (0 ms total)

[----------] Global test environment tear-down
[==========] 5 tests from 4 test suites ran. (22312 ms total)
[  PASSED  ] 5 tests.

```
