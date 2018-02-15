using System.Collections.Generic;

namespace NotPseudo.CodeAnalysis.Syntax
{
    public class ForBlock : Node
    {
        public Assign VariableInitializer { get; set; }
        public Node ToExpression { get; set; }
        public List<Node> Statements { get; set; }

        public override string ToString()
        {
            return $"for({VariableInitializer} to {ToExpression}) {{..{Statements.Count}}}";
        }
    }
}