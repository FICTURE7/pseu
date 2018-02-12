using System;
using System.Collections;
using System.Collections.Generic;
using Microsoft.CodeAnalysis.Text;
using NotPseudo.CodeAnalysis;
using Xunit;

namespace NotPseudo.Tests.CodeAnalysis
{
    public class LexerTests
    {
        [Fact]
        public void Ctor__src_Null__ThrowException()
        {
            var ex = Assert.Throws<ArgumentNullException>(() => new Lexer(null));
        }

        [Theory]
        [ClassData(typeof(LexerDataGenerator))]
        public void Lex__Returns_ExpectedToken(string src, (TokenType type, string value)[] expectations)
        {
            var lexer = new Lexer(SourceText.From(src));

            for (int i = 0; i < expectations.Length; i++)
            {
                var expected = expectations[i];
                var token = lexer.Lex();

                Assert.Equal(expected.type, token.Type);
                Assert.Equal(expected.value, token.Value);
            }
        }

        private class LexerDataGenerator : IEnumerable<object[]>
        {
            private static readonly List<object[]> _data = new List<object[]>
            {
                new object[] {
                    "1+4",
                    new (TokenType, string)[] {
                        (TokenType.NumberLiteral, "1"),
                        (TokenType.Plus, "+"),
                        (TokenType.NumberLiteral, "4"),
                        (TokenType.EoF, null)
                    }
                },
                new object[] {
                    "1+ 4",
                    new (TokenType, string)[] {
                        (TokenType.NumberLiteral, "1"),
                        (TokenType.Plus, "+"),
                        (TokenType.NumberLiteral, "4"),
                        (TokenType.EoF, null)
                    }
                },
                new object[] {
                    "120 + 50",
                    new (TokenType, string)[] {
                        (TokenType.NumberLiteral, "120"),
                        (TokenType.Plus, "+"),
                        (TokenType.NumberLiteral, "50"),
                        (TokenType.EoF, null)
                    }
                },
                new object[] {
                    "120 + 50 + 100",
                    new (TokenType, string)[] {
                        (TokenType.NumberLiteral, "120"),
                        (TokenType.Plus, "+"),
                        (TokenType.NumberLiteral, "50"),
                        (TokenType.Plus, "+"),
                        (TokenType.NumberLiteral, "100"),
                        (TokenType.EoF, null)
                    }
                },
                new object[] {
                    "120 + 50 - 100 * 190 / 30",
                    new (TokenType, string)[] {
                        (TokenType.NumberLiteral, "120"),
                        (TokenType.Plus, "+"),
                        (TokenType.NumberLiteral, "50"),
                        (TokenType.Minus, "-"),
                        (TokenType.NumberLiteral, "100"),
                        (TokenType.Multiply, "*"),
                        (TokenType.NumberLiteral, "190"),
                        (TokenType.Divide, "/"),
                        (TokenType.NumberLiteral, "30"),
                        (TokenType.EoF, null)
                    }
                },
                new object[] {
                    "60 + (30 / 3)",
                    new (TokenType, string)[] {
                        (TokenType.NumberLiteral, "60"),
                        (TokenType.Plus, "+"),
                        (TokenType.LeftParenthesis, "("),
                        (TokenType.NumberLiteral, "30"),
                        (TokenType.Divide, "/"),
                        (TokenType.NumberLiteral, "3"),
                        (TokenType.RightParenthesis, ")"),
                        (TokenType.EoF, null)
                    }
                },
                new object[] {
                    "y + x",
                    new (TokenType, string)[] {
                        (TokenType.Identifier, "y"),
                        (TokenType.Plus, "+"),
                        (TokenType.Identifier, "x")
                    }
                },
                new object[] {
                    "60 + (30 / 3) + x",
                    new (TokenType, string)[] {
                        (TokenType.NumberLiteral, "60"),
                        (TokenType.Plus, "+"),
                        (TokenType.LeftParenthesis, "("),
                        (TokenType.NumberLiteral, "30"),
                        (TokenType.Divide, "/"),
                        (TokenType.NumberLiteral, "3"),
                        (TokenType.RightParenthesis, ")"),
                        (TokenType.Plus, "+"),
                        (TokenType.Identifier, "x"),
                        (TokenType.EoF, null)
                    }
                },
                new object[] {
                    "DECLARE x",
                    new (TokenType, string)[] {
                        (TokenType.DeclareKeyword, "DECLARE"),
                        (TokenType.Identifier, "x"),
                        (TokenType.EoF, null)
                    }
                },
                new object[] {
                    "DECLARE x : INTEGER",
                    new (TokenType, string)[] {
                        (TokenType.DeclareKeyword, "DECLARE"),
                        (TokenType.Identifier, "x"),
                        (TokenType.Colon, ":"),
                        (TokenType.Identifier, "INTEGER"),
                        (TokenType.EoF, null)
                    }
                },
                new object[] {
                    "x <- 1",
                    new (TokenType, string)[] {
                        (TokenType.Identifier, "x"),
                        (TokenType.Assign, "<-"),
                        (TokenType.NumberLiteral, "1"),
                        (TokenType.EoF, null)
                    }
                },
                new object[] {
                    "x <- 1\ny <- 2",
                    new (TokenType, string)[] {
                        (TokenType.Identifier, "x"),
                        (TokenType.Assign, "<-"),
                        (TokenType.NumberLiteral, "1"),
                        (TokenType.LineFeed, "\n"),
                        (TokenType.Identifier, "y"),
                        (TokenType.Assign, "<-"),
                        (TokenType.NumberLiteral, "2"),
                        (TokenType.EoF, null)
                    }
                }
            };

            public IEnumerator<object[]> GetEnumerator() => _data.GetEnumerator();
            IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
        }
    }
}
