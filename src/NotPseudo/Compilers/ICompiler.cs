namespace NotPseudo.Compilers
{
    public interface ICompiler
    {
        ICompilation Compile(string code);
    }
}