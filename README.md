# NotPseudo
A wack programming language that resembles the pseudo code found in 
the [Cambridge International A & AS Level Computer Science](https://www.amazon.com/Cambridge-International-Computer-Coursebook-Examinations/dp/1107546737/)
book.

#### NOTE
This is still a work in progress thing and cannot do much at the moment until
things are fleshed out. But I've also written a working compiler which is pretty 
much complete in C# at the [`csharp`](https://github.com/FICTURE7/notpseudo/tree/csharp/) 
branch which makes use of the [Roslyn](https://github.com/dotnet/roslyn/) compiler platform 
to compile to IL by transpiling the code to Visual Basic.

## Why?
Because why not? ¯\\\_( ͡° ͜ʖ ͡°)\_/¯

To have a better understanding of how the frontend of compilers and interpreters works
and the pseudo code found in the book seem to have enough structure to be made into an
actual programming language.

### Why C?
Because real men manages their memory allocations. ( ͡° ͜ʖ ͡°)

I always wanted to write something which had a certain level of complexity in C
so I figured this was the perfect project for this.

## Goals
The main goal is to get as close as possible to the pseudo code found in the book and run
as much of it as possible. 

And also to give decent errors and warnings. High performance is not a goal of this project
because who would do anything compute-intensive with this. So compromising performance for
slightly fancier/simpler code should be OK, I think.

#### ETA
I will try to get this 'done' before the start of the third trimester which should be around
August, so I can actually start studying/revising to get decent grades XD.

#### Maybes?
Ideas & features which may be implemented, but likely not. ¯\\\_(ツ)\_/¯

* Transpiling to Python, Visual Basic, Pascal/Delphi
* LLVM backend
* Debugging

### License
MIT License
