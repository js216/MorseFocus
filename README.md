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

### TODO

- Document `morsefocus` CLI interface
- Character generator should be more precise (~1%) in following the weights
- Charset should use a macro string, and weights should determine which
  characters are present
- Make a simple GUI for the program
- Add a Markov chain text generator
- Diff for word probabilities

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
