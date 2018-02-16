namespace NotPseudo.CodeAnalysis.Syntax
{
    public class Input : Node
    {
        public Node Identifier { get; set; }

        public override string ToString()
        {
            return $"input({Identifier})";
        }
    }
}