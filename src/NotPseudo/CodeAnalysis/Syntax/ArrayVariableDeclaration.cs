namespace NotPseudo.CodeAnalysis.Syntax
{
    public class ArrayVariableDeclaration : Node
    {
        public IdentifierName Identifier { get; set; }
        public IdentifierName Type { get; set; }
        public Node Length { get; set; }

        public override string ToString()
        {
            return $"arraydecl({Identifier.Identifier}:{Type.Identifier}[{Length}])";
        }
    }
}