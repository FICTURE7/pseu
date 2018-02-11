namespace NotPseudo.CodeAnalysis.Syntax
{
    public class UnaryOperation : Node
    {
        public Token Operation { get; set; }
        public Node Right { get; set; }

        public override string ToString()
        {
            return $"uryop({Operation.Value}{Right})";
        }
    }
}