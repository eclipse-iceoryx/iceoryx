## benchmark_optional_and_expected

### Howto Perform a Benchmark
In bash you could execute this one liner directly in the root directory of iceoryx.
```sh
cd iceoryx
for i in $(seq 0 3); do
    echo Optimization level $i
    g++ iceoryx_examples/benchmark_optional_and_expected/benchmark_optional_and_expected.cpp iceoryx_hoofs/time/source/duration.cpp -Iiceoryx_hoofs/include -Iiceoryx_platform/linux/include -pthread -O$i
    ./a.out
done
```

If you compile it with our default cmake settings you would only compile it in
release mode which is naturally faster than debug mode since it does not inject debug symbols and uses optimization level `-O2`.

This shell command on the other hand does compile the benchmark for all 4 optimization
levels from `-O0` to `-O3` and prints the result for every level.

### Results (obtained from gcc-10.1.0)
How many calls could be performed. Higher is better.

**Optimization Level -O0**

| Test Case             | Plain       | Optional    | Expected    |
|----------------------:|:-----------:|:-----------:|:-----------:|
|simpleReturn           |**225524291**|47035725     |             |
|popFromFiFo            |**138717750**|26948663     |             |
|complexErrorValue      |**140045308**|             |14254503     |
|largeObject            |**143401942**|27128388     |12922971     |
|largeObjectComplexCTor |463355       |**26493097** |13212906     |

**Optimization Level -O1**

| Test Case             | Plain       | Optional    | Expected    |
|----------------------:|:-----------:|:-----------:|:-----------:|
|simpleReturn           |646117228    |**906108218**|             |
|popFromFiFo            |295797403    |**329602169**|             |
|complexErrorValue      |**359035415**|             |184509271    |
|largeObject            |379375596    |**379646378**|64153014     |
|largeObjectComplexCTor |2614313      |**347282395**|64807330     |

**Optimization Level -O2**

| Test Case             | Plain       | Optional    | Expected    |
|----------------------:|:-----------:|:-----------:|:-----------:|
|simpleReturn           |639481388    |**906377309**|             |
|popFromFiFo            |580475112    |**637568187**|             |
|complexErrorValue      |566417871    |             |**623039921**|
|largeObject            |615844467    |**656525440**|450771530    |
|largeObjectComplexCTor |446405409    |**637818315**|517100558    |

**Optimization Level -O3**

| Test Case             | Plain       | Optional    | Expected    |
|----------------------:|:-----------:|:-----------:|:-----------:|
|simpleReturn           |644953494    |**887507023**|             |
|popFromFiFo            |580091861    |**585706644**|             |
|complexErrorValue      |566601028    |             |**619879032**|
|largeObject            |**578625649**|462344358    |463635845    |
|largeObjectComplexCTor |445014611    |453039198    |**521274398**|
