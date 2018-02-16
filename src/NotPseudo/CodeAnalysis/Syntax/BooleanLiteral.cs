namespace NotPseudo.CodeAnalysis.Syntax
{
    public class BooleanLiteral : Node
    {
        public bool Value { get; set; }

        public override string ToString() 
        {
            return $"bool({Value})";
        }
    }
}