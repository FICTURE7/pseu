namespace NotPseudo.CodeAnalysis.Syntax
{
    public class VariableDeclaration : Node
    {
        public string Identifier { get; set; }
        public string Type { get; set; }
    }
}