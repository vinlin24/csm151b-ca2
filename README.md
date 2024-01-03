# Memory Hierarchy Simulator

This project involves designing a memory hierarchy. We implement a memory
controller to simulate the actions of a multi-level cache system (tagging,
evictions, etc.). Details descriptions can be found in [the spec](CA2-spec.pdf).


## Building

Use the following Makefile rule to build the executable:

```sh
make
```

Or without debug flags:

```sh
make build
```


## Running

Run the executable using a *trace file* as input:

```sh
./memory_driver path/to/trace.txt
```

Example trace files are provided under [traces/](traces/).


## Cleanup

To clean the directory of build files, run the Makefile rule:

```sh
make clean
```
