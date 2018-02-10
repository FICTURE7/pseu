using Microsoft.CodeAnalysis.Text;

namespace NotPseudo.CodeAnalysis
{
    /* Represents a token. */
    public class Token
    {
        public TextSpan Span;

        /* Type of the token. */
        public TokenType Type;
        /* Text value of the token. */
        public string Text;

        public override string ToString()
        {
            return $"Token({Type}, {Text})";
        }
    }
}