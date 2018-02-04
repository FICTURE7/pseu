using System;
using System.Collections.Generic;
using System.IO;
using NotPseudo.CodeGen;

namespace NotPseudo
{
    public static class Program
    {
        public static void Main(string[] args)
        {
            Console.WriteLine("Running lexer on helloworld.pseudo...");

            var src = File.ReadAllText("sample/helloworld.pseudo");
            var lexer = new Lexer(src);
            var parser = new Parser(lexer);

            parser.Walk();

            /*
            var tokens = new List<Token>();
            var token = default(Token);
            while (token == null || token.Type != TokenType.EoF)
            {
                token = lexer.Lex();
                Console.WriteLine($"{token.Type}: '{token.Value}'");
            }
            */
        }
    }
}
