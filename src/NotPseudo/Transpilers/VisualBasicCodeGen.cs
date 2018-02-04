using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Editing;

namespace NotPseudo.Transpilers
{
    public class VisualBasicTranspiler : RoslynTranspiler
    {
        public VisualBasicTranspiler()
        {
            Initialize(SyntaxGenerator.GetGenerator(Workspace, LanguageNames.VisualBasic));
        }
    }
}