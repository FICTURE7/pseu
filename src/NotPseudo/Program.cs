﻿using System;
using System.Collections.Generic;
using System.IO;
using Microsoft.CodeAnalysis.Text;
using NotPseudo.CodeAnalysis;
using NotPseudo.CodeAnalysis.Syntax;
using NotPseudo.Transpilers;

namespace NotPseudo
{
    public static class Program
    {
        public static void Main(string[] args)
        {
            Transpile<VisualBasicTranspiler>("sample/expression.pseudo");
            Transpile<VisualBasicTranspiler>("sample/for-loop.pseudo");
            Transpile<VisualBasicTranspiler>("sample/helloworld.pseudo");
            Transpile<VisualBasicTranspiler>("sample/more-helloworld.pseudo");
            Transpile<VisualBasicTranspiler>("sample/even-more-helloworld.pseudo");
        }

        private static void Transpile<TTranspiler>(string srcPath) where TTranspiler : ITranspiler, new()
        {
            Console.WriteLine($"Running syntatic analysis on {Path.GetFileName(srcPath)}...");
            Console.WriteLine("-------------------");
            var src = File.ReadAllText(srcPath);
            var lexer = new Lexer(SourceText.From(src));
            var parser = new Parser(lexer);

            var ast = (ProgramBlock)parser.Parse();

            var gen = new TTranspiler();
            var code = gen.Generate(ast);

            Console.WriteLine(code);
            Console.WriteLine("-------------------");
        }
    }
}
