using System.Collections.Generic;

namespace NotPseudo.CodeAnalysis.Syntax
{
    public class Call : Node
    {
        public IdentifierName Identifier { get; set; }
        public List<Node> Arguments { get; set; }
    }
}