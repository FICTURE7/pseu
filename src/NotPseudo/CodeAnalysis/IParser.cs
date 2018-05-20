using NotPseudo.CodeAnalysis.Syntax;

namespace NotPseudo.CodeAnalysis
{
    public interface IParser
    {
        ProgramBlock Parse();
    }
}