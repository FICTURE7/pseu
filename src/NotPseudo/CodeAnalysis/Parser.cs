using System;
using System.Collections.Generic;
using NotPseudo.CodeAnalysis.Syntax;

namespace NotPseudo.CodeAnalysis
{
    public class Parser : IParser
    {
        /* Current token we're processing. */
        private Token _token;
        /* Root node. */
        private readonly Node _root;
        /* Lexer thats going to provide the token to process. */
        private readonly ILexer _lexer;

        public Parser(ILexer lexer)
        {
            if (lexer == null)
                throw new ArgumentNullException(nameof(lexer));

            _lexer = lexer;
            _token = lexer.Lex();
            _root = new Node();
        }

        /*
            EBNF-like Definition
            ----------------
            DIV: '/'
            MUL: '*'
            PLUS: '+'
            MINUS: '-'

            LPAREN: '('
            RPAREN: ')'

            ASSIGN: '<-'
            COLON: ':'
            LF: '\n'

            DECLARE: "DECLARE"

            program: statement

            empty:
            statement: assign-statement | declare-statement | empty
            assign-statement: identifier ASSIGN expression
            declare-statement: DECLARE identifier COLON identifier

            expression: term ((DIV | MUL) term)*
            term: factor ((PLUS | MINUS) factor)*
            factor: (PLUS | MINUS) INTEGER | identifier | LPAREN expression RPAREN
            identifier: STRING
         */

        public Node Parse()
        {
            return ParseProgram();
        }

        private Node ParseProgram()
        {
            return ParseStatement();
        }

        private Node ParseStatement()
        {
            if (_token.Type == TokenType.DeclareKeyword)
                return ParseDeclareStatement();
            else if (_token.Type == TokenType.Identifier)
                return ParseAssignStatement();

            return null;
        }

        private Node ParseDeclareStatement()
        {
            Eat(TokenType.DeclareKeyword);

            var varIdentToken = _token;
            Eat(TokenType.Identifier);
            Eat(TokenType.Colon);

            var typeIdentToken = _token;
            Eat(TokenType.Identifier);

            return new VariableDeclaration
            {
                Identifier = new IdentifierName { Identifier = varIdentToken.Value },
                Type = new IdentifierName { Identifier = typeIdentToken.Value }
            };
        }

        private Node ParseAssignStatement()
        {
            var varIdentToken = _token;
            Eat(TokenType.Identifier);
            Eat(TokenType.Assign);

            var expr = ParseExpression();
            return null;
        }

        private Node ParseExpression()
        {
            /*
                expression: term ((DIV | MUL) term)*
             */
            var node = ParseTerm();

            while (_token.Type == TokenType.Multiply || _token.Type == TokenType.Divide)
            {
                var token = _token;
                if (token.Type == TokenType.Divide)
                    Eat(TokenType.Divide);
                else if (token.Type == TokenType.Multiply)
                    Eat(TokenType.Multiply);

                node = new BinaryOperation
                {
                    Left = node,
                    Operation = token,
                    Right = ParseTerm()
                };
            }

            return node;
        }

        private Node ParseTerm()
        {
            /*
                term: factor ((PLUS | MINUS) factor)*
             */
            var node = ParseFactor();

            while (_token.Type == TokenType.Plus || _token.Type == TokenType.Minus)
            {
                var token = _token;
                if (token.Type == TokenType.Plus)
                    Eat(TokenType.Plus);
                else if (token.Type == TokenType.Minus)
                    Eat(TokenType.Multiply);

                node = new BinaryOperation
                {
                    Left = node,
                    Operation = token,
                    Right = ParseFactor()
                };
            }

            return node;
        }

        private Node ParseFactor()
        {
            /*
                factor: (PLUS | MINUS) INTEGER | identifier | LPAREN expression RPAREN
             */
            if (_token.Type == TokenType.Plus)
            {
                var token = _token;
                Eat(TokenType.Plus);
                return new UnaryOperation { Operation = token, Right = ParseFactor() };
            }
            else if (_token.Type == TokenType.Minus)
            {
                var token = _token;
                Eat(TokenType.Minus);
                return new UnaryOperation { Operation = token, Right = ParseFactor() };
            }
            else if (_token.Type == TokenType.NumberLiteral)
            {
                var value = _token.Value;
                Eat(TokenType.NumberLiteral);
                return new NumberLiteral { Value = int.Parse(value) };
            }
            else if (_token.Type == TokenType.Identifier)
            {
                var ident = _token.Value;
                Eat(TokenType.Identifier);
                return new IdentifierName { Identifier = ident };
            }
            else if (_token.Type == TokenType.LeftParenthesis)
            {
                Eat(TokenType.LeftParenthesis);
                var expr = ParseExpression();
                Eat(TokenType.RightParenthesis);

                return expr;
            }

            Error();

            return null;
        }

        private void Eat(TokenType type)
        {
            if (_token.Type == type)
                _token = _lexer.Lex();
            else
                Error();
        }

        private void Error() => throw new Exception("Invalid syntax.");
    }
}