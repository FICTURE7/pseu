using System;
using System.Collections.Generic;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Editing;
using NotPseudo.CodeAnalysis;
using NotPseudo.CodeAnalysis.Syntax;

namespace NotPseudo.Transpilers
{
    public abstract class RoslynTranspiler : ITranspiler
    {
        private SyntaxGenerator _generator;
        private readonly AdhocWorkspace _workspace;

        protected RoslynTranspiler()
        {
            _workspace = new AdhocWorkspace();
        }

        protected Workspace Workspace => _workspace;
        protected SyntaxGenerator Generator => _generator;

        protected void Initialize(SyntaxGenerator generator)
        {
            if (generator == null)
                throw new ArgumentNullException(nameof(generator));

            _generator = generator;
        }

        public string Generate(ProgramBlock programNode)
        {
            if (programNode == null)
                throw new ArgumentNullException(nameof(programNode));

            var statements = new List<SyntaxNode>();
            foreach (var statement in programNode.Statements)
            {
                if (statement is NoOperation)
                    continue;

                var roslynStatement = TranspileStatement(statement);
                statements.AddRange(roslynStatement);
            }

            var methodDecl = _generator.MethodDeclaration(
                name: "Main",
                modifiers: DeclarationModifiers.Static,
                statements: statements
            );

            var classDecl = _generator.ClassDeclaration(
                name: "Program",
                members: new SyntaxNode[] { methodDecl }
            );

            var imports = _generator.NamespaceImportDeclaration("System");

            var newNode = _generator.CompilationUnit(new[] { imports, classDecl }).NormalizeWhitespace();

            return newNode.ToString();
        }

        protected abstract SyntaxNode TranspileForBlock(ForBlock forBlock);
        protected abstract SyntaxNode TranspileRepeatBlock(RepeatBlock repeatBlock);

        protected SyntaxNode[] TranspileStatement(Node statement)
        {
            if (statement is VariableDeclaration varDecl)
                return new[] { TranspileVariableDeclaration(varDecl) };
            else if (statement is ArrayVariableDeclaration arrayDecl)
                return new[] { TranspileArrayVariableDeclaration(arrayDecl) };
            else if (statement is ForBlock forBlock)
                return new[] { TranspileForBlock(forBlock) };
            else if (statement is RepeatBlock repeatBlock)
                return new[] { TranspileRepeatBlock(repeatBlock) };
            else if (statement is WhileBlock whileBlock)
                return new[] { TranspileWhileBlock(whileBlock) };
            else if (statement is IfBlock ifBlock)
                return new[] { TranspileIfBlock(ifBlock) };
            else if (statement is Output output)
                return new[] { TranspileOutput(output) };
            else if (statement is Input input)
                return TranspileInput(input);
            else if (statement is Assign assign)
                return new[] { TranspileAssign(assign) };

            return null;
        }

        protected SyntaxNode TranspileAssign(Assign assign)
        {
            /*TODO: Convert the types? */
            var roslynAssign = _generator.AssignmentStatement(
                left: _generator.IdentifierName(((IdentifierName)assign.Left).Identifier),
                right: TranspileExpression(assign.Right)
            );
            return roslynAssign;
        }

        protected SyntaxNode TranspileVariableDeclaration(VariableDeclaration varDecl)
        {
            var roslynVarDecl = _generator.LocalDeclarationStatement(
                type: TranspileType(varDecl.Type.Identifier),
                identifier: varDecl.Identifier.Identifier // Pretty wacky names.
            );
            return roslynVarDecl;
        }

        protected SyntaxNode TranspileArrayVariableDeclaration(ArrayVariableDeclaration arrayDecl)
        {
            var roslynArrayType = _generator.ArrayTypeExpression(TranspileType(arrayDecl.Type.Identifier));

            var roslynArrayCreation = _generator.ArrayCreationExpression(
                elementType: TranspileType(arrayDecl.Type.Identifier),
                size: TranspileExpression(arrayDecl.Length)
            );
            var roslynVarDecl = _generator.LocalDeclarationStatement(
                type: roslynArrayType,
                identifier: arrayDecl.Identifier.Identifier,
                initializer: roslynArrayCreation
            );
            return roslynVarDecl;
        }

        protected SyntaxNode TranspileOutput(Output output)
        {
            var roslynOutput = _generator.InvocationExpression(
                _generator.IdentifierName("Console.WriteLine"),
                arguments: new SyntaxNode[] { TranspileExpression(output.Expression) }
            );

            /* Wrap in ExpressionStatement to be able to cast to StatementSyntax. */
            return _generator.ExpressionStatement(roslynOutput);
        }

        protected SyntaxNode[] TranspileInput(Input input)
        {
            var roslynPrefix = (SyntaxNode)null;
            if (input.Prefix != null)
            {
                var strExpression = input.Prefix;
                var roslynOutput = _generator.InvocationExpression(
                    _generator.IdentifierName("Console.Write"),
                    arguments: new SyntaxNode[] { TranspileExpression(strExpression) }
                );

                /* Wrap in ExpressionStatement to be able to cast to StatementSyntax. */
                roslynPrefix = _generator.ExpressionStatement(roslynOutput);
            }

            var roslynInput = _generator.AssignmentStatement(
                left: _generator.IdentifierName(((IdentifierName)input.Identifier).Identifier),
                right: _generator.InvocationExpression(
                    _generator.IdentifierName("Console.ReadLine")
                )
            );

            return roslynPrefix == null ?
                new[] { roslynInput } :
                new[] { roslynPrefix, roslynInput };
        }

        protected SyntaxNode TranspileExpression(Node node)
        {
            if (node is NumberLiteral numNode)
                return _generator.LiteralExpression(numNode.Value);
            else if (node is StringLiteral strNode)
                return _generator.LiteralExpression(strNode.Value);
            else if (node is BooleanLiteral boolNode)
                return _generator.LiteralExpression(boolNode.Value);
            else if (node is IdentifierName identNode)
                return _generator.IdentifierName(identNode.Identifier);
            else if (node is BinaryOperation binOp)
                return TranspileBinaryOperation(binOp);
            else if (node is UnaryOperation unOp)
                return TranspileUnaryOperation(unOp);

            return null;
        }

        protected SyntaxNode TranspileBinaryOperation(BinaryOperation binOp)
        {
            var roslynLeft = TranspileExpression(binOp.Left);
            var roslynRight = TranspileExpression(binOp.Right);

            var opType = binOp.Operation.Type;
            if (opType == TokenType.Plus)
                return _generator.AddExpression(roslynLeft, roslynRight);
            else if (opType == TokenType.Minus)
                return _generator.SubtractExpression(roslynLeft, roslynRight);
            else if (opType == TokenType.Divide)
                return _generator.DivideExpression(roslynLeft, roslynRight);
            else if (opType == TokenType.Multiply)
                return _generator.MultiplyExpression(roslynLeft, roslynRight);
            else if (opType == TokenType.Equal)
                return _generator.ValueEqualsExpression(roslynLeft, roslynRight);
            else if (opType == TokenType.NotEqual)
                return _generator.ValueNotEqualsExpression(roslynLeft, roslynRight);
            else if (opType == TokenType.Greater)
                return _generator.GreaterThanExpression(roslynLeft, roslynRight);
            else if (opType == TokenType.GreaterEqual)
                return _generator.GreaterThanOrEqualExpression(roslynLeft, roslynRight);
            else if (opType == TokenType.Less)
                return _generator.LessThanExpression(roslynLeft, roslynRight);
            else if (opType == TokenType.LessEqual)
                return _generator.LessThanOrEqualExpression(roslynLeft, roslynRight);
            else if (opType == TokenType.AndKeyword)
                return _generator.LogicalAndExpression(roslynLeft, roslynRight);
            else if (opType == TokenType.OrKeyword)
                return _generator.LogicalOrExpression(roslynLeft, roslynRight);

            return null;
        }

        protected SyntaxNode TranspileUnaryOperation(UnaryOperation unOp)
        {
            var roslynRight = TranspileExpression(unOp.Right);

            if (unOp.Operation.Type == TokenType.Minus)
                return _generator.NegateExpression(roslynRight);
            else if (unOp.Operation.Type == TokenType.Plus)
                return TranspileUnaryPlusOperation(roslynRight);
            else if (unOp.Operation.Type == TokenType.NotKeyword)
                return _generator.LogicalNotExpression(roslynRight);

            return null;
        }

        protected SyntaxNode TranspileIfBlock(IfBlock ifBlock)
        {
            var roslynCondition = TranspileExpression(ifBlock.Condition);
            var roslynTrueStatements = new List<SyntaxNode>();
            var roslynFalseStatements = (List<SyntaxNode>)null;

            foreach (var statement in ifBlock.TrueStatements)
            {
                if (statement is NoOperation)
                    continue;

                var roslynStatement = TranspileStatement(statement);
                roslynTrueStatements.AddRange(roslynStatement);
            }

            if (ifBlock.FalseStatements?.Count > 0)
            {
                roslynFalseStatements = new List<SyntaxNode>(ifBlock.FalseStatements.Count);
                foreach (var statement in ifBlock.FalseStatements)
                {
                    if (statement is NoOperation)
                        continue;

                    var roslynStatement = TranspileStatement(statement);
                    roslynFalseStatements.AddRange(roslynStatement);
                }
            }

            return _generator.IfStatement(roslynCondition, roslynTrueStatements, roslynFalseStatements);
        }

        protected SyntaxNode TranspileWhileBlock(WhileBlock whileBlock)
        {
            var roslynCondition = TranspileExpression(whileBlock.Condition);
            var roslynStatements = new List<SyntaxNode>();
            foreach (var statement in whileBlock.Statements)
            {
                if (statement is NoOperation)
                    continue;

                var roslynStatement = TranspileStatement(statement);
                roslynStatements.AddRange(roslynStatement);
            }

            return _generator.WhileStatement(roslynCondition, roslynStatements);
        }

        protected abstract SyntaxNode TranspileUnaryPlusOperation(SyntaxNode roslynRight);

        protected SyntaxNode TranspileType(string type)
        {
            switch (type)
            {
                case "BOOLEAN":
                    return _generator.TypeExpression(SpecialType.System_Boolean);
                case "INTEGER":
                    return _generator.TypeExpression(SpecialType.System_Int32);
                case "STRING":
                    return _generator.TypeExpression(SpecialType.System_String);
                default:
                    return _generator.IdentifierName(type);
            }
        }
    }
}