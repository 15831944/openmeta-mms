﻿using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Windows;

namespace PETBrowser
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        protected override void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);

            var tempDir = Path.Combine(
                System.IO.Path.GetTempPath(),
                Path.Combine("META", "PETBrowser"));


            if (Directory.Exists(tempDir) == false)
            {
                Directory.CreateDirectory(tempDir);
            }

            var traceFileName = Path.Combine(tempDir, string.Format("PETBrowserLog.trace.{0}.txt", Process.GetCurrentProcess().Id));

            var fs = new FileStream(traceFileName, FileMode.Create);
            TraceListener fileTL = new TextWriterTraceListener(fs)
            {
                TraceOutputOptions = TraceOptions.DateTime
            };
            // use TraceXXX to get timestamp per http://stackoverflow.com/questions/863394/add-timestamp-to-trace-writeline

            Trace.AutoFlush = true;
            Trace.Listeners.Add(fileTL);

            //Trace.TraceInformation(META.Logger.Header());
            Trace.TraceInformation("Results Browser launched");
        }
    }
}
