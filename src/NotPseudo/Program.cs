using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text;
using CommandLine;
using Microsoft.CodeAnalysis.Text;
using NotPseudo.CodeAnalysis;
using NotPseudo.Compilers;
using NotPseudo.Transpilers;

namespace NotPseudo
{
    public static class Program
    {
        [Verb("run", HelpText = "Executes the specified source file.")]
        public class RunOptions
        {
            [Value(0, MetaName = "input-path", HelpText = "Source file to be executed.", Required = true)]
            public string InputPath { get; set; }
        }

        [Verb("compile", HelpText = "Compiles the specified source file.")]
        public class CompileOptions
        {
            [Value(0, MetaName = "input-path", HelpText = "Source file to be compiled.", Required = true)]
            public string InputPath { get; set; }

            [Option('o', "output", HelpText = "Output file path.")]
            public string OutputPath { get; set; }

            [Option('f', "framework", HelpText = "Target framework.", Default = "net")]
            public string Runtime { get; set; }
        }


        [Verb("transpile", HelpText = "Transpiles the specified source file.")]
        public class TranspileOptions
        {
            [Value(0, MetaName = "input-path", HelpText = "Source file to be transpiled.", Required = true)]
            public string InputPath { get; set; }

            [Option('o', "output", HelpText = "Output file path.")]
            public string OutputPath { get; set; }

            [Option('l', "language", HelpText = "Target language.", Default = "vb")]
            public string Language { get; set; }
        }

        private static int ExecuteRunCommand(RunOptions options)
        {
            var src = File.ReadAllText(options.InputPath);
            var lexer = new CodeAnalysis.Lexer(SourceText.From(src));
            var parser = new CodeAnalysis.Parser(lexer);
            var ast = parser.Parse();

            var transpiler = new VisualBasicTranspiler();
            var code = transpiler.Transpile(ast);

            var compiler = new VisualBasicCompiler();
            var compilation = compiler.Compile(code);

            if (compilation.CanRun == true)
                compilation.Run();

            return 0;
        }

        private static int ExecuteCompileCommand(CompileOptions options)
        {
            var sw = Stopwatch.StartNew();
            Console.WriteLine($"Compiling {options.InputPath}...");

            var src = File.ReadAllText(options.InputPath);
            var lexer = new CodeAnalysis.Lexer(SourceText.From(src));
            var parser = new CodeAnalysis.Parser(lexer);
            var ast = parser.Parse();

            var transpiler = new VisualBasicTranspiler();
            var code = transpiler.Transpile(ast);

            var compiler = new VisualBasicCompiler();
            var compilation = compiler.Compile(code);

            if (options.OutputPath == null)
            {
                var basePath = Path.GetDirectoryName(options.InputPath);
                var fileName = Path.GetFileNameWithoutExtension(options.InputPath) + ".exe";

                options.OutputPath = Path.Combine(basePath, fileName);
            }

            using (var fs = new FileStream(options.OutputPath, FileMode.Create, FileAccess.Write))
                compilation.Write(fs);

            sw.Stop();
            Console.WriteLine($"Done in {sw.Elapsed.TotalSeconds}s.");

            return 0;
        }

        private static int ExecuteTranspileCommand(TranspileOptions options)
        {
            if (options.Language != "vb")
            {
                Console.Error.WriteLine("ERROR: Only Visual Basic (vb) supported.");
                return 1;
            }

            var sw = Stopwatch.StartNew();
            Console.WriteLine($"Transpiling {options.InputPath}...");

            var src = File.ReadAllText(options.InputPath);
            var lexer = new CodeAnalysis.Lexer(SourceText.From(src));
            var parser = new CodeAnalysis.Parser(lexer);
            var ast = parser.Parse();

            var transpiler = new VisualBasicTranspiler();
            var code = transpiler.Transpile(ast);

            if (options.OutputPath == null)
            {
                var basePath = Path.GetDirectoryName(options.InputPath);
                var fileName = Path.GetFileNameWithoutExtension(options.InputPath) + ".vb";

                options.OutputPath = Path.Combine(basePath, fileName);
            }

            using (var fs = new FileStream(options.OutputPath, FileMode.Create, FileAccess.Write))
            {
                var bytes = Encoding.UTF8.GetBytes(code);
                fs.Write(bytes, 0, bytes.Length);
            }

            sw.Stop();
            Console.WriteLine($"Done in {sw.Elapsed.TotalSeconds}s.");
            return 0;
        }

        public static int Main(string[] args)
        {
            return CommandLine.Parser.Default
                    .ParseArguments<RunOptions, CompileOptions, TranspileOptions>(args)
                    .MapResult(
                        (RunOptions runOpt) => ExecuteRunCommand(runOpt),
                        (CompileOptions compileOpt) => ExecuteCompileCommand(compileOpt),
                        (TranspileOptions transpileOpt) => ExecuteTranspileCommand(transpileOpt),
                        (IEnumerable<Error> err) => 1
                    );

            /*
            var src = File.ReadAllText("sample/byref-procedure.pseudo");
            var lexer = new Lexer(SourceText.From(src));
            var parser = new Parser(lexer);
            var ast = parser.Parse();

            var transpiler = new VisualBasicTranspiler();
            var code = transpiler.Transpile(ast);

            var compiler = new VisualBasicCompiler();
            var compilation = compiler.Compile(code);

            if (compilation.CanRun == true)
                compilation.Run();

            var stream = File.OpenWrite("xD.exe");
            compilation.Write(stream);
            stream.Close();
            */

            /*
            Transpile<VisualBasicTranspiler>("sample/byref-procedure.pseudo");
            Transpile<VisualBasicTranspiler>("sample/factorial.pseudo");
            Transpile<VisualBasicTranspiler>("sample/function.pseudo");
            Transpile<VisualBasicTranspiler>("sample/procedure.pseudo");
            Transpile<VisualBasicTranspiler>("sample/greet-fancier.pseudo");
            Transpile<VisualBasicTranspiler>("sample/bubble-sort.pseudo");
            Transpile<VisualBasicTranspiler>("sample/array.pseudo");
            Transpile<VisualBasicTranspiler>("sample/assign-boolean.pseudo");
            Transpile<VisualBasicTranspiler>("sample/greet-fancy.pseudo");
            Transpile<VisualBasicTranspiler>("sample/no-choice.pseudo");
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
    }
}
