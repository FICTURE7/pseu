namespace NotPseudo.CodeAnalysis
{
    /* Represents a token. */
    public class Token
    {
        public Token(TokenType type, string value)
        {
            Type = type;
            Value = value;
        }

        /* Type of the token. */
        public TokenType Type;
        /* Text value of the token. */
        public string Value;

        public override string ToString()
        {
            return $"tok({Type}, {Value})";
        }
    }
}