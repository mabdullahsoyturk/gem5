### Building GEM5

To build gem5, you will need the following software:

  - g++ or clang
  - python (gem5 links in the Python interpreter)
  - SCons
  - SWIG
  - zlib
  - m4
  - protobuf (if you want trace capture and playback support)

On Ubuntu, you can install all of the required dependencies with the following command: 
```sh
$ sudo apt install build-essential git m4 scons zlib1g zlib1g-dev libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev python-dev python
```

Once you have all dependencies resolved:
```sh
$ cd gem5
$ scons build/X86/gem5.opt -j9
```

j parameter is the number of cores you have + 1.

### Running GEM5
With the simulator built, have a look at http://www.gem5.org/Running_gem5 for more information on how to use gem5. Since we have developed this repository to be used in another repository as a submodule, the running command is given by a script in the parent repository (https://github.com/koparasy/FaultModel).

### Project Structure

The basic source release includes these subdirectories:
   - configs: example simulation configuration scripts
   - ext: less-common external packages needed to build gem5
   - src: source code of the gem5 simulator
   - system: source for some optional system software for simulated systems
   - tests: regression tests
   - util: useful utility programs and files
