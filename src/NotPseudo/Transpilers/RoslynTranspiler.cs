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
                statements.Add(roslynStatement);
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

            var newNode = _generator.CompilationUnit(classDecl).NormalizeWhitespace();

            return newNode.ToString();
        }

        protected abstract SyntaxNode TranspileForBlock(ForBlock forBlock);

        protected SyntaxNode TranspileStatement(Node statement)
        {
            if (statement is VariableDeclaration varDecl)
                return TranspileVariableDeclaration(varDecl);
            else if (statement is ForBlock forBlock)
                return TranspileForBlock(forBlock);
            else if (statement is Output output)
                return TranspileOutputStatement(output);

            return null;
        }

        protected SyntaxNode TranspileAssignStatement(Assign assign)
        {
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

        protected SyntaxNode TranspileOutputStatement(Output output)
        {
            var roslynOutput = _generator.InvocationExpression(
                _generator.IdentifierName("Console.WriteLine"),
                arguments: new SyntaxNode[] { TranspileExpression(output.Expression) }
            );

            return _generator.ExpressionStatement(roslynOutput);
        }

        protected SyntaxNode TranspileExpression(Node node)
        {
            if (node is NumberLiteral numNode)
                return _generator.LiteralExpression(numNode.Value);
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

            return null;
        }

        protected SyntaxNode TranspileUnaryOperation(UnaryOperation unOp)
        {
            var roslynRight = TranspileExpression(unOp.Right);

            if (unOp.Operation.Type == TokenType.Minus)
                return _generator.NegateExpression(roslynRight);
            else if (unOp.Operation.Type == TokenType.Plus)
                return TranspileUnaryPlusOperation(roslynRight);

            return null;
        }

        protected abstract SyntaxNode TranspileUnaryPlusOperation(SyntaxNode roslynRight);

        protected SyntaxNode TranspileType(string type)
        {
            switch (type)
            {
                case "INTEGER":
                    return _generator.TypeExpression(SpecialType.System_Int32);
                case "STRING":
                    return _generator.TypeExpression(SpecialType.System_String);
            }

            return _generator.IdentifierName(type);
        }
    }
}