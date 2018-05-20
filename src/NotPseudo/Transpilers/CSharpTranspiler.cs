using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Editing;
using NotPseudo.CodeAnalysis.Syntax;

namespace NotPseudo.Transpilers
{
    public class CSharpTranspiler : RoslynTranspiler
    {
        public CSharpTranspiler() : base(LanguageNames.CSharp)
        {
            // Space
        }

        protected override SyntaxNode TranspileForBlock(ForBlock forBlock)
        {
            return null;
        }

        protected override SyntaxNode TranspileRepeatBlock(RepeatBlock repeatBlock)
        {
            throw new System.NotImplementedException();
        }

        protected override SyntaxNode TranspileUnaryPlusOperation(SyntaxNode roslynRight)
        {
            throw new System.NotImplementedException();
        }
    }
}