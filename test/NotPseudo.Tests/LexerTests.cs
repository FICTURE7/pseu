using System;
using Xunit;
using NotPseudo.CodeAnalysis;

namespace NotPseudo.Tests
{
    public class LexerTests
    {
        [Fact]
        public void Ctor__src_Null__ThrowException()
        {
            try
            {
                new Lexer(null);
            }
            catch(ArgumentNullException)
            {
                Assert.True(true);
            }

            Assert.False(false);
        }
    }
}
