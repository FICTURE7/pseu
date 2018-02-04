namespace NotPseudo.Transpilers
{
    public interface ITranspiler
    {
        string Generate(Node node);
    }
}