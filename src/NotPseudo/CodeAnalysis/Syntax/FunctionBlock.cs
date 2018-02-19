using System.Collections.Generic;

namespace NotPseudo.CodeAnalysis.Syntax
{
    public class FunctionBlock : Node
    {
        public IdentifierName Identifier { get; set; }
        public List<Node> Parameters { get; set; }
        public IdentifierName ReturnTypeIdentifier { get; set; }
        public List<Node> Statements { get; set; }
    }
}