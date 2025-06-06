# MorseFocus

Morse (CW) trainer with character sequence weighted by past performance. Each
time a mistake is made with a given character, it can be recorded, so that in
the next session this character will appear more frequently.

### Building

Pre-requisites: only `gcc` and `make` are required to build the command-line
tools. The code is written entirely in standard C99.

Clone the repository and run

    $ cd MorseFocus
    $ make

The executable files will appear under `build`.

To run static code analysis, run `make check`, for which we need

- `clang-tidy`
- `clang-format`
- `scan-build`
- `jq`

### Command-line interface

#### Random character generation

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

    $ ./run_gen
    Usage: ./run_gen num_char [options]
    options:
      -i MIN     set minimum word length (default 2)
      -x MAX     set maximum word length (default 7)
      -w FILE    load weights from last line of FILE
      -c CHARS   use custom charset (printable, no space)
      -o FILE    write output to FILE instead of stdout
      -s SCALE   multiply all weights by scale (default 1.0)

    $ ./run_diff
    Usage: ./run_diff f1 f2 [options]
    Options:
      -w FILE    load weights from file
      -d DECAY   scale output weights (default: 1.0)
      -o FILE    append output weights to file
      -s SCALE   scale to record to file (default: 0)
      -1 SPEED1  first speed to record to file (default: 0)
      -2 SPEED2  second speed to record to file (default: 0)
      -c CHARSET charset to record to file (default: "~")

#### Random word generation

### TODO

- Character generator should be more precise (~1%) in following the weights
- Charset should use a macro string, and weights should determine which
  characters are present
- Tiny GUI program to display the progress of weights over time, and
  github/lcwo-style squares for streak tracing
- Status bar is also where startup notifications get written to, rather than
  randomly across the splash screen
- Status tab (under Settings) displays uptime as communicated from motherboard.
  FP firmware also keeps its own uptime, and when there's a significant mismatch
  between the two, it displays an error message.
- Interactive CLI app
- Document `run_words`
- Diff should output recommended speed for the next run, using the formula
  `new_speed = old_speed * (1 - k*(error_rate - 0.1))`
- Diff for word probabilities
- Add a Markov chain text generator
- Make a simple GUI for the program

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
