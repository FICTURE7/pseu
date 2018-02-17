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

                if (roslynStatement.Length == 1)
                    roslynStatements = roslynStatements.Add((StatementSyntax)roslynStatement[0]);
                else
                {
                    foreach (var a in roslynStatement)
                        roslynStatements = roslynStatements.Add((StatementSyntax)a);
                }
            }

            var roslynNextStatement = SyntaxFactory.NextStatement();

            var roslynForBlock = SyntaxFactory.ForBlock(
                roslynForStatement,
                roslynStatements,
                roslynNextStatement
            );

            return roslynForBlock;
        }

        protected override SyntaxNode TranspileRepeatBlock(RepeatBlock repeatBlock)
        {
            var roslynCondition = TranspileExpression(repeatBlock.Condition);
            var roslynUntilStatement = SyntaxFactory.LoopUntilStatement(
                SyntaxFactory.UntilClause((ExpressionSyntax)roslynCondition)
            );

            var roslynStatements = new SyntaxList<StatementSyntax>();
            foreach (var statement in repeatBlock.Statements)
            {
                if (statement is NoOperation)
                    continue;

                var roslynStatement = TranspileStatement(statement);

                if (roslynStatement.Length == 1)
                    roslynStatements = roslynStatements.Add((StatementSyntax)roslynStatement[0]);
                else
                {
                    foreach (var a in roslynStatement)
                        roslynStatements = roslynStatements.Add((StatementSyntax)a);
                }
            }

            var roslynUntilBlock = SyntaxFactory.DoLoopUntilBlock(
                SyntaxFactory.SimpleDoStatement(),
                roslynStatements,
                roslynUntilStatement
            );

            return roslynUntilBlock;
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