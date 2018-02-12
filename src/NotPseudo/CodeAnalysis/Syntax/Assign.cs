namespace NotPseudo.CodeAnalysis.Syntax
{
    public class Assign : Node
    {
        public Node Left { get; set; }
        public Node Right { get; set; }

        public override string ToString()
        {
            return $"assign({Left}, {Right})";
        }
    }
}