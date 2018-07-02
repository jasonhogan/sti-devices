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


namespace STI
{
    ///////////////////////////////////////////////////////////////////////////
    //  Responds to STI device events (parse, play, etc) by operating the Lightfield 
    // program automatically.  Pushes the "Aquire" button when the device plays an event.
    // Saves the SPE and TIF files to a directory specified by the device and reports\
    // the filenames back to the device.
    ///////////////////////////////////////////////////////////////////////////
    [AddIn("STI Device for Pixis Camera",
    Version = "1.0.0",
    Publisher = "Stanford University",
    Description = "STI device interface for Pixis camera")]

    public class PixisAddIn : AddInBase, ILightFieldAddIn, PixisDeviceCallbackListener
    {
        private Thread deviceThread;
        private DeviceWrapper wrapper;
        private PixisDeviceUserControl userInterface;
        private DeviceCallbackHandler callbackHandler;

        EventHandler<ExperimentCompletedEventArgs> acquireCompletedEventHandler_;

        StopDelegate stopDelegate;
        AquireDelegate aquireDelegate;
        ClearImageCount clearCountDelegate;
        IncrementImageCount incrementCountDelegate;
        SetSaveDirDelegate setSaveDirDelegate;
        PrintDelegate printDelegate;
        ExternalTriggerOn externalTriggerOnDelegate;

        private int currentImageIndex = 0;
        private string saveDirectory = null;

        ///////////////////////////////////////////////////////////////////////
        public PixisAddIn()
        {
            //Install excpetion handler
            AppDomain currentDomain = AppDomain.CurrentDomain;
            currentDomain.UnhandledException += new UnhandledExceptionEventHandler(PixisExceptionHandler);

            userInterface = new PixisDeviceUserControl(LightFieldApplication, this);
//            controller = new LightFieldController(LightFieldApplication, this);
            callbackHandler = new DeviceCallbackHandler();

            makeDelegates();

            callbackHandler.addListener(userInterface);
            callbackHandler.addListener(this);            
        }
        ///////////////////////////////////////////////////////////////////////

        ~PixisAddIn()
        {
            if(wrapper != null)
            {
                wrapper.Dispose();
            }
        }
        ///////////////////////////////////////////////////////////////////////

        void makeDelegates()
        {
            aquireDelegate = new AquireDelegate(callbackHandler.Aquire);
            stopDelegate = new StopDelegate(callbackHandler.Stop);
            clearCountDelegate = new ClearImageCount(callbackHandler.ClearImageCount);
            incrementCountDelegate = new IncrementImageCount(callbackHandler.IncrementImageCount);
            setSaveDirDelegate = new SetSaveDirDelegate(setSaveDir);
            printDelegate = new PrintDelegate(printFromDevice);
            externalTriggerOnDelegate = new ExternalTriggerOn(userInterface.externalTriggerOn);
        }
        ///////////////////////////////////////////////////////////////////////

        void installDelegates()
        {
            wrapper.installDelegate(aquireDelegate);
            wrapper.installDelegate(stopDelegate);
            wrapper.installDelegate(clearCountDelegate);
            wrapper.installDelegate(incrementCountDelegate);
            wrapper.installDelegate(setSaveDirDelegate);
            wrapper.installDelegate(printDelegate);
            wrapper.installDelegate(externalTriggerOnDelegate);

        //    userInterface.printMessage("installDelegates" + "\n");
        }
        ///////////////////////////////////////////////////////////////////////

        //Called by device when printing messages (such as when debugging)
        void printFromDevice(string message)
        {
            userInterface.printMessage(message);
        }

        //Called by device on each Play to determine where SPE and TIF files will be saved.
        void setSaveDir(string dir)
        {
            //Need to strip off extra control character from string or Lightfield will crash on save.
            //Possibly the conversion from std::string to String adds an extra endline char (?)
            //Length of raw dir string is +1 char before cleanup.
            saveDirectory = new string(dir.Where(c => !char.IsControl(c)).ToArray());


            //string tmp = "C:\\Users\\Jason\\Code\\dev\\stidatatest\\2017\\9\\30\\data";
            //userInterface.printMessage("Raw: "+ dir.Length.ToString() + "\r\n" + "Hard: "+ tmp.Length.ToString() + "\r\n"
            //    + "saveDirectory: " + saveDirectory.Length.ToString() + "\r\n");

        }
        ///////////////////////////////////////////////////////////////////////

        public void connect()
        {
            new Thread(delegate () {

                try {
                    switch(userInterface.cameraSelection)
                    {
                        case PixisDeviceUserControl.CameraSelection.North:
                            wrapper = new DeviceWrapper(0);
                            break;
                        case PixisDeviceUserControl.CameraSelection.East:
                            wrapper = new DeviceWrapper(1);
                            break;
                        default:
                            //error; this should never happen
                            userInterface.printMessage("Error: Invalid camera selection.\n");
                            return;
                    }
                //    userInterface.printMessage("new DeviceWrapper: " + wrapper.ToString() + "\n");
                }
                catch (Exception e)
                {
                    //catch all
                    userInterface.printMessage("Failed to make DeviceWrapper\n");
                    MessageBox.Show("Failed to make DeviceWrapper: " + e.Message);
                    return;
                }
            
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
        ///////////////////////////////////////////////////////////////////////

        public void disconnect()
        {
            new Thread(delegate () {
                if (wrapper != null)
                {
                    wrapper.shutdown(); //DEBUG  This is crashing!
                }
                if (deviceThread != null && deviceThread.IsAlive)
                {
                    deviceThread.Join();
                }
                //                MessageBox.Show("Device disconnected successfully.");
            }).Start();
        }
        ///////////////////////////////////////////////////////////////////////

        public void startDeviceWrapper()
        {
            //userInterface.printMessage("startDeviceWrapper()\n");
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
                MessageBox.Show("startDeviceWrapper Error: " + mess);
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
                {
                    camera = device;
                }
            }

            //LightFieldApplication.Experiment.

            if (camera == null)
            {
                MessageBox.Show("This sample requires a camera!");
                return false;
            }

            if (!LightFieldApplication.Experiment.IsReadyToRun)
            {
                MessageBox.Show("The system is not ready for acquisition, is there an error?");
                return false;
            }
            return true;
        }
        ///////////////////////////////////////////////////////////////////////

        public void ClearImageCount() {}
        public void IncrementImageCount() {}

        public void Aquire(int index)
        {
            startAcquisition(index);
        }

        public void Stop()
        {
            stopAcquisition();
        }
        ///////////////////////////////////////////////////////////////////////

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
                // Attach Date/Time to filename
                experiment.SetValue(ExperimentSettings.FileNameGenerationAttachDate, true);
                experiment.SetValue(ExperimentSettings.FileNameGenerationAttachTime, true);


                // Save Spe (and tif) to the specified directory
                if (saveDirectory != null)
                {
                    //userInterface.printMessage(saveDirectory + "\r\n");
                    //experiment.SetValue(ExperimentSettings.FileNameGenerationDirectory, "C:\\Users\\Jason\\Code\\dev\\stidatatest\\2017\\9\\30\\" + "data");

                    experiment.SetValue(ExperimentSettings.FileNameGenerationDirectory, saveDirectory);

                    //string dir = experiment.GetValue(ExperimentSettings.FileNameGenerationDirectory).ToString();
                    //userInterface.printMessage(dir + "\r\n");
                    //userInterface.printMessage("Compare: "+ String.Compare(dir, saveDirectory).ToString());
                }

                //Save file as Specific.Spe to the default directory
                //experiment.SetValue(ExperimentSettings.FileNameGenerationBaseFileName, "Specific2");

                // Connnect the event handler
                acquireCompletedEventHandler_ = new EventHandler<ExperimentCompletedEventArgs>(AcquisitionCompleteHandler);
                experiment.ExperimentCompleted += acquireCompletedEventHandler_;

                // Begin the acquisition 
                experiment.Acquire();
            }
        }
        ///////////////////////////////////////////////////////////////////////
        // Acquire Completed Handler
        // Returns the saved filenames (spe, tif) to the device and signal the device to stop.
        ///////////////////////////////////////////////////////////////////////
        public void AcquisitionCompleteHandler(object sender, ExperimentCompletedEventArgs e)
        {
            ((IExperiment)sender).ExperimentCompleted -= acquireCompletedEventHandler_;

            string spe_filename = getRecentFilename();

            wrapper.setSavedSPEFilename(currentImageIndex, spe_filename);

            Export_Tiff(spe_filename);

            string tif_filename = Path.ChangeExtension(spe_filename, ".tif");
            wrapper.setSavedImageFilename(currentImageIndex, tif_filename);

            wrapper.stopWaiting(currentImageIndex);

        }
        ///////////////////////////////////////////////////////////////////////

        public void stopAcquisition()
        {
            IExperiment experiment = LightFieldApplication.Experiment;
            if (experiment != null)
            {
                experiment.Stop();
            }
        }
        ///////////////////////////////////////////////////////////////////////

        public string getRecentFilename()
        {
            IList<String> filenames = LightFieldApplication.FileManager.GetRecentlyAcquiredFileNames();

            return filenames.First<string>();
        }
        ///////////////////////////////////////////////////////////////////////

        //public string getFilePath()
        //{
        //    object tmp = LightFieldApplication.Experiment.GetValue(ExperimentSettings.FileNameGenerationFileFormatLocation);
        //    if (tmp == null)
        //    {
        //        return "Null object";
        //    }
        //    else
        //    {
        //        return tmp.ToString();
        //    }
        //}
        ///////////////////////////////////////////////////////////////////////
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
            disconnect();     //DEBUG This is crashing!
        }
        ///////////////////////////////////////////////////////////////////////
        public override string UIExperimentSettingTitle { get { return "PIXIS STI Device"; } }
        ///////////////////////////////////////////////////////////////////////
        static void PixisExceptionHandler(object sender, UnhandledExceptionEventArgs args)
        {
            
            Exception e = (Exception)args.ExceptionObject;
            MessageBox.Show("PIXIS STI Device handler caught : " + e.Message +"\n" 
                + "PixisExceptionHandler sender : " + sender.ToString());
            MessageBox.Show("Runtime terminating: " + Convert.ToString(args.IsTerminating));
        }

    }
}
