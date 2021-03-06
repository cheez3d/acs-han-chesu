[cecp]: https://www.gnu.org/software/xboard/engine-intf.html#9
[bitboards]: https://www.chessprogramming.org/Bitboards
[opening-book]: https://www.chessprogramming.org/Opening_Book
[demo]: doc/demo.gif

[makefile]: Makefile

# Han-Chesu Chess Engine
_Han-Chesu_ is a 2-person team effort chess engine which understands the [Chess Engine Communication Protocol][cecp], makes use of [bitboards][bitboards] for achieving high performance in its computations and also supports [opening books][opening-book]. It won 1st place in the faculty chess engine tournament against the engines of the other teams. The following is a small demonstration of playing against the engine:

![demo]

## Compiling & Running the Engine
The following commands can be used to compile and/or run the chess engine. For compiling in _debug_ mode, prefix the commands below with `DEBUG=1` (the [Makefile][makefile] compiles by default in _release_ mode).
```shell
$ make build  # build the engine
$ make run    # run the engine in the CLI
$ make xboard # run the engine in XBoard
```
