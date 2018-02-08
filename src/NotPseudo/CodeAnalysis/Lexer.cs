using System;
using System.Diagnostics;
using Microsoft.CodeAnalysis.Text;

namespace NotPseudo.CodeAnalysis
{
    /* The tokenizer/lexer. */
    public class Lexer : ILexer
    {
        /* 
            A character which represents an invalid character,
            usually used to indicate end of file. 
         */
        private const char InvalidChar = char.MaxValue;

        /* Line number we're at. */
        private int _line;
        /* Column number we're at. */
        private int _column;
        /* Index of were we are in the source. */
        private int _index;
        /* Source code we're lexing. */
        private readonly SourceText _src;

        public Lexer(SourceText src)
        {
            if (src == null)
                throw new ArgumentNullException(nameof(src));

            _src = src;
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

            /*TODO: Do proper scanning of end of lines. */
            if (c == '\n')
                return ScanEndOfLine();
            else if (c == ':')
                return ScanColon();
            else if (c == '=')
                return ScanEqual();
            else if (c == '"')
                return ScanStringLiteral();
            else if (char.IsNumber(c))
                return ScanNumberLiteral();
            else if (char.IsLetter(c))
                return ScanIdentifierOrKeyword();

            throw new Exception("Unknown token.");
        }

        private Token ScanEqual()
        {
            Debug.Assert(CurrentChar() == '=', "Current character was not an equal character.");

            AdvanceChar();
            return Create(null, TokenType.Equal);
        }

        private Token ScanColon()
        {
            Debug.Assert(CurrentChar() == ':', "Current character was not a colon character.");

            AdvanceChar();
            return Create(null, TokenType.Colon);
        }

        private Token ScanEndOfLine()
        {
            Debug.Assert(CurrentChar() == '\n', "Current character was not a line feed character.");

            AdvanceChar();
            _column++;
            _line = 0;
            return Create(null, TokenType.EoL);
        }

        private Token ScanNumberLiteral()
        {
            /*TODO: Decimals and stuffs. */
            var c = CurrentChar();
            var value = string.Empty;

            while (char.IsNumber(c))
            {
                value += c;
                c = NextChar();
            }

            return Create(value, TokenType.NumberLiteral);
        }

        private Token ScanIdentifierOrKeyword()
        {
            var c = CurrentChar();
            var value = string.Empty;

            while (char.IsLetter(c) || c == '_')
            {
                value += c;
                c = NextChar();
            }

            Debug.Assert(!char.IsLetter(CurrentChar()) && c != '_', "Current character should not be a letter or '_'.");

            return Create(value, TokenType.IdentifierOrKeyword);
        }

        private Token ScanStringLiteral()
        {
            /* TODO: Escaped characters and stuff. */
            char c = NextChar();
            string value = string.Empty;

            while (c != '"')
            {
                value += c;
                c = NextChar();

                if (c == InvalidChar)
                {
                    /*TODO: Add error, the string literal is unterminated. */
                    break;
                }
            }

            /* Skip the closing '"' character since we already processed it. */
            AdvanceChar();

            return Create(value, TokenType.StringLiteral);
        }

        private void SkipSpaces()
        {
            char c = CurrentChar();
            if (c == '\n')
                return;

            if (c == InvalidChar)
                return;

            while (char.IsWhiteSpace(c))
                c = NextChar();

            Debug.Assert(!char.IsWhiteSpace(CurrentChar()), "Current character should not be a whitespace.");
        }

        private void AdvanceChar()
        {
            _index++;
            _line++;
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

            _line++;
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
            return new Token
            {
                Text = value,
                Type = type,

                Column = _column,
                Line = _line
            };
        }
    }
}