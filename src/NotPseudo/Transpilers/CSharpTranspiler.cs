using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Editing;

namespace NotPseudo.Transpilers
{
    public class CSharpTranspiler : RoslynTranspiler
    {
        public CSharpTranspiler()
        {
            Initialize(SyntaxGenerator.GetGenerator(Workspace, LanguageNames.CSharp));
        }
    }
}