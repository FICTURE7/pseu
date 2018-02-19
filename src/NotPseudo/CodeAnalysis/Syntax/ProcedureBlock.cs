using System.Collections.Generic;

namespace NotPseudo.CodeAnalysis.Syntax
{
    public class ProcedureBlock : Node
    {
        public IdentifierName Identifier { get; set; }
        public List<Node> Parameters { get; set; }
        public List<Node> Statements { get; set; }
    }
}