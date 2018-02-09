namespace NotPseudo.CodeAnalysis.Syntax
{
    public class ForStatement : Statement
    {
        public Node FromExpression { get; set; }
        public Node ToExpression { get; set; }
        public ForBlock Block { get; set; }
    }
}