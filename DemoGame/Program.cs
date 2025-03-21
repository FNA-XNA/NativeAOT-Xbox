using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using SDL3;

namespace DemoGame
{
    class Program : Game
    {
        static void InitDebugLogging()
        {
#if DEBUG
            AppDomain.CurrentDomain.UnhandledException += OnUnhandledException;
            FNALoggerEXT.LogInfo = Log;
            FNALoggerEXT.LogWarn = Log;
            FNALoggerEXT.LogError = Log;

            static void OnUnhandledException(object sender, UnhandledExceptionEventArgs e)
            {
                Debug.WriteLine(e.ExceptionObject);
            }

            static void Log(string message)
            {
                Debug.WriteLine(message);
            }
#endif
        }

        // FIXME: This should be in SDL3-CS!
        [DllImport("SDL3", CallingConvention = CallingConvention.Cdecl)]
        static extern void SDL_GDKSuspendComplete();

        SDL.SDL_EventFilter sdlEventCallbackDelegate;
        bool suppressDrawing = false;

        private unsafe bool SDLEventCallback(IntPtr userdata, SDL.SDL_Event* sdlevent)
        {
            if (sdlevent->type == (uint) SDL.SDL_EventType.SDL_EVENT_DID_ENTER_BACKGROUND)
            {
                suppressDrawing = true;
                SDL_GDKSuspendComplete();
            }
            else if (sdlevent->type == (uint) SDL.SDL_EventType.SDL_EVENT_WILL_ENTER_FOREGROUND)
            {
                suppressDrawing = false;
            }

            return true;
        }

        static void Main(string[] args)
        {
            /* Set up custom logging because stdout and stderr are not redirected to the Visual Studio console.
             * Use FNALoggerEXT or Debug.WriteLine instead of Console.WriteLine in your application.
             * Note that this only applies to Debug mode, since Debug.WriteLine is compiled out for Release builds.
             */
            InitDebugLogging();

            SDL.SDL_RunApp(0, IntPtr.Zero, FakeMain, IntPtr.Zero);
        }

        static int FakeMain(int argc, IntPtr argv)
        {
            using (Program p = new Program())
                p.Run();

            return 0;
        }

        public Program() : base()
        {
            new GraphicsDeviceManager(this);
        }

        protected override void Initialize()
        {
            base.Initialize();

            /* After base.Initialize is called the GPU will be set up.
             * We should hook this event _afterward_ so that the suspend timing
             * is correct!
             * -flibit
             */
            unsafe
            {
                sdlEventCallbackDelegate = SDLEventCallback;
                SDL.SDL_AddEventWatch(sdlEventCallbackDelegate, IntPtr.Zero);
            }
        }

        protected override void Update(GameTime gameTime)
        {
            base.Update(gameTime);

            if (suppressDrawing)
            {
                SuppressDraw();
            }
        }

        protected override void Draw(GameTime gameTime)
        {
            GraphicsDevice.Clear(Color.CornflowerBlue);
        }
    }
}
