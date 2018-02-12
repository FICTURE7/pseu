namespace NotPseudo.CodeAnalysis.Syntax
{
    public class Output : Node
    {
        public Node Expression {get;set;}
        
        public override string ToString()
        {
            return $"output({Expression})";
        }
    }
}