using System.Collections.Generic;

namespace NotPseudo
{
    public class Node
    {
        public Node()
        {
            // Space
        }

        public NodeType Type { get; set; }
        public List<Node> Childrens { get; set; }
        public object Value { get; set; }
    }
}