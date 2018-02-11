namespace NotPseudo.CodeAnalysis.Syntax
{
    public class IdentifierName : Node
    {
        public string Identifier { get; set; }

        public override string ToString()
        {
            return $"ident({Identifier})";
        }
    }
}