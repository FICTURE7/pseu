using NotPseudo.CodeAnalysis.Syntax;

namespace NotPseudo.Transpilers
{
    public interface ITranspiler
    {
        string Generate(ProgramBlock programNode);
    }
}