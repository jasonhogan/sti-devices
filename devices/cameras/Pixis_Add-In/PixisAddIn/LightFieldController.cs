using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using PrincetonInstruments.LightField.AddIns;

namespace STI
{
    class LightFieldController
    {
        private DeviceWrapper device;
        private ILightFieldApplication application;

        public LightFieldController(ILightFieldApplication app, DeviceWrapper dev)
        {
            application = app;

            device = dev;
            installDelegates();
        }

        public void Aquire()
        {
            application.Experiment.Acquire();
        }
        public void Stop()
        {
            application.Experiment.Stop();
        }



        public int PixisCallback(int index)
        {
//            MessageBox.Show("Got Callback: " + Convert.ToString(index));
            return index;
        }

        private void installDelegates()
        {
            testDelegate = new TestDelegate(PixisCallback);
            device.installDelegate(testDelegate);

            aquireDelegate = new AquireDelegate(Aquire);
            device.installAquireDelegate(aquireDelegate);

 //           stopDelegate = new StopDelegate(Stop);
//            device.installStopDelegate(stopDelegate);
        }

        //Delegates
        TestDelegate testDelegate;
        AquireDelegate aquireDelegate;

    }
}
