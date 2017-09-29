using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using PrincetonInstruments.LightField.AddIns;


namespace STI
{
    /// <summary>
    /// Interaction logic for PixisDeviceUserControl.xaml
    /// </summary>
    public partial class PixisDeviceUserControl : UserControl, PixisDeviceCallbackListener
    {

        PixisAddIn controller_;
        ILightFieldApplication app_;
//        DeviceWrapper wrapper;
//        public Thread deviceThread;// = new Thread(startDeviceWrapper);

        public StatusTextManager statusText;

        public PixisDeviceUserControl(ILightFieldApplication application, PixisAddIn controller)
        {
            controller_ = controller;
            app_ = application;
//            wrapper = wrap;

            InitializeComponent();

            disconnectButton.IsEnabled = false;

            statusText = new StatusTextManager(aquireStatusLabel);
            statusText.setStatus(StatusTextManager.Status.Disconnected);
        }

        ~PixisDeviceUserControl()
        {
        //    deviceThread.Abort();
        }

        private void connect_button_Click(object sender, RoutedEventArgs e)
        {
            //            MessageBox.Show("Test button!");
            controller_.connect();

            Dispatcher.Invoke(
                System.Windows.Threading.DispatcherPriority.Background,
                new Action(delegate ()
                {
                    connectButton.IsEnabled = false;
                    disconnectButton.IsEnabled = true;
                    statusText.setStatus(StatusTextManager.Status.Idle);
                })
                );
        }

        private void disconnectButton_Click(object sender, RoutedEventArgs e)
        {
            controller_.disconnect();

            Dispatcher.Invoke(
                System.Windows.Threading.DispatcherPriority.Background,
                new Action(delegate ()
                {
                    connectButton.IsEnabled = true;
                    disconnectButton.IsEnabled = false;
                    statusText.setStatus(StatusTextManager.Status.Disconnected);
                })
                );
        }

        public void ClearImageCount()
        {
            Dispatcher.Invoke(
                System.Windows.Threading.DispatcherPriority.Background,
                new Action(delegate ()
                {
                    statusText.setCurrentImage(0);
                    statusText.setExpectedImages(0);
                    textBox.AppendText("ClearImageCount \r\n");
                })
                );
        }
        public void IncrementImageCount()
        {
            Dispatcher.Invoke(
                System.Windows.Threading.DispatcherPriority.Background,
                new Action(delegate ()
                {
                           //textBox.Text = "IncrementImageCount";
                           statusText.setExpectedImages(statusText.expectedImages + 1);
                    textBox.AppendText("IncrementImageCount \r\n");

                })
                       );
        }

        public void Aquire(int index)
        {
//            MessageBox.Show("Aquire! " + Convert.ToString(index));

            Dispatcher.Invoke(
           System.Windows.Threading.DispatcherPriority.Background,
           new Action(delegate ()
           {

               statusText.setStatus(StatusTextManager.Status.Aquiring);
               statusText.setCurrentImage(index);

               textBox.AppendText("Aquire " + Convert.ToString(index) + "\r\n");

                              
               //               statusText.setCurrentImage(22);
               //              textBox.Text = "Aquire: " + Convert.ToString(index);

           })
           );
        }
        public void Stop()
        {
            Dispatcher.Invoke(
           System.Windows.Threading.DispatcherPriority.Background,
           new Action(delegate ()
           {
               statusText.setStatus(StatusTextManager.Status.Idle);
           })
           );
        }


        private void testButton_Click(object sender, RoutedEventArgs e)
        {
            //controller_.acquireTest(0);
            printMessage(controller_.getRecentFilename() + "\r\n");
        }

        public void printMessage(string text)
        {
            Dispatcher.Invoke(

                System.Windows.Threading.DispatcherPriority.Background,
                new Action(delegate ()
                {
                    textBox.AppendText(text);
                })
                );
        }

    }

    public class StatusTextManager
    {
        public StatusTextManager(Label label)
        {
            statusLabel = label;
            currentImage = 0;
            expectedImages = 0;

            setStatus(Status.Idle);

        }

        public void refresh()
        {
            switch (status)
            {
                case Status.Idle:
                    statusLabel.Content = "Idle.  Number of images ready to aquire: "
                         + Convert.ToString(expectedImages);
//                        + " images ready to be aquired.";
                    break;
                case Status.Aquiring:
                    statusLabel.Content = "Aquiring image: " 
                        + Convert.ToString(currentImage + 1)  + "/" + Convert.ToString(expectedImages);
                    break;
                case Status.Disconnected:
                    statusLabel.Content = "Disconnected";
                    break;
            }
        }

        public void setExpectedImages(int number)
        {
            expectedImages = number;
            refresh();
        }

        public void setCurrentImage(int number)
        {
            currentImage = number;
            refresh();
        }

        public void setStatus(Status s)
        {
            status = s;
            refresh();
        }

        public enum Status { Aquiring, Idle, Disconnected };

        Status status;

        Label statusLabel;
        public int currentImage;
        public int expectedImages;
    }
}
