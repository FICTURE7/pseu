namespace NotPseudo.CodeAnalysis.Syntax
{
    public class Input : Node
    {
        public Node Prefix { get; set; }
        public Node Identifier { get; set; }

        public override string ToString()
        {
            return $"input({Identifier})";
        }
    }
}