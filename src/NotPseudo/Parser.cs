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
                        break;
                }
            }
            else if (_token.Type == TokenType.StringLiteral)
            {
                return ParseStringLiteral(_token);
            }
            return null;
        }

        private VariableDeclarationStatement ParseVariableDeclarationStatement(Token token)
        {
            var identifierToken = _lexer.Lex();
            if (identifierToken.Type != TokenType.IdentifierOrKeyword)
                throw new System.Exception("Expected variable name identifier.");

            var identifier = identifierToken.Value;

            var colonToken = _lexer.Lex();
            if (colonToken.Type != TokenType.Colon)
                throw new System.Exception("Expected colon.");

            var typeToken = _lexer.Lex();
            if (typeToken.Type != TokenType.IdentifierOrKeyword)
                throw new System.Exception("Expected variable type identifier.");

            var type = typeToken.Value;

            var stmt = new VariableDeclarationStatement
            {
                Name = identifier,
                Type = type
            };

            /*TODO: Expect '=' expression maybe? */

            _token = _lexer.Lex();

            if (_token.Type != TokenType.EoF && _token.Type != TokenType.EoL)
                throw new System.Exception("Expected EoL or EoF.");

            return stmt;
        }

        private OutputStatement ParseOutputStatement(Token token)
        {
            var stmt = new OutputStatement();

            /*TODO: Check if node returned by walk is a valid one. */
            stmt.Expression = Walk();

            _token = _lexer.Lex();

            if (_token.Type != TokenType.EoF && _token.Type != TokenType.EoL)
                throw new System.Exception("Expected EoL or EoF.");

            return stmt;
        }

        private Node ParseStringLiteral(Token token)
        {
            return new StringLiteralNode { Value = token.Value };
        }

        private static bool IsBuiltInFunction(Token token)
        {
            return token.Value == "OUTPUT";
        }
    }
}