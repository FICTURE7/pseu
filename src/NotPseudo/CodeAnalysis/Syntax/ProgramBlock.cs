using System.Collections.Generic;

namespace NotPseudo.CodeAnalysis.Syntax
{
    public class ProgramBlock : Node
    {
        public List<Node> Statements { get; set; }

        public override string ToString()
        {
            return $"program() {{..{Statements.Count}}}";
        }
    }
}