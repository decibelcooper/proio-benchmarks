# About
This repository contains code for benchmarking
[ProIO](https://github.com/go-proio).

# Usage
To run these benchmarks, Docker must be installed on your system, and you must
have permissions to use it.

```shell
docker build -t proio-bench:base base
docker build -t proio-bench:pythia8 pythia8
mkdir output
chmod 777 output
docker run --rm -it -v /proc:/writable_proc -v $PWD/output:/mnt/output proio-bench:pythia8
```

This will take some time (somewhere around an hour) to run, and you should have
at least several GB available for the `output` directory.  To rerun fresh,
clear the `output` directory and execute the `docker run` command again.

## Output

The output are some csv files and pdfs.  The CSV files give file sizes and
event rates for different benchmarks.  The 3rd column is event rate measured
with userspace CPU time, and the 4th column is event rate measured with
monotonic wall time.  The PDFs are profiling results for the benchmarks in
order to investigate features in the benchmarking data.
