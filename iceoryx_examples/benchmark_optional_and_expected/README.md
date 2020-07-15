# benchmark_optional_and_expected - Or howto benchmark and optimize correctly

## The Benchmark 

### Howto Perform a Benchmark
In bash you could execute this one liner directly in the root directory of iceoryx.
```sh 
cd iceoryx
for i in $(seq 0 3); do 
    echo Optimization level: $i
    g++ iceoryx_examples/benchmark_optional_and_expected/benchmark_optional_and_expected.cpp iceoryx_utils/source/units/duration.cpp -Iiceoryx_utils/include -Iiceoryx_utils/platform/linux/include -pthread -O$i
    ./a.out;
done
```

If you compile it with our default cmake settings you would only compile it in 
debug mode which is naturally slower since it is optimized for debugging and not 
performance.

This shell command on the other hand does compile the benchmark for all 4 optimization
levels from `-O0` to `-O3` and prints the result for every level.

### Results

**Optimization Level -O0**

| Test Case | Plain | Optional | Expected |
|----------:|:-----:|:--------:|:--------:|
|simpleReturn|**225524291**|47035725||
|popFromFiFo|**138717750**|26948663||
|complexErrorValue|**140045308**||14254503|

**Optimization Level -O1**

| Test Case | Plain | Optional | Expected |
|----------:|:-----:|:--------:|:--------:|
|simpleReturn|646117228|**906108218**||
|popFromFiFo|295797403|**329602169**||
|complexErrorValue|**359035415**||184509271|

**Optimization Level -O2**

| Test Case | Plain | Optional | Expected |
|----------:|:-----:|:--------:|:--------:|
|simpleReturn|639481388|**906377309**||
|popFromFiFo|580475112|**637568187**||
|complexErrorValue|566417871||**623039921**|

**Optimization Level -O3**

| Test Case | Plain | Optional | Expected |
|----------:|:-----:|:--------:|:--------:|
|simpleReturn|644953494|**887507023**||
|popFromFiFo|580091861|**585706644**||
|complexErrorValue|566601028||**619879032**|
