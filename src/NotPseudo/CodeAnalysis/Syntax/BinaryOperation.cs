namespace NotPseudo.CodeAnalysis.Syntax
{
    public class BinaryOperation : Node
    {
        public Node Left;
        public Token Operation;
        public Node Right;

        public override string ToString()
        {
            return $"binop({Left} {Operation.Value} {Right})";
        }
    }
}