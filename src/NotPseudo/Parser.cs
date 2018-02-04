using System.Collections.Generic;

namespace NotPseudo
{
    public class Parser : IParser
    {
        private readonly Lexer _lexer;

        public Parser(Lexer lexer)
        {
            if (lexer == null)
                throw new System.ArgumentNullException(nameof(lexer));

            _lexer = lexer;
        }

        public void Walk()
        {
            var token = _lexer.Lex();
            if (token.Type == TokenType.Identifier)
            {
                if (IsPrimitiveFunction(token))
                    ParseCallExpression(token);
            }
        }

        private Node ParseCallExpression(Token token)
        {
            var node = new Node();
            var argument = new Node
            {
                Value = _lexer.Lex().Value
            };

            node.Childrens.Add(argument);
            return node;
        }

        private static bool IsPrimitiveFunction(Token token)
        {
            return token.Value == "OUTPUT";
        }
    }
}