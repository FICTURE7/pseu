namespace NotPseudo.CodeAnalysis
{
    public enum TokenType
    {
        Plus,
        Minus,
        Divide,
        Multiply,


        Colon,
        Assign,

        Equal,
        NotEqual,
        Greater,
        GreaterEqual,
        Less,
        LessEqual,
        AndKeyword,
        OrKeyword,
        NotKeyword,

        LeftParenthesis,
        RightParenthesis,

        NumberLiteral,
        StringLiteral,
        TrueLiteral,
        FalseLiteral,

        Identifier,

        DeclareKeyword,

        ForKeyword,
        ToKeyword,
        EndForKeyword,
        
        RepeatKeyword,
        UntilKeyword,

        WhileKeyword,
        EndWhileKeyword,

        OutputKeyword,
        InputKeyword,

        IfKeyword,
        ThenKeyword,
        ElseKeyword,
        EndIfKeyword,

        LineFeed,
        EoF
    }
}