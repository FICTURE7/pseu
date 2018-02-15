using System.Collections.Generic;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Editing;
using Microsoft.CodeAnalysis.VisualBasic;
using Microsoft.CodeAnalysis.VisualBasic.Syntax;
using NotPseudo.CodeAnalysis.Syntax;

namespace NotPseudo.Transpilers
{
    public class VisualBasicTranspiler : RoslynTranspiler
    {
        public VisualBasicTranspiler()
        {
            Initialize(SyntaxGenerator.GetGenerator(Workspace, LanguageNames.VisualBasic));
        }

        protected override SyntaxNode TranspileForBlock(ForBlock forBlock)
        {

            var roslynForStatement = SyntaxFactory.ForStatement(
                SyntaxFactory.IdentifierName(
                    ((IdentifierName)forBlock.VariableInitializer.Left).Identifier
                ),
                (ExpressionSyntax)TranspileExpression(forBlock.VariableInitializer.Right),
                (ExpressionSyntax)TranspileExpression(forBlock.ToExpression)
            );

            var roslynStatements = new SyntaxList<StatementSyntax>();
            foreach (var statement in forBlock.Statements)
            {
                if (statement is NoOperation)
                    continue;

                var roslynStatement = TranspileStatement(statement);
                roslynStatements = roslynStatements.Add((StatementSyntax)roslynStatement);
            }

            var roslynNextStatement = SyntaxFactory.NextStatement();

            var roslynForBlock = SyntaxFactory.ForBlock(
                roslynForStatement,
                roslynStatements,
                roslynNextStatement
            );

            return roslynForBlock;
        }


        protected override SyntaxNode TranspileUnaryPlusOperation(SyntaxNode roslynRight)
        {
            return SyntaxFactory.UnaryPlusExpression(
                /* Parenthesize the expression to keep the parenthesis hell consistent. */
                SyntaxFactory.ParenthesizedExpression((ExpressionSyntax)roslynRight)
            );
        }
    }
}