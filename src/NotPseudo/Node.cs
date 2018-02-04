using System.Collections.Generic;

namespace NotPseudo
{
    public class Node
    {
        public Node()
        {
            Childrens = new List<Node>();
        }

        public List<Node> Childrens { get; set; }
    }
}