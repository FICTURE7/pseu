using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.VisualBasic;
using VBCompilation = Microsoft.CodeAnalysis.VisualBasic.VisualBasicCompilation;

namespace NotPseudo.Compilers
{
    public class VisualBasicCompiler : ICompiler
    {
        public ICompilation Compile(string code)
        {
            var tree = VisualBasicSyntaxTree.ParseText(code);
            var trustedAssmblies = ((string)AppContext.GetData("TRUSTED_PLATFORM_ASSEMBLIES")).Split(Path.PathSeparator);
            var neededAssemblies = new[]
            {
                    "mscorlib",
                    "System.Private.CoreLib",
                    "System.Runtime",
                    "System.Console",
                    "Microsoft.VisualBasic"
            };

            /*
            var references = trustedAssmblies
                .Where(p => neededAssemblies.Contains(Path.GetFileNameWithoutExtension(p)))
                .Select(p => MetadataReference.CreateFromFile(p))
                .ToList<MetadataReference>();
            */

            var references = new List<MetadataReference> {
                MetadataReference.CreateFromFile(@"C:\Windows\Microsoft.NET\Framework\v4.0.30319\mscorlib.dll"),
                MetadataReference.CreateFromFile(@"C:\Windows\Microsoft.NET\Framework\v4.0.30319\System.dll"),
                MetadataReference.CreateFromFile(@"C:\Windows\Microsoft.NET\Framework\v4.0.30319\System.Core.dll"),
                MetadataReference.CreateFromFile(@"C:\Windows\Microsoft.NET\Framework\v4.0.30319\Microsoft.VisualBasic.dll")
            };

            var assemblyName = Path.GetRandomFileName();
            var compilation = VBCompilation.Create(
                assemblyName,
                new[] { tree },
                references: references,
                options: new VisualBasicCompilationOptions(OutputKind.ConsoleApplication, mainTypeName: "Program")
            );

            return new VisualBasicCompilation(compilation);
        }
    }
}