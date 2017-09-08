using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.AddIn;
using System.AddIn.Pipeline;

using PrincetonInstruments.LightField.AddIns;

using System.Threading;

// Allow managed code to call unmanaged functions that are implemented in a DLL
//using System.Runtime.InteropServices;

using System.Runtime.CompilerServices;

namespace STI
{


    ///////////////////////////////////////////////////////////////////////////
    //  This sample sets an exposure time, disables date and time as part of the 
    // file name and sets a specific name. Note pressing the button multiple times
    // will continually overwrite the file. (This sample also overrides the file
    // already exists dialog)
    ///////////////////////////////////////////////////////////////////////////
    [AddIn("PIXIS STI Device Add-In",
    Version = "1.0.0",
    Publisher = "Stanford University",
    Description = "STI device interface for Pixis camera")]

    //    [QualificationData("IsSample", "True")]


    public class PixisAddIn : AddInBase, ILightFieldAddIn
    {
        public int PixisCallback(int index)
        {
            MessageBox.Show("Got Callback: " + Convert.ToString(index) );
            return index;
        }

        DeviceWrapper wrapper;// = new DeviceWrapper();
        Thread deviceThread;// = new Thread(startDeviceWrapper);


        TestDelegate testDelegate;

        //private delegate int Callback(string text);
        //private Callback mInstance;   // Ensure it doesn't get garbage collected

        public PixisAddIn()
        {
            wrapper = new DeviceWrapper();
            deviceThread = new Thread(startDeviceWrapper);

            testDelegate = new TestDelegate(PixisCallback);
            wrapper.installDelegate(testDelegate);
        }

        //public PixisAddIn()
        //{
        //    mInstance = new Callback(Handler);
        //    //            SetCallback(mInstance);
        //}
        //private int Handler(string text)
        //{
        //    // Do something...
        //    //           MessageBox.Show("Callback: " + text);
        //    //            Console.WriteLine(text);
        //    LightFieldApplication.Experiment.Acquire();

        //    return 42;
        //}
        ////        [DllImport("testcpp.dll")]
        ////       private static extern void SetCallback(Callback fn);
        ////        [DllImport("testcpp.dll")]
        ////       private static extern void TestCallback();


        public void startDeviceWrapper()
        {
            try
            {
                wrapper.start();
                MessageBox.Show("Starting device!");
            }
            catch (Exception ex)
            {
                string mess = ex.Message;
                MessageBox.Show("Error: " + mess);
            }
        }



        ////Note: Need to build the dll as x64.  dll need to be in addin directory
        ////               [DllImport("testcpp.dll", CallingConvention = CallingConvention.Cdecl)]
        ////               public static extern int add(int a, int b);

        ////       [DllImport("PixisDevice.dll", CallingConvention = CallingConvention.Cdecl)]
        ////        public static extern void startDevice();

        //[DllImport("testcpp.dll", CallingConvention = CallingConvention.Cdecl)]
        //public static extern void startDevice();

        EventHandler<ExperimentCompletedEventArgs> acquireCompletedEventHandler_;

        private Button control_;
        private Button control2_;
        ///////////////////////////////////////////////////////////////////////
        public UISupport UISupport { get { return UISupport.ExperimentSetting; } }
        ///////////////////////////////////////////////////////////////////////
        public void Activate(ILightFieldApplication app)
        {
            // Capture Interface
            LightFieldApplication = app;

            //LightFieldApplication.Experiment.Acquire();

            // Build your controls
            control_ = new Button();
            control_.Content = "Acquire STI: ";  // + Convert.ToString( add(2,7) );
            control_.Click += new RoutedEventHandler(control__Click);
            ExperimentSettingElement = control_;



            control2_ = new Button();
            control2_.Content = "Test callback";
            control2_.Click += new RoutedEventHandler(control__Click2);
            ExperimentSettingElement = control2_;
        }
        ///////////////////////////////////////////////////////////////////////
        bool ValidateAcquisition()
        {
            IDevice camera = null;
            foreach (IDevice device in LightFieldApplication.Experiment.ExperimentDevices)
            {
                if (device.Type == DeviceType.Camera)
                    camera = device;
            }
            ///////////////////////////////////////////////////////////////////////
            if (camera == null)
            {
                //int tmp = add(2, 7);
                int tmp = 0;
                string mess = "";
                try
                {
                    //                    tmp = add(2, 7);
                    //                   startDevice();
                }
                catch (Exception e)
                {
                    mess = e.Message;
                    //                    Console.WriteLine(mess);
                }

                MessageBox.Show("This sample requires a camera! " + mess + Convert.ToString(tmp));
                return false;
            }
            ///////////////////////////////////////////////////////////////////////
            if (!LightFieldApplication.Experiment.IsReadyToRun)
            {
                MessageBox.Show("The system is not ready for acquisition, is there an error?");
                return false;
            }
            return true;
        }
        private void control__Click2(object sender, RoutedEventArgs e)
        {
            //            deviceThread.Start();
            wrapper.start();

            //            TestCallback();
        }
        ///////////////////////////////////////////////////////////////////////
        // Override some typical settings and acquire an spe file with a 
        // specific name. 
        ///////////////////////////////////////////////////////////////////////
        private void control__Click(object sender, RoutedEventArgs e)
        {

//            deviceThread.Start();

            //try
            //{
            //    startDevice();
            //}
            //catch (Exception ex)
            //{
            //    string mess = ex.Message;
            //    MessageBox.Show("Error: " + mess);
            //}

            // Are we in a state where we can do this?
            if (!ValidateAcquisition())
                return;

            // Get the experiment object
            IExperiment experiment = LightFieldApplication.Experiment;
            if (experiment != null)
            {
                // Not All Systems Have an Exposure Setting, if they do get the minimum and set it
                if (experiment.Exists(CameraSettings.ShutterTimingExposureTime))
                {
                    ISettingRange currentRange = experiment.GetCurrentRange(CameraSettings.ShutterTimingExposureTime);
                    experiment.SetValue(CameraSettings.ShutterTimingExposureTime, currentRange.Minimum);
                }

                // Don't Attach Date/Time
                experiment.SetValue(ExperimentSettings.FileNameGenerationAttachDate, false);
                experiment.SetValue(ExperimentSettings.FileNameGenerationAttachTime, false);

                // Save file as Specific.Spe to the default directory
                experiment.SetValue(ExperimentSettings.FileNameGenerationBaseFileName, "Specific");

                // Connnect the event handler
                acquireCompletedEventHandler_ = new EventHandler<ExperimentCompletedEventArgs>(exp_AcquisitionComplete);
                experiment.ExperimentCompleted += acquireCompletedEventHandler_;

                // Begin the acquisition 
                experiment.Acquire();
            }
        }
        ///////////////////////////////////////////////////////////////////////
        // Acquire Completed Handler
        // This just fires a message saying that the data is acquired.
        ///////////////////////////////////////////////////////////////////////
        void exp_AcquisitionComplete(object sender, ExperimentCompletedEventArgs e)
        {
            ((IExperiment)sender).ExperimentCompleted -= acquireCompletedEventHandler_;

            //            add(2, 2); //signal device to stop waiting

            MessageBox.Show("Acquire Completed");

        }
        ///////////////////////////////////////////////////////////////////////
        public void Deactivate() { }
        ///////////////////////////////////////////////////////////////////////
        public override string UIExperimentSettingTitle { get { return "Custom Acquire Sample"; } }
    }

}
