using System.IO;

namespace NotPseudo.Compilers
{
    public interface ICompilation
    {
        bool CanRun { get; }

        void Run();

        void Write(Stream stream);
    }
}