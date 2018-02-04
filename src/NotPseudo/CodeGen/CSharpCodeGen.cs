using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Editing;

namespace NotPseudo.CodeGen
{
    public class CSharpCodeGen : RoslynCodeGen
    {
        public CSharpCodeGen()
        {
            Initialize(SyntaxGenerator.GetGenerator(Workspace, LanguageNames.CSharp));
        }
    }
}