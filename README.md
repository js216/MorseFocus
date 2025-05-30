# MorseFocus

Morse (CW) trainer with character sequence weighted by past performance. Each
time a mistake is made with a given character, it can be recorded, so that in
the next session this character will appear more frequently.

### Building

Pre-requisites: only `gcc` and `make` are required to build this project. The
code is written entirely in standard C99.

Clone the repository and run

    $ cd MorseFocus
    $ make

The executable files will appear under `build`.

### Command-line interface

To generate 75 random characters:

    $ ./run_gen.exe 75
    4epc=o 2/093k l6z rkg1q4p b.9,no jim?e =rjseq ux.s ckqhp=y uxh,/ 4q 6.wkh

To compare two files, showing the total number of mistakes, as well as a
breakdown per character:

    $ ./run_diff.exe sent.txt recvd.txt
    Distance: 7
    ',' : 1.000000
    '6' : 1.000000
    'h' : 4.000000
    'v' : 1.000000

To write these stats to a file for use in the future as weights:

    $ ./run_diff.exe sent.txt recvd.txt -o weights.csv

Note that `weight.csv` will contain the information in a different format,
one record per line.

To use the weights to influence future text generation:

    $ ./run_gen.exe 75 -w weights.csv
    lij 12. qh0?h 7=h2ovh p=f h/ ybhh cq at.?pr r643 lwlgy 1/l,m lhjz9 k3ej/i

To add the previous weights to the newly computed ones, so that over the course
of several sessions the more difficult characters will be shown more and more
frequently:

    $ ./run_diff.exe sent.txt recvd.txt -o weights.csv -w weights.csv

Use the `-d decay` optional argument to `run_diff` to make sure the weights
don't grow forever, monopolizing the text generation with just a few difficult
characters. For example, setting `scale` to `0.9` will decrease all weight
factors by 10% each run.

Running the utilities without any arguments will print usage information:

    $ ./run_gen.exe
    Usage: ./run_gen.exe num_char [options]
    options:
      -i MIN     set minimum word length (default 2)
      -x MAX     set maximum word length (default 7)
      -w FILE    load weights from last line of FILE
      -c CHARS   use custom charset (printable, no space)
      -o FILE    write output to FILE instead of stdout
      -s scale   multiply all weights by scale factor
      -s scale   multiply all weights by scale (default 1.0)
    
    $ ./run_diff.exe
    Usage: ./run_diff.exe f1 f2 [options]
    Options:
      -w file   load weights from file
      -d decay  scale output weights (default: 1.0)
      -o file   append output weights to file


### To do

- Makefile should run tests after each compile, so that running the final test
  program is a prerequisite of each program in the makefile
- introduce `struct record` to keep track of date/time, weights, speed, &c.,
  rather than ad hoc string manipulation
- make sure header inclusions, and function calls, form a DAG
- make test cases a lot more comprehensive, including edge cases and malformed
  input
- centralized error handling
- integer to character lookup table should be just a static const array 
- find and use a linter
- also save total length, distance, scale factor, decay
  factor, charset, and both speeds, and when reading ignore these fields
- GUI using Nuklear
- speed PID for constant accuracy

### License

BSD 2-Clause License

Copyright (c) 2025, Jakob Kastelic

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
