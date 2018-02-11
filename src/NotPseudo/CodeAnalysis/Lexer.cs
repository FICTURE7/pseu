using System;
using System.Diagnostics;
using Microsoft.CodeAnalysis.Text;

namespace NotPseudo.CodeAnalysis
{
    /* The lexer/tokenizer. */
    public class Lexer : ILexer
    {
        /* 
            A character which represents an invalid character,
            usually used to indicate end of file. 
         */
        private const char InvalidChar = char.MaxValue;

        /* Index of were we are in the source. */
        private int _pos;
        /* Current character we're processing. */
        private char _cur;
        /* Source code we're lexing. */
        private readonly SourceText _src;

        public Lexer(SourceText src)
        {
            if (src == null)
                throw new ArgumentNullException(nameof(src));

            _src = src;
            _pos = 0;

            /* 
                Check if src is empty, if it is set current char 
                to InvalidChar to indicate EoF.
             */
            if (_pos > src.Length - 1)
                _cur = InvalidChar;
            else
                _cur = src[_pos];
        }

        public Token Lex()
        {
            /* If end of file, emit EoF token. */
            if (_cur == InvalidChar)
                return new Token(TokenType.EoF, null);

            /* Skip whitespaces if we encounter any. */
            if (char.IsWhiteSpace(_cur))
                SkipWhiteSpace();

            /* Scan number literals. */
            if (char.IsDigit(_cur))
                return new Token(TokenType.NumberLiteral, ScanNumberLiteral());

            /* Scan identifiers or keywords. */
            if (char.IsLetter(_cur))
                return ScanIdentifierOrKeyword();

            /* Scan the plus character. */
            if (_cur == '+')
            {
                Adavance();
                return new Token(TokenType.Plus, "+");
            }

            /* Scan the minus character. */
            if (_cur == '-')
            {
                Adavance();
                return new Token(TokenType.Minus, "-");
            }

            /* Scan the divide character. */
            if (_cur == '/')
            {
                Adavance();
                return new Token(TokenType.Divide, "/");
            }

            /* Scan the multiply character. */
            if (_cur == '*')
            {
                Adavance();
                return new Token(TokenType.Multiply, "*");
            }

            /* Scan the left parenthesis character. */
            if (_cur == '(')
            {
                Adavance();
                return new Token(TokenType.LeftParenthesis, "(");
            }

            /* Scan the right parenthesis character. */
            if (_cur == ')')
            {
                Adavance();
                return new Token(TokenType.RightParenthesis, ")");
            }

            if (_cur == ':')
            {
                Adavance();
                return new Token(TokenType.Colon, ":");
            }

            Error();

            /* Will never be reached, since Error throws an exception. */
            return null;
        }

        private Token ScanIdentifierOrKeyword()
        {
            var value = (string)null;
            while (_cur != InvalidChar && char.IsLetter(_cur))
            {
                value += _cur;
                Adavance();
            }

            /* Check if value is a keyword. */
            if (value == "DECLARE")
                return new Token(TokenType.DeclareKeyword, value);                

            return new Token(TokenType.Identifier, value);
        }

        private string ScanNumberLiteral()
        {
            var value = (string)null;
            while (_cur != InvalidChar && char.IsDigit(_cur))
            {
                value += _cur;
                Adavance();
            }
            return value;
        }

        private void SkipWhiteSpace()
        {
            while (_cur != InvalidChar && char.IsWhiteSpace(_cur))
                Adavance();
        }

        private void Adavance()
        {
            /* 
                Sets the next character to process or 
                sets to InvalidChar if end of file.
            */
            _pos++;

            if (_pos > _src.Length - 1)
                _cur = InvalidChar;
            else
                _cur = _src[_pos];
        }

        private void Error() => throw new Exception("Invalid character.");
    }
}