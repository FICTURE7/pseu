using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.IO;
using System.Linq;
using System.Reflection;
using Microsoft.CodeAnalysis;
using VBCompilation = Microsoft.CodeAnalysis.VisualBasic.VisualBasicCompilation;

namespace NotPseudo.Compilers
{
    public class VisualBasicCompilation : ICompilation
    {
        private readonly ImmutableArray<Diagnostic> _diagnostics;
        private readonly VBCompilation _compilation;

        internal VisualBasicCompilation(VBCompilation compilation)
        {
            if (compilation == null)
                throw new ArgumentNullException(nameof(compilation));

            _compilation = compilation;
            _diagnostics = _compilation.GetDiagnostics();
        }

        public bool CanRun => _diagnostics.IsEmpty;
        public IEnumerable<Diagnostic> Diagnostics => _diagnostics;

        public void Run()
        {
            using (var stream = new MemoryStream())
            {
                var result = _compilation.Emit(stream);
                if (!result.Success)
                {
                    Console.ForegroundColor = ConsoleColor.Red;
                    foreach (var diag in result.Diagnostics)
                        Console.WriteLine(diag);
                }
                else
                {
                    stream.Position = 0;

                    var assembly = Assembly.Load(stream.ToArray());
                    var programType = assembly.GetType("Program");
                    var instance = Activator.CreateInstance(programType);
                    var ret = programType.InvokeMember("Main", BindingFlags.Default | BindingFlags.InvokeMethod, null, instance, null);
                }
            }
        }

        public void Write(Stream stream)
        {
            var result = _compilation.Emit(stream);
            if (!result.Success)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                foreach (var diag in result.Diagnostics)
                    Console.WriteLine(diag);
            }
        }
    }
}