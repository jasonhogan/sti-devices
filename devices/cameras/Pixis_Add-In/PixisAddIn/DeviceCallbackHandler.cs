using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using System.Windows;
//using System.Windows;
using System.Windows.Controls;
//using System.Threading;




namespace STI
{
    //make handler inherit the listener interface just to ensure all methods are implemented
    class DeviceCallbackHandler : PixisDeviceCallbackListener   
    {
        public void addListener(PixisDeviceCallbackListener listener)
        {
            listeners.Add(listener);
        }
        List<PixisDeviceCallbackListener> listeners = new List<PixisDeviceCallbackListener>();

        public void ClearImageCount()
        {
            foreach(PixisDeviceCallbackListener listener in listeners)
            {
                listener.ClearImageCount();
            }
        }
        public void IncrementImageCount()
        {
            foreach (PixisDeviceCallbackListener listener in listeners)
            {
                listener.IncrementImageCount();
            }
        }
        public void Aquire(int index)
        {
            foreach (PixisDeviceCallbackListener listener in listeners)
            {
                listener.Aquire(index);
            }
        }
        public void Stop()
        {
            foreach (PixisDeviceCallbackListener listener in listeners)
            {
                listener.Stop();
            }
        }
    }
}
