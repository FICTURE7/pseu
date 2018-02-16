using System.Collections.Generic;

namespace NotPseudo.CodeAnalysis.Syntax
{
    public class RepeatBlock : Node
    {
        public List<Node> Statements { get; set; }
        public Node Condition { get; set; }
    }
}