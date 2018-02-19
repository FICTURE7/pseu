namespace NotPseudo.CodeAnalysis.Syntax
{
    public class StringLiteral : Node
    {
        public string Value { get; set; }

        public override string ToString() 
        {
            return $"str(\"{Value}\")";
        }
    }
}