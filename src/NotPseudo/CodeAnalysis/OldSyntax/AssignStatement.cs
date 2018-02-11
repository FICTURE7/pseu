namespace NotPseudo.CodeAnalysis.Syntax
{
    public class AssignStatement : Statement
    {
        public string Identifier { get; set; }
        public Node Expression { get; set; }
    }
}