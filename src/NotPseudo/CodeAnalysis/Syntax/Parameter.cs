namespace NotPseudo.CodeAnalysis.Syntax
{
    public class Parameter : Node
    {
        public IdentifierName Identifier { get; set; }
        public IdentifierName TypeIdentifier { get; set; }
    }
}