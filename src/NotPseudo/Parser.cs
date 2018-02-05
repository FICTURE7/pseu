using System;
using System.Collections.Generic;
using NotPseudo.Syntax;

namespace NotPseudo
{
    public class Parser : IParser
    {
        private Token _token;
        private readonly Node _root;
        private readonly Lexer _lexer;

        public Parser(Lexer lexer)
        {
            if (lexer == null)
                throw new System.ArgumentNullException(nameof(lexer));

            _lexer = lexer;
            _root = new Node();
        }

        public Node Parse()
        {
            while (true)
            {
                var node = Walk();
                _root.Childrens.Add(node);

                /* Keep parsing until we reach end of file. */
                if (_token.Type == TokenType.EoF)
                    break;
            }

            return _root;
        }

        private Node Walk()
        {
            _token = _lexer.Lex();
            if (_token.Type == TokenType.IdentifierOrKeyword)
            {
                switch (_token.Value)
                {
                    case "OUTPUT":
                        return ParseOutputStatement(_token);

                    case "DECLARE":
                        return ParseVariableDeclarationStatement(_token);

                    default:
                        /* Assume its an assign statement. */
                        return ParsePossibleAssignStatement(_token);
                }
            }
            else if (_token.Type == TokenType.StringLiteral)
            {
                return ParseStringLiteral(_token);
            }
            else if (_token.Type == TokenType.NumberLiteral)
            {
                //return ParseNumberLiteral(_token);
            }
            return null;
        }

        private Node ParsePossibleAssignStatement(Token token)
        {
            var nextToken = _lexer.Lex();
            if (nextToken.Type == TokenType.Equal)
            {
                var expression = Walk();
                var stmt = new AssignStatement
                {
                    Identifier = token.Value,
                    Expression = expression
                };

                _token = _lexer.Lex();

                if (_token.Type != TokenType.EoF && _token.Type != TokenType.EoL)
                    throw new Exception("Expected EoL or EoF.");

                return stmt;
            }
            return null;
        }

        private VariableDeclarationStatement ParseVariableDeclarationStatement(Token token)
        {
            var identifierToken = _lexer.Lex();
            if (identifierToken.Type != TokenType.IdentifierOrKeyword)
                throw new Exception("Expected variable name identifier.");

            var identifier = identifierToken.Value;

            var colonToken = _lexer.Lex();
            if (colonToken.Type != TokenType.Colon)
                throw new Exception("Expected colon.");

            var typeToken = _lexer.Lex();
            if (typeToken.Type != TokenType.IdentifierOrKeyword)
                throw new Exception("Expected variable type identifier.");

            var type = typeToken.Value;

            var stmt = new VariableDeclarationStatement
            {
                Identifier = identifier,
                Type = type
            };

            /*TODO: Expect '=' expression maybe? */

            _token = _lexer.Lex();

            if (_token.Type != TokenType.EoF && _token.Type != TokenType.EoL)
                throw new Exception("Expected EoL or EoF.");

            return stmt;
        }

        private OutputStatement ParseOutputStatement(Token token)
        {
            var stmt = new OutputStatement();

            /*TODO: Check if node returned by walk is a valid one. */
            stmt.Expression = ParseOutputStatementExpression();

            _token = _lexer.Lex();

            if (_token.Type != TokenType.EoF && _token.Type != TokenType.EoL)
                throw new Exception("Expected EoL or EoF.");

            return stmt;
        }

        private Node ParseOutputStatementExpression()
        {
            _token = _lexer.Lex();
            if (_token.Type == TokenType.StringLiteral)
                return ParseStringLiteral(_token);
            else if (_token.Type == TokenType.IdentifierOrKeyword)
            {
                var identifierToken = _token;

                /*
                _token = _lexer.Lex();
                if (_token.Type != TokenType.EoF && _token.Type != TokenType.EoL)
                    throw new Exception("Expected EoL or EoF.");
                */

                return new VariableStatement
                {
                    Identifier = identifierToken.Value
                };
            }
            return null;
        }

        private Node ParseStringLiteral(Token token)
        {
            return new StringLiteralExpression { Value = token.Value };
        }

        private static bool IsBuiltInFunction(Token token)
        {
            return token.Value == "OUTPUT";
        }
    }
}