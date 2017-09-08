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
    public partial class PixisDeviceUserControl : UserControl
    {

        ILightFieldApplication app_;
        DeviceWrapper wrapper;

        public PixisDeviceUserControl(ILightFieldApplication application, DeviceWrapper wrap)
        {
            app_ = application;
            wrapper = wrap;
            InitializeComponent();
        }

        private void button_Click(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("Test button!");
            wrapper.start();
        }

        private void button1_Click(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("Test button 2!");
        }
    }
}
