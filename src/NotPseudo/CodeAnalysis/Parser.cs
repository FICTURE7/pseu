using System;
using System.Collections.Generic;
using NotPseudo.CodeAnalysis.Syntax;

namespace NotPseudo.CodeAnalysis
{
    public class Parser : IParser
    {
        private Token _token;
        private readonly Node _root;
        private readonly Lexer _lexer;

        public Parser(Lexer lexer)
        {
            if (lexer == null)
                throw new ArgumentNullException(nameof(lexer));

            _lexer = lexer;
            _root = new Node();
        }

        public Node Parse()
        {
            return Parse(_root);
        }

        private Node Parse(Node root)
        {
            while (true)
            {
                var node = Walk();
                if (_token.Type != TokenType.EoL && node == null)
                    throw new Exception("Unable to walk down tokens.");

                root.Childrens.Add(node);

                /* Keep parsing until we reach end of file. */
                if (_token.Type == TokenType.EoF)
                    break;
            }

            return root;
        }

        private Node Walk()
        {
            _token = _lexer.Lex();

            if (_token.Type == TokenType.IdentifierOrKeyword)
            {
                switch (_token.Text)
                {
                    case "OUTPUT":
                        return ParseOutputStatement(_token);

                    case "DECLARE":
                        return ParseVariableDeclaration(_token);

                    case "FOR":
                        return ParseForStatement(_token);

                    case "NEXT":
                        return ParseNextStatement(_token);

                    default:
                        /* Assume its an assign statement. */
                        var node = ParsePossibleAssignStatement(_token);
                        if (_token.Type != TokenType.EoF && _token.Type != TokenType.EoL)
                            throw new Exception("Expected EoL or EoF.");
                        return node;
                }
            }
            return null;
        }

        private Node ParseForStatement(Token token)
        {
            var identifierToken = _lexer.Lex();
            if (identifierToken.Type != TokenType.IdentifierOrKeyword)
                throw new Exception("Expected identifier.");

            var fromExpression = ParsePossibleAssignStatement(identifierToken);

            var toToken = _token;
            if (toToken.Text != "TO")
                throw new Exception("Expected TO.");

            var toExpression = ParseExpression();

            _token = _lexer.Lex();
            if (_token.Type != TokenType.EoF && _token.Type != TokenType.EoL)
                throw new Exception("Expected EoL or EoF.");

            var block = ParseForBlock();
            return new ForStatement
            {
                FromExpression = fromExpression,
                ToExpression = toExpression,
                Block = (ForBlock)block
            };
        }

        private Node ParseForBlock()
        {
            var block = new ForBlock();
            while (true)
            {
                var stmt = Walk();
                if (stmt is NextStatement)
                {
                    block.NextStatement = (NextStatement)stmt;
                    break;
                }

                block.Childrens.Add(stmt);
            }
            return block;
        }

        private Node ParseNextStatement(Token token)
        {
            var stmt = new NextStatement();

            _token = _lexer.Lex();
            if (_token.Type != TokenType.EoF && _token.Type != TokenType.EoL)
                throw new Exception("Expected EoL or EoF.");
            return stmt;
        }

        private Node ParsePossibleAssignStatement(Token token)
        {
            var equalToken = _lexer.Lex();
            if (equalToken.Type == TokenType.Equal)
            {
                var expression = ParseExpression();
                var stmt = new AssignStatement
                {
                    Identifier = token.Text,
                    Expression = expression
                };

                _token = _lexer.Lex();
                return stmt;
            }
            return null;
        }

        private VariableDeclaration ParseVariableDeclaration(Token token)
        {
            var identifierToken = _lexer.Lex();
            if (identifierToken.Type != TokenType.IdentifierOrKeyword)
                throw new Exception("Expected variable name identifier.");

            var identifier = identifierToken.Text;

            var colonToken = _lexer.Lex();
            if (colonToken.Type != TokenType.Colon)
                throw new Exception("Expected colon.");

            var typeToken = _lexer.Lex();
            if (typeToken.Type != TokenType.IdentifierOrKeyword)
                throw new Exception("Expected variable type identifier.");

            var type = typeToken.Text;

            var stmt = new VariableDeclaration
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

            /*TODO: Check if node returned is a valid one. */
            stmt.Expression = ParseExpression();

            _token = _lexer.Lex();

            if (_token.Type != TokenType.EoF && _token.Type != TokenType.EoL)
                throw new Exception("Expected EoL or EoF.");

            return stmt;
        }

        private Node ParseExpression()
        {
            _token = _lexer.Lex();
            if (_token.Type == TokenType.StringLiteral)
                return ParseStringLiteralExpression(_token);
            if (_token.Type == TokenType.NumberLiteral)
                return ParseNumberLiteralExpression(_token);
            else if (_token.Type == TokenType.IdentifierOrKeyword)
                return ParseIdentifierExpression(_token);
            return null;
        }

        private Node ParseNumberLiteralExpression(Token token)
        {
            return new NumberLiteralExpression { Value = int.Parse(token.Text) };
        }

        private Node ParseIdentifierExpression(Token token)
        {
            return new IdentifierExpression { Identifier = token.Text };
        }

        private Node ParseStringLiteralExpression(Token token)
        {
            return new StringLiteralExpression { Value = token.Text };
        }

        private static bool IsBuiltInFunction(Token token)
        {
            return token.Text == "OUTPUT";
        }
    }
}