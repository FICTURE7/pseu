using Microsoft.CodeAnalysis.Text;

namespace NotPseudo.CodeAnalysis
{
    public class Token
    {
        public TextSpan Span;
        public TokenType Type;
        public string Text;
    }
}