namespace NotPseudo.CodeAnalysis.Syntax
{
    public class UnaryOperation : Node
    {
        public Token Operation { get; set; }
        public Node Right { get; set; }

        public override string ToString()
        {
            var addSpace = Operation.Type == TokenType.NotKeyword;
            return $"uryop({(addSpace ? Operation.Value + " " : Operation.Value)}{Right})";
        }
    }
}