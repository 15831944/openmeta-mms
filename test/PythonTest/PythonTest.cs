﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;
using Xunit;
using META;
using System.Diagnostics;
using System.IO;
using System.Xml;

namespace PythonTest
{
    public class PythonTest
    {
        [STAThread]
        public static int Main(string[] args)
        {
            int ret = Xunit.ConsoleClient.Program.Main(new string[] {
                Assembly.GetAssembly(typeof(PythonTest)).CodeBase.Substring("file:///".Length),
                //"/noshadow",
            });
            Console.In.ReadLine();
            return ret;
        }

        private Exception ImportModule(string moduleName)
        {
            return CheckCommand(String.Format("import {0}", moduleName));
        }

        private Exception CheckPython(string command)
        {
            try
            {
                Process p = new Process();
                p.StartInfo = new ProcessStartInfo()
                {
                    FileName = VersionInfo.PythonVEnvExe,
                    Arguments = command,
                    CreateNoWindow = true,
                    UseShellExecute = false,

                    RedirectStandardError = true
                };
                p.Start();
                string stderr = p.StandardError.ReadToEnd();
                p.WaitForExit();
                if (p.ExitCode != 0)
                {
                    throw new Exception("python.exe exited with non-zero code " + p.ExitCode + ". " + stderr);
                }
            }
            catch (Exception e)
            {
                return e;
            }
            return null;
        }

        private Exception CheckCommand(string command)
        {
            return CheckPython(String.Format("-c \"{0}\"", command));
        }

        [Fact]
        public void TestImports()
        {
            var module_names = new string[] {
                    // Check openmdao.api to be sure bin\Python27\requirements directions were followed re: https://bugs.python.org/issue24960
                    "openmdao.api",
                    //"isis_meta", not used anywhere
                    "MaterialLibraryInterface",
                    "meta_nrmm",
                    "PCC",
                    "py_modelica",
                    "py_modelica_exporter",
                    "omniORB.CORBA", // needed by py_modelica_exporter for openmodelica
                    "run_mdao",
                    "run_mdao.cad.update_parameters",
                    "testbenchexecutor",
                    "excel_wrapper",
                    "matlab_wrapper",
                    "SpiceVisualizer",
                    "SpiceVisualizer.post_process",
                    "SpiceVisualizer.process_spice",
                    "SpiceVisualizer.spicedatareader",
                    "chipfit_display",
                    "layout_json",
                    "pyqtgraph",
                    "spice_viewer",
                    "matlab_wrapper",
                    "tables",
                    "vitables",
                    "get_eagle_path",
            };

            foreach (var test in module_names.Select(moduleName =>
                {
                    Func<Exception> m = () => ImportModule(moduleName);
                    var asyncResult = m.BeginInvoke(null, null);
                    return new Tuple<string, IAsyncResult, Func<Exception>>(moduleName, asyncResult, m);
                }).ToList())
            {
                Exception e = test.Item3.EndInvoke(test.Item2);
                if (e != null)
                {
                    Assert.True(e == null, "Importing " + test.Item1 + " failed: " + e.ToString());
                }
            }
        }

        [Fact]
        public void TestTk()
        {
            var e = CheckCommand("import Tkinter; Tkinter.Tk()");
            if (e != null)
            {
                Assert.True(e == null, "Tkinter failed: " + e.ToString());
            }
        }

        [Fact]
        public void TestPythonExitCode()
        {
            Process p = new Process();
            p.StartInfo = new ProcessStartInfo()
            {
                FileName = VersionInfo.PythonVEnvExe,
                Arguments = String.Format("-c \"import py_modelica; import scipy.io; import sys; sys.exit(42);\""),
                CreateNoWindow = true,
                UseShellExecute = false
            };
            p.Start();
            p.WaitForExit();
            Assert.Equal(42, p.ExitCode);
        }

        [Fact]
        public void TestModelicaExporter()
        {
            if (Environment.GetEnvironmentVariable("OPENMODELICAHOME") == null)
            {
                return;
            }
            var e = CheckPython("-m py_modelica_exporter --standard --json");
            if (e != null)
            {
                Assert.True(e == null, "py_modelica_exporter failed: " + e.ToString());
            }
        }

        [Fact]
        public void TestCyPhyPython()
        {
            string assemblyDir = Path.GetDirectoryName(new Uri(Assembly.GetExecutingAssembly().CodeBase).LocalPath);

            string stderr = "<process did not start>";
            int retcode = Run(VersionInfo.PythonVEnvExe, Path.Combine(assemblyDir, "..\\.."), "test_CyPhyPython.py", out stderr);
            Assert.True(0 == retcode, stderr);
        }

        [Fact]
        public void TestSmop()
        {
            var e = CheckPython(String.Format("\"{0}\" \"{1}\"",
                Path.Combine(VersionInfo.MetaPath, "bin\\Python27\\Lib\\site-packages\\smop\\main.py"),
                Path.Combine(VersionInfo.MetaPath, "bin/Python27/Lib/site-packages/matlab_wrapper/test/stat2.m")));
            if (e != null)
            {
                Assert.True(e == null, "smop failed: " + e.ToString());
            }
        }

        public int Run(string runCommand, string cwd, string args, out string stderr)
        {
            ProcessStartInfo info = new ProcessStartInfo()
            {
                FileName = runCommand,
                WorkingDirectory = cwd,
                Arguments = args,
                UseShellExecute = false,
                CreateNoWindow = true,
                RedirectStandardError = true,
                RedirectStandardOutput = true
            };

            Process proc = new Process();
            proc.StartInfo = info;

            proc.Start();
            string err = "";

            proc.ErrorDataReceived += ((sender, e) =>
            {
                if (e.Data != null)
                {
                    try
                    {
                        err = err + e.Data;
                    }
                    catch (System.ObjectDisposedException)
                    {
                    }
                }
            });
            proc.BeginErrorReadLine();
            var stdout = proc.StandardOutput.ReadToEnd();
            proc.WaitForExit();
            stdout = stdout.Replace("\r", "");
            stderr = err.Replace("\r", "");

            return proc.ExitCode;

        }


        [Fact]
        public void TestRunTestBenchesPy()
        {
            var model_dir = Path.Combine(Assembly.GetAssembly(typeof(PythonTest)).CodeBase.Substring("file:///".Length),
                "../../../../../models/MetricConstraints_Simple");
            var testXmlFilename = Path.Combine(model_dir, "nosetests.xml");
            if (File.Exists(testXmlFilename))
            {
                File.Delete(testXmlFilename);
            }
            Process proc = new Process();
            proc.StartInfo.Arguments = String.Format("\"{0}\" \"{1}\" -- --with-xunit",
                Path.Combine(VersionInfo.MetaPath, "bin", "RunTestBenches.py"),
                Path.Combine(model_dir, "MetricConstraints_Simple.xme"));

            proc.StartInfo.UseShellExecute = false;
            proc.StartInfo.FileName = VersionInfo.PythonVEnvExe;
            proc.StartInfo.RedirectStandardInput = true;
            proc.StartInfo.RedirectStandardOutput = true;
            proc.StartInfo.RedirectStandardError = true;
            proc.StartInfo.WorkingDirectory = model_dir;
            using (proc)
            {
                StringBuilder stdoutData = new StringBuilder();
                StringBuilder stderrData = new StringBuilder();
                proc.OutputDataReceived += (o, args) =>
                {
                    if (args.Data != null)
                    {
                        lock (stdoutData)
                        {
                            stdoutData.Append(args.Data);
                        }
                    }
                };
                proc.ErrorDataReceived += (o, args) =>
                {
                    if (args.Data != null)
                    {
                        lock (stderrData)
                        {
                            stderrData.Append(args.Data);
                        }
                    }
                };
                proc.Start();
                proc.BeginErrorReadLine();
                proc.BeginOutputReadLine();
                proc.StandardInput.Close();

                proc.WaitForExit();

                XmlDocument doc = new XmlDocument();
                doc.Load(testXmlFilename);
                var testsuite = doc.LastChild;
                Assert.Equal(2, int.Parse(testsuite.Attributes["tests"].Value));
                Assert.Equal(1, int.Parse(testsuite.Attributes["failures"].Value));
            }
        }

    }
}
