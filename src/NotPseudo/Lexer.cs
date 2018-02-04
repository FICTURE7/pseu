using System;
using System.Diagnostics;

namespace NotPseudo
{
    public class Lexer : ILexer
    {
        private const char InvalidChar = char.MaxValue;

        private int _index;
        private readonly string _src;

        public Lexer(string src)
        {
            _src = src ?? throw new ArgumentNullException(nameof(src));

            _index = 0;
        }

        public Token Lex()
        {
            /* Skip whitespaces including \t and stuff. */
            SkipSpaces();
            return ScanToken();
        }

        private Token ScanToken()
        {
            var c = CurrentChar();
            if (c == InvalidChar)
                return Create(null, TokenType.EoF);

            if (char.IsLetter(c))
                return ScanIdentifier();
            else if (c == '"')
                return ScanStringLiteral();

            throw new Exception("Unknown token.");
        }

        private Token ScanIdentifier()
        {
            /*TODO: Identifiers should be able to start with '_'. */
            var c = CurrentChar();
            var value = string.Empty;

            while (char.IsLetter(c))
            {
                value += c;
                c = NextChar();
            }

            Debug.Assert(!char.IsLetter(CurrentChar()), "Current character should not be a letter.");

            return Create(value, TokenType.Identifier);
        }

        private Token ScanStringLiteral()
        {
            char c = NextChar();
            string value = string.Empty;

            while (c != '"')
            {
                value += c;
                c = NextChar();

                if (c == InvalidChar)
                {
                    /*TOOD: Add error, the string literal is unterminated. */
                    break;
                }
            }

            /* Skip the closing '"' character since we already processed it. */
            NextChar();

            return Create(value, TokenType.StringLiteral);
        }

        private void SkipSpaces()
        {
            char c = CurrentChar();
            if (c == InvalidChar)
                return;

            while (char.IsWhiteSpace(c))
                c = NextChar();

            Debug.Assert(!char.IsWhiteSpace(_src[_index]), "Current character should not be a whitespace.");
        }

        private char CurrentChar()
        {
            if (_index > _src.Length - 1)
                return InvalidChar;

            return _src[_index];
        }

        private char NextChar()
        {
            if (++_index > _src.Length - 1)
                return InvalidChar;

            return _src[_index];
        }

        private char PeekChar()
        {
            if (_index + 1 > _src.Length - 1)
                return InvalidChar;

            return _src[_index + 1];
        }

        private Token Create(string value, TokenType type)
        {
            return new Token { Value = value, Type = type };
        }
    }
}