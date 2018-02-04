using System;
using System.Collections.Generic;
using System.IO;
using NotPseudo.Transpilers;

namespace NotPseudo
{
    public static class Program
    {
        public static void Main(string[] args)
        {
            Console.WriteLine("Running syntatic analysis on helloworld.pseudo...");

            var src = File.ReadAllText("sample/helloworld.pseudo");
            var lexer = new Lexer(src);
            var parser = new Parser(lexer);

            var ast = parser.Parse();

            var gen = new VisualBasicTranspiler();
            var code = gen.Generate(ast);

            Console.WriteLine(code);
        }
    }
}
