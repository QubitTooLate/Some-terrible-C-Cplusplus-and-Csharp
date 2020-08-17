
    public class DeviceContext : IDisposable
    {
        protected bool _Disposed;

        public DeviceContextHandle DeviceContextHandle { get => _DC; }
        protected DeviceContextHandle _DC;

        public WindowHandle WindowHandle { get; }

        protected DeviceContext() { }

        public DeviceContext(WindowHandle windowHandle)
        {
            _DC = Winuser.GetDeviceContext(windowHandle);
            WindowHandle = windowHandle;
        }

        public uint DrawOnDeviceContext(DeviceContext deviceContext, int x, int y, uint width, uint height, int offsetX, int offsetY, uint rasterOperationCode) => WinGDI.DrawDeviceContextOnDeviceContext(
                deviceContext._DC,
                x,
                y,
                width,
                height,
                _DC,
                offsetX,
                offsetY,
                rasterOperationCode
            );

        ~DeviceContext() => Dispose();

        public virtual void Dispose()
        {
            if (!_Disposed)
            {
                _Disposed = true;

                GC.SuppressFinalize(this);

                if (_DC != Handle.Zero)
                    _ = Winuser.ReleaseDeviceContext(WindowHandle, _DC);
            }
        }
    }

    public class CompatibleDeviceContext : DeviceContext
    {
        public BitmapHandle BitmapHandle { get; }
        public uint Width { get; }
        public uint Height { get; }

        public CompatibleDeviceContext(DeviceContext deviceContext, uint width, uint height)
        {
            _DC = WinGDI.CreateCompatibleDeviceContext(deviceContext.DeviceContextHandle);
            BitmapHandle = WinGDI.CreateCompatibleBitmap(deviceContext.DeviceContextHandle, width, height);
            _ = WinGDI.DeviceContextSetBitmap(_DC, BitmapHandle);
            Width = width;
            Height = height;
        }

        public uint CopyBitsToBuffer(uint[] buffer)
        {
            if (buffer.Length < Width * Height)
                return 87;

            WinGDI.BitmapInfo bitmapInfo = new WinGDI.BitmapInfo(Width, Height);

            return WinGDI.CopyDeviceContextBitsToBuffer(
                _DC,
                BitmapHandle,
                0,
                Height,
                ref buffer[0],
                in bitmapInfo,
                0
            );
        }

        public uint CopyBitsFromBuffer(uint[] buffer)
        {
            if (buffer.Length < Width * Height)
                return 87;

            WinGDI.BitmapInfo bitmapInfo = new WinGDI.BitmapInfo(Width, Height);

            return WinGDI.CopyBufferBitsToDeviceContext(
                _DC,
                BitmapHandle,
                0,
                Height,
                ref buffer[0],
                in bitmapInfo,
                0
            );
        }

        ~CompatibleDeviceContext() => Dispose();

        public override void Dispose()
        {
            if (!_Disposed)
            {
                _Disposed = true;

                GC.SuppressFinalize(this);

                if (_DC != Handle.Zero)
                    _ = WinGDI.DeleteDeviceContext(_DC);

                if (BitmapHandle != Handle.Zero)
                    _ = WinGDI.DeleteBitmap(BitmapHandle);
            }
        }
    }
