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
            LSPAREN: '['
            RSPAREN: ']'

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

            CALL: "CALL"
            RETURN: "RETURN"

            PROCEDURE: "PROCEDURE"
            ENDPROCEDURE: "ENDPROCEDURE"
            FUNCTION: "FUNCTION"
            ENDFUNCTION: "ENDFUNCTION"

            DECLARE: "DECLARE"
            ARRAY: "ARRAY"
            OF: "OF"

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
                       procedure-statement |
                       function-statement |
                       return-statement |
                       if-statement |
                       for-statement |
                       repeat-statement |
                       while-statement | 
                       output-statement |
                       input-statement |
                       call-statement |
                       empty-statement

            empty-statement:
            assign-statement: identifier ASSIGN expression
            call-statement: call-expression

            procedure-statement: PROCEDURE simple-identifier parameter-list statement-list ENDPROCEDURE
            function-statement: FUNCTION simple-identifier parameter-list RETURN simple-identifier ENDFUNCTION
            return-statement: RETURN expression

            parameter-list: LPAREN [parameter (, parameter)*] RPAREN
            parameter: simple-identifier COLON simple-identifier

            declare-statement: DECLARE simple-identifier COLON simple-identifier | declare-array-statement
            declare-array-statement: DECLARE identifier COLON ARRAY LSPAREN expression RSPAREN OF identifier

            for-statement: FOR assign-statement TO numeric-expression statement-list ENDFOR
            repeat-statement: REPEAT statement-list UNTIL expression
            while-statement: WHILE expression statement-list ENDWHILE

            output-statement: OUTPUT expression
            input-statement: INPUT [string-expression] identifier
            if-statement: IF expression THEN statement-list (ELSE statement-list) ENDIF

            identifier: simple-identifier | simple-identifier LSPAREN numeric-expression RSPAREN
            simple-identifier: STRING

            expression: boolean-term (OR boolean-term)*

            boolean-term: boolean-factor (AND boolean-factor)*
            boolean-factor: NOT boolean-factor | boolean-relation | LPAREN boolean-expression RPAREN
            boolean-relation: boolean-relation-term ((GREATER-EQUAL | GREATER | LESS-EQUAL | LESS | EQUAL | NOT-EQUAL) boolean-relation-term)
            boolean-relation-term: string-expression | numeric-expression | TRUE | FALSE

            string-expression: \" STRING \"

            numeric-expression: numeric-term ((PLUS | MINUS) numeric-term)*
            numeric-term: numeric-factor ((DIV | MUL) numeric-factor)*
            numeric-factor: (PLUS | MINUS) numeric-factor | INTEGER | identifier | LPAREN numeric-expression RPAREN | call-expression

            call-expression: CALL simple-identifier argument-list

            argument-list: LPAREN [argument (, argument)*] RPAREN
            argument: expression
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
            else if (_token.Type == TokenType.ProcedureKeyword)
                return ParseProcedureStatement();
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
            else if (_token.Type == TokenType.CallKeyword)
                return ParseCallStatement();
            else if (_token.Type == TokenType.IfKeyword)
                return ParseIfStatement();
            else
                return ParseEmptyStatement();
        }

        private Node ParseEmptyStatement()
        {
            return new NoOperation();
        }

        private Node ParseCallStatement()
        {
            return ParseCallExpression();
        }

        private Node ParseDeclareStatement()
        {
            Eat(TokenType.DeclareKeyword);

            var varIdentToken = _token;
            Eat(TokenType.Identifier);
            Eat(TokenType.Colon);

            if (_token.Type == TokenType.ArrayKeyword)
            {
                Eat(TokenType.ArrayKeyword);
                Eat(TokenType.LeftSquareParenthesis);
                var length = ParseNumericExpression();
                Eat(TokenType.RightSquareParenthesis);
                Eat(TokenType.OfKeyword);

                var typeIdentToken = _token;
                Eat(TokenType.Identifier);

                return new ArrayVariableDeclaration
                {
                    Identifier = new IdentifierName { Identifier = varIdentToken.Value },
                    Type = new IdentifierName { Identifier = typeIdentToken.Value },
                    Length = length
                };
            }
            else
            {
                var typeIdentToken = _token;
                Eat(TokenType.Identifier);

                return new VariableDeclaration
                {
                    Identifier = new IdentifierName { Identifier = varIdentToken.Value },
                    Type = new IdentifierName { Identifier = typeIdentToken.Value }
                };
            }
        }

        private Node ParseIdentifier()
        {
            var identToken = _token;
            Eat(TokenType.Identifier);

            if (_token.Type == TokenType.LeftSquareParenthesis)
            {
                Eat(TokenType.LeftSquareParenthesis);
                var indexerExpression = ParseNumericExpression();
                Eat(TokenType.RightSquareParenthesis);

                return new ArrayIdentifierName
                {
                    Identifier = identToken.Value,
                    IndexExpression = indexerExpression
                };
            }

            return new IdentifierName { Identifier = identToken.Value };
        }

        private Node ParseAssignStatement()
        {
            var left = ParseIdentifier();
            Eat(TokenType.Assign);
            var right = ParseExpression();
            return new Assign
            {
                Left = left,
                Right = right
            };
        }

        private List<Node> ParseParameterList()
        {
            var list = new List<Node>();

            Eat(TokenType.LeftParenthesis);

            if (_token.Type == TokenType.Identifier)
            {
                var param = ParseParameter();
                list.Add(param);

                while (_token.Type == TokenType.Comma)
                {
                    Eat(TokenType.Comma);
                    var otherParam = ParseParameter();
                    list.Add(otherParam);
                }
            }

            Eat(TokenType.RightParenthesis);

            return list;
        }

        private Node ParseParameter()
        {
            var identToken = _token;
            Eat(TokenType.Identifier);
            Eat(TokenType.Colon);
            var typeIdentToken = _token;
            Eat(TokenType.Identifier);

            return new Parameter
            {
                Identifier = new IdentifierName { Identifier = identToken.Value },
                TypeIdentifier = new IdentifierName { Identifier = typeIdentToken.Value }
            };
        }

        private Node ParseProcedureStatement()
        {
            Eat(TokenType.ProcedureKeyword);

            var identToken = _token;
            Eat(TokenType.Identifier);

            var paramList = ParseParameterList();
            var statements = ParseStatementList();

            Eat(TokenType.EndProcedureKeyword);
            return new ProcedureBlock
            {
                Identifier = new IdentifierName { Identifier = identToken.Value },
                Parameters = paramList,
                Statements = statements
            };
        }

        private Node ParseForStatement()
        {
            Eat(TokenType.ForKeyword);
            var assignment = ParseAssignStatement();
            Eat(TokenType.ToKeyword);
            var expression = ParseNumericExpression();
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
            var condition = ParseExpression();

            return new RepeatBlock
            {
                Statements = statements,
                Condition = condition
            };
        }

        private Node ParseWhileStatement()
        {
            Eat(TokenType.WhileKeyword);
            var condition = ParseExpression();
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
            var expression = ParseExpression();
            return new Output { Expression = expression };
        }

        private Node ParseInputStatement()
        {
            Eat(TokenType.InputKeyword);

            var prefix = (Node)null;
            if (_token.Type == TokenType.StringLiteral)
                prefix = ParseStringExpression();

            var ident = ParseIdentifier();

            return new Input
            {
                Prefix = prefix,
                Identifier = ident
            };
        }

        private Node ParseIfStatement()
        {
            Eat(TokenType.IfKeyword);
            var condition = ParseExpression();
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

        private Node ParseExpression()
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
            else if (_token.Type == TokenType.LeftParenthesis)
            {
                Eat(TokenType.LeftParenthesis);
                var expression = ParseExpression();
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
            var left = ParseBooleanRelationTerm();
            var opToken = _token;

            if (_token.Type == TokenType.Equal || _token.Type == TokenType.NotEqual ||
                _token.Type == TokenType.Less || _token.Type == TokenType.LessEqual ||
                _token.Type == TokenType.Greater || _token.Type == TokenType.GreaterEqual)
            {
                Eat(_token.Type);

                var right = ParseBooleanRelationTerm();
                return new BinaryOperation
                {
                    Left = left,
                    Operation = opToken,
                    Right = right
                };
            }
            else
            {
                return left;
            }
        }

        private Node ParseBooleanRelationTerm()
        {
            if (_token.Type == TokenType.TrueLiteral)
            {
                Eat(TokenType.TrueLiteral);
                return new BooleanLiteral { Value = true };
            }
            else if (_token.Type == TokenType.FalseLiteral)
            {
                Eat(TokenType.FalseLiteral);
                return new BooleanLiteral { Value = false };
            }
            else if (_token.Type == TokenType.StringLiteral)
            {
                return ParseStringExpression();
            }
            else
            {
                return ParseNumericExpression();
            }
        }

        private Node ParseNumericExpression()
        {
            var node = ParseNumericTerm();

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
                    Right = ParseNumericTerm()
                };
            }

            return node;
        }

        private Node ParseNumericTerm()
        {
            /*
                term: factor ((DIV | MUL) factor)*
             */
            var node = ParseNumericFactor();

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
                    Right = ParseNumericFactor()
                };
            }

            return node;
        }

        private Node ParseNumericFactor()
        {
            /*
                factor: (PLUS | MINUS) INTEGER | identifier | LPAREN expression RPAREN
             */
            if (_token.Type == TokenType.Plus)
            {
                var op = _token;
                Eat(TokenType.Plus);
                return new UnaryOperation { Operation = op, Right = ParseNumericFactor() };
            }
            else if (_token.Type == TokenType.Minus)
            {
                var op = _token;
                Eat(TokenType.Minus);
                return new UnaryOperation { Operation = op, Right = ParseNumericFactor() };
            }
            else if (_token.Type == TokenType.NumberLiteral)
            {
                var value = _token.Value;
                Eat(TokenType.NumberLiteral);
                return new NumberLiteral { Value = int.Parse(value) };
            }
            else if (_token.Type == TokenType.Identifier)
            {
                return ParseIdentifier();
            }
            else if (_token.Type == TokenType.LeftParenthesis)
            {
                Eat(TokenType.LeftParenthesis);
                var expr = ParseNumericExpression();
                Eat(TokenType.RightParenthesis);

                return expr;
            }
            else if (_token.Type == TokenType.CallKeyword)
            {
                return ParseCallExpression();
            }

            Error();

            return null;
        }

        private Node ParseCallExpression()
        {
            Eat(TokenType.CallKeyword);

            var identToken = _token;
            Eat(TokenType.Identifier);

            var argumentList = ParseArgumentList();
            return new Call
            {
                Identifier = new IdentifierName { Identifier = identToken.Value },
                Arguments = argumentList
            };
        }

        private List<Node> ParseArgumentList()
        {
            var list = new List<Node>();
            Eat(TokenType.LeftParenthesis);

            /* Figure if the next token is a possible expression. */
            var expression = false;

            switch (_token.Type)
            {
                case TokenType.Plus:
                case TokenType.Minus:
                case TokenType.TrueLiteral:
                case TokenType.FalseLiteral:
                case TokenType.StringLiteral:
                case TokenType.NumberLiteral:
                case TokenType.Identifier:
                case TokenType.CallKeyword:
                case TokenType.LeftParenthesis:
                    expression = true;
                    break;
            }

            if (expression)
            {
                var argument = ParseArgument();
                list.Add(argument);
                while (_token.Type == TokenType.Comma)
                {
                    Eat(TokenType.Comma);
                    var otherArgument = ParseArgument();
                    list.Add(otherArgument);
                }
            }

            Eat(TokenType.RightParenthesis);

            return list;
        }

        private Node ParseArgument()
        {
            /*
            switch (_token.Type)
            {
                case TokenType.
                case TokenType.LeftParenthesis:
                    return ParseExpression();
            }
            */

            return ParseExpression();
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