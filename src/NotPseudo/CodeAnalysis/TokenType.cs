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

        Comma,

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

        LeftSquareParenthesis,
        RightSquareParenthesis,

        NumberLiteral,
        StringLiteral,
        TrueLiteral,
        FalseLiteral,

        Identifier,

        DeclareKeyword,
        ArrayKeyword,
        OfKeyword,

        CallKeyword,
        ProcedureKeyword,
        EndProcedureKeyword,
        FunctionKeyword,
        EndFunctionKeyword,
        ReturnKeyword,

        ByRefKeyword,
        ByValKeyword,

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