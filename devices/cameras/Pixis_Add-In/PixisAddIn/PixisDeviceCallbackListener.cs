using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace STI
{
    interface PixisDeviceCallbackListener
    {
        void ClearImageCount();
        void IncrementImageCount();
//        bool IsReadyToAquire();
        void Aquire(int index);
        void Stop();
    }
}
