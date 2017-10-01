using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;


using PrincetonInstruments.LightField.AddIns;

namespace STI
{
    class LightFieldController : PixisDeviceCallbackListener
    {
        private ILightFieldApplication application;

        EventHandler<ExperimentCompletedEventArgs> acquireCompletedEventHandler_;


        public int imageCount;     //The number of images to aquire

        PixisAddIn controller_;

        public LightFieldController(ILightFieldApplication app, PixisAddIn controller)
        {
            controller_ = controller;
            application = app;
            imageCount = 0;
        }

        public void ClearImageCount()
        {
            imageCount = 0;
        }
        public void IncrementImageCount()
        {
            imageCount++;
        }
        public bool IsReadyToAquire()
        {
            return application.Experiment.IsReadyToRun;
        }

        public void Aquire(int index)
        {
            controller_.startAcquisition(index);
        //    testAquire(index);
        }


        public void Stop()
        {
            controller_.stopAcquisition();
//            application.Experiment.Stop();
        }

        private void testAquire(int index)
        {
     //       application.Experiment.Acquire();
     
            // Are we in a state where we can do this?
            //            if (!ValidateAcquisition())
            //                return;

            // Get the experiment object
            IExperiment experiment = application.Experiment;
            if (experiment != null)
            {
                // Not All Systems Have an Exposure Setting, if they do get the minimum and set it
                if (experiment.Exists(CameraSettings.ShutterTimingExposureTime))
                {
                    //                    ISettingRange currentRange = experiment.GetCurrentRange(CameraSettings.ShutterTimingExposureTime);
                    //                    experiment.SetValue(CameraSettings.ShutterTimingExposureTime, currentRange.Minimum);
                }

                // Attach Date/Time to filename
                experiment.SetValue(ExperimentSettings.FileNameGenerationAttachDate, true);
                experiment.SetValue(ExperimentSettings.FileNameGenerationAttachTime, true);

                // Save file as Specific.Spe to the default directory
                experiment.SetValue(ExperimentSettings.FileNameGenerationBaseFileName, "SpecificZ");

                // Connnect the event handler
                acquireCompletedEventHandler_ = new EventHandler<ExperimentCompletedEventArgs>(exp_AcquisitionComplete);
                experiment.ExperimentCompleted += acquireCompletedEventHandler_;

                // Begin the acquisition 
                experiment.Acquire();
            }
        }
        void exp_AcquisitionComplete(object sender, ExperimentCompletedEventArgs e)
        {
            ((IExperiment)sender).ExperimentCompleted -= acquireCompletedEventHandler_;
        }


    }
}
