namespace NotPseudo.CodeAnalysis.Syntax
{
    public class VariableDeclaration : Node
    {
        public IdentifierName Identifier { get; set; }
        public IdentifierName Type { get; set; }

        public override string ToString()
        {
            return $"vardecl({Identifier.Identifier}:{Type.Identifier})";
        }
    }
}