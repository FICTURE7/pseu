using Microsoft.CodeAnalysis.Text;

namespace NotPseudo.CodeAnalysis
{
    public class Token
    {
        public TextSpan Span;
        public TokenType Type;
        public int Column;
        public int Line;
        public string Text;
    }
}