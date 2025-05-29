# MorseFocus

Morse (CW) trainer with character sequence weighted by past performance. Each
time a mistake is made with a given character, it is recorded, so that in the
next session this character will appear more frequently.

### Building

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
in a one record per line manner.

To use the weights to influence future text generation:

    $ ./run_gen.exe 75 -w weights.csv
    lij 12. qh0?h 7=h2ovh p=f h/ ybhh cq at.?pr r643 lwlgy 1/l,m lhjz9 k3ej/i

To add the previous weights to the newly computed ones, so that over the course
of several sessions the more difficult characters will be shown more and more
frequently:

    $ ./run_diff.exe sent.txt recvd.txt -o weights.csv -w weights.csv

Use the `-s scale` optional argument to `run_diff` to make sure the weights
don't grow forever, monopolizing the text generation with just a few difficult
characters. For example, setting `scale` to `0.9` will decrease all weight
factors by 10% each run.

Running the utilities without any arguments will print usage information:

    $ ./run_gen.exe
    Usage: ./run_gen.exe num_char [options]
    options:
      -s MIN     set minimum word length (default 2)
      -x MAX     set maximum word length (default 7)
      -w FILE    load weights from last line of FILE
      -c CHARS   use custom charset (printable, no space)
      -o FILE    write output to FILE instead of stdout
    
    $ ./run_diff.exe
    Usage: ./run_diff.exe f1 f2 [options]
    Options:
      -w file   load weights from file
      -s scale  scale output weights (default: 1.0)
      -o file   append output weights to file


### Author

Jakob Kastelic
