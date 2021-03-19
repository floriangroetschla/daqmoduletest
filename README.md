# Write tests for SNB buffer
This repo contains tests that use produer/consumer pairs implemented as `DAQModule`s which push data of a specified size from the producer to the consumer. 
Both are connected by a `FollySPSCQueue` and the consumer writes the data to the specified directory. 
The goal is to test different ways to write the files and measure their performance. 

To run the tests, first setup the dbt environment for dunedaq-v2.3.0 as described [here](https://github.com/DUNE-DAQ/appfwk/wiki/Compiling-and-running-under-v2.3.0).
Then, go to the sourcecode folder and clone this repo. 
To start the tests, run
```
python sourcecode/daqmoduletest/python/run_tests -o output_directory result.csv
```
The results will be written to `results.csv` once the tests are finished.
To get information for all the options, run the script with `-h`.
