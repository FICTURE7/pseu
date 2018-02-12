using System;
using Microsoft.CodeAnalysis.Text;
using NotPseudo.CodeAnalysis;
using Xunit;

namespace NotPseudo.Tests.CodeAnalysis
{
    public class ParserTests
    {
        [Fact]
        public void Ctor__lexer_Null__ThrowsException()
        {
            Assert.Throws<ArgumentNullException>(() => new Parser(null));
        }

        [Fact]
        public void Parse__xD()
        {
            var lexer = new Lexer(SourceText.From("litfam <- 1234\nFOR i <- 1 TO 10\nxD <- i\nNEXT\nOUTPUT xD"));
            var parser = new Parser(lexer);

            var expr = parser.Parse();
        }
    }
}