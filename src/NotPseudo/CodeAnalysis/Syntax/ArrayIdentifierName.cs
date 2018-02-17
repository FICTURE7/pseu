namespace NotPseudo.CodeAnalysis.Syntax
{
    public class ArrayIdentifierName : Node
    {
        public string Identifier { get; set; }
        public Node IndexExpression { get; set; }

        public override string ToString()
        {
            return $"arrayident({Identifier}[{IndexExpression}])";
        }
    }
}