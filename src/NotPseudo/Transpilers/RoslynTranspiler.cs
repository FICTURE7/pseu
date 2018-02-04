using System;
using System.Collections.Generic;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Editing;
using NotPseudo.Syntax;

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

        public string Generate(Node node)
        {
            if (node == null)
                throw new ArgumentNullException(nameof(node));

            var statements = new List<SyntaxNode>();
            foreach (var child in node.Childrens)
            {
                if (child is OutputStatement)
                    statements.Add(TranspileOutputStatement((OutputStatement)child));
                else if (child is VariableDeclarationStatement)
                    statements.Add(TranspileVariableDeclarationStatement((VariableDeclarationStatement)child));
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

        private SyntaxNode TranspileVariableDeclarationStatement(VariableDeclarationStatement stmt)
        {
            var varDeclStmt = _generator.LocalDeclarationStatement(
                type: _generator.IdentifierName(stmt.Type),
                identifier: stmt.Name
            );
            return varDeclStmt;
        }

        private SyntaxNode TranspileOutputStatement(OutputStatement stmt)
        {
            var outputStmt = _generator.InvocationExpression(
                _generator.IdentifierName("Console.WriteLine"),
                arguments: new SyntaxNode[] { TranspileExpression(stmt.Expression) }
            );

            return outputStmt;
        }

        private SyntaxNode TranspileExpression(Node node)
        {
            if (node is StringLiteralNode)
            {
                var strNode = (StringLiteralNode)node;
                return _generator.LiteralExpression(strNode.Value);
            }
            return null;
        }
    }
}