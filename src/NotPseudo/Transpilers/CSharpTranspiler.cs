using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Editing;
using NotPseudo.CodeAnalysis.Syntax;

namespace NotPseudo.Transpilers
{
    public class CSharpTranspiler : RoslynTranspiler
    {
        public CSharpTranspiler()
        {
            Initialize(SyntaxGenerator.GetGenerator(Workspace, LanguageNames.CSharp));
        }

        protected override SyntaxNode TranspileForStatement(ForStatement stmt)
        {
            throw new System.NotImplementedException();
        }
    }
}