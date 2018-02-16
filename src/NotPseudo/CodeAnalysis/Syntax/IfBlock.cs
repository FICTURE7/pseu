using System.Collections.Generic;

namespace NotPseudo.CodeAnalysis.Syntax
{
    public class IfBlock : Node
    {
        public Node Condition { get; set; }
        public List<Node> TrueStatements { get; set; }
        public List<Node> FalseStatements { get; set; }
    }
}