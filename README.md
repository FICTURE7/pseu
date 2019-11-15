# pseu
A wack toy programming language that resembles the pseudo code found in
the [Cambridge International A & AS Level Computer Science](https://www.amazon.com/Cambridge-International-Computer-Coursebook-Examinations/dp/1107546737/)
book.

- fast boi
- written in c99 even though it can be ported to c89 easily

#### NOTE
I've also written a working compiler which is pretty much complete in C# at the
[`roslyn`](https://github.com/FICTURE7/pseu/tree/roslyn/) branch which makes
use of the [Roslyn](https://github.com/dotnet/roslyn/) compiler platform to
compile  to IL by transpiling the code to Visual Basic first.

## Goals
The main goal is to get as close as possible to the pseudo code found in the book
and run as much of it as possible. 

## Building
To build the project, CMake is required.

```bash
mkdir build
cd build
cmake ..
make
```

If MSVC is targeted, a Visual Studio solution file should be generated instead
of a Makefile.

## Acknowledgement
- wren
- lua
- luajit

## License
[MIT License](LICENSE)
