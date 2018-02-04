using System;
using System.Collections.Generic;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Editing;

namespace NotPseudo.CodeGen
{
    public abstract class RoslynCodeGen : ICodeGen
    {
        private SyntaxGenerator _generator;
        private readonly AdhocWorkspace _workspace;

        protected RoslynCodeGen()
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
            throw new NotImplementedException();
        }

        public string Generate(List<Token> tokens)
        {
            if (tokens == null)
                throw new ArgumentNullException(nameof(tokens));

            var methodDecl = _generator.MethodDeclaration(
                name: "Main",
                modifiers: DeclarationModifiers.Static,
                accessibility: Accessibility.Public,
                statements: new SyntaxNode[]
                {
                    _generator.InvocationExpression(
                        _generator.IdentifierName("Console.WriteLine"),
                        arguments: new SyntaxNode[]{ _generator.LiteralExpression("Hello World") }
                    )
                }
            );

            var classDecl = _generator.ClassDeclaration(
                name: "Program",
                modifiers: DeclarationModifiers.Static,
                members: new SyntaxNode[] { methodDecl }
            );

            var newNode = _generator.CompilationUnit(classDecl).NormalizeWhitespace();

            return newNode.ToString();
        }
    }
}