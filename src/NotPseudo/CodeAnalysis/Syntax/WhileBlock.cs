using System.Collections.Generic;

namespace NotPseudo.CodeAnalysis.Syntax
{
    public class WhileBlock : Node
    {
        public Node Condition { get; set; }
        public List<Node> Statements { get; set; }
    }
}