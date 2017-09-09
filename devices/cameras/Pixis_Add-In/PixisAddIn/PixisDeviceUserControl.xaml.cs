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
using System.Threading;

namespace STI
{
    /// <summary>
    /// Interaction logic for PixisDeviceUserControl.xaml
    /// </summary>
    public partial class PixisDeviceUserControl : UserControl
    {

        ILightFieldApplication app_;
        DeviceWrapper wrapper;
        Thread deviceThread;// = new Thread(startDeviceWrapper);

        public PixisDeviceUserControl(ILightFieldApplication application, DeviceWrapper wrap)
        {
            app_ = application;
            wrapper = wrap;
            deviceThread = new Thread(startDeviceWrapper);

            InitializeComponent();
        }

        private void button_Click(object sender, RoutedEventArgs e)
        {
//            MessageBox.Show("Test button!");

            if(!deviceThread.IsAlive)
            {
                deviceThread.Start();
            }
            else
            {
                MessageBox.Show("Cannot connect; device thread IsAlive already.");
            }
        }

        private void button1_Click(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("Test button 2!");
        }

        public void startDeviceWrapper()
        {
            try
            {
                wrapper.start();
            }
            catch (Exception ex)
            {
                string mess = ex.Message;
                MessageBox.Show("Error: " + mess);
            }
        }
    }
}
