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
            BNF-like Definition
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

            EQUAL: '='
            NOT-EQUAL: '<>'
            GREATER: '>'
            GREATER-EQUAL: ">="
            LESS: '<'
            LESS-EQUAL: "<="

            TRUE: "TRUE"
            FALSE: "FALSE"

            OUTPUT: "OUTPUT"
            INPUT: "INPUT"

            DECLARE: "DECLARE"

            FOR: "FOR"
            TO: "TO"
            ENDFOR: "ENDFOR"
            
            REPEAT: "REPEAT"
            UNTIL: "UNTIL"

            WHILE: "WHILE"
            ENDWHILE: "ENDWHILE"

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
                       repeat-statement |
                       while-statement | 
                       output-statement |
                       input-statement |
                       empty-statement

            empty-statement:
            assign-statement: identifier ASSIGN (expression | string-expression)
            declare-statement: DECLARE identifier COLON identifier

            for-statement: FOR assign-statement TO expression statement-list ENDFOR
            repeat-statement: REPEAT statement-list UNTIL boolean-expression
            while-statement: WHILE boolean-expression statement-list ENDWHILE

            output-statement: OUTPUT (expression | string-expression)
            input-statement: INPUT identifier
            if-statement: IF boolean-expression THEN statement-list (ELSE statement-list) ENDIF

            boolean-expression: boolean-term (OR boolean-term)*
            boolean-term: boolean-factor (AND boolean-factor)*
            boolean-factor: NOT boolean-factor | TRUE | FALSE | boolean-relation | LPAREN boolean-expression RPAREN
            boolean-relation: (string-expression | expression) ((GREATER-EQUAL | GREATER | LESS-EQUAL | LESS | EQUAL | NOT-EQUAL) (string-expression | expression)

            string-expression: \" STRING \"

            expression: term ((PLUS | MINUS) term)*
            term: factor ((DIV | MUL) factor)*
            factor: (PLUS | MINUS) factor | INTEGER | identifier | LPAREN expression RPAREN
            identifier: STRING
         */

        public Node Parse()
        {
            return ParseProgram();
        }

        private Node ParseProgram()
        {
            var nodes = ParseStatementList();
            var block = new ProgramBlock
            {
                Statements = nodes
            };

            Eat(TokenType.EoF);
            return block;
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
            else if (_token.Type == TokenType.RepeatKeyword)
                return ParseRepeatStatement();
            else if (_token.Type == TokenType.WhileKeyword)
                return ParseWhileStatement();
            else if (_token.Type == TokenType.OutputKeyword)
                return ParseOutputStatement();
            else if (_token.Type == TokenType.InputKeyword)
                return ParseInputStatement();
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
            Eat(TokenType.EndForKeyword);

            return new ForBlock
            {
                VariableInitializer = (Assign)assignment,
                ToExpression = expression,
                Statements = statements
            };
        }

        private Node ParseRepeatStatement()
        {
            Eat(TokenType.RepeatKeyword);
            var statements = ParseStatementList();
            Eat(TokenType.UntilKeyword);

            var condition = ParseBooleanExpression();

            return new RepeatBlock
            {
                Statements = statements,
                Condition = condition
            };
        }

        private Node ParseWhileStatement()
        {
            Eat(TokenType.WhileKeyword);
            var condition = ParseBooleanExpression();
            var statements = ParseStatementList();
            Eat(TokenType.EndWhileKeyword);

            return new WhileBlock
            {
                Condition = condition,
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

        private Node ParseInputStatement()
        {
            Eat(TokenType.InputKeyword);

            var identToken = _token;
            Eat(TokenType.Identifier);

            return new Input
            {
                Identifier = new IdentifierName { Identifier = identToken.Value }
            };
        }

        private Node ParseIfStatement()
        {
            Eat(TokenType.IfKeyword);
            var condition = ParseBooleanExpression();
            Eat(TokenType.ThenKeyword);
            var trueStatements = ParseStatementList();
            var falseStatements = (List<Node>)null;

            if (_token.Type == TokenType.ElseKeyword)
            {
                Eat(TokenType.ElseKeyword);
                falseStatements = ParseStatementList();
            }

            Eat(TokenType.EndIfKeyword);

            return new IfBlock
            {
                Condition = condition,
                TrueStatements = trueStatements,
                FalseStatements = falseStatements
            };
        }

        private Node ParseStringExpression()
        {
            var value = _token;
            Eat(TokenType.StringLiteral);

            return new StringLiteral { Value = value.Value };
        }

        private Node ParseBooleanExpression()
        {
            var node = ParseBooleanTerm();

            while (_token.Type == TokenType.OrKeyword)
            {
                var orOp = _token;
                Eat(TokenType.OrKeyword);
                node = new BinaryOperation
                {
                    Left = node,
                    Operation = orOp,
                    Right = ParseBooleanTerm()
                };
            }

            return node;
        }

        private Node ParseBooleanTerm()
        {
            var node = ParseBooleanFactor();

            while (_token.Type == TokenType.AndKeyword)
            {
                var andOp = _token;
                Eat(TokenType.AndKeyword);
                node = new BinaryOperation
                {
                    Left = node,
                    Operation = andOp,
                    Right = ParseBooleanFactor()
                };
            }

            return node;
        }

        private Node ParseBooleanFactor()
        {
            if (_token.Type == TokenType.NotKeyword)
            {
                var notOp = _token;
                Eat(TokenType.NotKeyword);
                return new UnaryOperation { Operation = notOp, Right = ParseBooleanFactor() };
            }
            else if (_token.Type == TokenType.TrueLiteral)
            {
                Eat(TokenType.TrueLiteral);
                return new BooleanLiteral { Value = true };
            }
            else if (_token.Type == TokenType.FalseLiteral)
            {
                Eat(TokenType.FalseLiteral);
                return new BooleanLiteral { Value = false };
            }
            else if (_token.Type == TokenType.LeftParenthesis)
            {
                Eat(TokenType.LeftParenthesis);
                var expression = ParseBooleanExpression();
                Eat(TokenType.RightParenthesis);

                return expression;
            }
            else
            {
                return ParseBooleanRelation();
            }
        }

        private Node ParseBooleanRelation()
        {
            var left = (Node)null;
            if (_token.Type == TokenType.StringLiteral)
                left = ParseStringExpression();
            else
                left = ParseExpression();

            if (_token.Type == TokenType.Equal)
            {
                var eqOp = _token;
                Eat(TokenType.Equal);

                var right = (Node)null;
                if (_token.Type == TokenType.StringLiteral)
                    right = ParseStringExpression();
                else
                    right = ParseExpression();

                return new BinaryOperation
                {
                    Left = left,
                    Operation = eqOp,
                    Right = right
                };
            }
            else if (_token.Type == TokenType.NotEqual)
            {
                var notEqOp = _token;
                Eat(TokenType.NotEqual);

                var right = (Node)null;
                if (_token.Type == TokenType.StringLiteral)
                    right = ParseStringExpression();
                else
                    right = ParseExpression();

                return new BinaryOperation
                {
                    Left = left,
                    Operation = notEqOp,
                    Right = right
                };
            }
            else if (_token.Type == TokenType.Greater)
            {
                var greaterOp = _token;
                Eat(TokenType.Greater);

                var right = (Node)null;
                if (_token.Type == TokenType.StringLiteral)
                    right = ParseStringExpression();
                else
                    right = ParseExpression();

                return new BinaryOperation
                {
                    Left = left,
                    Operation = greaterOp,
                    Right = right
                };
            }
            else if (_token.Type == TokenType.GreaterEqual)
            {
                var greaterEqOp = _token;
                Eat(TokenType.GreaterEqual);

                var right = (Node)null;
                if (_token.Type == TokenType.StringLiteral)
                    right = ParseStringExpression();
                else
                    right = ParseExpression();

                return new BinaryOperation
                {
                    Left = left,
                    Operation = greaterEqOp,
                    Right = right
                };
            }
            else if (_token.Type == TokenType.Less)
            {
                var lessOp = _token;
                Eat(TokenType.Less);

                var right = (Node)null;
                if (_token.Type == TokenType.StringLiteral)
                    right = ParseStringExpression();
                else
                    right = ParseExpression();

                return new BinaryOperation
                {
                    Left = left,
                    Operation = lessOp,
                    Right = right
                };
            }
            else if (_token.Type == TokenType.LessEqual)
            {
                var lessEqOp = _token;
                Eat(TokenType.LessEqual);

                var right = (Node)null;
                if (_token.Type == TokenType.StringLiteral)
                    right = ParseStringExpression();
                else
                    right = ParseExpression();

                return new BinaryOperation
                {
                    Left = left,
                    Operation = lessEqOp,
                    Right = right
                };
            }

            return left;
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