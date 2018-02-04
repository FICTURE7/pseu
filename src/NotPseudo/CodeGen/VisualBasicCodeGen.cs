using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Editing;

namespace NotPseudo.CodeGen
{
    public class VisualBasicCodeGen : RoslynCodeGen
    {
        public VisualBasicCodeGen()
        {
            Initialize(SyntaxGenerator.GetGenerator(Workspace, LanguageNames.VisualBasic));
        }
    }
}