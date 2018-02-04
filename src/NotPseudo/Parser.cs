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
            var node = Walk();
            _root.Childrens.Add(node);

            return _root;
        }

        private Node Walk()
        {
            _token = _lexer.Lex();
            if (_token.Type == TokenType.Identifier)
            {
                switch (_token.Value)
                {
                    case "OUTPUT":
                        return ParseOutputStatement(_token);
                }
            }
            else if (_token.Type == TokenType.StringLiteral)
            {
                return ParseStringLiteral(_token);
            }
            return null;
        }

        private OutputStatement ParseOutputStatement(Token token)
        {
            var node = new OutputStatement();
            node.Expression = Walk();
            return node;
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