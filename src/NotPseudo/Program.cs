using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.Text;
using Microsoft.CodeAnalysis.VisualBasic;
using NotPseudo.CodeAnalysis;
using NotPseudo.CodeAnalysis.Syntax;
using NotPseudo.Transpilers;

namespace NotPseudo
{
    public static class Program
    {
        public static void Main(string[] args)
        {
            CompileAndRun(Transpile<VisualBasicTranspiler>("sample/no-choice.pseudo"));

            /*
            Transpile<VisualBasicTranspiler>("sample/while-loop.pseudo");
            Transpile<VisualBasicTranspiler>("sample/repeat-until-loop.pseudo");
            Transpile<VisualBasicTranspiler>("sample/output-greatest-value.pseudo");
            Transpile<VisualBasicTranspiler>("sample/greet.pseudo");
            Transpile<VisualBasicTranspiler>("sample/expression.pseudo");
            Transpile<VisualBasicTranspiler>("sample/for-loop.pseudo");
            Transpile<VisualBasicTranspiler>("sample/helloworld.pseudo");
            Transpile<VisualBasicTranspiler>("sample/more-helloworld.pseudo");
            Transpile<VisualBasicTranspiler>("sample/even-more-helloworld.pseudo");
            Transpile<VisualBasicTranspiler>("sample/boolean-expression.pseudo");
            Transpile<VisualBasicTranspiler>("sample/if-condition.pseudo");
            */
        }

        private static string Transpile<TTranspiler>(string srcPath) where TTranspiler : ITranspiler, new()
        {
            Console.WriteLine($"Transpiling {Path.GetFileName(srcPath)} using the {typeof(TTranspiler).Name}...");
            Console.WriteLine("-------------------");
            var src = File.ReadAllText(srcPath);
            var lexer = new Lexer(SourceText.From(src));
            var parser = new Parser(lexer);

            var ast = (ProgramBlock)parser.Parse();

            var gen = new TTranspiler();
            var code = gen.Generate(ast);

            Console.WriteLine(code);
            Console.WriteLine("-------------------");
            return code;
        }

        private static void CompileAndRun(string code)
        {
            var vbTree = VisualBasicSyntaxTree.ParseText(code);

            var trustedAssmblies = ((string)AppContext.GetData("TRUSTED_PLATFORM_ASSEMBLIES")).Split(Path.PathSeparator);

            var neededAssemblies = new[]
            {
                    "mscorlib",
                    "System.Private.CoreLib",
                    "System.Runtime",
                    "System.Console",
                    "Microsoft.VisualBasic"
            };
            List<MetadataReference> references = trustedAssmblies
                .Where(p => neededAssemblies.Contains(Path.GetFileNameWithoutExtension(p)))
                .Select(p => MetadataReference.CreateFromFile(p))
                .ToList<MetadataReference>();

            var assemblyName = Path.GetRandomFileName();

            var compilation = VisualBasicCompilation.Create(
                assemblyName,
                new[] { vbTree },
                references: references,
                options: new VisualBasicCompilationOptions(OutputKind.DynamicallyLinkedLibrary)
            );

            using (var stream = new MemoryStream())
            {
                var result = compilation.Emit(stream);

                stream.Position = 0;

                var assembly = Assembly.Load(stream.ToArray());
                var programType = assembly.GetType("Program");
                var instance = Activator.CreateInstance(programType);
                var ret = programType.InvokeMember("Main", BindingFlags.Default | BindingFlags.InvokeMethod, null, instance, null);
            }
        }
    }
}
