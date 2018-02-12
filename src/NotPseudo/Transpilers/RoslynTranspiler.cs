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
                var roslynStatement = TranspileStatement(statement);
            }

            /*
            foreach (var child in node.Childrens)
            {
                if (child is OutputStatement)
                    statements.Add(TranspileOutputStatement((OutputStatement)child));
                else if (child is VariableDeclaration)
                    statements.Add(TranspileVariableDeclaration((VariableDeclaration)child));
                else if (child is AssignStatement)
                    statements.Add(TranspileAssignStatement((AssignStatement)child));
            }
            */

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

        private SyntaxNode TranspileStatement(Node statement)
        {
            if (statement is VariableDeclaration varDecl)
                return TranspileVariableDeclaration(varDecl);
            else if (statement is ForBlock forBlock)
                return TranspileForBlock(forBlock);
            return null;
        }

        /*
        private SyntaxNode TranspileForStatement(ForStatement stmt)
        {
            return null;
        }
        */

        private SyntaxNode TranspileAssignStatement(AssignStatement stmt)
        {
            var assignStmt = _generator.AssignmentStatement(
                left: _generator.IdentifierName(stmt.Identifier),
                right: TranspileExpression(stmt.Expression)
            );
            return assignStmt;
        }

        private SyntaxNode TranspileVariableDeclaration(VariableDeclaration varDecl)
        {
            var varDeclStmt = _generator.LocalDeclarationStatement(
                type: TranspileType(varDecl.Type.Identifier),
                identifier: varDecl.Identifier.Identifier // Pretty wacky names.
            );
            return varDeclStmt;
        }

        private SyntaxNode TranspileOutputStatement(Output stmt)
        {
            var outputStmt = _generator.InvocationExpression(
                _generator.IdentifierName("Console.WriteLine"),
                arguments: new SyntaxNode[] { TranspileExpression(stmt.Expression) }
            );

            return outputStmt;
        }

        private SyntaxNode TranspileForBlock(ForBlock forBlock)
        {
            return null;
        }

        private SyntaxNode TranspileExpression(Node node)
        {
            if (node is StringLiteralExpression)
            {
                var strNode = (StringLiteralExpression)node;
                return _generator.LiteralExpression(strNode.Value);
            }
            else if (node is IdentifierExpression)
            {
                var varNode = (IdentifierExpression)node;
                return _generator.IdentifierName(varNode.Identifier);
            }
            return null;
        }

        private SyntaxNode TranspileType(string type)
        {
            switch (type)
            {
                case "STRING":
                    return _generator.TypeExpression(SpecialType.System_String);
            }

            return _generator.IdentifierName(type);
        }

        protected abstract SyntaxNode TranspileForStatement(ForStatement stmt);
    }
}