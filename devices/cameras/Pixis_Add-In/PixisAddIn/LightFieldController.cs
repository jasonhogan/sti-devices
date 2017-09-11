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
        }

        public void Aquire()
        {
            application.Experiment.Acquire();
        }
        public void Stop()
        {
            application.Experiment.Stop();
        }


    }
}
