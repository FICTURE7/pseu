namespace NotPseudo.CodeAnalysis.Syntax
{
    public class Parameter : Node
    {
        public IdentifierName Identifier { get; set; }
        public IdentifierName TypeIdentifier { get; set; }
        public ParameterKind Kind { get; set; }

        public enum ParameterKind
        {
            None, /* -> Also ByVal */
            ByRef
        }
    }
}