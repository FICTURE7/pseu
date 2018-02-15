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
            FOR: "FOR"
            TO: "TO"
            NEXT: "NEXT"
            OUTPUT: "OUTPUT"
            IF: "IF"
            THEN: "THEN"
            ELSE: "ELSE"
            ELSEIF: "ELSEIF"
            ENDIF: "ENDIF"

            program: statement-list

            statement-list: statement | statement LF statement-list
            statement: assign-statement | 
                       declare-statement | 
                       if-statement |
                       for-statement | 
                       output-statement | 
                       empty-statement

            empty-statement:
            assign-statement: identifier ASSIGN (expression | string-expression)
            declare-statement: DECLARE identifier COLON identifier
            for-statement: FOR assign-statement TO expression statement-list NEXT
            output-statement: OUTPUT (expression | string-expression)
            if-statement: IF expression THEN statement-list (ELSEIF statement-list THEN)* (ELSE statement-list) ENDIF

            string-expression: \" STRING \"
            expression: term ((PLUS | MINUS) term)*
            term: factor ((DIV | MUL) factor)*
            factor: (PLUS | MINUS) INTEGER | identifier | LPAREN expression RPAREN
            identifier: STRING
         */

        public Node Parse()
        {
            return ParseProgram();
        }

        private Node ParseProgram()
        {
            var nodes = ParseStatementList();
            return new ProgramBlock
            {
                Statements = nodes
            };
        }

        private List<Node> ParseStatementList()
        {
            var nodes = new List<Node>();
            var statement = ParseStatement();

            nodes.Add(statement);
            while (_token.Type == TokenType.LineFeed)
            {
                Eat(TokenType.LineFeed);
                nodes.Add(ParseStatement());
            }

            return nodes;
        }

        private Node ParseStatement()
        {
            if (_token.Type == TokenType.DeclareKeyword)
                return ParseDeclareStatement();
            else if (_token.Type == TokenType.Identifier)
                return ParseAssignStatement();
            else if (_token.Type == TokenType.ForKeyword)
                return ParseForStatement();
            else if (_token.Type == TokenType.OutputKeyword)
                return ParseOutputStatement();
            else if (_token.Type == TokenType.IfKeyword)
                return ParseIfStatement();
            else
                return ParseEmptyStatement();
        }

        private Node ParseEmptyStatement()
        {
            return new NoOperation();
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
            var left = _token;
            Eat(TokenType.Identifier);
            Eat(TokenType.Assign);

            var right = (Node)null;
            if (_token.Type == TokenType.StringLiteral)
                right = ParseStringExpression();
            else
                right = ParseExpression();

            return new Assign
            {
                Left = new IdentifierName { Identifier = left.Value },
                Right = right
            };
        }

        private Node ParseForStatement()
        {
            Eat(TokenType.ForKeyword);
            var assignment = ParseAssignStatement();
            Eat(TokenType.ToKeyword);
            var expression = ParseExpression();
            var statements = ParseStatementList();
            Eat(TokenType.NextKeyword);

            return new ForBlock
            {
                VariableInitializer = (Assign)assignment,
                ToExpression = expression,
                Statements = statements
            };
        }

        private Node ParseOutputStatement()
        {
            Eat(TokenType.OutputKeyword);
            var expression = (Node)null;
            if (_token.Type == TokenType.StringLiteral)
                expression = ParseStringExpression();
            else
                expression = ParseExpression();

            return new Output { Expression = expression };
        }

        private Node ParseIfStatement()
        {
            Eat(TokenType.IfKeyword);
            var expression = ParseExpression();
            Eat(TokenType.ThenKeyword);
            var statements = ParseStatementList();

            while (_token.Type == TokenType.ElseIfKeyword)
            {
                Eat(TokenType.ElseIfKeyword);
                var moreStatements = ParseStatementList();
                Eat(TokenType.ThenKeyword);
            }

            if (_token.Type == TokenType.ElseKeyword)
            {
                Eat(TokenType.ElseKeyword);
                var moreStatements = ParseStatementList();
            }

            Eat(TokenType.EndIfKeyword);
            return null;
        }

        private Node ParseExpression()
        {
            /*
                expression: term ((PLUS | MINUS) term)*
             */
            var node = ParseTerm();

            while (_token.Type == TokenType.Plus || _token.Type == TokenType.Minus)
            {
                var token = _token;
                if (token.Type == TokenType.Plus)
                    Eat(TokenType.Plus);
                else if (token.Type == TokenType.Minus)
                    Eat(TokenType.Minus);

                node = new BinaryOperation
                {
                    Left = node,
                    Operation = token,
                    Right = ParseTerm()
                };
            }

            return node;
        }

        private Node ParseStringExpression()
        {
            var value = _token;
            Eat(TokenType.StringLiteral);

            return new StringLiteral { Value = value.Value };
        }

        private Node ParseTerm()
        {
            /*
                term: factor ((DIV | MUL) factor)*
             */
            var node = ParseFactor();

            while (_token.Type == TokenType.Divide || _token.Type == TokenType.Multiply)
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
                var op = _token;
                Eat(TokenType.Plus);
                return new UnaryOperation { Operation = op, Right = ParseFactor() };
            }
            else if (_token.Type == TokenType.Minus)
            {
                var op = _token;
                Eat(TokenType.Minus);
                return new UnaryOperation { Operation = op, Right = ParseFactor() };
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