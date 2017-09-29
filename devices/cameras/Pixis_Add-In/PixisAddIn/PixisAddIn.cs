using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.AddIn;
using System.AddIn.Pipeline;

using System.IO;

using System.Threading;

using PrincetonInstruments.LightField.AddIns;




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
        static void MyHandler(object sender, UnhandledExceptionEventArgs args)
        {
            Exception e = (Exception)args.ExceptionObject;
            MessageBox.Show("PIXIS STI Device handler caught : " + e.Message);
            MessageBox.Show("Runtime terminating: "+ Convert.ToString(args.IsTerminating));
        }

        public PixisAddIn()
        {

            AppDomain currentDomain = AppDomain.CurrentDomain;
            currentDomain.UnhandledException += new UnhandledExceptionEventHandler(MyHandler);

            userInterface = new PixisDeviceUserControl(LightFieldApplication, this);
            controller = new LightFieldController(LightFieldApplication, this);


            callbackHandler = new DeviceCallbackHandler();
            makeDelegates();

            callbackHandler.addListener(userInterface);
            callbackHandler.addListener(controller);

        }

        ~PixisAddIn()
        {
            wrapper.Dispose();
        }


        //public String GetImageFilename(int index)
        //{
        //    return "filename.tif";
        //}



        DeviceWrapper wrapper;// = new DeviceWrapper();
        LightFieldController controller;

        DeviceCallbackHandler callbackHandler;// = new DeviceCallbackHandler();

        private STI.PixisDeviceUserControl userInterface;
        private Thread deviceThread;


        StopDelegate stopDelegate;
        AquireDelegate aquireDelegate;
        ClearImageCount clearCountDelegate;
        IncrementImageCount incrementCountDelegate;


        void makeDelegates()
        {
            aquireDelegate = new AquireDelegate(callbackHandler.Aquire);
            stopDelegate = new StopDelegate(callbackHandler.Stop);
            clearCountDelegate = new ClearImageCount(callbackHandler.ClearImageCount);
            incrementCountDelegate = new IncrementImageCount(callbackHandler.IncrementImageCount);
        }

        void installDelegates()
        {
            wrapper.installDelegate(aquireDelegate);
            wrapper.installDelegate(stopDelegate);
            wrapper.installDelegate(clearCountDelegate);
            wrapper.installDelegate(incrementCountDelegate);
        }


        EventHandler<ExperimentCompletedEventArgs> acquireCompletedEventHandler_;


        public void connect()
        {
            new Thread(delegate () {

                wrapper = new DeviceWrapper();
                
                installDelegates();

                deviceThread = new Thread(startDeviceWrapper);
//                deviceThread.IsBackground = true;

                if (!deviceThread.IsAlive)
                {
                    deviceThread.Start();
                }
                else
                {
                    MessageBox.Show("Cannot connect; device thread IsAlive already.");
                }

            }).Start();

        }

        public void disconnect()
        {
            new Thread(delegate () {
                if (wrapper != null)
                {
                    wrapper.shutdown();
                }
                if (deviceThread != null && deviceThread.IsAlive)
                {
                    deviceThread.Join();
                }
                //                MessageBox.Show("Device disconnected successfully.");

            }).Start();
        }

        public void startDeviceWrapper()
        {
            try
            {
                if(wrapper != null)
                {
                    wrapper.startDevice();
                }
            }
            catch (Exception ex)
            {
                string mess = ex.Message;
                MessageBox.Show("Error: " + mess);
            }
        }

        ///////////////////////////////////////////////////////////////////////
        public UISupport UISupport { get { return UISupport.ExperimentSetting; } }
        ///////////////////////////////////////////////////////////////////////
        public void Activate(ILightFieldApplication app)
        {
            // Capture Interface
            LightFieldApplication = app;

            ExperimentSettingElement = userInterface;

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
                MessageBox.Show("This sample requires a camera!");
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


        private int currentImageIndex = 0;

        public void stopAcquisition()
        {
            IExperiment experiment = LightFieldApplication.Experiment;
            if (experiment != null)
            {
                experiment.Stop();
            }
        }

        public void startAcquisition(int index)
        {
            // Are we in a state where we can do this?
            if (!ValidateAcquisition())
                return;

            currentImageIndex = index;

            // Get the experiment object
            IExperiment experiment = LightFieldApplication.Experiment;
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
//                experiment.SetValue(ExperimentSettings.FileNameGenerationBaseFileName, "Specific");

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
        public void exp_AcquisitionComplete(object sender, ExperimentCompletedEventArgs e)
        {
            ((IExperiment)sender).ExperimentCompleted -= acquireCompletedEventHandler_;
            //            MessageBox.Show("Acquire Completed");
            string spe_filename = getRecentFilename();

            wrapper.setSavedSPEFilename(currentImageIndex, spe_filename);

            Export_Tiff(spe_filename);

            string tif_filename = Path.ChangeExtension(spe_filename, ".tif");
            wrapper.setSavedImageFilename(currentImageIndex, tif_filename);

            wrapper.stopWaiting(currentImageIndex);

        }

        public string getRecentFilename()
        {
            IList<String> filenames = LightFieldApplication.FileManager.GetRecentlyAcquiredFileNames();

            return filenames.First<string>();
        }

        private void Export_Tiff(string sourceFilename)
        {
            // Get the file manager
            IFileManager fm = LightFieldApplication.FileManager;

            // Block User Pop Ups
            LightFieldApplication.UserInteractionManager.SuppressUserInteraction = true;

            try
            {
                // Add Files
                List<string> itemsForExport = new List<string>();
                itemsForExport.Add(sourceFilename);
//                itemsForExport.AddRange(FilesListBox.Items.Cast<string>());

                IList<IExportSelectionError> selectedErrors = new List<IExportSelectionError>();

                ITiffExportSettings tifSettings = (ITiffExportSettings)fm.CreateExportSettings(ExportFileType.Tiff);

                // Specifics
                tifSettings.IncludeAllExperimentInformation = true;

                IExportSettings baseSettings_ = null;

                baseSettings_ = tifSettings;

                ExportOutputPathOption options_ = ExportOutputPathOption.InputPath;

                // Common Settings for all types
//                baseSettings_.CustomOutputPath = customPath_;
                baseSettings_.OutputPathOption = options_;
                baseSettings_.OutputMode = (ExportOutputMode)Enum.Parse(typeof(ExportOutputMode), "OneFilePerFrame");

                selectedErrors = baseSettings_.Validate(itemsForExport);

                // Busy cursor during write
                using (new AddInStatusHelper(LightFieldApplication, ApplicationBusyStatus.Busy))
                {
                    fm.Export(baseSettings_, itemsForExport);
                }


                //// Async Mode
                //if (AsyncCheckBox.IsChecked == true)
                //{
                //    fm.ExportCompleted += new EventHandler<ExportCompletedEventArgs>(fm_ExportCompleted);

                //    application_.UserInteractionManager.ApplicationBusyStatus = ApplicationBusyStatus.Busy;
                //    fm.ExportAsync(baseSettings_, itemsForExport);

                //    // Enable cancel
                //    CancelButton.IsEnabled = true;
                //    ExportButton.IsEnabled = false;
                //}
                //// Sync Mode
                //else
                //{
                //    ExportButton.IsEnabled = false;

                //    // Busy cursor during write
                //    using (new AddInStatusHelper(LightFieldApplication, ApplicationBusyStatus.Busy))
                //    {
                //        fm.Export(baseSettings_, itemsForExport);
                //    }
                //    ExportButton.IsEnabled = true;
                //}

            }
            finally
            {
                LightFieldApplication.UserInteractionManager.SuppressUserInteraction = false;
            }
        }
        ///////////////////////////////////////////////////////////////////////////
        //
        ///////////////////////////////////////////////////////////////////////////
        void fm_ExportCompleted(object sender, ExportCompletedEventArgs e)
        {
            LightFieldApplication.UserInteractionManager.ApplicationBusyStatus = ApplicationBusyStatus.NotBusy;
            ((IFileManager)sender).ExportCompleted -= fm_ExportCompleted;
        }

        ///////////////////////////////////////////////////////////////////////
        public void Deactivate()
        {
            disconnect();
        }
        ///////////////////////////////////////////////////////////////////////
        public override string UIExperimentSettingTitle { get { return "PIXIS STI Device"; } }
    }

}
