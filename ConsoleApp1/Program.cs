using Microsoft.Extensions.Logging.Abstractions;
using Xbim.Common.Configuration;
using Xbim.Geometry.Abstractions;
using Xbim.Ifc;

namespace ConsoleApp1
{
    internal class Program
    {
        static void Main(string[] args)
        {
            var source = args.FirstOrDefault()
                ?? throw new ArgumentException("Expected target path in first argument.");
            var target = args.Skip(1).FirstOrDefault()
                ?? throw new ArgumentException("Expected target path in second argument.");
            Console.WriteLine("Hello, World!");

            // In Startup
            //var services = new ServiceCollection();
            //services
            //    .AddXbimToolkit(conf => conf.AddGeometryServices(opt => opt.Configure(o => o.GeometryEngineVersion = XGeometryEngineVersion.V6)));
            ////services.AddXbimToolkit(builder => builder.AddGeometryServices());

            //var servicesProvider = services.BuildServiceProvider();
            //var ef = servicesProvider.GetRequiredService<IXbimGeometryServicesFactory>();

            // ... then either inject the IXbimGeometryEngine in a ctor, or get from serviceProvider
            //var engine = servicesProvider.GetRequiredService<IXbimGeometryEngine>();

            // Register the current model, which associates a new native Geometry Engine

            //XbimServices.Current.ConfigureServices(sc => sc.AddXbimToolkit(builder => builder.AddGeometryServices()));
            XbimServices.Current.ConfigureServices(services =>
            {
                services.AddXbimToolkit(builder => builder.AddHeuristicModel().AddGeometryServices());
                services.AddXbimGeometryServices();
            });
            Directory.CreateDirectory(Path.GetDirectoryName(target)!);
            using (var fs = new FileStream(target, FileMode.Create))
            using (var fs2 = new FileStream(Path.ChangeExtension(target, ".new.ifc"), FileMode.Create))
            using (var bw = new BinaryWriter(fs))
            using (var ret = IfcStore.Open(source, null, 0))
            {
                var model = ret.Model;
                var loggerFactory = NullLoggerFactory.Instance;
                //var engine = ef.CreateGeometryEngineV6(model, loggerFactory);
                //var model = new MemoryModel(new Xbim.Ifc2x3.EntityFactoryIfc2x3());
                //var context = new Xbim3DModelContext(model, loggerFactory, XGeometryEngineVersion.V6);

                var mc = new Xbim.ModelGeometry.Scene.Xbim3DModelContext(ret.Model, engineVersion: XGeometryEngineVersion.V6);
                mc.CreateContext();

                ret.Model.SaveAsWexBim(bw);
                ret.SaveAsIfc(fs2);
                ret.SaveAs(Path.ChangeExtension(target, ".xbim"), format: Xbim.IO.StorageType.Xbim);
                //
                //var context = new Xbim3DModelContext(ret, null);
                //context.CreateContext();
                //ret.SaveAs(xbimFileName, Xbim.IO.StorageType.Xbim);
                ret.Close();
            }
        }
    }
}