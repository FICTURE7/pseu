namespace NotPseudo.CodeAnalysis.Syntax
{
    public class NumberLiteral : Node
    {
        public int Value { get; set; }

        public override string ToString()
        {
            return $"num({Value})";
        }
    }
}