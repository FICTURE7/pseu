namespace NotPseudo.CodeAnalysis.Syntax
{
    public class VariableDeclarationStatement : Node
    {
        public string Identifier { get; set; }
        public string Type { get; set; }
    }
}