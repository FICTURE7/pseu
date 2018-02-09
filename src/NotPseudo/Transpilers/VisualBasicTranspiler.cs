using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Editing;
using Microsoft.CodeAnalysis.VisualBasic;
using NotPseudo.CodeAnalysis.Syntax;

namespace NotPseudo.Transpilers
{
    public class VisualBasicTranspiler : RoslynTranspiler
    {
        public VisualBasicTranspiler()
        {
            Initialize(SyntaxGenerator.GetGenerator(Workspace, LanguageNames.VisualBasic));
        }

        protected override SyntaxNode TranspileForStatement(ForStatement stmt)
        {
            //var forStatement = SyntaxFactory.ForStatement()
            return null;
        }
    }
}